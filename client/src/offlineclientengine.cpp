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
#include "desktoputils.h"
#include "server.h"
#include "utils_.h"
#include "application.h"


OfflineClientEngine::OfflineClientEngine(Server *server, QObject *parent)
	: QObject{parent}
	, OfflineEngine()
	, m_server(server)
	, m_campaignList(new CampaignList)
	, m_user(new User)
{
	Q_ASSERT(m_server);
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

	setEngineState(EngineActive);

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
	LOG_CDEBUG("client") << "Uninit engine";

	setEngineDb(QString());
	setEngineState(EngineInvalid);
}



/**
 * @brief OfflineClientEngine::getPermit
 * @param campaign
 */

void OfflineClientEngine::getPermit(const int &campaign)
{
	if (!m_client)
		return;

	LOG_CDEBUG("client") << "Get permit for" << campaign;

	m_client->send(HttpConnection::ApiUser,
				   campaign > 0 ? QStringLiteral("campaign/%1/permit").arg(campaign)
								: QStringLiteral("freeplay/permit")
								  , {
					   { QStringLiteral("clock"), (qint64) DesktopUtils::msecSinceBoot() }
				   })
			->done(this, &OfflineClientEngine::onPermitDownloaded);
	//->fail(this, &Client::onLoginFailed);
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
		saveEngine();
		reloadData(false);
	} else {
		Application::instance()->messageError(tr("Szinkronizálatlan játékok találhatók"), tr("Sikertelen letöltés"));
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
	m_alg = AlgCbor;
}



/**
 * @brief OfflineDb::loadDb
 * @param data
 * @return
 */

bool OfflineDb::loadDb(const QByteArray &data)
{
	QDataStream stream(data);
	stream.setVersion(QDataStream::Qt_6_7);

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

	m_permitList[p.permitContent().campaign] = std::move(p);

	return true;
}



/**
 * @brief OfflineDb::loadCbor
 * @param stream
 * @return
 */

bool OfflineDb::loadCbor(QDataStream &stream, const quint32 &version)
{
	QByteArray data;
	stream >> data;

	QCborArray list = QCborValue::fromCbor(data).toArray();

	m_permitList.clear();
	m_permitList.reserve(list.size());

	for (const QCborValue &v : list) {
		OfflinePermit p = OfflinePermit::fromCbor(v);
		/// check
		m_permitList[p.permitContent().campaign] = std::move(p);
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
	return false;
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

	/// load receipt list

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

	/// save receipt list

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

