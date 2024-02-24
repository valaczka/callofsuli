/*
 * ---- Call of Suli ----
 *
 * standaloneclient.h
 *
 * Created on: 2022. 12. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * StandaloneClient
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

#ifndef STANDALONECLIENT_H
#define STANDALONECLIENT_H

#include "client.h"
#include "server.h"
#include "utils_.h"


/**
 * @brief The StandaloneClient class
 */

class StandaloneClient : public Client
{
	Q_OBJECT

	Q_PROPERTY(ServerList *serverList READ serverList CONSTANT)
	Q_PROPERTY(int serverListSelectedCount READ serverListSelectedCount NOTIFY serverListSelectedCountChanged)
	Q_PROPERTY(bool vibrate READ vibrate WRITE setVibrate NOTIFY vibrateChanged)


public:
	explicit StandaloneClient(Application *app);
	virtual ~StandaloneClient();

	ServerList *serverList() const;

	Q_INVOKABLE void serverSetAutoConnect(Server *server) const;
	Q_INVOKABLE Server *serverAdd();
	Q_INVOKABLE bool serverDelete(Server *server);
	Q_INVOKABLE void serverDeleteTemporary();
	Q_INVOKABLE bool serverDeleteSelected();
	Q_INVOKABLE Server *serverAddWithUrl(const QUrl &url) override;

	int serverListSelectedCount() const;

	bool vibrate() const;
	void setVibrate(bool newVibrate);

public slots:
	void performVibrate() const;

protected slots:
	void onStartPageLoaded();
	void onOAuthFinished() override;
	void onOAuthStarted(const QUrl &url) override;
	void onServerConnected() override;
	void onUserLoggedIn() override;

private slots:
	void onMainWindowChanged();
	void onOrientationChanged(Qt::ScreenOrientation orientation);

private:
	void serverListLoad(const QDir &dir = Utils::standardPath(QStringLiteral("servers")));
	void serverListSave(const QDir &dir = Utils::standardPath(QStringLiteral("servers")));

signals:
	void serverListSelectedCountChanged();
	void vibrateChanged();

private:
	std::unique_ptr<ServerList> m_serverList = nullptr;
	bool m_vibrate = true;

};




#endif

