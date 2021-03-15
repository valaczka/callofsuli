/*
 * ---- Call of Suli ----
 *
 * client.h
 *
 * Created on: 2020. 03. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Client
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  Call of Suli is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef CLIENT_H
#define CLIENT_H

#include <QWebSocket>
#include <QObject>
#include <QQuickWindow>
#include <QSettings>
#include <QJsonObject>
#include <QUrl>

#include "cosmessage.h"
#include "variantmapmodel.h"
#include "gameblock.h"
#include "gamemap.h"
#include "gamematch.h"
#include "cossound.h"
#include "question.h"


struct TerrainData;

class Client : public QObject
{
	Q_OBJECT


public:
	enum ConnectionState { Standby, Connecting, Connected, Disconnected, Reconnecting, Reconnected, Closing };
	Q_ENUM(ConnectionState)


	Q_PROPERTY(QWebSocket * socket READ socket WRITE setSocket NOTIFY socketChanged)
	Q_PROPERTY(ConnectionState connectionState READ connectionState WRITE setConnectionState NOTIFY connectionStateChanged)

	Q_PROPERTY(QString serverDataDir READ serverDataDir WRITE setServerDataDir NOTIFY serverDataDirChanged)
	Q_PROPERTY(QString sessionToken READ sessionToken WRITE setSessionToken NOTIFY sessionTokenChanged)
	Q_PROPERTY(QString serverName READ serverName WRITE setServerName NOTIFY serverNameChanged)
	Q_PROPERTY(bool registrationEnabled READ registrationEnabled WRITE setRegistrationEnabled NOTIFY registrationEnabledChanged)
	Q_PROPERTY(bool passwordResetEnabled READ passwordResetEnabled WRITE setPasswordResetEnabled NOTIFY passwordResetEnabledChanged)
	Q_PROPERTY(QVariantList registrationDomains READ registrationDomains WRITE setRegistrationDomains NOTIFY registrationDomainsChanged)
	Q_PROPERTY(QVariantList registrationClasses READ registrationClasses WRITE setRegistrationClasses NOTIFY registrationClassesChanged)

	Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
	Q_PROPERTY(CosMessage::ClientRoles userRoles READ userRoles WRITE setUserRoles NOTIFY userRolesChanged)
	Q_PROPERTY(int userXP READ userXP WRITE setUserXP NOTIFY userXPChanged)
	Q_PROPERTY(int userRank READ userRank WRITE setUserRank NOTIFY userRankChanged)
	Q_PROPERTY(QString userRankName READ userRankName WRITE setUserRankName NOTIFY userRankNameChanged)
	Q_PROPERTY(QString userFirstName READ userFirstName WRITE setUserFirstName NOTIFY userFirstNameChanged)
	Q_PROPERTY(QString userLastName READ userLastName WRITE setUserLastName NOTIFY userLastNameChanged)
	Q_PROPERTY(QString userNickName READ userNickName WRITE setUserNickName NOTIFY userNickNameChanged)
	Q_PROPERTY(QString userRankImage READ userRankImage WRITE setUserRankImage NOTIFY userRankImageChanged)
	Q_PROPERTY(int userRankLevel READ userRankLevel WRITE setUserRankLevel NOTIFY userRankLevelChanged)
	Q_PROPERTY(QString userPlayerCharacter READ userPlayerCharacter WRITE setUserPlayerCharacter NOTIFY userPlayerCharacterChanged)

	Q_PROPERTY(QVariantList rankList READ rankList WRITE setRankList NOTIFY rankListChanged)
	Q_PROPERTY(QVariantMap characterData READ characterData)
	Q_PROPERTY(QStringList musicList READ musicList)

	Q_PROPERTY(qreal sfxVolume READ sfxVolume WRITE setSfxVolume NOTIFY sfxVolumeChanged)

	explicit Client(QObject *parent = nullptr);
	virtual ~Client();


	static void registerResources();
	static void registerTypes();
	static void initialize();
	static void standardPathCreate();

	Q_INVOKABLE static void loadTerrains();
	Q_INVOKABLE static void loadCharacters();
	Q_INVOKABLE static void loadMusics();
	Q_INVOKABLE static void reloadGameResources();

	Q_INVOKABLE void windowSaveGeometry(QQuickWindow *window, const int &fontSize = -1);
	Q_INVOKABLE int windowRestoreGeometry(QQuickWindow *window, const bool &forceFullscreen = false);
	Q_INVOKABLE void windowSetIcon(QQuickWindow *window);

	Q_INVOKABLE static QString standardPath(const QString &path = QString());
	Q_INVOKABLE static void setSetting(const QString &key, const QVariant &value);
	Q_INVOKABLE static QVariant getSetting(const QString &key);

	Q_INVOKABLE static QVariant readJsonFile(QString filename);
	static QJsonDocument readJsonDocument(QString filename);
	static bool saveJsonDocument(QJsonDocument doc, const QString &filename);

	Q_INVOKABLE static QVariantMap byteArrayToJsonMap(const QByteArray &data);
	Q_INVOKABLE static QByteArray jsonMapToByteArray(const QVariantMap &map);
	Q_INVOKABLE static QString formattedDataSize(const qint64 &size);

	Q_INVOKABLE static QList<QPointF> rotatePolygon(const QList<QPointF> &points, const qreal &angle, const QRectF &boundRect, Qt::Axis axis = Qt::ZAxis);
	Q_INVOKABLE static QList<QPointF> rotatePolygon(const QVariantList &points, const qreal &angle, const QRectF &boundRect, Qt::Axis axis = Qt::ZAxis);

	Q_INVOKABLE static QUrl rankImageSource(const int &rank, const int &rankLevel = -1, const QString &rankImage = "");

	Q_INVOKABLE VariantMapModel *newModel(const QStringList &list) {
		return VariantMapModel::newModel(list, this);
	}

	Q_INVOKABLE QVariantMap nextRank(const int &rankId) const;

	Q_INVOKABLE static QVariantList mapToList(const QVariantMap &map, const QString &keyName = "name");
	Q_INVOKABLE static QVariantMap terrainMap();
	Q_INVOKABLE static QVariantMap objectiveModuleMap() { return Question::objectivesMap(); }
	Q_INVOKABLE static QVariantMap storageModuleMap() { return Question::storagesMap(); }


	QWebSocket * socket() const { return m_socket; }
	ConnectionState connectionState() const { return m_connectionState; }
	QString userName() const { return m_userName; }
	CosMessage::ClientRoles userRoles() const { return m_userRoles; }
	QString sessionToken() const { return m_sessionToken; }
	int userXP() const { return m_userXP; }
	int userRank() const { return m_userRank; }
	QString userRankName() const { return m_userRankName; }
	QString userFirstName() const { return m_userFirstName; }
	QString userLastName() const { return m_userLastName; }
	QString userRankImage() const { return m_userRankImage; }
	int userRankLevel() const { return m_userRankLevel; }
	QString userNickName() const { return m_userNickName; }
	QString serverDataDir() const { return m_serverDataDir; }
	QVariantList rankList() const { return m_rankList; }
	QString userPlayerCharacter() const { return m_userPlayerCharacter; }

	QString serverName() const { return m_serverName; }
	bool registrationEnabled() const { return m_registrationEnabled; }
	bool passwordResetEnabled() const { return m_passwordResetEnabled; }
	QVariantList registrationDomains() const { return m_registrationDomains; }
	QVariantList registrationClasses() const { return m_registrationClasses; }
	QStringList waitForResources() const { return m_waitForResources; }

	static QList<TerrainData> availableTerrains() { return m_availableTerrains; }
	static TerrainData terrain(const QString &name);
	static QVariantMap characterData() { return m_characterData; }
	static QStringList musicList() { return m_musicList; }

	qreal sfxVolume() const { return m_sfxVolume; }



public slots:
	void sendMessageWarning(const QString &title, const QString &informativeText, const QString &detailedText = "") {
		emit messageSent("warning", title, informativeText, detailedText);
	}
	void sendMessageError(const QString &title, const QString &informativeText, const QString &detailedText = "") {
		emit messageSent("error", title, informativeText, detailedText);
	}
	void sendMessageInfo(const QString &title, const QString &informativeText, const QString &detailedText = "") {
		emit messageSent("info", title, informativeText, detailedText);
	}
	void sendDatabaseError(const QString &informativeText) {
		emit messageSent("error", tr("Adatbázis hiba"), informativeText, "");
	}

	void setConnectionState(ConnectionState connectionState);
	void closeConnection();
	void login(const QString &username, const QString &session, const QString &password = "", const bool &isPasswordReset = false);
	void logout();
	void passwordRequest(const QString &email, const QString &code = "");

	int socketSend(const CosMessage::CosClass &cosClass, const QString &cosFunc,
				   const QJsonObject &jsonData = QJsonObject(), const QByteArray &binaryData = QByteArray());
	void setServerDataDir(QString serverDataDir);
	void clearSession();

	void checkPermissions();

	void playSound(const QString &source, const CosSound::SoundType &soundType = CosSound::GameSfx);
	void stopSound(const QString &source, const CosSound::SoundType &soundType = CosSound::Music);
	int volume(const CosSound::ChannelType &channel) const;
	void setVolume(const CosSound::ChannelType &channel, const int &volume) const;
	void setSfxVolume(qreal sfxVolume);
	void setSfxVolumeInt(int sfxVolume);



private slots:
	void setSocket(QWebSocket * socket);
	void socketPing();

	void onSocketConnected();
	void onSocketDisconnected();
	void onSocketBinaryFrameReceived(const QByteArray &frame, const bool &isLastFrame);
	void onSocketBinaryMessageReceived(const QByteArray &message);
	void onSocketSslErrors(const QList<QSslError> &errors);
	void onSocketStateChanged(QAbstractSocket::SocketState state);
	void onSocketBytesWritten(const qint64);

	void setUserName(QString userName);
	void setUserRoles(CosMessage::ClientRoles userRoles);
	void setSessionToken(QString sessionToken);
	void setUserXP(int userXP);
	void setUserRank(int userRank);
	void setUserRankName(QString userRankName);
	void setUserRankLevel(int userRankLevel);
	void setUserFirstName(QString userFirstName);
	void setUserLastName(QString userLastName);
	void setUserNickName(QString userNickName);
	void setUserRankImage(QString userRankImage);
	void setUserPlayerCharacter(QString userPlayerCharacter);
	void setServerName(QString serverName);
	void setRegistrationEnabled(bool registrationEnabled);
	void setPasswordResetEnabled(bool passwordResetEnabled);
	void setRegistrationDomains(QVariantList registrationDomains);
	void setRegistrationClasses(QVariantList registrationClasses);
	void setRankList(QVariantList rankList);

signals:
	void messageSent(const QString &type,
					 const QString &title,
					 const QString &informativeText,
					 const QString &detailedText);
	void reconnecting();

	void messageFrameReceived(const CosMessage &message);
	void messageReceived(const CosMessage &message);

	void authInvalid();
	void authRequirePasswordReset();
	void authPasswordResetSuccess();

	void registrationRequest();
	void registrationRequestSuccess();
	void registrationRequestFailed(QString errorString);

	void settingsLoaded(const QJsonObject &data);
	void settingsError();
	void settingsSuccess();

	void socketBytesToWrite(qint64 bytes);

	void storagePermissionsGranted();
	void storagePermissionsDenied();

	void socketChanged(QWebSocket * socket);
	void connectionStateChanged(ConnectionState connectionState);
	void userNameChanged(QString userName);
	void userRolesChanged(CosMessage::ClientRoles userRoles);
	void sessionTokenChanged(QString sessionToken);
	void userXPChanged(int userXP);
	void userRankChanged(int userRank);
	void userRankNameChanged(QString userRankName);
	void userFirstNameChanged(QString userFirstName);
	void userLastNameChanged(QString userLastName);
	void serverNameChanged(QString serverName);
	void serverDataDirChanged(QString serverDataDir);
	void registrationEnabledChanged(bool registrationEnabled);
	void passwordResetEnabledChanged(bool passwordResetEnabled);
	void registrationDomainsChanged(QVariantList registrationDomains);
	void registrationClassesChanged(QVariantList registrationClasses);
	void waitForResourcesChanged(QStringList waitForResources);
	void userRankImageChanged(QString userRankImage);
	void userRankLevelChanged(int userRankLevel);
	void userPlayerCharacterChanged(QString userPlayerCharacter);
	void userNickNameChanged(QString userNickName);
	void rankListChanged(QVariantList rankList);
	void sfxVolumeChanged(qreal sfxVolume);



private:
	void performUserInfo(const CosMessage &message);
	void performError(const CosMessage &message);

	QWebSocket* m_socket;
	QTimer* m_timer;
	QUrl m_connectedUrl;
	CosMessage *m_cosMessage;

	ConnectionState m_connectionState;
	QString m_userName;
	CosMessage::ClientRoles m_userRoles;
	QString m_sessionToken;
	int m_userXP;
	int m_userRank;
	QString m_userFirstName;
	QString m_userLastName;
	QString m_serverName;
	QString m_serverDataDir;
	QString m_userRankName;
	bool m_registrationEnabled;
	bool m_passwordResetEnabled;
	QVariantList m_registrationDomains;
	QStringList m_waitForResources;
	QStringList m_registeredServerResources;
	QString m_userRankImage;
	static QList<TerrainData> m_availableTerrains;
	CosSound *m_clientSound;
	QThread m_workerThread;
	int m_userRankLevel;
	QString m_userNickName;
	QVariantList m_rankList;
	static QVariantMap m_characterData;
	static QStringList m_musicList;
	qreal m_sfxVolume;
	QString m_userPlayerCharacter;
	QVariantList m_registrationClasses;
};



/**
 * @brief The TerrainData struct
 */

struct TerrainData {
	Q_GADGET

	Q_PROPERTY(QString name MEMBER name)
	Q_PROPERTY(QMap<int, int> blocks MEMBER blocks)
	Q_PROPERTY(int enemies MEMBER enemies)

public:

	QString name;
	QMap<int, int> blocks;
	int enemies;
	QVariantMap data;

	TerrainData(const QString &name = "", const QMap<int, int> &blocks = QMap<int, int>(), const int &enemies = 0, const QVariantMap &data = QVariantMap())
		: name(name)
		, blocks(blocks)
		, enemies(enemies)
		, data(data)
	{}

	friend inline bool operator== (const TerrainData &b1, const TerrainData &b2) {
		return b1.name == b2.name
				&& b1.blocks == b2.blocks
				&& b1.enemies == b2.enemies
				&& b1.data == b2.data;
	}
};



#endif // CLIENT_H