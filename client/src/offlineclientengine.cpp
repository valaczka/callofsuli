/*
 * ---- Call of Suli ----
 *
 * offlineclientengine.cpp
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

#include <QDir>
#include <sodium.h>
#include "offlineclientengine.h"
#include "Logger.h"
#include "server.h"
#include "utils_.h"
#include "application.h"


#ifndef Q_OS_WASM
#include "desktoputils.h"
#else
namespace DesktopUtils {
qint64 msecSinceBoot() { return 0; }
}
#endif


/**
 * @brief OfflineClientEngine::OfflineClientEngine
 * @param server
 * @param parent
 */

OfflineClientEngine::OfflineClientEngine(Server *server, QObject *parent)
	: QObject{parent}
	, OfflineEngine()
	, m_server(server)
	, m_campaignList(new CampaignList)
	, m_user(new User)
	, m_receiptModel(new QSListModel)
{
	Q_ASSERT(m_server);

	m_receiptModel->setRoleNames(Utils::getRolesFromObject(OfflineReceipt().metaObject()));

}


/**
 * @brief OfflineClientEngine::~OfflineClientEngine
 */

OfflineClientEngine::~OfflineClientEngine()
{
	uninitEngine();
}


/**
 * @brief OfflineClientEngine::initEngine
 * @return
 */

bool OfflineClientEngine::initEngine(Client *client)
{
	static const QString basename("offline.db");

	m_client = client;

	if (!m_studentMapHandler) {
		m_studentMapHandler.reset(new StudentMapHandler);
		m_studentMapHandler->setOfflineEngine(this);
	}

	m_db.clearDb();

	QDir dir = m_server->directory();

	setEngineDb(dir.absoluteFilePath(basename));

	if (!dir.exists(basename)) {
		LOG_CTRACE("client") << "Offline engineDB missing" << m_engineDb;
		setEngineState(EngineDisabled);
		return false;
	}


	const auto &b = Utils::fileContent(m_engineDb);

	if (!b) {
		Application::instance()->messageError(tr("Nem olvasható fájl"), tr("Belső hiba"));
		return false;
	}

	LOG_CDEBUG("client") << "Load engineDB" << m_engineDb;

	if (!m_db.loadDb(*b)) {
		setEngineState(EngineError);
		return false;
	}

	setEngineState(hasPermit() ? EngineActive : EngineDisabled);

	return true;
}



/**
 * @brief OfflineClientEngine::saveEngine
 */

void OfflineClientEngine::saveEngine()
{
	LOG_CDEBUG("client") << "Save offline engine";

	if (m_engineState <= EngineDisabled || m_engineState >= EngineError) {
		LOG_CDEBUG("client") << "Offline engine unavailable";
		return;
	}

	const QByteArray &b = m_db.saveDb();

	QFile f(m_engineDb);

	if (!f.open(QIODevice::WriteOnly)) {
		LOG_CERROR("client") << "Can't write file:" << m_engineDb;
		return;
	}

	f.write(b);

	f.close();
}


/**
 * @brief OfflineClientEngine::uninitEngine
 */

void OfflineClientEngine::uninitEngine()
{
	setEngineDb(QString());
	setEngineState(EngineInvalid);
}





/**
 * @brief OfflineClientEngine::getPermit
 * @param campaign
 */

bool OfflineClientEngine::getPermit(const int &campaign)
{
	if (!m_client)
		return false;

	LOG_CDEBUG("client") << "Get permit for" << campaign;

	if (!m_db.permitList().value(campaign).receiptList().empty()) {
		LOG_CERROR("client") << "Receipt list not empty" << campaign;
		Application::instance()->messageError(tr("Szinkronizálatlan játékok találhatók"), tr("Sikertelen letöltés"));
		return false;
	}

	m_client->send(HttpConnection::ApiUser,
				   campaign > 0 ? QStringLiteral("campaign/%1/permit").arg(campaign)
								: QStringLiteral("freeplay/permit")
								  , {
					   { QStringLiteral("clock"), (qint64) DesktopUtils::msecSinceBoot() }
				   })
			->done(this, &OfflineClientEngine::onPermitDownloaded)
			->fail(this, std::bind(&OfflineClientEngine::onSyncFailed, this, std::placeholders::_1, campaign))
			->error(this, &OfflineClientEngine::onSyncError);


	return true;
}


/**
 * @brief OfflineClientEngine::removePermit
 * @param campaign
 * @param forced
 * @return
 */

bool OfflineClientEngine::removePermit(const int &campaign, const bool &forced)
{
	if (!m_client)
		return false;

	LOG_CDEBUG("client") << "Remove permit for" << campaign;

	if (!m_db.permitList().value(campaign).receiptList().empty()) {
		LOG_CERROR("client") << "Receipt list not empty" << campaign;
		//Application::instance()->messageError(tr("Szinkronizálatlan játékok találhatók"), tr("Sikertelen letöltés"));
		if (!forced)
			return false;
	}

	m_db.permitList().remove(campaign);
	emit dbUpdated();

	saveEngine();

	setEngineState(hasPermit() ? EngineActive : EngineDisabled);

	return true;
}


/**
 * @brief OfflineClientEngine::refreshPermits
 * @return
 */

