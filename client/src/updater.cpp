/*
 * ---- Call of Suli ----
 *
 * updater.cpp
 *
 * Created on: 2023. 07. 12.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Updater
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

#include "updater.h"
#include "Logger.h"
#include "qcryptographichash.h"
#include "qnetworkreply.h"
#include "qnetworkrequest.h"
#include "qsettings.h"
#include "qurl.h"
#include "utils.h"
#include "websocket.h"
#include <QFile>
#include "client.h"

#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_WIN) || defined (Q_OS_MACOS)
#include <QProcess>
#endif

/// DEFINES

#define _APPIMAGE_UPDATE_TOOL QCoreApplication::applicationDirPath()+QStringLiteral("/appimageupdatetool-x86_64.AppImage")
#define _UPDATER_EXECUTABLE QCoreApplication::applicationDirPath()+QStringLiteral("/_auto_updater")
#define _UPDATER_EXECUTABLE_WIN QCoreApplication::applicationDirPath()+QStringLiteral("/_auto_updater.exe")
#define _URL_GITHUB QStringLiteral("https://valaczka.github.io/callofsuli/version.json")
#define _URL_MARKET QStringLiteral("market://details?id=hu.piarista.vjp.callofsuli")
#define _URL_APPSTORE QStringLiteral("itms-apps://itunes.apple.com/app/id1639323717")

/**
 * @brief Updater::Updater
 * @param client
 */

Updater::Updater(Client *client)
	: QObject{client}
	, m_client(client)
	#ifdef Q_OS_WIN
	, m_updaterExecutable(_UPDATER_EXECUTABLE_WIN)
	#else
	, m_updaterExecutable(_UPDATER_EXECUTABLE)
	#endif
{
	Q_ASSERT(m_client);

	connect(this, &Updater::appImageUpdateCheckFailed, this, &Updater::githubUpdateCheckNoForce);

	removeUpdaterExecutable();

}




/**
 * @brief Updater::checkAvailableUpdates
 * @param force
 */

void Updater::checkAvailableUpdates(const bool &force)
{
	if (!force) {
		QSettings s;
		if (!s.contains(QStringLiteral("update/enabled"))) {
			LOG_CDEBUG("updater") << "First run, skip automatic update";
			setAutoUpdate(true);
			return;
		}
	}

	const bool updateEnabled = autoUpdate();

	if (!updateEnabled && !force) {
		LOG_CINFO("updater") << "Automatic update disabled";
		return;
	}

	if (appImageUpdateCheck(force))
		return;

	githubUpdateCheck(force);
}



/**
 * @brief Updater::updateAppImage
 */

void Updater::updateAppImage()
{
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
	const QByteArray &appImage = qgetenv("APPIMAGE");

	if (appImage.isEmpty()) {
		LOG_CDEBUG("updater") << "AppImage not present";
		emit appImageUpdateFailed(tr("Nem található AppImage"));
		return;
	}

	if (!QFile::exists(_APPIMAGE_UPDATE_TOOL)) {
		LOG_CWARNING("updater") << "AppImageUpdateTool not found";
		emit appImageUpdateFailed(tr("Az AppImageToolUpdate nem található"));
		return;
	}


	QProcess *process = new QProcess(this);

	process->setProgram(_APPIMAGE_UPDATE_TOOL);
	process->setArguments({QStringLiteral("-O"), QString::fromUtf8(appImage)});

	connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError error){
		LOG_CERROR("updater") << "AppImateUpdateTool failed:" << error;

		emit appImageUpdateFailed(tr("AppImageUpdateTool futtatása sikertelen"));

		process->deleteLater();
	});

	connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
			[this, process](int exitCode, QProcess::ExitStatus exitStatus){

		LOG_CWARNING("updater") << "AppImateUpdateTool update finished with code:" << exitCode << exitStatus;

		if (exitStatus == QProcess::NormalExit && exitCode == 0)
			emit appImageUpdateSuccess();
		else
			emit appImageUpdateFailed(tr("A frissítés sikertelen"));

		process->deleteLater();
	});

	connect(process, &QProcess::readyReadStandardOutput, this, [process](){
		LOG_CTRACE("updater") << process->readAllStandardOutput();
	});

	connect(process, &QProcess::readyReadStandardError, this, [process](){
		LOG_CTRACE("updater") << process->readAllStandardError();
	});

	m_client->snack(tr("Szoftverfrissítés..."));

	process->start();

	LOG_CDEBUG("updater") << "AppImageUpdateTool update started";

