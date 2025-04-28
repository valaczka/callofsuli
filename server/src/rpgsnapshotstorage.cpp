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





template<typename T, typename T2, typename T3, typename T4>
bool Renderer::addObjects(const RpgGameData::SnapshotList<T, T2> &list)
{
	bool success = true;

	for (const RpgGameData::SnapshotData<T, T2> &d : list) {
		if (find(d.data)) {
			LOG_CERROR("engine") << "Object already exists" << d.data.o << d.data.s << d.data.id;
			success = false;
			continue;
		} else {
			RendererObject<T2> *obj = new RendererObject<T2>();
			obj->baseData = d.data;
			obj->template fillSnap<T>(m_size);
			std::unique_ptr<RendererObjectType> ptr(obj);
			m_objects.push_back(std::move(ptr));
		}
	}

	return success;
}



/**
 * @brief Renderer::find
 * @param baseData
 * @return
 */

template<typename T, typename T2>
RendererObject<T> *Renderer::find(const T &baseData) const
{
	for (const auto &ptr : m_objects) {
		RendererObject<T> *item = dynamic_cast<RendererObject<T> *>(ptr.get());
		if (item && item->baseData == baseData)
			return item;
	}

	return nullptr;
}




/**
 * @brief RendererObjectType::fill
 * @param size
 * @return
 */

template<typename T, typename T2>
void RendererObjectType::fillSnap(const int &size)
{
	Q_ASSERT(snap.empty());

	snap.reserve(size);

	for (int i=0; i<size; ++i) {
		RendererItem<T> *item = new RendererItem<T>();
		std::unique_ptr<RendererType> ptr(item);
		snap.push_back(std::move(ptr));
	}

	m_iterator = snap.cbegin();
}





/**
 * @brief Renderer::saveObjects
 * @param list
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
int Renderer::saveObjects(RpgGameData::SnapshotList<T, T2> &dst)
{
	int num = 0;

	for (auto &ptr : dst) {
		RendererObject<T2> *obj = find(ptr.data);

		if (!obj) {
			LOG_CDEBUG("engine") << "Missing object" << ptr.data.o << ptr.data.s << ptr.data.id;
			continue;
		}


		qint64 tick = m_startTick;

		for (auto &sp : obj->snap) {
			RendererItem<T> *item = dynamic_cast<RendererItem<T>*>(sp.get());
			if (!item) {
				LOG_CERROR("engine") << "Missing snap" << ptr.data.o << ptr.data.s << ptr.data.id << tick;
				++tick;
				continue;
			}

			if (!item->flags().testFlag(RendererType::ReadOnly) && item->flags().testFlag(RendererType::Modified)) {
				T d = item->data();
				d.f = tick;
				ptr.list.insert_or_assign(tick, d);
			}

			++tick;
		}

		++num;
	}

	return num;
}





/**
 * @brief RendererItem::setData
 * @param data
 * @return
 */

template<typename T, typename T2>
bool RendererItem<T, T2>::setData(const T &data)
{
	if (m_flags.testFlag(ReadOnly))
		return false;

	m_data = data;

	return true;
}





/**
 * @brief RendererObject::dump
 * @return
 */

template<typename T, typename T2>
QString RendererObject<T, T2>::dump(const qint64 &start, const int &size) const
{
	QString txt;

	txt += QStringLiteral("Object %1 %2 %3\n-------------------------------------------\n")
		   .arg(baseData.o)
		   .arg(baseData.s)
		   .arg(baseData.id);

	const int tSize = snap.size();

	if (size <= 0)
		return txt;

	for (int i=size-1; i>=0; --i) {
		if (!snap.at(i)->hasContent())
			continue;

		txt += QStringLiteral("%1: ").arg(start+i, 6);
		if (i < tSize && snap.at(i))
			txt += snap.at(i)->dump();
		else
			txt += QStringLiteral("==== INVALID SNAP ====");

		txt += QStringLiteral("\n");
	}

	return txt;
}





template<typename B, typename T2>
QString RendererType::dumpAs(const B &data, const QList<B> &subData) const
{
	QString txt = QStringLiteral("%1 [%2] %3")
				  .arg(m_flags)
				  .arg(data.f)
				  .arg(subData.size())
				  ;

	return txt;
}



/**
 * @brief RendererType::dumpAs
 * @param data
 * @param subData
 * @return
 */

void RendererType::addFlags(const RendererFlags &flags)
{
	m_flags |= flags;
}





/**
 * @brief RendererType::dumpAs
 * @param data
 * @param subData
 * @return
 */

