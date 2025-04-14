/*
 * ---- Call of Suli ----
 *
 * rpgsnapshotstorage.cpp
 *
 * Created on: 2025. 03. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgSnapshotStorage
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

#include "rpgsnapshotstorage.h"
#include "Logger.h"
#include "rpgengine.h"


RpgSnapshotStorage::RpgSnapshotStorage(RpgEngine *engine)
	: RpgGameData::SnapshotStorage()
	, m_engine(engine)
{

}



/**
 * @brief RpgSnapshotStorage::playerAdd
 * @param base
 * @param data
 * @return
 */

void RpgSnapshotStorage::playerAdd(const RpgGameData::PlayerBaseData &base, const RpgGameData::Player &data)
{
	QMutexLocker locker(&m_mutex);

	RpgGameData::SnapshotData<RpgGameData::Player, RpgGameData::PlayerBaseData> snapdata;

	snapdata.data = base;
	snapdata.list.insert_or_assign(data.f, data);

	m_players.push_back(snapdata);
}



/**
 * @brief RpgSnapshotStorage::enemyAdd
 * @param base
 * @param data
 */

void RpgSnapshotStorage::enemyAdd(const RpgGameData::EnemyBaseData &base, const RpgGameData::Enemy &data)
{
	QMutexLocker locker(&m_mutex);

	RpgGameData::SnapshotData<RpgGameData::Enemy, RpgGameData::EnemyBaseData> snapdata;

	snapdata.data = base;
	snapdata.list.insert_or_assign(data.f, data);

	m_enemies.push_back(snapdata);
}



/**
 * @brief RpgSnapshotStorage::bulletAdd
 * @param base
 * @param data
 */

bool RpgSnapshotStorage::bulletAdd(const RpgGameData::BulletBaseData &base, const RpgGameData::Bullet &data)
{
	QMutexLocker locker(&m_mutex);

	auto it = std::find_if(m_lastBulletId.begin(), m_lastBulletId.end(),
						   [&base](const RpgGameData::BaseData &d){
		return d.o == base.o && d.s == base.s;
	});

	if (it != m_lastBulletId.end() && it->id >= base.id) {
		///LOG_CDEBUG("engine") << "BULLET ALREADY EXISTS -> SKIP" << base.o << base.s << base.id;
		return false;
	}

	RpgGameData::SnapshotData<RpgGameData::Bullet, RpgGameData::BulletBaseData> snapdata;

	snapdata.data = base;
	snapdata.list.insert_or_assign(data.f, data);

	m_bullets.push_back(snapdata);

	if (it != m_lastBulletId.end()) {
		it->id = base.id;
	} else {
		m_lastBulletId.emplace_back(base.o, base.s, base.id);
	}

	return true;
}



/**
 * @brief RpgSnapshotStorage::registerSnapshot
 * @param cbor
 * @return
 */

bool RpgSnapshotStorage::registerSnapshot(RpgEnginePlayer *player, const QCborMap &cbor)
{
	if (!player)
		return false;

	QMutexLocker locker(&m_mutex);

	bool ret = true;


	if (!registerPlayers(player, cbor))
		ret = false;

	if (player->isHost()) {
		if (!registerEnemies(cbor))
			ret = false;
	}

	return ret;
}



/**
 * @brief RpgSnapshotStorage::render
 * @param tick
 */

void RpgSnapshotStorage::render(const qint64 &tick)
{
	QMutexLocker locker(&m_mutex);


	// Remove outdated bullets

	for (auto it = m_bullets.begin(); it != m_bullets.end(); ) {
		auto i = findState(it->list, RpgGameData::LifeCycle::StageDead);

		if (i != it->list.end()) {
			if (tick - i->second.f > 5000) {

				LOG_CINFO("engine") << "ERASE BULLET" << it->data.o << it->data.id;

				it = m_bullets.erase(it);
				continue;
			}

			/*for (++i; i != it->list.end(); ++i) {
				i->second.st = RpgGameData::LifeCycle::StageDead;
			}*/
		}

		++it;
	}

}




/**
 * @brief RpgSnapshotStorage::actionPlayer
 * @param pdata
 * @param it
 * @param snap
 */