#endif
}



/**
 * @brief Updater::updateGitHub
 * @param installer
 */

void Updater::updateGitHub(const QString &installer, const QString &sha1)
{
	LOG_CDEBUG("updater") << "GitHub download update:" << qPrintable(installer);

	WebSocket *webSocket = m_client->webSocket();

	if (!webSocket) {
		LOG_CERROR("updater") << "WebSocket missing";
		return;
	}

	removeUpdaterExecutable();

	m_fileUpdater.setFileName(m_updaterExecutable);

	if (!m_fileUpdater.open(QIODevice::WriteOnly)) {
		LOG_CERROR("updater") << "Update installer write error:" << qPrintable(m_updaterExecutable);
		emit updateDownloadFailed();
		return;
	}

	QNetworkRequest r{QUrl(installer)};
	r.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

	m_client->snack(tr("Telepítő letöltése..."));

	QNetworkReply *reply = webSocket->networkManager()->get(r);

	connect(reply, &QNetworkReply::errorOccurred, this, [this](const QNetworkReply::NetworkError &err){
		LOG_CERROR("updater") << "Update download error:" << err;
		emit updateDownloadFailed();
	});

	connect(reply, &QNetworkReply::readyRead, this, [this, reply] {
		const QByteArray &data = reply->readAll();
		m_fileUpdater.write(data);
	});

	connect(reply, &QNetworkReply::finished, this, [reply, sha1, this]{
		LOG_CDEBUG("updater") << "Update install download finished";

		m_client->snack(tr("Telepítő letöltődött"));

		const QByteArray &data = reply->readAll();
		m_fileUpdater.write(data);
		m_fileUpdater.close();

		reply->deleteLater();

		if (!sha1.isEmpty()) {
			QCryptographicHash hash(QCryptographicHash::Sha1);

			QByteArray result;

			if (m_fileUpdater.open(QFile::ReadOnly))
				if (hash.addData(&m_fileUpdater)) {
					result = hash.result();
				}
			m_fileUpdater.close();

			LOG_CDEBUG("updater") << "Update installer downloaded, size:" << data.size() << "SHA1:" << result.toHex();

			if (sha1.toLatin1() != result.toHex()) {
				LOG_CERROR("updater") << "Update installer SHA1 checksum mismatch";
				emit updateDownloadFailed();
				return;
			}
		}


#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
		m_fileUpdater.setPermissions(QFile::ReadOwner | QFile::ExeOwner | QFile::WriteOwner);
#endif

		LOG_CINFO("updater") << "Update installer ready:" << m_updaterExecutable;

		emit updateReady();

	});



}



/**
 * @brief Updater::updateNative
 */

void Updater::updateNative()
{
#if defined(Q_OS_ANDROID)
	LOG_CDEBUG("updater") << "Update native" << qPrintable(_URL_MARKET);
	Utils::openUrl(_URL_MARKET);
#elif defined (Q_OS_IOS)
	LOG_CDEBUG("updater") << "Update native" << qPrintable(_URL_APPSTORE);
	Utils::openUrl(_URL_APPSTORE);
#endif
}



/**
 * @brief Updater::updateExecute
 */

void Updater::updateExecute()
{
#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_WIN) || defined (Q_OS_MACOS)
	if (!QFile::exists(m_updaterExecutable)) {
		LOG_CWARNING("updater") << "Update installer not found";
		emit updateExecuteFailed();
	}

	QProcess *p = new QProcess(this);

	p->setProgram(m_updaterExecutable);

	if (p->startDetached()) {
		emit updateExecuteStarted();
	} else {
		emit updateExecuteFailed();
		p->deleteLater();
	}
#else
	emit updateExecuteFailed();
#endif
}




/**
 * @brief Updater::removeUpdaterExecutable
 */

void Updater::removeUpdaterExecutable()
{
	if (QFile::exists(m_updaterExecutable)) {
		LOG_CDEBUG("updater") << "Remove update installer:" << qPrintable(m_updaterExecutable);
		if (!QFile::remove(m_updaterExecutable))
			LOG_CERROR("updater") << "Remove update installer failed:" << qPrintable(m_updaterExecutable);
	}
}




/**
 * @brief Updater::githubUpdateCheck
 */

