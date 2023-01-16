/*
 * ---- Call of Suli ----
 *
 * utils.cpp
 *
 * Created on: 2022. 12. 11.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Utils
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

#include "utils.h"
#include "qdesktopservices.h"
#include "qjsondocument.h"
#include "qmath.h"
#include "qfile.h"
#include "selectableobject.h"
#include <random>



#ifdef Q_OS_ANDROID
#define FLAG_SCREEN_ORIENTATION_LANDSCAPE       0x00000000
#endif

Q_LOGGING_CATEGORY(lcUtils, "app.utils")

/**
 * @brief Utils::Utils
 * @param client
 */

Utils::Utils(QObject *parent)
	: QObject{parent}
{

}


/**
 * @brief Utils::~Utils
 */

Utils::~Utils()
{

}


/**
 * @brief Utils::fileContent
 * @param filename
 * @param error
 * @return
 */

QByteArray Utils::fileContent(const QString &filename, bool *error)
{
	QFile f(filename);

	if (!f.exists()) {
		qCWarning(lcUtils).noquote() << tr("A fájl nem olvasható:") << filename;

		if (error)
			*error = true;

		return QByteArray();
	}

	if (!f.open(QIODevice::ReadOnly)) {
		qCWarning(lcUtils).noquote() << tr("Nem lehet megnyitni a fájlt:") << filename;

		if (error)
			*error = true;

		return QByteArray();
	}

	QByteArray data = f.readAll();

	f.close();

	return data;
}


/**
 * @brief Utils::jsonDocumentToFile
 * @param doc
 * @param filename
 * @return
 */

bool Utils::jsonDocumentToFile(const QJsonDocument &doc, const QString &filename, const QJsonDocument::JsonFormat &format)
{
	const QByteArray &b = doc.toJson(format);

	QFile f(filename);

	if (!f.open(QIODevice::WriteOnly)) {
		qCWarning(lcUtils).noquote() << tr("Nem lehet írni a fájlt:") << filename;

		return false;
	}

	f.write(b);

	f.close();

	return true;
}


/**
 * @brief Utils::jsonObjectToFile
 * @param doc
 * @param filename
 * @param format
 * @return
 */

bool Utils::jsonObjectToFile(const QJsonObject &object, const QString &filename, const QJsonDocument::JsonFormat &format)
{
	return jsonDocumentToFile(QJsonDocument(object), filename, format);
}


/**
 * @brief Utils::jsonArrayToFile
 * @param array
 * @param filename
 * @param format
 * @return
 */

bool Utils::jsonArrayToFile(const QJsonArray &array, const QString &filename, const QJsonDocument::JsonFormat &format)
{
	return jsonDocumentToFile(QJsonDocument(array), filename, format);
}




/**
 * @brief Utils::byteArrayToJsonDocument
 * @param data
 * @return
 */

QJsonDocument Utils::byteArrayToJsonDocument(const QByteArray &data)
{
	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(data, &error);

	if (error.error != QJsonParseError::NoError) {
		qCWarning(lcUtils).noquote() << tr("JSON feldolgozási hiba: %1 (%2)").arg(error.errorString()).arg(error.error);
	}

	return doc;
}


/**
 * @brief Utils::byteArrayToJsonObject
 * @param data
 * @return
 */

QJsonObject Utils::byteArrayToJsonObject(const QByteArray &data, bool *error)
{
	const QJsonDocument &doc = byteArrayToJsonDocument(data);

	if (error)
		*error = doc.isNull();

	return doc.object();
}


/**
 * @brief Utils::byteArrayToJsonArray
 * @param data
 * @return
 */

QJsonArray Utils::byteArrayToJsonArray(const QByteArray &data, bool *error)
{
	const QJsonDocument &doc = byteArrayToJsonDocument(data);

	if (error)
		*error = doc.isNull();

	return doc.array();
}


/**
 * @brief Utils::fileToJsonDocument
 * @param filename
 * @return
 */

QJsonDocument Utils::fileToJsonDocument(const QString &filename, bool *error)
{
	QFile f(filename);

	if (!f.exists()) {
		qCWarning(lcUtils).noquote() << tr("A fájl nem olvasható:") << filename;

		if (error)
			*error = true;

		return QJsonDocument();
	}

	if (!f.open(QIODevice::ReadOnly)) {
		qCWarning(lcUtils).noquote() << tr("Nem lehet megnyitni a fájlt:") << filename;

		if (error)
			*error = true;

		return QJsonDocument();
	}

	QByteArray data = f.readAll();

	f.close();

	return byteArrayToJsonDocument(data);
}


/**
 * @brief Utils::fileToJsonObject
 * @param filename
 * @param error
 * @return
 */

