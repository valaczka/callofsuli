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
 * @brief The OfflineReceipt class
 */

class OfflineReceipt : public Receipt
{
	Q_GADGET

public:
	enum UploadState {
		UploadInvalid,
		Uploading,
		UploadSuccess,
		UploadFailed
	};

	Q_ENUM(UploadState)

	OfflineReceipt()
		: Receipt()
		, uploadState(UploadInvalid)
	{}

	QS_SERIALIZABLE

	QS_FIELD(UploadState, uploadState)
	QS_FIELD(QString, readableMap)
	QS_FIELD(QString, readableMission)
	QS_FIELD(QDateTime, timestamp)

	public:
	quint64 permitId = 0;
	QByteArray canonicalContent;
};



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

	const std::vector<OfflineReceipt> &receiptList() const { return m_list; }
	std::vector<OfflineReceipt> &receiptList() { return m_list; }

	OfflineReceipt requireReceipt() const;
	bool appendReceipt(const OfflineReceipt &receipt);
	QByteArray getSignedReceiptList() const;

	static OfflinePermit fromCbor(const QCborValue &cbor);
	QCborMap toCbor() const;

	void updateUserData(User *dst) const;
	Campaign *createCampaign() const;


	bool check();

	bool isValid() const;
	void setIsValid(bool newIsValid);

private:
	QByteArray m_permit;
	PermitContent m_permitContent;
	PermitFull m_permitFull;

	bool m_isValid = false;
	std::vector<OfflineReceipt> m_list;
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
	QHash<int, OfflinePermit> &permitList() { return m_permitList; }

	std::optional<int> getCampaignId(const quint64 &permitId) const;

	bool permitAdd(const PermitFull &permitFull);

	bool checkPermitList();

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
	Q_PROPERTY(bool allPermitValid READ allPermitValid WRITE setAllPermitValid NOTIFY allPermitValidChanged FINAL)
	Q_PROPERTY(QSListModel* receiptModel READ receiptModel CONSTANT FINAL)

public:
	explicit OfflineClientEngine(Server *server, QObject *parent = nullptr);
	virtual ~OfflineClientEngine();

	enum EngineState {
		EngineInvalid = 0,
		EngineDisabled,
		EngineUpload,
		EngineActive,
		EngineUpdate,

		EngineError = 0xff,
	};

	Q_ENUM(EngineState);

	Q_INVOKABLE bool initEngine(Client *client);
	Q_INVOKABLE void saveEngine();
	Q_INVOKABLE void uninitEngine();

	Q_INVOKABLE bool getPermit(const int &campaign);
	Q_INVOKABLE bool removePermit(const int &campaign, const bool &forced = false);
	Q_INVOKABLE bool refreshPermits();

	Q_INVOKABLE bool loadOfflineMode();
	Q_INVOKABLE void loadOfflineMode(const QNetworkReply::NetworkError &) { loadOfflineMode(); }

	Q_INVOKABLE void unloadPage();


	Q_INVOKABLE void loadSyncMode(const QString &nextPage);
	Q_INVOKABLE void loadNextPage();

	Q_INVOKABLE bool synchronize();
	Q_INVOKABLE void updateReceiptModel();
	Q_INVOKABLE void updateCampaignModel(CampaignList *list, Campaign *freePlay);

	Q_INVOKABLE void removeFailedReceipts();


	std::optional<OfflineReceipt> requireReceipt(const qint64 &id) const;
	std::optional<OfflineReceipt> requireReceipt(Campaign *campaign) const { return requireReceipt(campaign ? campaign->campaignid() : 0); }
	bool appendReceipt(const OfflineReceipt &receipt);

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

	bool checkCampaignValid(const qint64 &id) const;
	Q_INVOKABLE bool checkCampaignValid(Campaign *campaign) const { return checkCampaignValid(campaign ? campaign->campaignid() : 0); }

	GameMap::GameModes getCampaignGameModes(const qint64 &id) const;
	Q_INVOKABLE GameMap::GameModes getCampaignGameModes(Campaign *campaign) const {
		return getCampaignGameModes(campaign ? campaign->campaignid() : 0);
	}


	bool loadCampaignResult(StudentCampaignOffsetModel *model, const qint64 &id) const;
	Q_INVOKABLE bool loadCampaignResult(StudentCampaignOffsetModel *model, Campaign *campaign) const {
		return loadCampaignResult(model, campaign ? campaign->campaignid() : 0);
	}

	bool allPermitValid() const;
	void setAllPermitValid(bool newAllPermitValid);

	QSListModel* receiptModel() const;

signals:
	void syncStarted();
	void syncFinished(bool success);
	void dbUpdated();

	void engineStateChanged();
	void engineDbChanged();
	void freeplayChanged();
	void allPermitValidChanged();

private:
	void onPermitDownloaded(const QJsonObject &json);
	void reloadData(const bool &offline);
	bool hasPermit() const;
	bool hasReceipt() const;
	void syncNextPermit();
	void syncFinish();
	void updateNextPermit();
	void onUpdateFinish(const bool &success);
	void onSyncSuccess(const QJsonObject &data);
	void onSyncFailed(const QString &err, const int &campaignId);
	void onSyncError(const QNetworkReply::NetworkError &err);


	Server *const m_server;
	Client *m_client = nullptr;
	EngineState m_engineState = EngineInvalid;
	QString m_engineDb;
	OfflineDb m_db;
	std::unique_ptr<CampaignList> m_campaignList;
	std::unique_ptr<User> m_user;
	std::unique_ptr<StudentMapHandler> m_studentMapHandler;
	std::unique_ptr<QSListModel> m_receiptModel;
	bool m_freeplay = false;
	bool m_allPermitValid = true;
	QString m_nextPage;

	QQuickItem *m_page = nullptr;

	QList<int> m_syncState;
};

#endif // OFFLINECLIENTENGINE_H