RpgGameData::Player RpgSnapshotStorage::actionPlayer(RpgGameData::PlayerBaseData &pdata, RpgGameData::Player &snap)
{
	RpgGameData::Player ret = snap;

	if (snap.st == RpgGameData::Player::PlayerHit || snap.st == RpgGameData::Player::PlayerShot) {
		LOG_CDEBUG("engine") << "+++ HIT/SHOT" << snap.f << snap.arm.cw;

		if (auto it = ret.arm.find(ret.arm.cw); it == ret.arm.wl.end()) {
			LOG_CERROR("engine") << "Missing weapon" << ret.f << ret.arm.cw;
			return ret;
		} else {
			it->b = std::max(0, it->b-1);
			LOG_CDEBUG("engine") << "--- weapon" << it->t << it->b;
		}
	} else if (snap.st == RpgGameData::Player::PlayerAttack) {
		auto it = find(snap.tg, m_enemies);

		LOG_CDEBUG("engine") << "+++ ATTACK" << snap.f;

		if (it == m_enemies.end()) {
			LOG_CERROR("engine") << "Invalid enemy" << snap.tg.id;
		} else {
			std::map<qint64, RpgGameData::Enemy>::iterator esnapIt;
			auto ptr = getPreviousSnap(it->list, snap.f, &esnapIt);

			if (!ptr) {
				LOG_CERROR("engine") << "Invalid enemy snap" << snap.tg.id << snap.f;
			} else {
				int oldHp = ptr->hp;
				ptr->attacked(it->data, snap.arm.cw, pdata);
				LOG_CINFO("engine") << "****ENEMY HP" << oldHp << ptr->hp;

				if (esnapIt == it->list.end()) {
					ptr->f = snap.f;
					it->list.insert_or_assign(ptr->f, ptr.value());
				}
				for (auto ii = esnapIt; ii != it->list.end(); ++ii) {
					ii->second.hp = ptr->hp;
				}
			}
		}
	}

	return ret;
}



/**
 * @brief RpgSnapshotStorage::actionBullet
 * @param pdata
 * @param snap
 * @return
 */

RpgGameData::Bullet RpgSnapshotStorage::actionBullet(RpgGameData::Bullet &snap,
													 RpgGameData::SnapshotList<RpgGameData::Bullet, RpgGameData::BulletBaseData>::iterator snapIterator,
													 std::map<qint64, RpgGameData::Bullet>::iterator nextIterator)
{
	RpgGameData::Bullet ret = snap;

	if (snap.st == RpgGameData::LifeCycle::StageDestroy)
		snap.st = RpgGameData::LifeCycle::StageDead;

	if (snapIterator != m_bullets.end()) {
		if (nextIterator != snapIterator->list.begin()) {
			nextIterator = std::prev(nextIterator);
		}

		if (nextIterator != snapIterator->list.end()) {
			if (nextIterator->second.st == RpgGameData::LifeCycle::StageDead ||
					nextIterator->second.st == RpgGameData::LifeCycle::StageDestroy) {
				ret.st = RpgGameData::LifeCycle::StageDead;
				ret.f = nextIterator->second.f;

				nextIterator = std::next(nextIterator);
				snapIterator->list.erase(nextIterator, snapIterator->list.end());
			}
		}
	}

	return ret;
}




/**
 * @brief RpgSnapshotStorage::registerPlayers
 * @param player
 * @param cbor
 * @return
 */