QJsonObject Utils::fileToJsonObject(const QString &filename, bool *error)
{
	bool myerror = false;

	const QJsonDocument &doc = fileToJsonDocument(filename, &myerror);

	if (myerror) {
		if (error)
			*error = true;

		return QJsonObject();
	}

	if (error)
		*error = doc.isNull();

	return doc.object();
}



/**
 * @brief Utils::fileToJsonArray
 * @param filename
 * @param error
 * @return
 */

QJsonArray Utils::fileToJsonArray(const QString &filename, bool *error)
{
	bool myerror = false;

	const QJsonDocument &doc = fileToJsonDocument(filename, &myerror);

	if (myerror) {
		if (error)
			*error = true;

		return QJsonArray();
	}

	if (error)
		*error = doc.isNull();

	return doc.array();
}


/**
 * @brief Utils::colorSetAlpha
 * @param color
 * @param alpha
 * @return
 */

QColor Utils::colorSetAlpha(QColor color, const qreal &alpha)
{
	color.setAlphaF(alpha);
	return color;
}



/**
 * @brief Utils::formatMsecs
 * @param msec
 * @param decimals
 * @return
 */

QString Utils::formatMSecs(const int &msec, const int &decimals, const bool &withMinute)
{
	int s = qFloor((qreal)msec / 1000.0);
	int ms = msec - 1000*s;

	QString r;

	if (withMinute) {
		int h = qFloor((qreal)msec / (60*60*1000.0));
		int m = qFloor((qreal)msec / (60*1000.0)) - h*60;
		s -= m*60;

		if (h > 0)
			r += QStringLiteral("%1:").arg(h, 2, 10, QLatin1Char('0'));

		r += QStringLiteral("%1:%2").arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0'));

		if (decimals > 0) {
			QString n = QStringLiteral("%1").arg(ms, 3, 10, QLatin1Char('0'));
			r += QStringLiteral(".")+n.left(decimals);
		}
	} else {
		r = QString::number(s);
		if (decimals > 0) {
			QString n = QStringLiteral("%1").arg(ms, 3, 10, QLatin1Char('0'));
			r += QStringLiteral(".")+n.left(decimals);
		}
	}

	return r;
}




/**
 * @brief Utils::openUrl
 * @param url
 */
void Utils::openUrl(const QUrl &url)
{
	qCDebug(lcUtils).noquote() << tr("Open url:") << url;
	QDesktopServices::openUrl(url);
}


/**
 * @brief Utils::standardPath
 * @param path
 * @return
 */

QString Utils::standardPath(const QString &path)
{
	if (!path.isEmpty())
		return QStandardPaths::standardLocations(QStandardPaths::DataLocation).at(0)+QStringLiteral("/")+path;
	else
		return QStandardPaths::standardLocations(QStandardPaths::DataLocation).at(0);
}




/**
 * @brief Utils::genericDataPath
 * @param path
 * @return
 */

QString Utils::genericDataPath(const QString &path)
{
#ifdef Q_OS_ANDROID
	QAndroidJniObject mediaDir = QAndroidJniObject::callStaticObjectMethod("android/os/Environment", "getExternalStorageDirectory", "()Ljava/io/File;");
	QAndroidJniObject mediaPath = mediaDir.callObjectMethod( "getAbsolutePath", "()Ljava/lang/String;" );
	if (!path.isEmpty())
		return QStringLiteral("file://")+mediaPath.toString()+QStringLiteral("/")+path;
	else
		return QStringLiteral("file://")+mediaPath.toString();
#endif

	if (!path.isEmpty())
		return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+QStringLiteral("/")+path;
	else
		return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
}





/**
 * @brief Utils::generateRandomString
 * @param length
 * @return
 */

QByteArray Utils::generateRandomString(quint8 length)
{
	const char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	return generateRandomString(length, characters);
}


/**
 * @brief Utils::generateRandomString
 * @param length
 * @param characters
 * @return
 */

QByteArray Utils::generateRandomString(quint8 length, const char *characters)
{
	static std::mt19937 randomEngine(QDateTime::currentDateTime().toMSecsSinceEpoch());
	std::uniform_int_distribution<int> distribution(0, sizeof(characters) - 2);
	QByteArray data;
	data.reserve(length);
	for (quint8 i = 0; i < length; ++i)
		data.append(characters[distribution(randomEngine)]);
	return data;
}


/**
 * @brief Utils::selectedCount
 * @param list
 * @return
 */

#ifdef CLIENT_UTILS

int Utils::selectedCount(qolm::QOlmBase *list)
{
	if (!list)
		return 0;

	int num = 0;

	foreach (QObject *o, list->children()) {
		SelectableObject *s = qobject_cast<SelectableObject*>(o);
		if (s && s->selected())
			++num;
	}

	return num;
}

#endif