bool OfflineClientEngine::refreshPermits()
{
	if (!m_client)
		return false;

	if (m_engineState != EngineActive)
		return false;

	LOG_CDEBUG("client") << "Refresh permits";

	for (const auto &[id, p] : m_db.permitList().asKeyValueRange()) {
		getPermit(id);
	}

	return true;
}



/**
 * @brief OfflineClientEngine::loadOfflineMode
 */

bool OfflineClientEngine::loadOfflineMode()
{
	if (!m_client)
		return false;

	LOG_CDEBUG("client") << "Try to load offline mode";

	if (m_client->httpConnection()->state() != HttpConnection::Connecting)
		return false;

	if (!m_client->httpConnection()->server() || m_client->httpConnection()->server() != m_server)
		return false;


	if (m_engineState == OfflineClientEngine::EngineActive) {
		if (m_page) {
			LOG_CDEBUG("client") << "Page already loaded";
			return true;
		}

		if (!m_db.checkPermitList()) {
			setAllPermitValid(false);

			saveEngine();

			Application::instance()->messageError(QObject::tr("A gép órája érvénytelen (újraindítás volt?), szinkronizálj!"),
												  QObject::tr("Adatbázis zárolva"));
		}


		reloadData(true);

		m_page = m_client->stackPushPage(QStringLiteral("PageStudentOffline.qml"), {
											 { QStringLiteral("engine"), QVariant::fromValue(this) }
										 });


		if (m_page)
			return true;
	}

	return false;
}



/**
 * @brief OfflineClientEngine::unloadPage
 */

void OfflineClientEngine::unloadPage()
{
	LOG_CDEBUG("client") << "Unload offline page";

	m_page = nullptr;
}






/**
 * @brief OfflineClientEngine::loadSyncMode
 * @param nextPage
 */

void OfflineClientEngine::loadSyncMode(const QString &nextPage)
{
	if (!m_client) {
		LOG_CERROR("client") << "Missing client";
		return;
	}

	if (nextPage.isEmpty()) {
		LOG_CERROR("client") << "Missing page";
		return;
	}

	if (m_engineState != EngineActive) {
		LOG_CDEBUG("client") << "Offline engine disabled, skip sync page";
		m_client->stackPushPage(nextPage);
		return;
	}

	if (!hasReceipt()) {
		LOG_CDEBUG("client") << "No receipt, skip sync page";
		m_client->stackPushPage(nextPage);
		refreshPermits();
		return;
	}

	LOG_CINFO("client") << "Offline engine sync";

	m_nextPage = nextPage;

	m_client->stackPushPage(QStringLiteral("PageStudentOfflineSync.qml"), QVariantMap {
								{ QStringLiteral("engine"), QVariant::fromValue(this) }
							});
}



/**
 * @brief OfflineClientEngine::loadNextPage
 */

void OfflineClientEngine::loadNextPage()
{
	setEngineState(hasPermit() ? EngineActive : EngineDisabled);

	if (!m_client) {
		LOG_CERROR("client") << "Missing client";
		return;
	}

	if (m_nextPage.isEmpty()) {
		LOG_CERROR("client") << "Missing page";
		return;
	}

	m_client->stackPushPage(m_nextPage);
}



/**
 * @brief OfflineClientEngine::synchronize
 * @return
 */

bool OfflineClientEngine::synchronize()
{
	if (m_engineState != EngineActive) {
		LOG_CWARNING("client") << "Offline engine state error" << m_engineState;
		return false;
	}

	if (!hasReceipt()) {
		LOG_CDEBUG("client") << "No receipt, sync finished";
		emit syncFinished(true);
		return false;
	}

	LOG_CDEBUG("client") << "Offline engine sync started";

	if (!m_syncState.isEmpty()) {
		LOG_CERROR("client") << "Sync state failed";
		setEngineState(EngineError);
		return false;
	}

	emit dbUpdated();
	emit syncStarted();
	setEngineState(EngineUpload);

	for (OfflinePermit &p : m_db.permitList()) {
		for (OfflineReceipt &r : p.receiptList()) {
			r.uploadState = OfflineReceipt::Uploading;
		}
	}

	m_syncState = m_db.permitList().keys();

	syncNextPermit();

	return true;
}



/**
 * @brief OfflineClientEngine::updateReceiptModel
 */

void OfflineClientEngine::updateReceiptModel()
{
	QVariantList list;

	for (const OfflinePermit &p : m_db.permitList()) {
		for (qint64 i = 0; const OfflineReceipt &r : p.receiptList()) {
			QVariantMap m = r.toJson().toVariantMap();

			m[QStringLiteral("receiptId")] = p.permitContent().id * 10000 + i;
			m[QStringLiteral("medal")] = "";
			m[QStringLiteral("timestamp")] = QDateTime::fromSecsSinceEpoch(p.permitContent().getClientTime(r));


			const PermitExtraMap map = p.permitFull().map.value(QString::fromUtf8(r.map));

			m[QStringLiteral("readableMap")] = map.name;

			QJsonArray mList = map.cache.value(QStringLiteral("missions")).toArray();

			for (const QJsonValue &v : mList) {
				const QJsonObject o = v.toObject();
				if (o.value(QStringLiteral("uuid")).toString() == r.mission) {
					m[QStringLiteral("readableMission")] = o.value(QStringLiteral("name")).toString();
					break;
				}
			}

			list.append(m);

			++i;
		}
	}

	Utils::patchSListModel(m_receiptModel.get(), list, QStringLiteral("receiptId"));
}





