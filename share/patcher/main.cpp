/*
 * ---- Call of Suli ----
 *
 * %{Cpp:License:FileName}
 *
 * Created on: 2026. 01. 24.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QCoreApplication>
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <QCommandLineParser>
#include <QCborMap>
#include <QCborArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QProcess>

#include <iostream>
#include <sodium.h>

#include "../../lib/callofsuli-core/desktoputils.h"
#include "../../version/version.h"



static void die(const QString &msg)
{
	qCritical() << "ERROR:" << qPrintable(msg);
	std::exit(2);
}






/**
 * @brief readSignDb
 * @param baseName
 * @return
 */

static QJsonObject readSignDb(const QString &baseName)
{
	const QString file = baseName + '-' + QStringLiteral("proof.json");

	QFile f(file);

	if (!f.open(QIODevice::ReadOnly))
		die(QStringLiteral("Proof file read error: ").append(file));

	QByteArray data = f.readAll();

	f.close();

	return QJsonDocument::fromJson(data).object();
}


/**
 * @brief writeSignDb
 * @param baseName
 * @param db
 * @return
 */

static bool writeSignDb(const QString &baseName, const QJsonObject &db)
{
	const QString file = baseName + '-' + QStringLiteral("proof.json");

	QFile f(file);

	if (!f.open(QIODevice::WriteOnly))
		die(QStringLiteral("Proof file write error: ").append(file));

	f.write(QJsonDocument(db).toJson(QJsonDocument::Indented));
	f.close();

	qDebug() << "Proof file written:" << file;

	return true;
}




/**
 * @brief hash
 * @param path
 */

static int hash(const QString &manifest, const QString &platform, const QString &path)
{
	QString err;

	const auto ptr = DesktopUtils::getExeHash(path, &err);

	if (!ptr)
		die(QStringLiteral("file open failed %1: %2").arg(path).arg(err));


	qDebug() << qPrintable(path) << ":" << qPrintable(ptr->toHex());


	QJsonObject db = readSignDb(manifest);

	QJsonArray list = db.value(platform).toArray();

	list.append(QString::fromLatin1(ptr->toBase64()));

	db.insert(platform, list);

	writeSignDb(manifest, db);

	return 0;
}






/**
 * @brief fromhex
 * @param manifest
 * @param platform
 * @param path
 * @return
 */

static int fromHex(const QString &manifest, const QString &platform, const QString &input)
{
	QByteArray data = QByteArray::fromHex(input.toLatin1());

	qDebug() << "Read hex data:" << data.toHex(':');

	QJsonObject db = readSignDb(manifest);

	QJsonArray list = db.value(platform).toArray();

	list.append(QString::fromLatin1(data.toBase64()));

	db.insert(platform, list);

	writeSignDb(manifest, db);

	return 0;
}


/**
 * @brief manifest
 * @param baseName
 * @return
 */