QString RendererType::dumpAs(const RpgGameData::Player &data, const QList<RpgGameData::Player> &subData) const
{
	QString txt = QStringLiteral("%1 [%2] %3 hp w:%4  ")
				  .arg(m_flags)
				  .arg(data.st)
				  .arg(data.hp)
				  .arg(data.arm.cw)
				  ;

	if (data.p.size() > 1)
		txt +=  QStringLiteral("(%1, %2) ")
				.arg(data.p.at(0))
				.arg(data.p.at(1))
				;

	txt += QString::number(data.a);

	return txt;
}







/**
 * @brief RendererObjectType::snapAt
 * @param index
 * @return
 */

RendererType *RendererObjectType::snapAt(const int &index) const
{
	if (index < 0 || index >= (int) snap.size()) {
		LOG_CERROR("engine") << "Invalid index" << index << "of size" << snap.size();
		return nullptr;
	}

	return snap.at(index).get();
}




/**
 * @brief Renderer::snapAt
 * @param object
 * @param index
 * @return
 */

template<typename T, typename T2>
RendererItem<T> *Renderer::snapAt(RendererObjectType *object, const int &index) const
{
	Q_ASSERT(object);

	return dynamic_cast<RendererItem<T>*> (object->snapAt(index));
}





/**
 * @brief RendererObjectType::setSnap
 * @param index
 * @param data
 * @return
 */

template<typename T, typename T2>
bool RendererObjectType::setSnap(const int &index, const T &data)
{
	RendererItem<T>* item = dynamic_cast<RendererItem<T>*> (snap.at(index).get());

	Q_ASSERT(item);

	return item->setData(data);
}



/**
 * @brief RendererObjectType::setSnap
 * @param index
 * @param data
 * @param addFlags
 * @return
 */

template<typename T, typename T2>
bool RendererObjectType::setSnap(const int &index, const T &data, const RendererType::RendererFlags &addFlags)
{
	RendererItem<T>* item = dynamic_cast<RendererItem<T>*> (snap.at(index).get());

	Q_ASSERT(item);

	const bool r = item->setData(data);
	item->addFlags(addFlags);

	return r;
}




/**
 * @brief RendererObjectType::addSubSnap
 * @param index
 * @param data
 * @return
 */

template<typename T, typename T2>
bool RendererObjectType::addSubSnap(const int &index, const T &data)
{
	RendererItem<T>* item = dynamic_cast<RendererItem<T>*> (snap.at(index).get());

	Q_ASSERT(item);

	item->addSubData(data);
	item->addFlags(RendererType::Temporary);

	return true;
}



/**
 * @brief RendererObjectType::setAuthSnap
 * @param data
 * @return
 */


template<typename T, typename T2>
bool RendererObjectType::setAuthSnap(const T &data)
{
	return setSnap(0, data, RendererType::Storage | RendererType::ReadOnly);
}