/**
 * @brief OfflineClientEngine::updateCampaignModel
 * @param list
 * @param freePlay
 */

void OfflineClientEngine::updateCampaignModel(CampaignList *list, Campaign *freePlay)
{
	LOG_CDEBUG("client") << "Update campaign list";

	if (list) {
		for (Campaign *c : *list) {
			if (!m_db.permitList().contains(c->campaignid())) {
				c->setOfflineState(Campaign::OfflineInvalid);
				continue;
			}

			if (m_engineState == EngineUpload || m_engineState == EngineUpdate) {
				c->setOfflineState(Campaign::OfflineUpdate);
				continue;
			}

			if (!m_db.permitList().value(c->campaignid()).receiptList().empty()) {
				c->setOfflineState(Campaign::OfflineError);
				continue;
			}

			c->setOfflineState(m_db.permitList().value(c->campaignid()).isValid() ? Campaign::OfflineReady : Campaign::OfflineUpdate);
		}
	}

	if (freePlay) {
		if (!m_db.permitList().contains(0)) {
			freePlay->setOfflineState(Campaign::OfflineInvalid);
			return;
		}

		if (m_engineState == EngineUpload || m_engineState == EngineUpdate) {
			freePlay->setOfflineState(Campaign::OfflineUpdate);
			return;
		}

		if (!m_db.permitList().value(0).receiptList().empty()) {
			freePlay->setOfflineState(Campaign::OfflineError);
			return;
		}

		freePlay->setOfflineState(m_db.permitList().value(0).isValid() ? Campaign::OfflineReady : Campaign::OfflineUpdate);
	}
}



/**
 * @brief OfflineClientEngine::removeFailedReceipts
 */

void OfflineClientEngine::removeFailedReceipts()
{
	LOG_CINFO("client") << "Remove failed receipts";

	for (OfflinePermit &p : m_db.permitList()) {
		for (auto it = p.receiptList().cbegin(); it != p.receiptList().cend(); ) {
			if (it->uploadState == OfflineReceipt::UploadFailed) {
				LOG_CDEBUG("client") << "Remove receipt" << it->map << it->mission << it->level << it->success;

				it = p.receiptList().erase(it);

				continue;
			}

			++it;
		}

		p.setIsValid(false);			// Invalidate permit
	}

	updateReceiptModel();

	onUpdateFinish(true);
}



/**
 * @brief OfflineClientEngine::requireReceipt
 * @param id
 * @return
 */

std::optional<OfflineReceipt> OfflineClientEngine::requireReceipt(const qint64 &id) const
{
	if (m_engineState == EngineUpload) {
		LOG_CERROR("client") << "Engine uploading";
		return std::nullopt;
	}

	if (!m_db.permitList().contains(id)) {
		LOG_CERROR("client") << "Invalid campaign id" << id;
		return std::nullopt;
	}

	return m_db.permitList().value(id).requireReceipt();
}



/**
 * @brief OfflineClientEngine::appendReceipt
 * @param receipt
 * @return
 */

bool OfflineClientEngine::appendReceipt(const OfflineReceipt &receipt)
{
	if (m_engineState == EngineUpload) {
		LOG_CERROR("client") << "Engine uploading";
		return false;
	}

	const auto &ptr = m_db.getCampaignId(receipt.permitId);

	if (!ptr) {
		LOG_CERROR("client") << "Invalid permit id" << receipt.permitId;
		return false;
	}

	if (m_db.permitList()[*ptr].appendReceipt(receipt)) {
		saveEngine();

		return true;
	}

	return false;
}




/**
 * @brief OfflineClientEngine::engineState
 * @return
 */

OfflineClientEngine::EngineState OfflineClientEngine::engineState() const
{
	return m_engineState;
}

void OfflineClientEngine::setEngineState(const EngineState &newEngineState)
{
	if (m_engineState == newEngineState)
		return;
	m_engineState = newEngineState;
	emit engineStateChanged();
}

QString OfflineClientEngine::engineDb() const
{
	return m_engineDb;
}

void OfflineClientEngine::setEngineDb(const QString &newEngineDb)
{
	if (m_engineDb == newEngineDb)
		return;
	m_engineDb = newEngineDb;
	emit engineDbChanged();
}



/**
 * @brief OfflineClientEngine::onPermitDownloaded
 * @param json
 */

void OfflineClientEngine::onPermitDownloaded(const QJsonObject &json)
{
	if (m_engineState >= EngineError) {
		LOG_CERROR("client") << "Offline engine error";
		return;
	}

	if (m_engineState <= EngineDisabled)
		setEngineState(EngineActive);

	PermitFull full;
	full.fromJson(json);

	if (m_db.permitAdd(full)) {
		if (m_engineState == EngineUpdate) {
			updateNextPermit();
		} else {
			saveEngine();
			reloadData(false);
			emit dbUpdated();
		}
	} else {
		Application::instance()->messageError(tr("Szinkronizálatlan játékok találhatók"), tr("Sikertelen letöltés"));
		onSyncFailed(QStringLiteral("permit update error"), -1);
	}
}



/**
 * @brief OfflineClientEngine::reloadData
 */

