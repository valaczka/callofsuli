/*
 * ---- Call of Suli ----
 *
 * offlineclientengine.h
 *
 * Created on: 2026. 02. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * OfflineClientEngine
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

#ifndef OFFLINECLIENTENGINE_H
#define OFFLINECLIENTENGINE_H

#include "campaign.h"
#include "studentmaphandler.h"
#include "user.h"
#include "studentmap.h"
#include <QObject>
#include <QNetworkReply>
#include <offlineengine.h>

class Server;
class Client;


/**
 * @brief The OfflinePermit class
 */

class OfflinePermit
{
public:
	OfflinePermit() = default;

	void setPermit(const PermitFull &permitFull);
	void setPermit(const QByteArray &data);
	const PermitContent &permitContent() const { return m_permitContent; }
	const PermitFull &permitFull() const { return m_permitFull; }


	const ReceiptList &receiptList() const { return m_list; }

	static OfflinePermit fromCbor(const QCborValue &cbor);
	QCborMap toCbor() const;

	void updateUserData(User *dst) const;
	Campaign *createCampaign() const;

	bool isValid() const;
	void setIsValid(bool newIsValid);

private:
	QByteArray m_permit;
	PermitContent m_permitContent;
	PermitFull m_permitFull;

	bool m_isValid = false;
	ReceiptList m_list;
};





/**
 * @brief The OfflineDb class
 */

class OfflineDb
{
public:
	OfflineDb();

	enum Alg {
		AlgCbor = 0,
		AlgAEGIS256,
		AlgXChaCha20Poly1305IETF
	};

	bool loadDb(const QByteArray &data);
	QByteArray saveDb() const;
	void clearDb();

	const QHash<int, OfflinePermit> &permitList() const { return m_permitList; }

	bool permitAdd(const PermitFull &permitFull);

private:
	bool loadCbor(QDataStream &stream, const quint32 &version);
	bool loadXChaCha20Poly(QDataStream &stream, const quint32 &version);

	void writeCbor(QDataStream &stream) const;
	void writeXChaCha20Poly(QDataStream &stream) const;


	Alg m_alg = AlgCbor;
	QHash<int, OfflinePermit> m_permitList;
};





/**
 * @brief The OfflineMap class
 */

class OfflineMap : public StudentMap
{
	Q_OBJECT

public:
	explicit OfflineMap(QObject *parent = nullptr);

	bool loadFromPermitMap(const PermitMap &map, const PermitFull &full);

	const PermitMap &map() const { return m_map; }

private:
	PermitMap m_map;
};




/**
 * @brief The OfflineClientEngine class
 */

class OfflineClientEngine : public QObject, public OfflineEngine
{
	Q_OBJECT

	Q_PROPERTY(EngineState engineState READ engineState WRITE setEngineState NOTIFY engineStateChanged FINAL)
	Q_PROPERTY(QString engineDb READ engineDb WRITE setEngineDb NOTIFY engineDbChanged FINAL)
	Q_PROPERTY(CampaignList* campaignList READ campaignList CONSTANT FINAL)
	Q_PROPERTY(User* user READ user CONSTANT FINAL)
	Q_PROPERTY(StudentMapHandler* studentMapHandler READ studentMapHandler CONSTANT FINAL)
	Q_PROPERTY(bool freeplay READ freeplay WRITE setFreeplay NOTIFY freeplayChanged FINAL)

public:
	explicit OfflineClientEngine(Server *server, QObject *parent = nullptr);
	virtual ~OfflineClientEngine();

	enum EngineState {
		EngineInvalid = 0,
		EngineDisabled,
		EngineUpload,
		EngineDownload,
		EngineActive,

		EngineError = 0xff,
	};

	Q_ENUM(EngineState);

	Q_INVOKABLE bool initEngine(Client *client);
	Q_INVOKABLE void saveEngine();
	Q_INVOKABLE void uninitEngine();

	Q_INVOKABLE void getPermit(const int &campaign);

	Q_INVOKABLE bool loadOfflineMode();
	Q_INVOKABLE void loadOfflineMode(const QNetworkReply::NetworkError &) { loadOfflineMode(); }

	Q_INVOKABLE void unloadPage();

	EngineState engineState() const;
	void setEngineState(const EngineState &newEngineState);

	QString engineDb() const;
	void setEngineDb(const QString &newEngineDb);

	CampaignList* campaignList() const;
	User* user() const;
	StudentMapHandler* studentMapHandler() const;

	const OfflineDb &db() const { return m_db; }

	OfflineMap *findMap(const QString &uuid) const;

	bool freeplay() const;
	void setFreeplay(bool newFreeplay);

signals:
	void engineStateChanged();
	void engineDbChanged();
	void freeplayChanged();

private:
	void onPermitDownloaded(const QJsonObject &json);
	void reloadData(const bool &offline);

	Server *const m_server;
	Client *m_client = nullptr;
	EngineState m_engineState = EngineInvalid;
	QString m_engineDb;
	OfflineDb m_db;
	std::unique_ptr<CampaignList> m_campaignList;
	std::unique_ptr<User> m_user;
	std::unique_ptr<StudentMapHandler> m_studentMapHandler;
	bool m_freeplay = false;

	QQuickItem *m_page = nullptr;
};

#endif // OFFLINECLIENTENGINE_H