/**
 * @brief Renderer::loadSnaps
 * @param list
 * @param isStorage
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
bool Renderer::loadSnaps(RpgGameData::SnapshotList<T, T2> &list, const bool &isStorage)
{
	bool success = true;

	for (auto &ptr : list) {
		RendererObject<T2> *o = find(ptr.data);
		if (!o) {
			LOG_CDEBUG("engine") << "Missing object" << ptr.data.o << ptr.data.s << ptr.data.id;
			success = false;
			continue;
		}

		if (isStorage) {
			if (!loadStorageSnaps(o, ptr))
				success = false;
		} else {
			if (!loadTemporarySnaps(o, ptr))
				success = false;
		}
	}

	return success;
}







/**
 * @brief RpgSnapshotStorage::RpgSnapshotStorage
 * @param engine
 */


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

	std::unique_ptr<Renderer> renderer = getRenderer(tick);

	QString txt = QStringLiteral("RENDER %1\n---------------------------------------------\n").arg(tick);

	if (!renderer) {
		LOG_CERROR("engine") << "INVALID RENDERER" << tick << m_lastAuthTick;
	} else {

		renderer->render();

		txt.append(renderer->dump());

		txt.append(QStringLiteral("---------------------------------------\n\n"));


		saveRenderer(renderer.get());

		// A mutex miatt törölni kell!

		renderer.reset();
	}

	m_lastAuthTick = tick - 60;

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
		/*	auto i = findState(it->list, RpgGameData::LifeCycle::StageDead);

		if (i != it->list.end()) {
			if (tick - i->second.f > 5000) {

				LOG_CINFO("engine") << "ERASE BULLET" << it->data.o << it->data.id;

				it = m_bullets.erase(it);
				continue;
			}

			for (++i; i != it->list.end(); ++i) {
				i->second.st = RpgGameData::LifeCycle::StageDead;
			}
		}

		++it;*/
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

	if (snapshot.fromCbor(cbor) < 0) {
		LOG_CERROR("engine") << "Wrong";
		return false;
	}


	RpgGameData::PlayerBaseData pdata = *player;

	const auto &dstIt = RpgGameData::CurrentSnapshot::find(m_tmpSnapshot.players, pdata);
	const auto &srcIt = RpgGameData::CurrentSnapshot::find(snapshot.players, pdata);

	if (srcIt == snapshot.players.cend()) {
		LOG_CERROR("engine") << "Invalid player" << cbor;
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

std::unique_ptr<Renderer> RpgSnapshotStorage::getRenderer(const qint64 &tick)
{
	if (tick <= m_lastAuthTick)
		return {};

	std::unique_ptr<Renderer> r = std::make_unique<Renderer>(m_lastAuthTick, tick-m_lastAuthTick+1);

	if (!r->addObjects(m_players))
		return {};

	if (!r->loadSnaps(m_players, true))
		return {};

	if (!r->loadSnaps(m_tmpSnapshot.players, false))
		return {};

	return r;

}


/**
 * @brief RpgSnapshotStorage::saveRenderer
 * @param renderer
 * @return
 */

bool RpgSnapshotStorage::saveRenderer(Renderer *renderer)
{
	Q_ASSERT(renderer);

	renderer->saveObjects(m_players);

	return true;
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
 * @brief Renderer::Renderer
 * @param start
 * @param size
 */

Renderer::Renderer(const qint64 &start, const int &size)
	: m_startTick(start)
	, m_size(size)
{
	if (size <= 0)
		LOG_CWARNING("engine") << "Renderer size <= 0";
}


/**
 * @brief Renderer::~Renderer
 */

Renderer::~Renderer()
{
	m_objects.clear();
}



/**
 * @brief Renderer::dump
 * @return
 */

QString Renderer::dump() const
{
	QString txt;

	for (const auto &ptr : m_objects) {
		if (ptr.get())
			txt += ptr->dump(m_startTick, m_size);
		else
			txt += QStringLiteral("***** INVALID OBJECT *****\n");
		txt += QStringLiteral("\n");
	}

	return txt;
}




/**
 * @brief Renderer::render
 * @param dst
 * @param src
 */

void Renderer::render(RendererItem<RpgGameData::Player> *dst, RendererObject<RpgGameData::PlayerBaseData> *src)
{
	Q_ASSERT(dst);
	Q_ASSERT(src);

	if (!dst->hasContent())
		return;

	// Ezzel hasonlítjük össze, hogy változott-e

	const RpgGameData::Player original = dst->m_data;

	if (!dst->flags().testFlag(RendererType::ReadOnly)) {

		const auto &ptr = extendFromLast(src);

		if (!dst->flags().testFlag(RendererType::Storage)) {
			if (ptr) {
				dst->m_data = ptr.value();

				// Nem szabad áthozni pl. lövést vagy más eventet

				if (dst->m_data.st != RpgGameData::Player::PlayerMoving)
					dst->m_data.st = RpgGameData::Player::PlayerIdle;

				dst->addFlags(RendererType::Storage | RendererType::Modified);
			} else {
				LOG_CERROR("engine") << "Extend failed" << src->baseData.o;
				return;
			}
		} else {
			if (ptr)
				restore(&dst->m_data, ptr.value());
		}
	}

	const RpgGameData::Player saved = dst->m_data;

	if (!dst->m_subData.empty()) {

		if (dst->flags().testFlag(RendererType::ReadOnly)) {
			LOG_CERROR("engine") << "Update read only snap" << dst->data().f;
		}

		RpgGameData::Player::PlayerState specialSt = RpgGameData::Player::PlayerInvalid;
		RpgGameData::Weapon::WeaponType cw = RpgGameData::Weapon::WeaponInvalid;

		for (const RpgGameData::Player &p : dst->m_subData) {
			if ( p.st != RpgGameData::Player::PlayerIdle &&
				 p.st != RpgGameData::Player::PlayerMoving &&
				 p.st != RpgGameData::Player::PlayerInvalid) {

				// TODO: conflict solver (Attack,...) -> continue

				if (specialSt == RpgGameData::Player::PlayerInvalid) {

					if (p.st == RpgGameData::Player::PlayerWeaponChange) {
						cw = p.arm.cw;
					} else {
						specialSt = p.st;
					}

				} else {
					LOG_CWARNING("engine") << "State conflict" << specialSt << p.st;
				}
			}

			dst->m_data = p;
		}

		// Az érzékeny adatokat visszaállítjuk, mindegy, mit kaptunk

		restore(&dst->m_data, saved);


		// Az egyszerű statusokat beállítjuk

		if (specialSt != RpgGameData::Player::PlayerInvalid)
			dst->m_data.st = specialSt;

		// A current weapon külön állítódik (status nélkül)

		if (cw != RpgGameData::Weapon::WeaponInvalid)
			dst->m_data.arm.cw = cw;


		// Töröljük a sub-okat

		dst->clearSubData();
	}

	if (dst->m_data != original)
		dst->addFlags(RendererType::Modified);
}





/**
 * @brief Renderer::restore
 * @param dst
 * @param data
 */

void Renderer::restore(RpgGameData::Player *dst, const RpgGameData::Player &data)
{
	Q_ASSERT(dst);

	// A current weapon nem érzékeny

	RpgGameData::Weapon::WeaponType cw = dst->arm.cw;

	dst->tg = data.tg;
	dst->hp = data.hp;
	dst->mhp = data.mhp;

	dst->arm = data.arm;
	dst->arm.cw = cw;
}



/**
 * @brief Renderer::extendFromFirst
 * @param dst
 * @param src
 */

std::optional<RpgGameData::Player> Renderer::extendFromLast(RendererObject<RpgGameData::PlayerBaseData> *src)
{
	Q_ASSERT(src);

	auto it = src->iterator();

	if (it == src->snap.cbegin())
		return std::nullopt;

	for (--it; ; --it) {
		RendererItem<RpgGameData::Player> *p = dynamic_cast<RendererItem<RpgGameData::Player>*>(it->get());

		if (p && (p->flags().testFlag(RendererType::Storage))) {
			return p->m_data;
		}

		if (it == src->snap.cbegin())
			break;
	}

	return std::nullopt;
}





/**
 * @brief Renderer::loadStorageSnaps
 * @param dst
 * @param list
 * @return
 */

bool Renderer::loadStorageSnaps(RendererObject<RpgGameData::PlayerBaseData> *dst,
								RpgGameData::SnapshotData<RpgGameData::Player, RpgGameData::PlayerBaseData> &ptr)
{
	auto it = RpgSnapshotStorage::getPreviousSnap(ptr.list, m_startTick);

	if (it == ptr.list.end()) {
		LOG_CDEBUG("engine") << "Missing snap" << ptr.data.o << m_startTick;
		return false;
	}

	if (!dst->setAuthSnap(it->second)) {
		LOG_CDEBUG("engine") << "Snap error" << ptr.data.o << m_startTick;
		return false;
	}

	bool success = true;

	const qint64 last = m_startTick+m_size-1;

	for (++it; it != ptr.list.end(); ++it) {
		if (it->first > last)
			break;

		const qint64 index = it->first - m_startTick;

		Q_ASSERT(index >= 0);
		Q_ASSERT(index < m_size);

		if (!dst->setSnap(index, it->second, RendererType::Storage)) {
			LOG_CDEBUG("engine") << "Snap error" << ptr.data.o << m_startTick << index << it->first;
			success = false;
		}
	}


	return success;
}




/**
 * @brief Renderer::loadTemporarySnaps
 * @param dst
 * @param ptr
 * @return
 */

bool Renderer::loadTemporarySnaps(RendererObject<RpgGameData::PlayerBaseData> *dst,
								  RpgGameData::SnapshotData<RpgGameData::Player, RpgGameData::PlayerBaseData> &ptr)
{
	const int first = (m_startTick+1)*10;
	const int last = (m_startTick+m_size-1)*10;

	for (const auto &m : ptr.list) {
		if (m.first < first || m.first >= last)
			continue;

		const qint64 index = (qint64)(m.first/10) - m_startTick;

		Q_ASSERT(index >= 0);
		Q_ASSERT(index < m_size);

		dst->addSubSnap(index, m.second);
	}

	return true;
}





/**
 * @brief Renderer::render
 * @return
 */

bool Renderer::render()
{
	for (;;) {

		// TODO: conflict solver

		for (const auto &ptr : m_objects) {
			ptr->render(this);
		}

		if (!step())
			break;
	}

	return true;
}




/**
 * @brief Renderer::step
 * @return
 */

bool Renderer::step()
{
	if (m_current >= m_size)
		return false;

	++m_current;

	for (const auto &ptr : m_objects)
		ptr->next();

	return m_current < m_size;
}