void OfflineClientEngine::reloadData(const bool &offline)
{
	LOG_CDEBUG("client") << "Reload offline data";

	m_user->clear();
	m_campaignList->clear();
	setFreeplay(false);

	if (!m_studentMapHandler)
		return;

	m_studentMapHandler->mapList()->clear();

	for (const OfflinePermit &p : m_db.permitList()) {
		if (m_user->username().isEmpty())
			p.updateUserData(m_user.get());

		for (const PermitMap &m : p.permitContent().maps) {
			if (findMap(m.map))
				continue;

			std::unique_ptr<OfflineMap> map(new OfflineMap);

			map->loadFromPermitMap(m, p.permitFull());

			if (!m_studentMapHandler->check(map.get())) {
				if (offline) {
					m_client->messageError(tr("Egy pálya nincs letöltve"));
					continue;
				} else {
					m_studentMapHandler->download(map.get(), HttpConnection::ApiUser, QStringLiteral("map/%1").arg(m.map));
				}
			}

			LOG_CDEBUG("client") << "Loaded map" << m.map;

			m_studentMapHandler->mapList()->append(map.release());
		}

		if (p.permitContent().campaign <= 0) {
			setFreeplay(true);
			continue;
		}

		Campaign *campaign = p.createCampaign();

		if (!campaign) {
			LOG_CERROR("client") << "Campaign create error";
			continue;
		}



		m_campaignList->append(campaign);
	}
}


/**
 * @brief OfflineClientEngine::hasPermit
 * @return
 */

bool OfflineClientEngine::hasPermit() const
{
	return !m_db.permitList().isEmpty()	;
}


/**
 * @brief OfflineClientEngine::hasReceipt
 * @return
 */

bool OfflineClientEngine::hasReceipt() const
{
	if (m_db.permitList().isEmpty())
		return false;

	for (const OfflinePermit &p : m_db.permitList()) {
		if (!p.receiptList().empty())
			return true;
	}

	return false;
}



/**
 * @brief OfflineClientEngine::syncNextPermit
 */

void OfflineClientEngine::syncNextPermit()
{
	if (m_engineState != EngineUpload) {
		LOG_CWARNING("client") << "Offline engine state error" << m_engineState;
		setEngineState(EngineError);
		return;
	}

	updateReceiptModel();

	while (!m_syncState.isEmpty()) {
		const int id = m_syncState.first();

		OfflinePermit &p = m_db.permitList()[id];

		if (p.receiptList().empty()) {
			LOG_CDEBUG("client") << "Skip empty permit" << p.permitContent().id;
			m_syncState.removeFirst();
			continue;
		}

		QByteArray data = p.getSignedReceiptList();

		LOG_CINFO("client") << "Upload permit" << p.permitContent().id;

		m_client->send(HttpConnection::ApiUser, QStringLiteral("offline"), {
						   { QStringLiteral("data"), QString::fromLatin1(data.toBase64()) }
					   })
				->done(this, &OfflineClientEngine::onSyncSuccess)
				->fail(this, std::bind(&OfflineClientEngine::onSyncFailed, this, std::placeholders::_1, id))
				->error(this, &OfflineClientEngine::onSyncError);

		return;
	}


	syncFinish();
}



/**
 * @brief OfflineClientEngine::syncFinish
 */

void OfflineClientEngine::syncFinish()
{
	if (m_engineState != EngineUpload)
		return;

	LOG_CINFO("client") << "Offline engine sync finished";


	for (OfflinePermit &p : m_db.permitList()) {
		for (auto it = p.receiptList().cbegin(); it != p.receiptList().cend(); ) {
			if (it->uploadState == OfflineReceipt::UploadSuccess) {
				LOG_CDEBUG("client") << "Remove receipt" << it->map << it->mission << it->level << it->success;

				it = p.receiptList().erase(it);

				continue;
			}

			++it;
		}

		p.setIsValid(false);			// Invalidate permit
	}

	saveEngine();

	///updateReceiptModel();

	if (!m_syncState.empty()) {
		LOG_CERROR("client") << "Sync state error";
		setEngineState(EngineError);
		return;
	}

	// Get new permits

	for (const auto &[id, p] : m_db.permitList().asKeyValueRange()) {
		if (p.receiptList().empty())
			m_syncState.append(id);
	}

	setEngineState(EngineUpdate);

	LOG_CDEBUG("client") << "Update permits:" << m_syncState;

	updateNextPermit();
}



/**
 * @brief OfflineClientEngine::updateNextPermit
 */

void OfflineClientEngine::updateNextPermit()
{
	if (m_engineState != EngineUpdate) {
		LOG_CWARNING("client") << "Offline engine state error" << m_engineState;
		setEngineState(EngineError);
		return;
	}

	if (m_syncState.empty()) {
		onUpdateFinish(true);
		return;
	}

	const int id = m_syncState.takeFirst();

	if (!getPermit(id)) {
		LOG_CERROR("client") << "Get permit error";
		setEngineState(EngineError);
		return;
	}
}


/**
 * @brief OfflineClientEngine::onUpdateFinish
 * @param success
 */

void OfflineClientEngine::onUpdateFinish(const bool &success)
{
	LOG_CDEBUG("client") << "Update finished" << success;

	setAllPermitValid(m_db.checkPermitList());

	saveEngine();
	reloadData(false);

	emit syncFinished(success);
	setEngineState(EngineActive);
}


/**
 * @brief OfflineClientEngine::onSyncSuccess
 * @param data
 */

