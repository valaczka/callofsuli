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
 * @brief Renderer::findByBase
 * @param baseData
 * @return
 */

template<typename T, typename T2>
RendererObject<T> *Renderer::findByBase(const RpgGameData::BaseData &baseData) const
{
	for (const auto &ptr : m_objects) {
		RendererObject<T> *item = dynamic_cast<RendererObject<T> *>(ptr.get());
		if (item && item->baseData.isBaseEqual(baseData))
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
	int diff = 0;

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

			if (item->flags().testFlag(RendererType::Modified)) {
				T d = item->data();
				d.f = tick;
				ptr.list.insert_or_assign(tick, d);
			}

			if (item->flags().testFlags(RendererType::FromAuth)) {
				diff = tick-m_startTick;
			}

			++tick;
		}
	}

	return diff;
}







/**
 * @brief Renderer::rewind
 * @param src
 * @return
 */

template<typename T, typename T2>
RendererItem<T> *Renderer::rewind(RendererObjectType *src) const
{
	Q_ASSERT(src);

	auto it = src->iterator();

	if (it == src->snap.cbegin())
		return nullptr;

	for (--it; ; --it) {
		RendererItem<T> *p = dynamic_cast<RendererItem<T>*>(it->get());

		if (p && (p->flags().testFlag(RendererType::Storage))) {
			return p;
		}

		if (it == src->snap.cbegin())
			break;
	}

	return nullptr;
}




/**
 * @brief Renderer::forward
 * @param src
 * @return
 */

template<typename T, typename T2>
RendererItem<T> *Renderer::forward(RendererObjectType *src) const
{
	Q_ASSERT(src);

	auto it = src->iterator();

	if (it == src->snap.cend() || std::next(it) == src->snap.cend())
		return nullptr;

	for (++it; ;) {
		RendererItem<T> *p = dynamic_cast<RendererItem<T>*>(it->get());

		if (p && (p->flags().testFlag(RendererType::Storage))) {
			return p;
		}

		if (++it == src->snap.cend())
			break;
	}

	return nullptr;
}




/**
 * @brief Renderer::extendFromLast
 * @param src
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
std::optional<T> Renderer::extendFromLast(RendererObject<T2> *src)
{
	Q_ASSERT(src);

	RendererItem<T> *p = rewind<T>(src);

	if (p)
		return p->m_data;
	else
		return std::nullopt;
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
 * @brief RendererItem::dataOverride
 * @param data
 */

template<typename T, typename T2>
void RendererItem<T, T2>::dataOverride(const T &data)
{
	m_data = data;
}




/**
 * @brief Renderer::loadStorageSnaps
 * @param dst
 * @param ptr
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
bool Renderer::loadStorageSnaps(RendererObject<T2> *dst, RpgGameData::SnapshotData<T, T2> &ptr)
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

template<typename T, typename T2, typename T3, typename T4>
bool Renderer::loadTemporarySnaps(RendererObject<T2> *dst, RpgGameData::SnapshotData<T, T2> &ptr)
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






/**
 * @brief RendererType::dumpAs
 * @param data
 * @param subData
 * @return
 */

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

QString RendererType::dumpAs(const RpgGameData::Player &data, const QList<RpgGameData::Player> &/*subData*/) const
{
	auto it = data.arm.find(RpgGameData::Weapon::WeaponLongsword);

	QString txt = QStringLiteral("[%1] %2 hp w:%3 {%4} ")
				  .arg(data.st)
				  .arg(data.hp)
				  .arg(data.arm.cw)
				  .arg(it == data.arm.wl.cend() ? -2 : it->b)
				  ;

	if (data.p.size() > 1)
		txt +=  QStringLiteral("(%1, %2) ")
				.arg(data.p.at(0))
				.arg(data.p.at(1))
				;

	if (data.l) {
		txt += QStringLiteral(" LOCKED");
	}

	txt += QString::number(data.a);

	return txt;
}



/**
 * @brief RendererType::dumpAs
 * @param data
 * @param subData
 * @return
 */

