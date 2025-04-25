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
#include "serverservice.h"


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


	Renderer renderer = getRenderer(m_lastAuthTick);

	QString txt = QStringLiteral("RENDER %1\n---------------------------------------------\n").arg(tick);

	m_tmpTxt.clear();

	while (renderer.tick <= tick) {
		for (auto &p : renderer.players) {
			renderPlayer(&renderer, p);
		}

		renderer.step();
	}

	txt.append(m_tmpTxt);

	txt.append(QStringLiteral("---------------------------------------\n\n"));


	m_lastAuthTick = tick - 120;

	removeLessThan(m_tmpSnapshot.players, m_lastAuthTick-60);
	zapSnapshots(m_lastAuthTick-60);

#ifdef WITH_FTXUI
	QCborMap map;
	map.insert(QStringLiteral("mode"), QStringLiteral("SND"));
	map.insert(QStringLiteral("txt"), txt);
	m_engine->service()->writeToSocket(map.toCborValue());

#endif



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
	/// MOVE TO RENDER

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

	RpgGameData::CurrentSnapshot snapshot;

	if (snapshot.fromProtectedCbor(cbor) < 0) {
		LOG_CERROR("engine") << "Wrong";
		return false;
	}


	RpgGameData::PlayerBaseData pdata = *player;

	const auto &dstIt = RpgGameData::CurrentSnapshot::find(m_tmpSnapshot.players, pdata);
	const auto &srcIt = RpgGameData::CurrentSnapshot::find(snapshot.players, pdata);

	if (srcIt == snapshot.players.cend()) {
		LOG_CERROR("engine") << "Invalid player";
		return false;
	}

	if (dstIt == m_tmpSnapshot.players.end()) {
		auto &r = m_tmpSnapshot.players.emplace_back(pdata, std::map<qint64, RpgGameData::Player>{});
		copy(r.list, srcIt->list, m_lastAuthTick);
	} else {
		copy(dstIt->list, srcIt->list, m_lastAuthTick);
	}


#ifdef WITH_FTXUI
	QString txt;

	for (const auto &ptr : m_tmpSnapshot.players) {
		txt += QStringLiteral("PLAYER %1\n").arg(ptr.data.o);

		for (auto it = ptr.list.crbegin(); it != ptr.list.crend(); ++it) {
			const RpgGameData::Player &p = it->second;

			txt += QStringLiteral("%1 [%3]: %2 ").arg(p.f).arg(p.st).arg(it->first);

			if (p.p.size() > 1)
				txt += QStringLiteral("(%1,%2) ").arg(p.p.at(0)).arg(p.p.at(1));

			if (p.cv.size() > 1)
				txt += QStringLiteral("->(%1,%2) ").arg(p.cv.at(0)).arg(p.cv.at(1));

			txt += QString::number(p.a) + QStringLiteral(" %1\n").arg(p.arm.cw);

		}

		txt += QStringLiteral("-----------------------------\n\n");
	}

	QCborMap map;
	map.insert(QStringLiteral("mode"), QStringLiteral("RCV"));
	map.insert(QStringLiteral("txt"), txt);
	m_engine->service()->writeToSocket(map.toCborValue());

#endif




	return true;

	if (const auto &it = cbor.find(QStringLiteral("pp")); it != cbor.cend()) {
		const QCborArray list = it->toArray();

		for (const QCborValue &v : list) {
			const QCborMap m = v.toMap();


			/*
			if (!checkBaseData(pdata, m, QStringLiteral("pd"))) {
				LOG_CWARNING("engine") << "Invalid player" << m;
				continue;
			}
*/
			auto it = find(pdata, m_players);

			if (it == m_players.end()) {
				LOG_CWARNING("engine")
						<< "Invalid player data" << pdata.o << pdata.s << pdata.id;
				continue;
			}

			const QCborArray array = m.value(QStringLiteral("p")).toArray();
			/*
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
			}
*/		}

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
			/*
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
*/		}
	}

	return false;
}

/**
 * @brief RpgSnapshotStorage::registerEnemies
 * @param cbor
 * @return
 */

bool RpgSnapshotStorage::registerEnemies(const QCborMap &cbor)
{
	return false;

	QMutexLocker locker(&m_mutex);

	bool success = true;

	return success;
}


/**
 * @brief RpgSnapshotStorage::getRenderer
 * @param tick
 * @return
 */

Renderer RpgSnapshotStorage::getRenderer(const qint64 &tick)
{
	QMutexLocker locker(&m_mutex);

	Renderer r;
	r.tick = tick;

	for (auto &p : m_players) {
		auto it = m_tmpSnapshot.find(m_tmpSnapshot.players, p.data);
		addRenderer(r.players, &p, (it == m_tmpSnapshot.players.end() ? nullptr : &(*it)), tick);
	}

	for (auto &p : m_enemies) {
		auto it = m_tmpSnapshot.find(m_tmpSnapshot.enemies, p.data);
		addRenderer(r.enemies, &p, (it == m_tmpSnapshot.enemies.end() ? nullptr : &(*it)), tick);
	}

	for (auto &p : m_bullets) {
		auto it = m_tmpSnapshot.find(m_tmpSnapshot.bullets, p.data);
		addRenderer(r.bullets, &p, (it == m_tmpSnapshot.bullets.end() ? nullptr : &(*it)), tick);
	}

	return r;
}