void OfflineClientEngine::onSyncSuccess(const QJsonObject &data)
{
	if (!m_syncState.empty()) {
		const int id = m_syncState.takeFirst();

		OfflinePermit &p = m_db.permitList()[id];

		int step = p.permitContent().hashStep - data.value(QStringLiteral("hashStep")).toInteger();

		for (OfflineReceipt &r : p.receiptList()) {
			r.uploadState = (step-- > 0) ? OfflineReceipt::UploadSuccess : OfflineReceipt::UploadFailed;
		}
	}

	syncNextPermit();
}


/**
 * @brief OfflineClientEngine::onSyncFailed
 * @param err
 */

void OfflineClientEngine::onSyncFailed(const QString &err, const int &campaignId)
{
	if (m_engineState != EngineUpload && m_engineState != EngineUpdate) {
		LOG_CERROR("client") << "Permit download failed, campaign exists?" << campaignId;
		if (removePermit(campaignId)) {
			m_client->snack(tr("Offline kihívás törölve lett"));
		} else {
			m_client->snack(tr("Offline kihívás törlése sikertelen"));
		}
		return;
	}

	m_client->messageError(tr("Hiba történt"), tr("Szinkronizálás"));
	LOG_CERROR("client") << "Offline engine sync failed" << err << campaignId;
	///setEngineState(EngineError);

	if (m_engineState == EngineUpdate) {
		onUpdateFinish(false);
	} else {
		setEngineState(hasPermit() ? EngineActive : EngineDisabled);
		emit dbUpdated();
	}
}



/**
 * @brief OfflineClientEngine::onSyncError
 * @param err
 */

void OfflineClientEngine::onSyncError(const QNetworkReply::NetworkError &err)
{
	if (m_engineState != EngineUpload && m_engineState != EngineUpdate) {
		LOG_CERROR("client") << "Invalid permit";

		return;
	}

	LOG_CERROR("client") << "Offline engine sync failed" << err;
	setEngineState(EngineError);
}


/**
 * @brief OfflineClientEngine::receiptModel
 * @return
 */

QSListModel* OfflineClientEngine::receiptModel() const
{
	return m_receiptModel.get();
}


/**
 * @brief OfflineClientEngine::findMap
 * @param uuid
 * @return
 */

OfflineMap *OfflineClientEngine::findMap(const QString &uuid) const
{
	if (!m_studentMapHandler)
		return nullptr;

	for (StudentMap *map : *m_studentMapHandler->mapList())	{
		OfflineMap *omap = qobject_cast<OfflineMap*>(map);
		if (!omap)
			continue;

		if (omap->uuid() == uuid)
			return omap;
	}

	return nullptr;
}


/**
 * @brief OfflineClientEngine::studentMapHandler
 * @return
 */

StudentMapHandler* OfflineClientEngine::studentMapHandler() const
{
	return m_studentMapHandler.get();
}


/**
 * @brief OfflineClientEngine::user
 * @return
 */

User* OfflineClientEngine::user() const
{
	return m_user.get();
}

CampaignList* OfflineClientEngine::campaignList() const
{
	return m_campaignList.get();
}



/**
 * @brief OfflineDb::loadDb
 * @param data
 * @return
 */

OfflineDb::OfflineDb()
{
	m_alg = AlgXChaCha20Poly1305IETF;
}



/**
 * @brief OfflineDb::loadDb
 * @param data
 * @return
 */

bool OfflineDb::loadDb(const QByteArray &data)
{
	QDataStream stream(data);

	quint32 magic = 0;
	QByteArray str;

	stream >> magic >> str;

	if (magic != 0x434F53 || str != "OFL") {			// COS
		LOG_CWARNING("client") << "Invalid map data";
		return false;
	}

	clearDb();

	qint32 version = -1;
	qint32 alg = -1;

	stream >> version >> alg;

	LOG_CTRACE("client") << "Load offline data version:" << version;

	stream.setVersion(QDataStream::Qt_6_7);				// version check

	switch (alg) {
		case AlgCbor:
			return loadCbor(stream, version);
		case AlgXChaCha20Poly1305IETF:
			return loadXChaCha20Poly(stream, version);
		case AlgAEGIS256:
			LOG_CERROR("client") << "Invalid alg" << alg;
	}

	return false;
}



/**
 * @brief OfflineDb::saveDb
 * @param data
 * @return
 */

QByteArray OfflineDb::saveDb() const
{
	QByteArray s;
	QDataStream stream(&s, QIODevice::WriteOnly);
	stream.setVersion(QDataStream::Qt_6_7);

	static const qint32 version = 1;

	stream << (quint32) 0x434F53;			// COS
	stream << QByteArray("OFL");
	stream << version;
	stream << (qint32) m_alg;

	switch (m_alg) {
		case AlgCbor:
			writeCbor(stream);
			break;
		case AlgXChaCha20Poly1305IETF:
			writeXChaCha20Poly(stream);
			break;
		case AlgAEGIS256:
			LOG_CERROR("client") << "Invalid alg" << m_alg;
			break;
	}

	return s;
}


/**
 * @brief OfflineDb::clearDb
 */

void OfflineDb::clearDb()
{
	m_permitList.clear();
}


/**
 * @brief OfflineDb::getCampaignId
 * @param permitId
 * @return
 */

std::optional<int> OfflineDb::getCampaignId(const quint64 &permitId) const
{
	for (const auto &[id, p] : m_permitList.asKeyValueRange()) {
		if (p.permitContent().id == permitId)
			return id;
	}

	return std::nullopt;
}



