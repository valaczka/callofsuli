/*
 * ---- Call of Suli ----
 *
 * offlineserverengine.cpp
 *
 * Created on: 2026. 02. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * OfflineServerEngine
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

#include "offlineserverengine.h"
#include "userapi.h"
#include <querybuilder.hpp>


#define MAX_HASH_LENGTH			1000


OfflineServerEngine::OfflineServerEngine(ServerService *service)
	: OfflineEngine()
	, m_service(service)
{
	Q_ASSERT(service);

	m_signer.setSecret(m_service->settings()->jwtSecret());
}



/**
 * @brief OfflineServerEngine::createPermit
 * @param username
 * @param campaign
 * @param device
 * @return
 */

std::optional<PermitFull> OfflineServerEngine::createPermit(const QString &username, const int &campaign, const QByteArray &device,
															const qint64 &clientClock)
{
	Q_ASSERT(m_service->databaseMain());

	DatabaseMain *dbMain = m_service->databaseMain();

	QDefer ret;
	PermitContent permit;
	PermitFull full;

	struct MapHash {
		QString name;
		QByteArray hash;
		QJsonObject cache;
	};

	QHash<QString, MapHash> mapHash;

	m_service->databaseMainWorker()->execInThread([this, dbMain, username, ret, campaign, device, clientClock,
												  &permit, &full, &mapHash]() mutable {
		QSqlDatabase db = QSqlDatabase::database(dbMain->dbName());

		QMutexLocker _locker(dbMain->mutex());

		{
			QueryBuilder q(db);
			q.addQuery("SELECT id, anchor, step, expected FROM permit "
					   "WHERE username=").addValue(username)
					.addQuery(" AND device=").addValue(QString::fromLatin1(device.toBase64()));

			if (campaign > 0)
				q.addQuery(" AND campaignid=").addValue(campaign);
			else
				q.addQuery(" AND campaignid IS NULL");


			if (!q.execCheckExists()) {				// CHECK STEP! (<300 -> create new)
				if (!generateHash(permit, username, campaign, device)) {
					LOG_CERROR("client") << "Permit create error";
					return ret.reject();
				}

			} else {
				permit.id = q.value("id").toInt();
				permit.hashAnchor = q.value("anchor").toByteArray();
				permit.hashStep = q.value("step").toInt();
			}
		}

		permit.username = username;
		permit.deviceid = device;
		permit.campaign = campaign;
		permit.clientClock = clientClock;
		permit.serverTime = QDateTime::currentSecsSinceEpoch();
		permit.expire = permit.serverTime + 24*60*60;
		permit.modes = GameMap::Practice | GameMap::Lite | GameMap::Test;

		// Get campaing


		if (campaign > 0) {
			{
				QueryBuilder q(db);
				q.addQuery("SELECT endtime, description FROM campaign "
						   "WHERE campaign.started IS TRUE AND campaign.finished IS FALSE AND "
						   "id=").addValue(campaign);

				if (!q.exec() || !q.sqlQuery().first()) {
					LOG_CWARNING("client") << "Invalid campaign" << campaign;
					return ret.reject();
				}

				if (const QDateTime &dt = q.value("endtime").toDateTime(); dt.isValid()) {
					permit.expire = std::min(permit.expire, dt.toSecsSinceEpoch());
				}

				full.campaign.description = q.value("description").toString();
			}

			{
				QueryBuilder q(db);
				q.addQuery("SELECT DISTINCT mapuuid, mapdb.map.data AS data, mapdb.map.name AS mapname, "
						   "mapdb.cache.data AS cache FROM task "
						   "LEFT JOIN mapdb.map ON (mapdb.map.uuid=task.mapuuid) "
						   "LEFT JOIN mapdb.cache ON (mapdb.cache.uuid=mapdb.map.uuid) "
						   "WHERE campaignid=").addValue(campaign)
						;

				if (!q.exec()) {
					LOG_CERROR("client") << "SQL error";
					return ret.reject();
				}


				while (q.sqlQuery().next()) {
					const QString uuid = q.value("mapuuid").toString();

					if (!mapHash.contains(uuid)) {
						MapHash mh;

						mh.name = q.value("mapname").toString();
						mh.hash = computeMapHash(q.value("data").toByteArray());
						mh.cache = QJsonDocument::fromJson(q.value("cache").toString().toUtf8()).object();

						mapHash[uuid] = mh;
					}

					PermitMap map;
					map.map = uuid;
					permit.maps.push_back(std::move(map));
				}
			}

		} else {

			// Freeplay

			{
				QueryBuilder q(db);
				q.addQuery("SELECT DISTINCT mapuuid, mission, mapdb.map.data AS data, mapdb.cache.data AS cache, "
						   "mapdb.map.name AS mapname FROM freeplay "
						   "LEFT JOIN mapdb.map ON (mapdb.map.uuid=freeplay.mapuuid) "
						   "LEFT JOIN mapdb.cache ON (mapdb.cache.uuid=mapdb.map.uuid) "
						   "WHERE groupid IN ("
						   "SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(username)
						.addQuery(")")
						;

				if (!q.exec()) {
					LOG_CERROR("client") << "SQL error";
					return ret.reject();
				}


				while (q.sqlQuery().next()) {
					const QString uuid = q.value("mapuuid").toString();

					if (!mapHash.contains(uuid)) {
						MapHash mh;

						mh.name = q.value("mapname").toString();
						mh.hash = computeMapHash(q.value("data").toByteArray());
						mh.cache = QJsonDocument::fromJson(q.value("cache").toString().toUtf8()).object();

						mapHash[uuid] = mh;
					}

					PermitMap map;
					map.map = uuid;
					map.mission = q.value("mission").toString();
					permit.maps.push_back(std::move(map));
				}
			}
		}

		ret.resolve();
	});

	QDefer::await(ret);

	if (ret.state() == REJECTED)
		return std::nullopt;


	for (const auto &[uuid, hash] : mapHash.asKeyValueRange()) {
		QJsonObject o;

		// Load map names

		PermitExtraMap &em = full.map[uuid];
		em.name = hash.name;
		em.cache = hash.cache;


		if (const auto &solver = UserAPI::solverInfo(m_service->databaseMain(), username, uuid)) {
			// QMap<QString, GameMap::SolverInfo>
			for (const auto &[mission, info] : solver->asKeyValueRange()) {
				o[mission] = info.toJsonArray();
			}
		}

		for (PermitMap &map : permit.maps) {
			if (map.map != uuid)
				continue;

			map.hash = hash.hash;
			map.solver = o;
		}
	}

	QDefer ret2;

	m_service->databaseMainWorker()->execInThread([dbMain, username, ret2, &full, campaign]() mutable {
		QSqlDatabase db = QSqlDatabase::database(dbMain->dbName());

		QMutexLocker _locker(dbMain->mutex());


		if (const auto &ptr = QueryBuilder::q(db)
				.addQuery("SELECT familyName, givenName, nickname AS nickName FROM user "
						  "WHERE username=").addValue(username)
				.execToJsonObject()) {
			full.user.fromJson(*ptr);
		} else {
			return ret2.reject();
		}



		if (const auto &ptr = QueryBuilder::q(db)
				.addQuery("SELECT task.id, gradeid, grade.value AS gradeValue, xp, required, mapuuid, criterion, map.name as mapname, "
						  "taskSuccess.result as result, (taskSuccess.username IS NOT NULL) AS success FROM task "
						  "LEFT JOIN mapdb.map ON (mapdb.map.uuid=task.mapuuid) "
						  "LEFT JOIN grade ON (grade.id=task.gradeid) "
						  "LEFT JOIN taskSuccess ON (taskSuccess.taskid=task.id AND taskSuccess.username=")
				.addValue(username)
				.addQuery(") WHERE campaignid=").addValue(campaign)
				.execToJsonArray({
		{ QStringLiteral("criterion"), [](const QVariant &v) {
								 return QJsonDocument::fromJson(v.toString().toUtf8()).object();
	} }
	})
				) {
			full.campaign.task = *ptr;
		} else {
			return ret2.reject();
		}

		ret2.resolve();
	});

	QDefer::await(ret2);

	if (ret2.state() == REJECTED)
		return std::nullopt;

	full.permit = QString::fromLatin1(signPermit(permit).toBase64());

	return full;
}




/**
 * @brief OfflineServerEngine::signPermit
 * @param permit
 * @return
 */

QByteArray OfflineServerEngine::signPermit(const PermitContent &permit) const
{
	return m_signer.signToRaw(permit);
}


/**
 * @brief OfflineServerEngine::verifyPermit
 * @param data
 * @return
 */

std::optional<PermitContent> OfflineServerEngine::verifyPermit(const QByteArray &data) const
{
	return m_signer.verifyTo<PermitContent>(data);
}



/**
 * @brief OfflineServerEngine::uploadReceipts
 * @param permit
 * @param list
 * @return
 */

std::optional<PermitResponse> OfflineServerEngine::uploadReceipts(UserAPI *api, const PermitContent &permit, const std::vector<Receipt> &list)
{
	Q_ASSERT(api);
	Q_ASSERT(m_service->databaseMain());

	DatabaseMain *dbMain = m_service->databaseMain();

	QDefer ret;

	int step = 0;
	QByteArray chain;

	m_service->databaseMainWorker()->execInThread([api, dbMain, ret, &permit, &list, &step, &chain]() mutable {
		QSqlDatabase db = QSqlDatabase::database(dbMain->dbName());

		QMutexLocker _locker(dbMain->mutex());

		{
			if (const auto &ptr = QueryBuilder::q(db)
					.addQuery("SELECT expected FROM permit WHERE id=").addValue(permit.id)
					.execToValue("expected")) {
				chain = ptr->toByteArray();
			} else {
				LOG_CWARNING("client") << "Hash chain error for permit" << permit.id;
				return ret.reject();
			}
		}

		bool hasSuccess = false;

		for (const Receipt &r : list) {
			if (OfflineEngine::computeMapHash(r.chainHash) != chain) {
				LOG_CWARNING("client") << "Hash chain error";
				break;
			}

			UserAPI::UserGame game;

			game.map = QString::fromUtf8(r.map);
			game.mission = QString::fromUtf8(r.mission);
			game.level = r.level;
			game.mode = r.mode;
			game.campaign = permit.campaign;
			game.timestamp = QDateTime::fromSecsSinceEpoch(permit.getClientTime(r)).toMSecsSinceEpoch();

			int gameId = -1;
			api->gameCreate(permit.username, permit.campaign, game, {}, &gameId);

			if (gameId == -1) {
				LOG_CERROR("client") << "Game create error" << permit.username;
				break;
			}

			bool ok = false;

			api->gameFinish(permit.username, gameId, game, {}, r.stat,
							r.success, r.xp, r.duration, &ok, nullptr, UserAPI::GameFinishGameOnly);

			if (!ok) {
				LOG_CERROR("client") << "Game finish error" << permit.username;
				break;
			}

			if (r.success)
				hasSuccess = true;

			chain = r.chainHash;

			++step;
		}


		if (hasSuccess) {
			bool ok = false;
			UserAPI::UserGame game;
			game.campaign = permit.campaign;

			api->gameFinish(permit.username, 0, game, {}, {},
							true, 0, 0, &ok, nullptr, UserAPI::GameFinishCampaignOnly);

			if (!ok) {
				LOG_CERROR("client") << "Game finish error" << permit.username;
				return ret.reject();
			}
		}

		if (step > 0) {
			LOG_CDEBUG("client") << "Permit" << permit.id << "decrease step" << step;

			if (!QueryBuilder::q(db)
					.addQuery("UPDATE permit SET step=step-").addValue(step)
					.addQuery(",  expected=").addValue(chain)
					.addQuery(" WHERE id=").addValue(permit.id)
					.exec()) {
				step = 0;
				return ret.reject();
			}
		}

		ret.resolve();
	});

	QDefer::await(ret);

	PermitResponse resp;

	resp.id = permit.id;
	resp.campaign = permit.campaign;
	resp.hashStep = permit.hashStep-step;

	return resp;
}



/**
 * @brief OfflineServerEngine::generateHash
 * @param permit
 * @param username
 * @param campaign
 * @param device
 * @return
 */

bool OfflineServerEngine::generateHash(PermitContent &permit, const QString &username, const int &campaign, const QByteArray &device)
{
	// Generate anchor

	QByteArray anchor(crypto_generichash_BYTES, Qt::Uninitialized);

	randombytes_buf(anchor.data(), anchor.size());


	const QByteArray last = computeHashChain(anchor, MAX_HASH_LENGTH);

	if (last.isEmpty()) {
		LOG_CERROR("client") << "Hash create error" << username << campaign << device.toHex(':');
		return false;
	}


	QSqlDatabase db = QSqlDatabase::database(m_service->databaseMain()->dbName());

	QMutexLocker _locker(m_service->databaseMain()->mutex());

	if (campaign <= 0) {
		if (!QueryBuilder::q(db)
				.addQuery("DELETE FROM permit WHERE username=").addValue(username)
				.addQuery(" AND device=").addValue(QString::fromLatin1(device.toBase64()))
				.addQuery(" AND campaignid IS NULL")
				.exec()) {
			LOG_CERROR("client") << "SQL error";
			return false;
		}
	}

	const auto &ptr = QueryBuilder::q(db)
					  .addQuery("INSERT OR REPLACE INTO permit (")
					  .setFieldPlaceholder()
					  .addQuery(") VALUES (")
					  .setValuePlaceholder()
					  .addQuery(")")
					  .addField("username", username)
					  .addField("campaignid", campaign > 0 ? campaign : QVariant(QMetaType::fromType<int>()))
					  .addField("device", QString::fromLatin1(device.toBase64()))
					  .addField("anchor", anchor)
					  .addField("step", MAX_HASH_LENGTH)
					  .addField("expected", last)
					  .execInsertAsInt();

	if (!ptr)
	{
		LOG_CWARNING("client") << "Hash create error" << username << campaign << device.toHex(':');
		return false;
	}

	permit.id = *ptr;
	permit.hashAnchor = anchor;
	permit.hashStep = MAX_HASH_LENGTH;

	return true;
}