QString RendererType::dumpAs(const RpgGameData::Enemy &data, const QList<RpgGameData::Enemy> &/*subData*/) const
{
	QString txt = QStringLiteral("%1 [%2] %3 hp ")
				  .arg(m_flags)
				  .arg(data.st)
				  .arg(data.hp)
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
 * @brief RendererType::dumpAs
 * @param data
 * @param subData
 * @return
 */

QString RendererType::dumpAs(const RpgGameData::Bullet &data, const QList<RpgGameData::Bullet> &/*subData*/) const
{
	QString txt = QStringLiteral("%1 [%2] %3")
				  .arg(data.f)
				  .arg(data.st)
				  .arg(data.p)
				  ;

	if (data.tg.isValid())
		txt += QStringLiteral(" -> %1 %2 %3")
			   .arg(data.tg.o)
			   .arg(data.tg.s)
			   .arg(data.tg.id)
			   ;


	return txt;
}


/**
 * @brief RendererType::dumpAs
 * @param data
 * @param subData
 * @return
 */

QString RendererType::dumpAs(const RpgGameData::ControlLight &data, const QList<RpgGameData::ControlLight> &/*subData*/) const
{
	QString txt = QStringLiteral("%1 [%2] %3")
				  .arg(data.f)
				  .arg(data.st)
				  .arg(data.st == RpgGameData::ControlLight::LightOn ? QStringLiteral("(**)") : QStringLiteral("(  )"))
				  ;

	return txt;
}


/**
 * @brief RendererType::dumpAs
 * @param data
 * @param subData
 * @return
 */

QString RendererType::dumpAs(const RpgGameData::ControlContainer &data, const QList<RpgGameData::ControlContainer> &/*subData*/) const
{
	QString txt = QStringLiteral("%1 [%2] ")
				  .arg(data.f)
				  .arg(data.a ? '+' : ' ')
				  ;

	if (data.st == RpgGameData::ControlContainer::ContainerClose)
		txt += QStringLiteral("closed");
	else
		txt += QStringLiteral("OPENED");

	if (data.lck)
		txt += QStringLiteral(" LOCKED");

	if (data.u.isValid())
		txt += QStringLiteral("  holder %1").arg(data.u.o);


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
 * @brief RendererObjectType::carrySubSnap
 * @param data
 * @return
 */

template<typename T, typename T2>
bool RendererObjectType::carrySubSnap(const T &data)
{
	RendererItem<T>* item = dynamic_cast<RendererItem<T>*>(cnext());

	if (!item)
		return false;

	item->addSubData(data);
	item->addFlags(RendererType::Temporary);

	// Auth snap növelésének jelzése

	if (iterator() == snap.cbegin()+1) {
		LOG_CERROR("engine") << "CARRY";
		item->addFlags(RendererType::FromAuth);
	}

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
 * @brief RendererObjectType::overrideAuthSnap
 * @param data
 * @return
 */

template<typename T, typename T2>
void RendererObjectType::overrideAuthSnap(const T &data)
{
	RendererItem<T>* item = dynamic_cast<RendererItem<T>*> (snap.at(0).get());

	Q_ASSERT(item);

	item->dataOverride(data);
	item->addFlags(RendererType::Storage | RendererType::ReadOnly | RendererType::Modified);
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
 * @brief Renderer::loadSnaps
 * @param list
 * @param snapshot
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
bool Renderer::loadAuthSnaps(const RpgGameData::SnapshotList<T, T2> &src)
{
	bool success = true;

	for (const auto &ptr : src) {
		if (ptr.list.empty())
			continue;

		RendererObject<T2> *o = find(ptr.data);
		if (!o) {
			LOG_CDEBUG("engine") << "Missing object" << ptr.data.o << ptr.data.s << ptr.data.id;
			success = false;
			continue;
		}

		o->overrideAuthSnap(ptr.list.cbegin()->second);

		LOG_CINFO("engine") << "OVERRIDE" << ptr.list.cbegin()->second.f << "VS" << m_startTick << m_current;
	}

	return success;
}




/**
 * @brief RpgSnapshotStorage::generateEvents
 * @param snapshot
 * @param tick
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
int RpgSnapshotStorage::generateEvents(const RpgGameData::SnapshotData<T, T2> &snapshot, const qint64 &tick)
{
	const auto &it = snapshot.list.find(tick);

	if (it == snapshot.list.cend())
		return -1;

	std::optional<T> prev;

	if (it != snapshot.list.cbegin())
		prev = std::prev(it)->second;

	return m_engine->createEvents(tick, snapshot.data, it->second, prev);
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
	RpgGameData::SnapshotData<RpgGameData::Enemy, RpgGameData::EnemyBaseData> snapdata;

	snapdata.data = base;
	snapdata.list.insert_or_assign(data.f, data);

	m_enemies.push_back(snapdata);
}


/**
 * @brief RpgSnapshotStorage::lightAdd
 * @param base
 * @param data
 * @return
 */

void RpgSnapshotStorage::lightAdd(const RpgGameData::ControlBaseData &base, const RpgGameData::ControlLight &data)
{
	RpgGameData::SnapshotData<RpgGameData::ControlLight, RpgGameData::ControlBaseData> snapdata;

	snapdata.data = base;
	snapdata.list.insert_or_assign(data.f, data);

	m_controls.lights.push_back(snapdata);
}



/**
 * @brief RpgSnapshotStorage::containerAdd
 * @param base
 * @param data
 * @return
 */

void RpgSnapshotStorage::containerAdd(const RpgGameData::ControlContainerBaseData &base, const RpgGameData::ControlContainer &data)
{
	RpgGameData::SnapshotData<RpgGameData::ControlContainer, RpgGameData::ControlContainerBaseData> snapdata;

	snapdata.data = base;
	snapdata.list.insert_or_assign(data.f, data);

	m_controls.containers.push_back(snapdata);
}




/**
 * @brief RpgSnapshotStorage::bulletAdd
 * @param base
 * @param data
 */

bool RpgSnapshotStorage::bulletAdd(const RpgGameData::BulletBaseData &base, const RpgGameData::Bullet &data)
{
	auto it = std::find_if(m_lastBulletId.begin(), m_lastBulletId.end(),
						   [&base](const RpgGameData::BaseData &d){
		return d.o == base.o && d.s == base.s;
	});

	if (it != m_lastBulletId.end() && it->id >= base.id) {
		LOG_CWARNING("engine") << "BULLET ALREADY EXISTS -> SKIP" << base.o << base.s << base.id;
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

	LOG_CWARNING("engine") << "BULLET ADD" << base.o << base.s << base.id;
	return true;
}



/**
 * @brief RpgSnapshotStorage::append
 * @param key
 * @param data
 * @return
 */

bool RpgSnapshotStorage::append(const RpgGameData::EnemyBaseData &key, const RpgGameData::Enemy &data)
{
	const auto &dstIt = RpgGameData::CurrentSnapshot::find(m_enemies, key);

	if (dstIt == m_enemies.end())
		return false;

	dstIt->list.insert_or_assign(data.f, data);

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

	bool ret = true;


	if (!registerPlayers(player, cbor))
		ret = false;

	return ret;
}




/**
 * @brief RpgSnapshotStorage::render
 * @param tick
 */

QString RpgSnapshotStorage::render(const qint64 &tick)
{
	std::unique_ptr<Renderer> renderer = getRenderer(tick);

	QString txt = QStringLiteral("RENDER %1\n---------------------------------------------\n").arg(tick);

	int diff = 0;

	if (!renderer) {
		LOG_CERROR("engine") << "INVALID RENDERER" << tick << m_lastAuthTick;
		return {};
	}


	QElapsedTimer t1;
	t1.start();

	renderer->render();

	m_engine->renderTimerLog(t1.elapsed());

	txt.append(renderer->dump());

	txt.append(QStringLiteral("---------------------------------------\n\n"));

	diff = saveRenderer(renderer.get());

	if (diff > 0)
		LOG_CERROR("engine") << "****DIFF" << diff;

	m_lastAuthTick = tick + diff - 30;

	return txt;

}





/**
 * @brief RpgSnapshotStorage::renderEnd
 * @param tick
 */

void RpgSnapshotStorage::renderEnd(const QString &txt)
{
	// TODO: lifecycle
	removeList(m_tmpSnapshot.bullets, removeOutdated(m_bullets, m_lastAuthTick-120));

	removeLessThan(m_tmpSnapshot.players, m_lastAuthTick-5);
	removeLessThan(m_tmpSnapshot.enemies, m_lastAuthTick-5);
	removeLessThan(m_tmpSnapshot.bullets, m_lastAuthTick-5);

	removeLessThan(m_tmpSnapshot.controls.lights, m_lastAuthTick-5);
	removeLessThan(m_tmpSnapshot.controls.containers, m_lastAuthTick-5);


	zapSnapshots(m_lastAuthTick-5);


#ifdef WITH_FTXUI
	QCborMap map;
	map.insert(QStringLiteral("mode"), QStringLiteral("SND"));
	map.insert(QStringLiteral("txt"), txt);
	m_engine->service()->writeToSocket(map.toCborValue());

#endif
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

	RpgGameData::CurrentSnapshot snapshot;

	if (snapshot.fromCbor(cbor) < 0) {
		LOG_CWARNING("engine") << "Empty snapshot received";
		return false;
	}


	RpgGameData::PlayerBaseData pdata = *player;

	const auto &dstIt = RpgGameData::CurrentSnapshot::find(m_tmpSnapshot.players, pdata);
	const auto &srcIt = RpgGameData::CurrentSnapshot::find(snapshot.players, pdata);

	if (srcIt == snapshot.players.cend()) {
		//LOG_CERROR("engine") << "Invalid player" << cbor;
		return false;
	}

	if (dstIt == m_tmpSnapshot.players.end()) {
		auto &r = m_tmpSnapshot.players.emplace_back(pdata, std::map<qint64, RpgGameData::Player>{});
		copy(r.list, srcIt->list, m_lastAuthTick);
	} else {
		copy(dstIt->list, srcIt->list, m_lastAuthTick);
	}

	if (!registerBullets(snapshot, pdata))
		return false;

	if (player->isHost()) {
		if (!registerEnemies(snapshot))
			return false;
	}

	/*
#ifdef WITH_FTXUI
	QString txt;

	for (const auto &ptr : m_tmpSnapshot.bullets) {
		txt += QStringLiteral("BULLET %1\n").arg(ptr.data.o);

		for (auto it = ptr.list.crbegin(); it != ptr.list.crend(); ++it) {
			const RpgGameData::Bullet &p = it->second;

			txt += QStringLiteral("%1 [%3]: %2 ").arg(p.f).arg(p.st).arg(it->first);

			txt += QString::number(p.destroyTick());
			txt += QStringLiteral("\n");

		}

		txt += QStringLiteral("-----------------------------\n\n");
	}

	QCborMap map;
	map.insert(QStringLiteral("mode"), QStringLiteral("RCV"));
	map.insert(QStringLiteral("txt"), txt);
	m_engine->service()->writeToSocket(map.toCborValue());

#endif
*/



	return true;
}

/**
 * @brief RpgSnapshotStorage::registerEnemies
 * @param snapshot
 * @return
 */

bool RpgSnapshotStorage::registerEnemies(const RpgGameData::CurrentSnapshot &snapshot)
{
	for (const auto &e : snapshot.enemies) {
		const auto &dstIt = RpgGameData::CurrentSnapshot::find(m_tmpSnapshot.enemies, e.data);

		if (dstIt == m_tmpSnapshot.enemies.end()) {
			auto &r = m_tmpSnapshot.enemies.emplace_back(e.data, std::map<qint64, RpgGameData::Enemy>{});
			copy(r.list, e.list, m_lastAuthTick);
		} else {
			copy(dstIt->list, e.list, m_lastAuthTick);
		}
	}

	return true;
}



/**
 * @brief RpgSnapshotStorage::registerBullets
 * @param snapshot
 * @return
 */

bool RpgSnapshotStorage::registerBullets(const RpgGameData::CurrentSnapshot &snapshot, const RpgGameData::PlayerBaseData &player)
{
	for (const auto &e : snapshot.bullets) {

		if (e.data.o != player.o) {
			LOG_CWARNING("engine") << "Invalid bullet" << e.data.o << e.data.s << e.data.id;
			continue;
		}

		const auto &mainIt = RpgGameData::CurrentSnapshot::find(m_bullets, e.data);

		if (mainIt == m_bullets.end()) {
			RpgGameData::Bullet b;
			b.f = 0;
			b.st = RpgGameData::LifeCycle::StageCreate;

			if (!bulletAdd(e.data, b)) {
				continue;
			}
		}

		const auto &dstIt = RpgGameData::CurrentSnapshot::find(m_tmpSnapshot.bullets, e.data);

		if (dstIt == m_tmpSnapshot.bullets.end()) {
			auto &r = m_tmpSnapshot.bullets.emplace_back(e.data, std::map<qint64, RpgGameData::Bullet>{});
			copy(r.list, e.list, m_lastAuthTick);
		} else {
			copy(dstIt->list, e.list, m_lastAuthTick);
		}
	}

	return true;
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


	/// TODO REMOVE
	if (!r->addObjects(m_controls.containers))
		return {};



	if (!r->addObjects(m_players))
		return {};

	if (!r->addObjects(m_enemies))
		return {};

	if (!r->addObjects(m_bullets))
		return {};



	if (!r->addObjects(m_controls.lights))
		return {};

	/*if (!r->addObjects(m_controls.containers))
		return {};*/


	RpgGameData::CurrentSnapshot snapshot = m_engine->processEvents(r->startTick());

	if (!r->loadSnaps(m_players, true))
		return {};

	if (!r->loadSnaps(m_enemies, true))
		return {};

	if (!r->loadSnaps(m_bullets, true))
		return {};



	if (!r->loadSnaps(m_controls.lights, true))
		return {};

	if (!r->loadSnaps(m_controls.containers, true))
		return {};



	if (!r->loadAuthSnaps(snapshot.players))
		return {};

	if (!r->loadAuthSnaps(snapshot.enemies))
		return {};

	if (!r->loadAuthSnaps(snapshot.bullets))
		return {};


	if (!r->loadAuthSnaps(snapshot.controls.lights))
		return {};

	if (!r->loadAuthSnaps(snapshot.controls.containers))
		return {};



	r->loadSnaps(m_tmpSnapshot.players, false);
	r->loadSnaps(m_tmpSnapshot.enemies, false);
	r->loadSnaps(m_tmpSnapshot.bullets, false);

	r->loadSnaps(m_tmpSnapshot.controls.lights, false);
	r->loadSnaps(m_tmpSnapshot.controls.containers, false);

	return r;

}


/**
 * @brief RpgSnapshotStorage::saveRenderer
 * @param renderer
 * @return
 */

int RpgSnapshotStorage::saveRenderer(Renderer *renderer)
{
	Q_ASSERT(renderer);

	// Ennyivel kell az auth ticket megnövelni (mivel a 0. snap read only, ezért a 2. lesz először carried, így diff-1 (=+1)
	// frame kell majd pluszban

	int diff = 1;

	diff = std::max(diff, renderer->saveObjects(m_players));
	diff = std::max(diff, renderer->saveObjects(m_enemies));
	diff = std::max(diff, renderer->saveObjects(m_bullets));

	diff = std::max(diff, renderer->saveObjects(m_controls.lights));
	diff = std::max(diff, renderer->saveObjects(m_controls.containers));

	const qint64 &t = renderer->startTick()+diff-1;

	for (const auto &p : m_players)
		generateEvents(p, t);

	for (const auto &p : m_enemies)
		generateEvents(p, t);

	for (const auto &p : m_bullets)
		generateEvents(p, t);


	for (const auto &p : m_controls.lights)
		generateEvents(p, t);

	for (const auto &p : m_controls.containers)
		generateEvents(p, t);

	return diff-1;
}





/**
 * @brief RpgSnapshotStorage::copy
 * @param dest
 * @param src
 */

template<typename T, typename T2>
void RpgSnapshotStorage::copy(std::map<qint64, T> &dest, const std::map<qint64, T> &src, const qint64 &firstTick) const
{
	const qint64 ft = firstTick*10;

	for (const auto &ptr : src) {
		if (ptr.first < ft) {
			LOG_CERROR("engine") << "Snapshot out of time:" << ptr.first << "vs." << firstTick << "CURRENT" << m_engine->currentTick();
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
 * @brief RpgSnapshotStorage::removeOutdated
 * @param list
 * @param tick
 */

template<typename T, typename T2, typename T3, typename T4, typename T5>
QList<T2> RpgSnapshotStorage::removeOutdated(RpgGameData::SnapshotList<T, T2> &list, const qint64 &tick)
{
	QList<T2> ret;

	for (auto it = list.begin(); it != list.end(); ) {
		if (it->list.empty())
			continue;

		if (it->list.cend() !=
				std::find_if(it->list.cbegin(),
							 it->list.cend(),
							 [&tick](const auto &ptr) {
							 return ptr.second.stage() == RpgGameData::LifeCycle::StageDestroy &&
							 ptr.second.destroyTick() < tick;
	})) {
			ret.append(it->data);
			it = list.erase(it);
			continue;
		}

		++it;
	}

	return ret;
}



/**
 * @brief RpgSnapshotStorage::removeList
 * @param list
 * @param ids
 */

template<typename T, typename T2, typename T3, typename T4>
void RpgSnapshotStorage::removeList(RpgGameData::SnapshotList<T, T2> &list, const QList<T2> &ids)
{
	if (ids.isEmpty())
		return;

	std::erase_if(list, [&ids](const RpgGameData::SnapshotData<T, T2> &ptr){
		return ids.contains(ptr.data);
	});
}





/**
 * @brief Renderer::Renderer
 * @param start
 * @param size
 */

Renderer::Renderer(const qint64 &start, const int &size)
	: m_startTick(std::max(start, 0LL))
	, m_size(start < 0 ? 1 : size)
	, m_solver(this)
{
	if (size <= 0)
		LOG_CERROR("engine") << "Renderer size <= 0";
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
 * @brief Renderer::findByBase
 * @param baseData
 * @return
 */

RendererObjectType *Renderer::findByBase(const RpgGameData::BaseData &baseData) const
{
	for (const auto &ptr : m_objects) {
		if (RendererObjectType *item = ptr.get(); item->isBaseEqual(baseData))
			return item;
	}

	return nullptr;
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

		// A legutolsó rögzített (storage) snapból indulunk ki

		const auto &ptr = extendFromLast<RpgGameData::Player>(src);

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
			// Ez elvileg nem lehetséges...

			LOG_CERROR("engine") << "Update read only snap" << dst->data().f;
		}

		RpgGameData::Player::PlayerState specialSt = RpgGameData::Player::PlayerInvalid;
		RpgGameData::Weapon::WeaponType cw = RpgGameData::Weapon::WeaponInvalid;

		// Külön eseményt nem okozó állapotok

		static const QList<RpgGameData::Player::PlayerState> normalStates = {
			RpgGameData::Player::PlayerIdle,
			RpgGameData::Player::PlayerMoving,
			RpgGameData::Player::PlayerInvalid,
		};

		for (const RpgGameData::Player &p : dst->m_subData) {

			// Ha már volt egy különleges eset, akkor azt továbbvisszük

			if (dst->m_data.st != p.st && !normalStates.contains(dst->m_data.st) && !normalStates.contains(p.st)) {
				if (!src->carrySubSnap(p)) {
					LOG_CWARNING("engine") << "State conflict" << specialSt << p.st;
				}
			} else if (!normalStates.contains(p.st)) {

				// Ha ez az első különleges eset, akkor feldolgozzuk

				if (p.st == RpgGameData::Player::PlayerHit ||
						p.st == RpgGameData::Player::PlayerShot) {

					m_solver.add(m_current, src, p.arm.cw);

				} else if (p.st == RpgGameData::Player::PlayerAttack) {
					RendererObject<RpgGameData::EnemyBaseData> *tg = findByBase<RpgGameData::EnemyBaseData>(p.tg);

					m_solver.add(m_current, src, tg, p.arm.cw);
				} else if (p.st == RpgGameData::Player::PlayerLockControl) {
					if (RendererObjectType *tg = findByBase(p.tg)) {
						LOG_CWARNING("engine") << "LOCK CONTROL" << tg->asBaseData().id;
						m_solver.addUnique(m_current, src, tg);
					} else {
						LOG_CERROR("engine") << "Invalid control" << p.tg.o << p.tg.s << p.tg.id;
					}

				} else if (p.st == RpgGameData::Player::PlayerUseControl) {
					if (RendererObjectType *tg = findByBase(p.tg)) {
						LOG_CWARNING("engine") << "USE CONTROL" << tg->asBaseData().id;
						if (auto *iface = m_solver.addUnique(m_current, src, tg)) {
							iface->releaseSuccess(m_current, src);
						}
					} else {
						LOG_CERROR("engine") << "Invalid control" << p.tg.o << p.tg.s << p.tg.id;
					}

				} else if (p.st == RpgGameData::Player::PlayerUnlockControl) {
					if (RendererObjectType *tg = findByBase(p.tg)) {
						LOG_CWARNING("engine") << "UNLOCK CONTROL" << tg->asBaseData().id;
						if (auto *iface = m_solver.addUnique(m_current, src, tg)) {
							iface->releaseFailed(m_current, src);
						}
					} else {
						LOG_CERROR("engine") << "Invalid control" << p.tg.o << p.tg.s << p.tg.id;
					}
				}

				// A fegyvercsere nem számít eseménynek

				if (specialSt == RpgGameData::Player::PlayerInvalid) {
					if (p.st == RpgGameData::Player::PlayerWeaponChange) {
						cw = p.arm.cw;
					} else {
						specialSt = p.st;
					}
				} else {
					if (!src->carrySubSnap(p)) {
						LOG_CWARNING("engine") << "State conflict" << specialSt << p.st;
					}
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
 * @brief Renderer::render
 * @param dst
 * @param src
 */

void Renderer::render(RendererItem<RpgGameData::Enemy> *dst, RendererObject<RpgGameData::EnemyBaseData> *src)
{
	Q_ASSERT(dst);
	Q_ASSERT(src);

	if (!dst->hasContent())
		return;

	// Ezzel hasonlítjük össze, hogy változott-e

	const RpgGameData::Enemy original = dst->m_data;

	if (!dst->flags().testFlag(RendererType::ReadOnly)) {

		const auto &ptr = extendFromLast<RpgGameData::Enemy>(src);

		if (!dst->flags().testFlag(RendererType::Storage)) {
			if (ptr) {
				dst->m_data = ptr.value();

				// Nem szabad áthozni pl. lövést vagy más eventet

				if (dst->m_data.st != RpgGameData::Enemy::EnemyMoving)
					dst->m_data.st = RpgGameData::Enemy::EnemyIdle;

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

	const RpgGameData::Enemy saved = dst->m_data;

	if (!dst->m_subData.empty()) {

		if (dst->flags().testFlag(RendererType::ReadOnly)) {
			LOG_CERROR("engine") << "Update read only snap" << dst->data().f;
		}

		RpgGameData::Enemy::EnemyState specialSt = RpgGameData::Enemy::EnemyInvalid;

		// Külön eseményt nem okozó állapotok

		static const QList<RpgGameData::Enemy::EnemyState> normalStates = {
			RpgGameData::Enemy::EnemyIdle,
			RpgGameData::Enemy::EnemyMoving,
			RpgGameData::Enemy::EnemyInvalid,
		};

		for (const RpgGameData::Enemy &p : dst->m_subData) {
			if (dst->m_data.st != p.st && !normalStates.contains(dst->m_data.st) && !normalStates.contains(p.st)) {
				if (!src->carrySubSnap(p)) {
					LOG_CWARNING("engine") << "State conflict" << specialSt << p.st;
				}
			} else if (!normalStates.contains(p.st)) {

				if (p.st == RpgGameData::Enemy::EnemyHit ||
						p.st == RpgGameData::Enemy::EnemyShot) {

					m_solver.add(m_current, src, p.arm.cw);

				} else if (p.st == RpgGameData::Enemy::EnemyAttack) {
					RendererObject<RpgGameData::PlayerBaseData> *tg = findByBase<RpgGameData::PlayerBaseData>(p.tg);

					m_solver.add(m_current, src, tg, p.arm.cw);

				}

				if (specialSt == RpgGameData::Enemy::EnemyInvalid) {
					specialSt = p.st;
				} else {
					if (!src->carrySubSnap(p)) {
						LOG_CWARNING("engine") << "State conflict" << specialSt << p.st;
					}
				}
			}

			dst->m_data = p;
		}

		// Az érzékeny adatokat visszaállítjuk, mindegy, mit kaptunk

		restore(&dst->m_data, saved);


		// Az egyszerű statusokat beállítjuk

		if (specialSt != RpgGameData::Enemy::EnemyInvalid)
			dst->m_data.st = specialSt;


		// Töröljük a sub-okat

		dst->clearSubData();
	}

	if (dst->m_data != original)
		dst->addFlags(RendererType::Modified);
}




/**
 * @brief Renderer::render
 * @param dst
 * @param src
 */

void Renderer::render(RendererItem<RpgGameData::Bullet> *dst, RendererObject<RpgGameData::BulletBaseData> *src)
{
	Q_ASSERT(dst);
	Q_ASSERT(src);


	bool isLiving = true;

	const auto &ptr = extendFromLast<RpgGameData::Bullet>(src);

	if (ptr &&
			(ptr->st == RpgGameData::LifeCycle::StageDead ||
			 ptr->st == RpgGameData::LifeCycle::StageDestroy)) {
		isLiving = false;

		// Create snap for destroy

		if (!dst->hasContent() && ptr->st == RpgGameData::LifeCycle::StageDead) {
			dst->m_data = ptr.value();
			dst->m_data.destroy(m_startTick+m_current);
			dst->clearSubData();
			dst->addFlags(RendererType::Storage | RendererType::Modified);
			return;
		}
	}

	if (!dst->hasContent())
		return;

	const RpgGameData::Bullet original = dst->m_data;

	if (!dst->flags().testFlag(RendererType::ReadOnly)) {
		if (!dst->flags().testFlag(RendererType::Storage)) {
			if (ptr) {
				dst->m_data = ptr.value();
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



	if (!isLiving) {
		if (ptr->st == RpgGameData::LifeCycle::StageDestroy) {
			dst->removeFlags();
			return;
		} else {
			dst->m_data.destroy(m_startTick+m_current);
			dst->clearSubData();
			dst->addFlags(RendererType::Storage | RendererType::Modified);
			return;
		}
	}


	const RpgGameData::Bullet saved = dst->m_data;

	if (!dst->m_subData.empty()) {
		if (dst->flags().testFlag(RendererType::ReadOnly)) {
			LOG_CERROR("engine") << "Update read only snap" << dst->data().f;
		}

		RpgGameData::LifeCycle::Stage specialSt = RpgGameData::LifeCycle::StageInvalid;
		RpgGameData::BaseData target;

		for (const RpgGameData::Bullet &p : dst->m_subData) {
			if ( p.st != RpgGameData::LifeCycle::StageCreate &&
				 p.st != RpgGameData::LifeCycle::StageInvalid) {

				if (!target.isValid() && p.tg.isValid()) {
					RendererObject<RpgGameData::PlayerBaseData> *srcPlayer =
							src->baseData.own == RpgGameData::BulletBaseData::OwnerPlayer && src->baseData.ownId.isValid() ?
								findByBase<RpgGameData::PlayerBaseData>(src->baseData.ownId) : nullptr;

					RendererObject<RpgGameData::EnemyBaseData> *srcEnemy =
							src->baseData.own == RpgGameData::BulletBaseData::OwnerEnemy && src->baseData.ownId.isValid() ?
								findByBase<RpgGameData::EnemyBaseData>(src->baseData.ownId) : nullptr;


					RendererObject<RpgGameData::EnemyBaseData> *tgEnemy =
							src->baseData.tar.testFlag(RpgGameData::BulletBaseData::TargetEnemy) && p.tg.isValid() ?
								findByBase<RpgGameData::EnemyBaseData>(p.tg) : nullptr;

					RendererObject<RpgGameData::PlayerBaseData> *tgPlayer =
							src->baseData.tar.testFlag(RpgGameData::BulletBaseData::TargetPlayer) && p.tg.isValid() ?
								findByBase<RpgGameData::PlayerBaseData>(p.tg) : nullptr;

					// TODO: targetGround

					if (srcEnemy && tgPlayer)
						m_solver.add(m_current, srcEnemy, tgPlayer, src->baseData.t);
					else if (srcPlayer && tgEnemy)
						m_solver.add(m_current, srcPlayer, tgEnemy, src->baseData.t);


					specialSt = RpgGameData::LifeCycle::StageDead;
					target = p.tg;

				} else if (specialSt == RpgGameData::LifeCycle::StageInvalid) {
					specialSt = p.st;
				}

			}

			dst->m_data = p;
		}

		// Az érzékeny adatokat visszaállítjuk, mindegy, mit kaptunk

		restore(&dst->m_data, saved);


		// Az egyszerű statusokat beállítjuk

		if (specialSt != RpgGameData::LifeCycle::StageInvalid) {
			dst->m_data.st = specialSt;
			dst->m_data.tg = target;
		}


		// Töröljük a sub-okat

		dst->clearSubData();
	}


	if (dst->m_data != original)
		dst->addFlags(RendererType::Modified);
}





/**
 * @brief Renderer::render
 * @param dst
 * @param src
 */

void Renderer::render(RendererItem<RpgGameData::ControlLight> *dst, RendererObject<RpgGameData::ControlBaseData> *src)
{
	Q_ASSERT(dst);
	Q_ASSERT(src);

	if (!dst->hasContent())
		return;

	// Ezzel hasonlítjük össze, hogy változott-e

	const RpgGameData::ControlLight original = dst->m_data;

	if (!dst->flags().testFlag(RendererType::ReadOnly)) {

		const auto &ptr = extendFromLast<RpgGameData::ControlLight>(src);

		if (!dst->flags().testFlag(RendererType::Storage)) {
			if (ptr) {
				dst->m_data = ptr.value();
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

	const RpgGameData::ControlLight saved = dst->m_data;

	if (!dst->m_subData.empty()) {

		if (dst->flags().testFlag(RendererType::ReadOnly)) {
			LOG_CERROR("engine") << "Update read only snap" << dst->data().f;
		}

		for (const RpgGameData::ControlLight &p : dst->m_subData)
			dst->m_data = p;

		dst->clearSubData();
	}

	if (dst->m_data != original)
		dst->addFlags(RendererType::Modified);
}



/**
 * @brief Renderer::render
 * @param dst
 * @param src
 */

void Renderer::render(RendererItem<RpgGameData::ControlContainer> *dst, RendererObject<RpgGameData::ControlContainerBaseData> *src)
{
	Q_ASSERT(dst);
	Q_ASSERT(src);

	if (!dst->hasContent())
		return;

	// Ezzel hasonlítjük össze, hogy változott-e

	const RpgGameData::ControlContainer original = dst->m_data;

	if (!dst->flags().testFlag(RendererType::ReadOnly)) {

		const auto &ptr = extendFromLast<RpgGameData::ControlContainer>(src);

		if (!dst->flags().testFlag(RendererType::Storage)) {
			if (ptr) {
				dst->m_data = ptr.value();
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

	const RpgGameData::ControlContainer saved = dst->m_data;

	if (!dst->m_subData.empty()) {

		if (dst->flags().testFlag(RendererType::ReadOnly)) {
			LOG_CERROR("engine") << "Update read only snap" << dst->data().f;
		}

		for (const RpgGameData::ControlContainer &p : dst->m_subData)
			dst->m_data = p;


		// Az érzékeny adatokat visszaállítjuk, mindegy, mit kaptunk

		restore(&dst->m_data, saved);

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

	//dst->tg = data.tg;
	dst->hp = data.hp;
	dst->mhp = data.mhp;

	dst->arm = data.arm;
	dst->arm.cw = cw;
}



/**
 * @brief Renderer::restore
 * @param dst
 * @param data
 */

void Renderer::restore(RpgGameData::Enemy *dst, const RpgGameData::Enemy &data)
{
	Q_ASSERT(dst);

	//dst->tg = data.tg;
	dst->hp = data.hp;
	dst->mhp = data.mhp;
}





/**
 * @brief Renderer::restore
 * @param dst
 * @param data
 */

void Renderer::restore(RpgGameData::Bullet *dst, const RpgGameData::Bullet &data)
{
	Q_ASSERT(dst);

	dst->tg = data.tg;
	dst->st = data.st;
}



/**
 * @brief Renderer::restore
 * @param dst
 * @param data
 */

void Renderer::restore(RpgGameData::ControlLight *dst, const RpgGameData::ControlLight &data)
{
	Q_ASSERT(dst);

	dst->st = data.st;
}


/**
 * @brief Renderer::restore
 * @param dst
 * @param data
 */

void Renderer::restore(RpgGameData::ControlContainer *dst, const RpgGameData::ControlContainer &data)
{
	Q_ASSERT(dst);

	dst->st = data.st;
	dst->lck = data.lck;
	dst->a = data.a;
	dst->u = data.u;
}









/**
 * @brief Renderer::render
 * @return
 */

bool Renderer::render()
{
	for (;;) {

		for (const auto &ptr : m_objects) {
			ptr->render(this);
		}

		m_solver.solve();

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










/**
 * @brief ConflictSolver::add
 * @param args
 */
template<typename T, typename ...Args, typename T3>
T *ConflictSolver::addData(Args &&...args)
{
	m_list.emplace_back(new T(std::forward<Args>(args)...));
	return dynamic_cast<T*>(m_list.back().get());
}



/**
 * @brief ConflictSolver::ConflictSolver
 * @param renderer
 */

ConflictSolver::ConflictSolver(Renderer *renderer)
	: m_renderer(renderer)
{

}



/**
 * @brief ConflictSolver::addUnique
 * @param tick
 * @param src
 * @param dest
 * @return
 */

ConflictSolver::ConflictUniqueIface* ConflictSolver::addUnique(const int &tick, RendererObjectType *src, RendererObjectType *dest)
{
	Q_ASSERT(src);
	Q_ASSERT(dest);

	if (RendererObject<RpgGameData::ControlContainerBaseData> *c = dynamic_cast<RendererObject<RpgGameData::ControlContainerBaseData>*>(dest)) {
		return addUniqueData<ConflictContainer>(tick, c, dynamic_cast<RendererObject<RpgGameData::PlayerBaseData> *>(src));
	} else {
		LOG_CERROR("engine") << "Invalid unique object type";
	}

	return nullptr;
}



/**
 * @brief ConflictSolver::solve
 * @return
 */

bool ConflictSolver::solve()
{
	if (m_list.empty())
		return true;

	bool r = true;

	for (auto &ptr : m_list) {
		if (ptr->solved)
			continue;

		if (!ptr->solve(this))
			r = false;
		else
			ptr->solved = true;
	}

	std::erase_if(m_list, [](const auto &p){
		return p->solved;
	});

	return r;
}




/**
 * @brief ConflictSolver::ConflictWeaponUsage::solveData
 * @param solver
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
bool ConflictSolver::ConflictWeaponUsage<T, T2, T3, T4>::solveData(ConflictSolver *solver)
{
	Q_ASSERT(solver);

	RendererItem<T> *e = solver->m_renderer->get<T>(src);

	if (!e) {
		LOG_CERROR("engine") << "Invalid item" << e << weaponType;
		return false;
	}

	T data = e->data();

	auto it = data.arm.find(weaponType);

	if (it == data.arm.wl.end()) {
		LOG_CERROR("engine") << "Missing weapon" << e << weaponType;
		return false;
	}

	if (it->b == 0) {
		LOG_CERROR("engine") << "No bullet" << e << weaponType;
		return false;
	}


	if (it->b > 0) {
		--(it->b);
	}

	e->setData(data);
	e->addFlags(RendererType::Modified);

	return true;
}






/**
 * @brief ConflictSolver::ConflictDataWeaponUsage::solve
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
bool ConflictSolver::ConflictWeaponUsage<T, T2, T3, T4>::solve(ConflictSolver *solver)
{
	Q_ASSERT(solver);

	if (weaponType == RpgGameData::Weapon::WeaponInvalid) {
		LOG_CERROR("engine") << "Invalid weapon";
		return false;
	}

	if (!src) {
		LOG_CERROR("engine") << "Invalid source" << src;
		return false;
	}

	RendererType *t = src->get();

	if (!t) {
		LOG_CERROR("engine") << "Invalid source item" << src;
		return false;
	}

	if (t->flags() & RendererType::ReadOnly) {
		LOG_CERROR("engine") << "Read only storage item" << t;
		return false;
	}

	if (!(t->flags() & RendererType::Storage)) {
		LOG_CERROR("engine") << "Missing storage flag" << t;
		return false;
	}

	return solveData(solver);
}







/**
 * @brief ConflictSolver::ConflictAttack::solve
 * @param solver
 * @return
 */

template<typename T, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
bool ConflictSolver::ConflictAttack<T, T2, T3, T4, T5, T6, T7, T8>::solve(ConflictSolver *solver)
{
	Q_ASSERT(solver);
	Q_ASSERT(src);


	if (!src) {
		LOG_CERROR("engine") << "Missing source";
		return false;
	}

	if (!dest) {
		LOG_CERROR("engine") << "Missing destination";
		return false;
	}

	RendererItem<T> *p = solver->m_renderer->get<T>(src);
	RendererItem<T2> *other = solver->m_renderer->get<T2>(dest);

	if (!p || !other) {
		LOG_CERROR("engine") << "Invalid data";
		return false;
	}

	if (!(p->flags() & RendererType::Storage)) {
		LOG_CERROR("engine") << "Missing source storage flag" << p;
		return false;
	}

	T2 eData = other->data();

	if (!(other->flags() & RendererType::Storage)) {
		RendererItem<T2> *o = solver->m_renderer->forward<T2>(dest);

		if (o && (o->flags() & RendererType::Storage)) {
			eData = o->data();
		} else {
			LOG_CWARNING("engine") << "Missing snap" << src->baseData.o << src->baseData.id;

			o = solver->m_renderer->rewind<T2>(dest);

			if (!o || !(o->flags() & RendererType::Storage)) {
				LOG_CERROR("engine") << "Snap create error" << src->baseData.o << src->baseData.id;
				return false;
			}

			eData = o->data();
		}
	}

	T pData = p->data();

	if (pData.hp <= 0) {
		LOG_CWARNING("engine") << "Attacker hp <= 0" << p;
		return true;
	}

	eData.attacked(dest->baseData, weaponType, src->baseData);

	if (!other->setData(eData)) {
		LOG_CWARNING("engine") << "Read only snap forced override" << src->baseData.o << src->baseData.id;
		other->dataOverride(eData);
	}
	other->addFlags(RendererType::Modified | RendererType::Storage);

	return true;
}





/**
 * @brief ConflictSolver::ConflictContainer::ConflictContainer
 * @param _tick
 * @param _dst
 * @param _src
 * @param _isLock
 */

ConflictSolver::ConflictContainer::ConflictContainer(const int &_tick,
													 RendererObject<RpgGameData::ControlContainerBaseData> *_dst,
													 RendererObject<RpgGameData::PlayerBaseData> *_src)
	: ConflictDataUnique(_tick, _dst)
{
	Q_ASSERT(_src);
	Q_ASSERT(_dst);


	LOG_CDEBUG("engine") << "CONTAINER SET" << _src->baseData.o;

	player = _src;
	tick = _tick;

}



/**
 * @brief ConflictSolver::ConflictContainer::solve
 * @param solver
 * @return
 */

bool ConflictSolver::ConflictContainer::solve(ConflictSolver *solver)
{
	Q_ASSERT(solver);

	if (!unique) {
		LOG_CERROR("engine") << "Invalid destination" << unique;
		return false;
	}

	RendererItem<RpgGameData::ControlContainer> *e = solver->m_renderer->get<RpgGameData::ControlContainer>(unique);

	if (!e) {
		LOG_CERROR("engine") << "Invalid item" << e;
		return false;
	}

	RpgGameData::ControlContainer data = e->data();

	if (data.st == RpgGameData::ControlContainer::ContainerOpen) {
		LOG_CWARNING("engine") << "Container already opened";
		return false;
	}

	RendererType *t = unique->get();

	if (!t) {
		LOG_CERROR("engine") << "Invalid destination item" << unique;
		return false;
	}

	if (t->flags() & RendererType::ReadOnly) {
		LOG_CERROR("engine") << "Read only storage item" << t;
		return false;
	}


	const State &state = getState();

	// Lockolta a containert

	if (state == StateInvalid) {
		if (player) {
			RendererItem<RpgGameData::Player> *p = solver->m_renderer->get<RpgGameData::Player>(player);

			if (p->data().hp <= 0) {
				LOG_CWARNING("engine") << "Locker hp <= 0" << p;
			} else {

			}

			if (!(t->flags() & RendererType::Storage)) {
				LOG_CERROR("engine") << "Missing storage flag" << t;
			}

			data.st = RpgGameData::ControlContainer::ContainerClose;
			data.a = false;
			data.u = player->baseData;

			LOG_CDEBUG("engine") << "SOLVE LOCK" << this;

		} else {
			data.st = RpgGameData::ControlContainer::ContainerClose;
			data.a = true;
			data.u = {};

			LOG_CDEBUG("engine") << "SOLVE NO PLAYER" << this;
		}
	} else {
		if (player) {
			RendererItem<RpgGameData::Player> *p = solver->m_renderer->get<RpgGameData::Player>(player);

			if (!p || !(p->flags() & RendererType::Storage)) {
				LOG_CWARNING("engine") << "Missing snap" << player->baseData.o << player->baseData.id;

				p = solver->m_renderer->rewind<RpgGameData::Player>(player);

				if (!p || !(p->flags() & RendererType::Storage)) {
					LOG_CERROR("engine") << "Snap create error" << player->baseData.o << player->baseData.id;
					return false;
				}
			}

			RpgGameData::Player pdata = p->data();

			if (state == StateSuccess) {

				LOG_CINFO("engine") << "PLAYER CONTROL SUCCESS";

				data.st = RpgGameData::ControlContainer::ContainerOpen;
				data.a = false;

			} else {

				LOG_CINFO("engine") << "PLAYER CONTROL FAILED";

				pdata.controlFailed(RpgConfig::ControlContainer);
				p->setData(pdata);
				p->addFlags(RendererType::Modified);

				data.st = RpgGameData::ControlContainer::ContainerClose;
				data.a = true;
			}
		}


		data.u = {};

		LOG_CDEBUG("engine") << "SOLVE OPEN" << this << data.f << data.st;
	}


	e->setData(data);
	e->addFlags(RendererType::Modified | RendererType::Storage);

	return true;
}



/**
 * @brief ConflictSolver::ConflictContainer::releaseObjectSuccess
 * @param _tick
 * @param src
 * @param solver
 */

bool ConflictSolver::ConflictContainer::releaseSuccess(const int &_tick, RendererObjectType *src)
{
	Q_ASSERT(src);

	RendererObject<RpgGameData::PlayerBaseData> *p = dynamic_cast<RendererObject<RpgGameData::PlayerBaseData> *>(src);

	if (!p) {
		LOG_CERROR("engine") << "Invalid player data" << src->asBaseData().s << src->asBaseData().o << src->asBaseData().id;
		return false;
	}

	releaseList.emplace_back(p, _tick, StateSuccess);

	return true;
}







/**
 * @brief ConflictSolver::ConflictContainer::releaseObjectFailed
 * @param _tick
 * @param src
 * @param solver
 */

bool ConflictSolver::ConflictContainer::releaseFailed(const int &_tick, RendererObjectType *src)
{
	Q_ASSERT(src);

	RendererObject<RpgGameData::PlayerBaseData> *p = dynamic_cast<RendererObject<RpgGameData::PlayerBaseData> *>(src);

	if (!p) {
		LOG_CERROR("engine") << "Invalid player data" << src->asBaseData().s << src->asBaseData().o << src->asBaseData().id;
		return false;
	}

	releaseList.emplace_back(p, _tick, StateFailed);

	return true;
}



/**
 * @brief ConflictSolver::ConflictContainer::add
 * @param _tick
 * @param src
 */

void ConflictSolver::ConflictContainer::add(const int &_tick, RendererObjectType *src)
{
	Q_ASSERT(src);

	RendererObject<RpgGameData::PlayerBaseData> *p = dynamic_cast<RendererObject<RpgGameData::PlayerBaseData> *>(src);

	if (!p) {
		LOG_CERROR("engine") << "Invalid player data" << src->asBaseData().s << src->asBaseData().o << src->asBaseData().id;
		return;
	}

	if (tick < 0) {
		LOG_CDEBUG("engine") << "CONTAINER UPDATE" << p->baseData.o;

		player = p;
		tick = _tick;
		return;
	}


	if (player && player != p) {
		LOG_CERROR("engine") << "CONTAINER ALREADY USED BY" << player->baseData.o;
	}
}





/**
 * @brief ConflictSolver::ConflictContainer::remove
 * @param _tick
 * @param src
 * @return
 */

bool ConflictSolver::ConflictContainer::remove(const int &_tick, RendererObjectType *src)
{
	Q_ASSERT(src);

	if (tick < 0 || _tick < tick)
		return false;

	RendererObject<RpgGameData::PlayerBaseData> *p = dynamic_cast<RendererObject<RpgGameData::PlayerBaseData> *>(src);

	if (!p) {
		LOG_CERROR("engine") << "Invalid player data" << src->asBaseData().s << src->asBaseData().o << src->asBaseData().id;
		return false;
	}

	if (player != p) {
		LOG_CERROR("engine") << "CONTAINER UNLOCK FAILED BY" << player->baseData.o << "vs" << p->baseData.o;
		return false;
	}

	LOG_CDEBUG("engine") << "CONTAINER FREE" << player->baseData.o;

	player = nullptr;
	tick = -1;

	return true;
}



/**
 * @brief ConflictSolver::ConflictContainer::getState
 * @return
 */

ConflictSolver::ConflictUniqueIface::State ConflictSolver::ConflictContainer::getState()
{
	ReleaseData *ptr = nullptr;

	for (ReleaseData &d : releaseList) {
		if (!ptr || d.tick < ptr->tick)
			ptr = &d;
	}

	if (!ptr)
		return StateInvalid;

	player = ptr->player;
	tick = ptr->tick;

	return ptr->state;
}