/**
 * @brief RpgSnapshotStorage::renderPlayer
 * @param renderer
 * @param player
 */

void RpgSnapshotStorage::renderPlayer(Renderer *renderer, RendererData<RpgGameData::Player, RpgGameData::PlayerBaseData> &player)
{
	Q_ASSERT(renderer);

	QMutexLocker locker(&m_mutex);


	if (!player.snap) {
		LOG_CERROR("engine") << "Missing snap" << renderer->tick;
	}


	QString t = QString::number(renderer->tick)+": ";

	if (player.ptr) {
		t += "PTR: " + QString::number(player.ptr->size()) + " ";
		if (player.it != player.ptr->end()) {
			t += "IT: " + QString::number(player.it->first) + " ";
		} else {
			t += "IT: - ";
		}
	}

	if (player.tmpPtr) {
		t += "TMP: " + QString::number(player.tmpPtr->size()) + " ";
		if (player.tmpIt != player.tmpPtr->end()) {
			t += "IT: " + QString::number(player.tmpIt->first) + " ";
		} else {
			t += "IT: - ";
		}
	}

	if (player.snap) {
		t += "SNAP: " + QString::number(player.snap->f) + " = " + QString::number(player.snap->st);
	} else {
		t += "SNAP: ---";
	}


	if (player.ptr && player.it->first <= m_lastAuthTick) {
		t += "    ######";
	}

	m_tmpTxt.prepend(t+QStringLiteral("\n"));

	if (player.ptr && player.it->first <= m_lastAuthTick) {
		return;
	}

	if (!player.snap)
		return;

	if (!player.tmpPtr || player.tmpIt == player.tmpPtr->end())
		return;

	const qint64 nextTick = (renderer->tick+1)*10;

	while (player.tmpIt != player.tmpPtr->end() && player.tmpIt->first < nextTick ) {
		const RpgGameData::Player &p = player.tmpIt->second;
		player.snap->f = renderer->tick;
		player.snap->st = p.st;
		player.snap->p = p.p;
		player.snap->cv = p.cv;
		player.snap->tg = p.tg;
		player.snap->arm = p.arm;

		player.tmpIt++;
	}

	if (player.it->first == renderer->tick) {
		//LOG_CDEBUG("engine") << "UPDATE" << renderer->tick << player.snap.has_value();
		//player.it = player.ptr->insert(player.it, {renderer->tick, player.snap.value()});
		m_tmpTxt.prepend(QStringLiteral("    (%1) -> %2 %3,%4\n").arg(renderer->tick).arg(player.snap->st)
						 .arg(player.snap->p.at(0))
						 .arg(player.snap->p.at(1))
						 );
	} else {
		LOG_CDEBUG("engine") << "ADD" << renderer->tick << player.snap.has_value();
		player.tmpPtr->insert_or_assign(renderer->tick, player.snap.value());
		m_tmpTxt.prepend(QStringLiteral("    ++(%1) -> %2 %3,%4\n").arg(renderer->tick).arg(player.snap->st)
						 .arg(player.snap->p.at(0))
						 .arg(player.snap->p.at(1))
						 );
	}
}







/**
 * @brief RpgSnapshotStorage::copy
 * @param dest
 * @param src
 */

template<typename T, typename T2>
void RpgSnapshotStorage::copy(std::map<qint64, T> &dest, const std::map<qint64, T> &src, const qint64 &firstTick)
{
	const qint64 ft = firstTick*10;

	for (const auto &ptr : src) {
		if (ptr.first < ft) {
			LOG_CERROR("engine") << "Snapshot out of time:" << ptr.first << "vs." << firstTick;
			continue;
		}

		qint64 f = ptr.first;

		const qint64 next = 1+(qint64)(f/10);
		bool ready = true;

		while (dest.contains(f)) {
			++f;

			if (f >= next) {
				LOG_CERROR("engine") << "Snapshot storage full:" << ptr.first;
				ready = false;
				break;
			}
		}

		if (!ready)
			continue;

		T d = ptr.second;
		d.f = f;

		dest.insert_or_assign(f, d);
	}
}


/**
 * @brief RpgSnapshotStorage::removeLessThan
 * @param list
 * @param tick
 */

template<typename T, typename T2, typename T3, typename T4>
void RpgSnapshotStorage::removeLessThan(RpgGameData::SnapshotList<T, T2> &list, const qint64 &tick)
{
	for (auto &ptr : list) {
		if (ptr.list.empty())
			continue;

		zapSnapshots(ptr.list, tick*10);
	}
}



/**
 * @brief Renderer::step
 * @return
 */

qint64 Renderer::step()
{
	++tick;

	for (auto &p : players)
		p.step(tick);

	for (auto &p : enemies)
		p.step(tick);

	for (auto &p : bullets)
		p.step(tick);

	return tick;
}
