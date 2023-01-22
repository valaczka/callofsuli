/*
 * ---- Call of Suli ----
 *
 * desktopclient.h
 *
 * Created on: 2022. 12. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * DesktopClient
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

#ifndef DESKTOPCLIENT_H
#define DESKTOPCLIENT_H

#include "client.h"
#include "qsoundeffect.h"
#include "qthread.h"
#include "sound.h"
#include "oauth2replyhandler.h"
#include "googleoauth2authenticator.h"
#include <QOlm/QOlm.hpp>


class DesktopCodeFlow;
class DesktopInternalHandler;

/**
 * @brief The DesktopClient class
 */

class DesktopClient : public Client
{
	Q_OBJECT

	Q_PROPERTY(qreal sfxVolume READ sfxVolume WRITE setSfxVolume NOTIFY sfxVolumeChanged)
	Q_PROPERTY(ServerList *serverList READ serverList CONSTANT)
	Q_PROPERTY(int serverListSelectedCount READ serverListSelectedCount NOTIFY serverListSelectedCountChanged)

public:
	explicit DesktopClient(Application *app, QObject *parent = nullptr);
	virtual ~DesktopClient();

	qreal sfxVolume() const;
	void setSfxVolume(qreal newSfxVolume);

	QSoundEffect *newSoundEffect();

	ServerList *serverList() const;

	Q_INVOKABLE void serverSetAutoConnect(Server *server) const;
	Q_INVOKABLE Server *serverAdd();
	Q_INVOKABLE bool serverDelete(Server *server);
	Q_INVOKABLE bool serverDeleteSelected();

	GoogleOAuth2Authenticator *googleAuthenticator() const;

	Q_INVOKABLE virtual void loginGoogle() override;
	Q_INVOKABLE virtual void registrationGoogle(const QString &code) override;

	int serverListSelectedCount() const;

public slots:
	void playSound(const QString &source, const Sound::SoundType &soundType);
	void stopSound(const QString &source, const Sound::SoundType &soundType);
	int volume(const Sound::ChannelType &channel) const;
	void setVolume(const Sound::ChannelType &channel, const int &volume) const;
	void setSfxVolumeInt(int sfxVolume);

protected slots:
	void onServerConnected() override;
	void onServerDisconnected() override;
	void onStartPageLoaded();

private slots:
	void onMainWindowChanged();
	void onOrientationChanged(Qt::ScreenOrientation orientation);

private:
	void serverListLoad(const QDir &dir = Utils::standardPath(QStringLiteral("servers")));
	void serverListSave(const QDir &dir = Utils::standardPath(QStringLiteral("servers")));

	GoogleOAuth2Authenticator *createGoogleAuthenticator(const QString &clientId, const QString &clientKey);

signals:
	void sfxVolumeChanged(const qreal &volume);
	void serverListSelectedCountChanged();

private:
	Sound *m_sound = nullptr;
	QThread m_soundThread;
	qreal m_sfxVolume = 1.0;
	ServerList *m_serverList = nullptr;
	GoogleOAuth2Authenticator *m_googleAuthenticator = nullptr;
	DesktopCodeFlow *m_codeFlow = nullptr;
	DesktopInternalHandler *m_internalHandler = nullptr;

	friend class DesktopInternalHandler;
	friend class DesktopCodeFlow;
};




/**
 * @brief The DesktopCodeFlow class
 */

class DesktopCodeFlow : public OAuth2CodeFlow
{
	Q_OBJECT

	Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)

public:
	explicit DesktopCodeFlow(OAuth2Authenticator *authenticator, DesktopClient *client) :
		OAuth2CodeFlow(authenticator, client),
		m_client(client)
	{
		connect(this, &OAuth2CodeFlow::authenticationSuccess, this, &DesktopCodeFlow::onAuthSuccess);
	}
	virtual ~DesktopCodeFlow() {}

	enum Mode {
		Login,
		Registration
	};

	Q_ENUM(Mode);

	const Mode &mode() const { return m_mode; }
	void setMode(const Mode &newMode)
	{
		if (m_mode == newMode)
			return;
		m_mode = newMode;
		emit modeChanged();
	}

	QQuickItem *page() const { return m_page; }
	void setPage(QQuickItem *newPage) { m_page = newPage; }

	QVariantMap &internalData() { return m_internalData; }

signals:
	void modeChanged();
	void pageRemoved();

private slots:
	void onAuthSuccess(const QVariantMap &data);

private:
	Mode m_mode = Login;
	QQuickItem *m_page = nullptr;
	DesktopClient *m_client = nullptr;
	QVariantMap m_internalData;
};





/**
 * @brief The DesktopInternalHandler class
 */

class DesktopInternalHandler : public AsyncMessageHandler
{
	Q_OBJECT

public:
	explicit DesktopInternalHandler(DesktopClient *client) : AsyncMessageHandler(client) , m_desktopClient(client) {}
	virtual ~DesktopInternalHandler() {}

public slots:
	void getGoogleLocalClientId(const QJsonObject &json) const;
	//void getConfig(const QJsonObject &json) const;
	//void rankList(const QJsonObject &json) const;

private:
	DesktopClient *m_desktopClient = nullptr;
};

#endif // DESKTOPCLIENT_H