static int sign(const QString &baseName)
{
	static const QStringList platforms = {
		"linux",
		"win",
		"android",
		"ios",
		"wasm"
	};

	QByteArray build;

	QProcess git;
	git.start("git", QStringList{"rev-parse", "--short=12", "HEAD"});

	if (!git.waitForStarted()) {
		build = QByteArray("nogit");
	} else {
		if (!git.waitForFinished())
			die("git exec error");

		build = git.readAll().simplified();
	}

	if (build.isEmpty())
		build = QByteArray("nogit");

	QByteArray rnd(4, Qt::Uninitialized);

	randombytes_buf(rnd.data(), rnd.size());

	build.append('-');
	build.append(rnd.toHex());


	unsigned char pk[crypto_sign_PUBLICKEYBYTES];
	unsigned char sk[crypto_sign_SECRETKEYBYTES];

	crypto_sign_keypair(pk, sk);




	QJsonObject sigDb;

	const QByteArray publicKey(reinterpret_cast<const char*>(pk), crypto_sign_PUBLICKEYBYTES);

	sigDb.insert(QStringLiteral("public_key"), QString::fromLatin1(publicKey.toBase64()));
	sigDb.insert(QStringLiteral("build_id"), QString::fromLatin1(build));

	for (const QString &p : platforms) {
		const QString file = baseName + '-' + p + QStringLiteral(".dat");

		QFile f(file);

		if (!f.open(QIODevice::WriteOnly))
			die(QStringLiteral("File write error: ").append(file));

		QCborArray m;
		m.append((quint32) 1);
		m.append(QByteArray::fromHex(build));
		m.append((quint32) VERSION_MAJOR);
		m.append((quint32) VERSION_MINOR);
		m.append((quint32) VERSION_BUILD);
		m.append(p.toLatin1());
		m.append((qint64) QDateTime::currentSecsSinceEpoch());

		const QByteArray manifestCbor = QCborValue(m).toCbor();

		if (manifestCbor.isEmpty())
			die("Manifest CBOR encoding failed (empty).");


		QByteArray sig(crypto_sign_BYTES, Qt::Uninitialized);

		crypto_sign_detached(reinterpret_cast<unsigned char*>(sig.data()),
							 nullptr,
							 reinterpret_cast<const unsigned char*>(manifestCbor.constData()),
							 (unsigned long long) manifestCbor.size(),
							 sk
							 );

		QCborArray obj;
		obj.append(manifestCbor);
		obj.append(sig);


		f.write(QCborValue(obj).toCbor());
		f.close();

		qDebug() << "Generated" << file;

		sigDb.insert(p, QJsonArray());
	}

	writeSignDb(baseName, sigDb);

	return 0;
}



/**
 * @brief main
 * @param argc
 * @param argv
 * @return
 */

int main(int argc, char** argv) {
	QCoreApplication app(argc, argv);

	QCoreApplication::setApplicationName(QStringLiteral("callofsuli-patcher"));
	QCoreApplication::setOrganizationDomain(QStringLiteral("callofsuli-patcher"));
	QCoreApplication::setApplicationVersion(VERSION_FULL);

	if (sodium_init() < 0) {
		die("libsodium init failed.");
	}


	QCommandLineParser parser;
	parser.setApplicationDescription(QString::fromUtf8("Call of Suli patcher – Copyright © 2012-2026 Valaczka János Pál"));
	parser.addHelpOption();
	parser.addVersionOption();

	parser.addPositionalArgument(QStringLiteral("manifest"), QObject::tr("A mainfest azonosítója (basename)"));
	parser.addOption({{QStringLiteral("linux")}, QObject::tr("Linux bináris fájl hash"), QObject::tr("bin")});
	parser.addOption({{QStringLiteral("windows")}, QObject::tr("Windows bináris fájl hash"), QObject::tr("bin")});
	parser.addOption({{QStringLiteral("android")}, QObject::tr("Android fingerprint"), QObject::tr("hex")});
	parser.addOption({{QStringLiteral("ios")}, QObject::tr("iOS proof"), QObject::tr("hex")});
	parser.addOption({{QStringLiteral("wasm")}, QObject::tr("WASM proof"), QObject::tr("hex")});
	parser.addOption({{QStringLiteral("s"), QStringLiteral("sign")}, QObject::tr("Aláírások készítése")});

	parser.process(app);

	QStringList args = parser.positionalArguments();

	if (args.isEmpty()) {
		qWarning() << "Hiányzó manifest azonosító";
		std::cout << parser.helpText().toUtf8().constData();
		return 1;
	}

	if (parser.isSet(QStringLiteral("linux")))
		return hash(args.first(), QStringLiteral("linux"), parser.value(QStringLiteral("linux")));

	if (parser.isSet(QStringLiteral("windows")))
		return hash(args.first(), QStringLiteral("win"), parser.value(QStringLiteral("windows")));

	if (parser.isSet(QStringLiteral("android")))
		return fromHex(args.first(), QStringLiteral("android"), parser.value(QStringLiteral("android")));

	if (parser.isSet(QStringLiteral("ios")))
		return fromHex(args.first(), QStringLiteral("ios"), parser.value(QStringLiteral("ios")));

	if (parser.isSet(QStringLiteral("wasm")))
		return fromHex(args.first(), QStringLiteral("wasm"), parser.value(QStringLiteral("wasm")));

	if (parser.isSet(QStringLiteral("sign")))
		return sign(args.first());


	return 0;
}