/**
 * @brief OfflineDb::permitAdd
 * @param permitFull
 */

bool OfflineDb::permitAdd(const PermitFull &permitFull)
{
	OfflinePermit p;
	p.setPermit(permitFull);
	p.setIsValid(true);

	if (!m_permitList.value(p.permitContent().campaign).receiptList().empty()) {
		LOG_CERROR("client") << "Receipt list not empty" << p.permitContent().campaign;
		return false;
	}

	const int id = std::max(p.permitContent().campaign, 0);

	m_permitList[id] = std::move(p);

	return true;
}



/**
 * @brief OfflineDb::checkPermitList
 * @return
 */

bool OfflineDb::checkPermitList()
{
	LOG_CTRACE("client") << "Check permit list";

	bool success = true;
	bool setAllInvalid = false;

	QString userPrev;

	for (OfflinePermit &p : m_permitList) {
		if (!p.isValid())
			success = false;

		if (!p.check()) {
			p.setIsValid(false);
			success = false;
		}

		if (!userPrev.isEmpty() && userPrev != p.permitContent().username) {
			LOG_CERROR("client") << "Offline user mismatch" << userPrev << p.permitContent().username;
			setAllInvalid = true;
		}

		userPrev = p.permitContent().username;
	}

	if (setAllInvalid) {
		for (OfflinePermit &p : m_permitList)
			p.setIsValid(false);

		success = false;
	}

	return success;
}



/**
 * @brief OfflineDb::loadCbor
 * @param stream
 * @return
 */

bool OfflineDb::loadCbor(QDataStream &stream, const quint32 &/*version*/)
{
	QByteArray data;
	stream >> data;

	QCborArray list = QCborValue::fromCbor(data).toArray();

	m_permitList.clear();
	m_permitList.reserve(list.size());

	for (const QCborValue &v : list) {
		OfflinePermit p = OfflinePermit::fromCbor(v);

		const int id = std::max(p.permitContent().campaign, 0);

		m_permitList[id] = std::move(p);
	}

	return true;
}


/**
 * @brief OfflineDb::loadXChaCha20Poly
 * @param stream
 * @param nonce
 * @return
 */

bool OfflineDb::loadXChaCha20Poly(QDataStream &stream, const quint32 &version)
{
	QByteArray nonce;

	stream >> nonce;

	if (nonce.size() != crypto_aead_xchacha20poly1305_ietf_NPUBBYTES) {
		LOG_CERROR("client") << "Invalid nonce length";
		return false;
	}

	QByteArray ciphertext;

	stream >> ciphertext;

	if (ciphertext.size() < crypto_aead_xchacha20poly1305_ietf_ABYTES) {
		LOG_CERROR("client") << "Invalid cyphertext size";
		return false;
	}


	std::array<unsigned char, crypto_aead_xchacha20poly1305_ietf_KEYBYTES> priv =
			Application::instance()->deriveFromSeed<crypto_aead_xchacha20poly1305_ietf_KEYBYTES>(1, "STRG_KEY");

	std::vector<unsigned char> plaintext;
	plaintext.resize(ciphertext.size() - crypto_aead_xchacha20poly1305_ietf_ABYTES);

	unsigned long long outLen = 0;

	if (crypto_aead_xchacha20poly1305_ietf_decrypt(plaintext.data(), &outLen,
												   nullptr,
												   reinterpret_cast<const unsigned char*>(ciphertext.constData()), ciphertext.size(),
												   nullptr, 0,
												   reinterpret_cast<const unsigned char*>(nonce.constData()),
												   priv.data()
												   ) == -1) {
		LOG_CERROR("client") << "Decryption error";
		return false;
	}

	QDataStream cborStream(QByteArray::fromRawData(reinterpret_cast<const char *>(plaintext.data()), outLen));

	cborStream.setVersion(QDataStream::Qt_6_7);				// version check

	return loadCbor(cborStream, version);
}


/**
 * @brief OfflineDb::writeCbor
 * @param stream
 */

void OfflineDb::writeCbor(QDataStream &stream) const
{
	QCborArray list;

	for (const OfflinePermit &p : m_permitList)
		list.append(p.toCbor());

	stream << list.toCborValue().toCbor();
}


/**
 * @brief OfflineDb::writeXChaCha20Poly
 * @param stream
 */

void OfflineDb::writeXChaCha20Poly(QDataStream &stream) const
{
	// Generate random nonce

	std::array<unsigned char, crypto_aead_xchacha20poly1305_ietf_KEYBYTES> priv =
			Application::instance()->deriveFromSeed<crypto_aead_xchacha20poly1305_ietf_KEYBYTES>(1, "STRG_KEY");

	std::array<unsigned char, crypto_aead_xchacha20poly1305_ietf_NPUBBYTES> nonce;

	randombytes_buf(nonce.data(), nonce.size());

	stream << QByteArray::fromRawData(reinterpret_cast<const char *>(nonce.data()), nonce.size());


	// Save Cbor to bytearray with QDataStream

	QByteArray s;
	QDataStream cborStream(&s, QIODevice::WriteOnly);
	stream.setVersion(QDataStream::Qt_6_7);

	writeCbor(cborStream);


	std::vector<unsigned char> msg;
	msg.resize(s.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES);
	unsigned long long msgLen = 0;

	crypto_aead_xchacha20poly1305_ietf_encrypt(msg.data(), &msgLen,
											   reinterpret_cast<const unsigned char*>(s.constData()), s.size(),
											   nullptr, 0,
											   nullptr,
											   nonce.data(),
											   priv.data());

	if (msgLen == 0) {
		LOG_CERROR("client") << "Encryption error";
		return;
	}

	stream << QByteArray::fromRawData(reinterpret_cast<const char*>(msg.data()), msgLen);
}





