/*
 * ---- Call of Suli ----
 *
 * updater.h
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

#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QFile>

class Client;

class Updater : public QObject
{
	Q_OBJECT

	Q_PROPERTY(bool autoUpdate READ autoUpdate WRITE setAutoUpdate NOTIFY autoUpdateChanged)

public:
	explicit Updater(Client *client = nullptr);

	Q_INVOKABLE void checkAvailableUpdates(const bool &force = false);

	Q_INVOKABLE void updateAppImage();
	Q_INVOKABLE void updateGitHub(const QString &installer, const QString &sha1);

	Q_INVOKABLE void updateNative();
	Q_INVOKABLE void updateExecute();

	void removeUpdaterExecutable();

	bool autoUpdate() const;
	void setAutoUpdate(bool newAutoUpdate);

signals:
	void updateNotAvailable();

	void updateDownloadFailed();
	void updateReady();

	void updateExecuteFailed();
	void updateExecuteStarted();

	void appImageUpdateCheckFailed();
	void appImageUpdateFailed(QString errorString);
	void appImageUpdateSuccess();
	void appImageUpdateAvailable();

	void gitHubUpdateCheckFailed();
	void gitHubUpdateAvailable(const QJsonObject &data);

	void autoUpdateChanged();

private:
	void githubUpdateCheck(const bool &force);
	void githubUpdateCheckNoForce() { githubUpdateCheck(false); }
	bool appImageUpdateCheck(const bool &force);

	Client *m_client = nullptr;

	const QString m_updaterExecutable;
	QFile m_fileUpdater;
};

#endif // UPDATER_H