bool RpgSnapshotStorage::registerPlayers(RpgEnginePlayer *player,
										 const QCborMap &cbor) {
	Q_ASSERT(player);

	QMutexLocker locker(&m_mutex);

	bool success = false;

	if (const auto &it = cbor.find(QStringLiteral("pp")); it != cbor.cend()) {
		const QCborArray list = it->toArray();

		for (const QCborValue &v : list) {
			const QCborMap m = v.toMap();

			RpgGameData::PlayerBaseData pdata = *player;

			if (!checkBaseData(pdata, m, QStringLiteral("pd"))) {
				LOG_CWARNING("engine") << "Invalid player" << m;
				continue;
			}

			auto it = find(pdata, m_players);

			if (it == m_players.end()) {
				LOG_CWARNING("engine")
						<< "Invalid player data" << pdata.o << pdata.s << pdata.id;
				continue;
			}

			const QCborArray array = m.value(QStringLiteral("p")).toArray();

			for (const QCborValue &pv : array) {
				std::map<qint64, RpgGameData::Player>::iterator snapIt;
				std::optional<RpgGameData::Player> snap = addToPreviousSnap(
															  *it, pv, &snapIt,
															  &RpgGameData::CurrentSnapshot::removePlayerProtectedFields);

				if (!snap) {
					LOG_CWARNING("engine") << "---" << pv;
					continue;
				}

				RpgGameData::Player cSnap = actionPlayer(pdata, snap.value());
				assignLastSnap(cSnap, *it);

				success = true;
			}
		}
	}

	if (const auto &it = cbor.find(QStringLiteral("bb")); it != cbor.cend()) {
		const QCborArray list = it->toArray();

		for (const QCborValue &v : list) {
			const QCborMap m = v.toMap();

			RpgGameData::BulletBaseData bdata;

			bdata.fromCbor(m.value(QStringLiteral("bd")));

			if (bdata.o != player->o) {
				LOG_CWARNING("engine") << "Invalid bullet" << m;
				continue;
			}

			auto it = find(bdata, m_bullets);

			if (it == m_bullets.end()) {
				///LOG_CWARNING("engine") << "Invalid bullet data" << bdata.o << bdata.s << bdata.id;
				RpgGameData::Bullet b;
				b.f = 0;

				if (!bulletAdd(bdata, b)) {
					continue;
				}

				it = std::prev(m_bullets.end());
			}


			const QCborArray array = m.value(QStringLiteral("b")).toArray();

			for (const QCborValue &pv : array) {
				std::map<qint64, RpgGameData::Bullet>::iterator snapIt;
				std::optional<RpgGameData::Bullet> snap = addToPreviousSnap(*it, pv, &snapIt,
																			&RpgGameData::CurrentSnapshot::removeBulletProtectedFields);

				if (!snap) {
					LOG_CWARNING("engine") << "---" << pv;
					continue;
				}

				RpgGameData::Bullet bSnap = actionBullet(snap.value(), it, snapIt);
				assignLastSnap(bSnap, *it);
			}
		}
	}

	return success;
}

/**
 * @brief RpgSnapshotStorage::registerEnemies
 * @param cbor
 * @return
 */

bool RpgSnapshotStorage::registerEnemies(const QCborMap &cbor)
{
	QMutexLocker locker(&m_mutex);

	bool success = true;

	if (const auto &it = cbor.find(QStringLiteral("ee")); it != cbor.cend()) {
		const QCborArray list = it->toArray();

		for (const QCborValue &v : list) {
			const QCborMap m = v.toMap();

			RpgGameData::EnemyBaseData edata;
			edata.fromCbor(m.value(QStringLiteral("ed")));

			auto it = find(edata, m_enemies);

			if (it == m_enemies.end()) {
				LOG_CWARNING("engine") << "Invalid enemy data" << edata.o << edata.s << edata.id;
				RpgGameData::Enemy e;
				e.f = 0;

				enemyAdd(edata, e);

				it = std::prev(m_enemies.end());
			}

			const QCborArray array = m.value(QStringLiteral("e")).toArray();

			for (const QCborValue &pv : array) {
				std::map<qint64, RpgGameData::Enemy>::iterator snapIt;
				std::optional<RpgGameData::Enemy> snap = addToPreviousSnap(*it, pv, &snapIt,
																		   &RpgGameData::CurrentSnapshot::removeEnemyProtectedFields);

				if (!snap) {
					LOG_CWARNING("engine") << "---" << pv;
					continue;
				}

				assignLastSnap(snap.value(), *it);

				if (snap->st == RpgGameData::Enemy::EnemyHit) {
					LOG_CDEBUG("engine") << "+++ ENEMYHIT" << snap->f << snap->st << snap->p << snap->a;
				}
			}
		}
	}

	return success;
}