/**
 * @brief OfflinePermit::setPermit
 * @param permitFull
 */

void OfflinePermit::setPermit(const PermitFull &permitFull)
{
	m_permitFull = permitFull;
	setPermit(QByteArray::fromBase64(permitFull.permit.toLatin1()));
	m_permitFull.permit.clear();				// Nem lesz rá szükség
}




/**
 * @brief OfflinePermit::setPermit
 * @param data
 */

void OfflinePermit::setPermit(const QByteArray &data)
{
	m_permit = data;
	m_permitContent = {};

	const QCborMap m = QCborValue::fromCbor(QCborValue::fromCbor(data).toMap().value(QStringLiteral("content")).toByteArray()).toMap();

	m_permitContent.fromCbor(m);
}



/**
 * @brief OfflinePermit::requireReceipt
 * @return
 */

OfflineReceipt OfflinePermit::requireReceipt() const
{
	OfflineReceipt r;

	if (!m_list.empty()) {
		r.prevHash = OfflineEngine::computeMapHash(m_list.back().canonicalContent);
	}

	r.permitId = m_permitContent.id;
	r.clock = DesktopUtils::msecSinceBoot();

	return r;
}


/**
 * @brief OfflinePermit::appendReceipt
 * @param receipt
 * @return
 */

bool OfflinePermit::appendReceipt(const OfflineReceipt &receipt)
{
	if (receipt.permitId != m_permitContent.id) {
		LOG_CERROR("client") << "Permit id mismatch";
		return false;
	}

	if (m_list.empty()) {
		if (!receipt.prevHash.isEmpty()) {
			LOG_CERROR("client") << "Invalid hash";
			return false;
		}

		if (receipt.clock <= m_permitContent.clientClock) {
			LOG_CERROR("client") << "Invalid clock";
			return false;
		}
	} else {
		if (receipt.prevHash != OfflineEngine::computeMapHash(m_list.back().canonicalContent)) {
			LOG_CERROR("client") << "Invalid hash";
			return false;
		}

		if (receipt.clock <= m_list.back().clock) {
			LOG_CERROR("client") << "Invalid clock";
			return false;
		}
	}


	int step =  m_permitContent.hashStep - m_list.size();

	if (step < 1) {
		LOG_CERROR("client") << "Hash step error" << step;
		return false;
	}

	OfflineReceipt r = receipt;

	r.chainHash = OfflineEngine::computeHashChain(m_permitContent.hashAnchor, step-1);

	r.canonicalContent = dynamic_cast<Receipt*>(&r)->toCborMap().toCborValue().toCbor();

	m_list.push_back(std::move(r));

	return true;
}



/**
 * @brief OfflinePermit::getSignedReceiptList
 * @return
 */

QByteArray OfflinePermit::getSignedReceiptList() const
{
	ReceiptList list;

	list.permit = m_permit;
	list.receipts.reserve(m_list.size());

	for (const OfflineReceipt &r : m_list)
		list.receipts.push_back(r.canonicalContent);

	return Application::instance()->signToRaw(list.toCborMap().toCborValue().toCbor());
}





/**
 * @brief OfflinePermit::fromCbor
 * @param cbor
 * @return
 */

OfflinePermit OfflinePermit::fromCbor(const QCborValue &cbor)
{
	OfflinePermit p;

	if (!cbor.isMap())
		return p;

	const QCborMap m = cbor.toMap();


	PermitFull f;
	f.fromCbor(m.value(QStringLiteral("full")));
	p.setPermit(f);

	p.setPermit(m.value(QStringLiteral("permit")).toByteArray());
	p.setIsValid(m.value(QStringLiteral("valid")).toBool());

	QCborArray a = m.value(QStringLiteral("receipts")).toArray();

	QByteArray prev;

	for (const QCborValue &v : a) {
		const QByteArray content = v.toByteArray();
		const QCborMap m = QCborValue::fromCbor(content).toMap();

		OfflineReceipt r;
		r.fromCbor(m);
		r.canonicalContent = content;

		if (!prev.isEmpty()) {
			if (r.prevHash != OfflineEngine::computeMapHash(prev)) {
				LOG_CERROR("client") << "Invalid receipt list";
				break;
			}
		}

		p.m_list.push_back(std::move(r));

		prev = content;
	}

	return p;
}



/**
 * @brief OfflinePermit::toCbor
 * @return
 */

QCborMap OfflinePermit::toCbor() const
{
	QCborMap m;

	m[QStringLiteral("permit")] = m_permit;
	m[QStringLiteral("valid")] = m_isValid;
	m[QStringLiteral("full")] = m_permitFull.toCborMap();

	QCborArray a;

	for (const OfflineReceipt &r : m_list) {
		a.append(r.canonicalContent);
	}

	m[QStringLiteral("receipts")] = a;

	return m;
}



/**
 * @brief OfflinePermit::updateUserData
 * @param dst
 */