void Updater::githubUpdateCheck(const bool &force)
{
	LOG_CDEBUG("updater") << "GitHub update check";

	WebSocket *webSocket = m_client->webSocket();

	if (!webSocket) {
		LOG_CERROR("updater") << "WebSocket missing";
		return;
	}

	QString platform;

#if defined(Q_OS_WIN)
	platform = QStringLiteral("windows");
#elif defined(Q_OS_IOS)
	platform = QStringLiteral("ios");
#elif defined(Q_OS_ANDROID)
	platform = QStringLiteral("android");
#elif defined(Q_OS_MACOS)
	platform = QStringLiteral("mac");
#elif defined(Q_OS_LINUX)
	platform = QStringLiteral("linux");
#endif

	if (platform.isEmpty()) {
		LOG_CERROR("updater") << "Unknow platform";
		return;
	}

	QNetworkRequest r(QUrl(_URL_GITHUB));

	QNetworkReply *reply = webSocket->networkManager()->get(r);

	WebSocketReply *wr = new WebSocketReply(reply, webSocket);
	connect(wr, &WebSocketReply::finished, webSocket, &WebSocket::checkPending);

	LOG_CDEBUG("updater") << "Download update installer started";


	wr->done(this, [this, platform, force](const QJsonObject &data){

		/*	QJsonObject platformData = {
			{ "version", "3.3.998" },
			{ "installer", "https://github.com/valaczka/callofsuli/releases/download/3.2.32/Call_of_Suli_3.2.32_install.exe" },
			{ "sha1", "671fd69d09b4767e923c063e1e6eb52cb1260e79" }
		};*/

		const QJsonObject &platformData = data.value(platform).toObject();
		const QString &vstr = platformData.value(QStringLiteral("version")).toString();
		uint vMajor = vstr.section('.', 0, 0).toUInt();
		uint vMinor = vstr.section('.', 1, 1).toUInt();
		uint vBuild = vstr.section('.', 2, 2).toUInt();

		const QString &lastNotified = Utils::settingsGet(QStringLiteral("update/lastVersion")).toString();

		if (!force && !lastNotified.isEmpty() && vstr == lastNotified)
			return;

		if (vMajor > Utils::versionMajor() ||
				(vMajor == Utils::versionMajor() && vMinor > Utils::versionMinor()) ||
				(vMajor == Utils::versionMajor() && vMinor == Utils::versionMinor() && vBuild > Utils::versionBuild())
				) {
			Utils::settingsSet(QStringLiteral("update/lastVersion"), vstr);
			emit gitHubUpdateAvailable(platformData);
		} else if (force) {
			emit updateNotAvailable();
		}

	})
			->error(this, [this](const QNetworkReply::NetworkError &){
		emit gitHubUpdateCheckFailed();
	});

}






/**
 * @brief Updater::appImageUpdateCheck
 */

bool Updater::appImageUpdateCheck(const bool &force)
{
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
	const QByteArray &appImage = qgetenv("APPIMAGE");

	LOG_CDEBUG("updater") << "AppImage update check:" << appImage;

	if (appImage.isEmpty()) {
		LOG_CDEBUG("updater") << "AppImage not present";
		return false;
	}


	if (!QFile::exists(_APPIMAGE_UPDATE_TOOL)) {
		LOG_CWARNING("updater") << "AppImageUpdateTool not found";
		return false;
	}

	QProcess *process = new QProcess(this);

	process->setProgram(_APPIMAGE_UPDATE_TOOL);
	process->setArguments({QStringLiteral("-j"), QString::fromUtf8(appImage)});

	connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
			[this, process, force](int exitCode, QProcess::ExitStatus exitStatus){
		LOG_CDEBUG("updater") << "AppImateUpdateTool check finished with code:" << exitCode << exitStatus;

		if (exitStatus != QProcess::NormalExit || exitCode > 1) {
			emit appImageUpdateCheckFailed();
		}

		if (exitCode == 1)
			emit appImageUpdateAvailable();
		else if (force)
			emit updateNotAvailable();

		process->deleteLater();
	});

	process->start();

	LOG_CDEBUG("updater") << "AppImageUpdateTool check started";

	return true;
#else
	Q_UNUSED(force);
	return false;
#endif
}



/**
 * @brief Updater::autoUpdate
 * @return
 */

bool Updater::autoUpdate() const
{
	return Utils::settingsGet(QStringLiteral("update/enabled"), true).toBool();
}

void Updater::setAutoUpdate(bool newAutoUpdate)
{
	Utils::settingsSet(QStringLiteral("update/enabled"), newAutoUpdate);
	emit autoUpdateChanged();
}