void OfflinePermit::updateUserData(User *dst) const
{
	Q_ASSERT(dst);

	dst->setUsername(m_permitContent.username);
	dst->setFamilyName(m_permitFull.user.familyName);
	dst->setGivenName(m_permitFull.user.givenName);
	dst->setNickName(m_permitFull.user.nickName);
}



/**
 * @brief OfflinePermit::createCampaign
 * @return
 */

Campaign *OfflinePermit::createCampaign() const
{
	Campaign *campaign = new Campaign;

	campaign->setCampaignid(m_permitContent.campaign);
	campaign->setDescription(m_permitFull.campaign.description);
	campaign->setStarted(true);
	campaign->setFinished(false);
	campaign->setEndTime(QDateTime::fromSecsSinceEpoch(m_permitContent.expire));

	OlmLoader::loadFromJsonArray<Task>(campaign->taskList(),
									   m_permitFull.campaign.task, "id", "taskid", true);

	return campaign;
}


/**
 * @brief OfflinePermit::check
 * @return
 */

bool OfflinePermit::check()
{
	const qint64 current = DesktopUtils::msecSinceBoot();

	if (std::abs(m_permitContent.getClientTime(current) - QDateTime::currentSecsSinceEpoch()) > 10*60) {
		LOG_CERROR("client") << "Time difference error";
		return false;
	}


	if (m_permitContent.clientClock > current) {
		LOG_CERROR("client") << "Invalid boot time";
		return false;
	}

	qint64 ref = m_permitContent.clientClock;

	for (const OfflineReceipt &r : m_list) {
		if (r.clock <= ref) {
			LOG_CERROR("client") << "Invalid clock" << r.clock;
			return false;
		}

		ref = r.clock;
	}


	if (m_permitContent.hashStep < 1) {
		LOG_CERROR("client") << "Insufficient hash chain step";
		return false;
	}

	if (m_permitContent.getClientTime(current) >= m_permitContent.expire) {
		LOG_CINFO("client") << "Permit expired";
		return false;
	}


	return true;
}


/**
 * @brief OfflinePermit::isValid
 * @return
 */

bool OfflinePermit::isValid() const
{
	return m_isValid;
}

void OfflinePermit::setIsValid(bool newIsValid)
{
	m_isValid = newIsValid;
}

bool OfflineClientEngine::freeplay() const
{
	return m_freeplay;
}

void OfflineClientEngine::setFreeplay(bool newFreeplay)
{
	if (m_freeplay == newFreeplay)
		return;
	m_freeplay = newFreeplay;
	emit freeplayChanged();
}


/**
 * @brief OfflineClientEngine::checkCampaignValid
 * @param id
 * @return
 */

bool OfflineClientEngine::checkCampaignValid(const qint64 &id) const
{
	return m_db.permitList().value(id > 0 ? id : 0).isValid();
}


/**
 * @brief OfflineClientEngine::getCampaignGameModes
 * @param id
 * @return
 */

GameMap::GameModes OfflineClientEngine::getCampaignGameModes(const qint64 &id) const
{
	return m_db.permitList().value(id > 0 ? id : 0).permitContent().modes;
}



/**
 * @brief OfflineClientEngine::loadCampaignResult
 * @param model
 * @param id
 * @return
 */

bool OfflineClientEngine::loadCampaignResult(StudentCampaignOffsetModel *model, const qint64 &id) const
{
	if (!model)
		return false;

	QJsonArray a;

	const OfflinePermit &permit = m_db.permitList().value(id > 0 ? id : 0);

	for (const OfflineReceipt &r : permit.receiptList()) {
		QJsonObject o;

		o[QStringLiteral("id")] = 0;
		o[QStringLiteral("timestamp")] = permit.permitContent().getClientTime(r);
		o[QStringLiteral("mapid")] = QString::fromUtf8(r.map);
		o[QStringLiteral("missionid")] = QString::fromUtf8(r.mission);
		o[QStringLiteral("level")] = (int) r.level;
		o[QStringLiteral("mode")] = (int) r.mode;
		o[QStringLiteral("deathmatch")] = false;
		o[QStringLiteral("success")] = r.success;
		o[QStringLiteral("duration")] = (int) r.duration;
		o[QStringLiteral("xp")] = (int) r.xp;

		a.append(o);
	}


	return model->replaceFromJson(QJsonObject{
									  { QStringLiteral("list"), a }
								  });
}


/**
 * @brief OfflineMap::OfflineMap
 * @param parent
 */

OfflineMap::OfflineMap(QObject *parent)
	: StudentMap{parent}
{

}



/**
 * @brief OfflineMap::loadFromPermitMap
 * @param map
 * @return
 */
bool OfflineMap::loadFromPermitMap(const PermitMap &map, const PermitFull &full)
{
	m_map = map;

	setUuid(map.map);
	setHash(map.hash);

	if (full.map.contains(map.map)) {
		const PermitExtraMap &em = full.map.value(map.map);
		setName(em.name);
		setCache(em.cache);
	}

	return true;
}


bool OfflineClientEngine::allPermitValid() const
{
	return m_allPermitValid;
}

void OfflineClientEngine::setAllPermitValid(bool newAllPermitValid)
{
	if (m_allPermitValid == newAllPermitValid)
		return;
	m_allPermitValid = newAllPermitValid;
	emit allPermitValidChanged();
}
