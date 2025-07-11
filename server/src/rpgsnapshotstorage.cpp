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


#define RENDERER_SIZE		15				// renderer "cache" size in frames


/**
 * @brief Renderer::addObjects
 * @param list
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
bool Renderer::addObjects(const RpgGameData::SnapshotList<T, T2> &list)
{
	bool success = true;

	for (const RpgGameData::SnapshotData<T, T2> &d : list) {
		if (findByBase(d.data)) {
			ELOG_ERROR << "Object already exists" << d.data.o << d.data.s << d.data.id;
			success = false;
			continue;
		} else {
			RendererObject<T2> *obj = new RendererObject<T2>(m_logger);
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
		RendererObject<T2> *obj = findByBase<T2>(ptr.data);

		if (!obj) {
			ELOG_DEBUG << "Missing object" << ptr.data.o << ptr.data.s << ptr.data.id;
			continue;
		}


		qint64 tick = m_startTick;

		for (auto &sp : obj->snap) {
			RendererItem<T> *item = dynamic_cast<RendererItem<T>*>(sp.get());
			if (!item) {
				ELOG_ERROR << "Missing snap" << ptr.data.o << ptr.data.s << ptr.data.id << tick;
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
		ELOG_DEBUG << "Missing snap" << ptr.data.o << m_startTick;
		return false;
	}

	if (!dst->setAuthSnap(it->second)) {
		ELOG_DEBUG << "Snap error" << ptr.data.o << m_startTick;
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
			ELOG_DEBUG << "Snap error" << ptr.data.o << m_startTick << index << it->first;
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
	QString txt = Renderer::dumpBaseDataAs(this);

	if (txt.isEmpty())
		return txt;

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
	QString txt = QStringLiteral("[%1] %2 hp c:%3 {pck: %4} %5 ")
				  .arg(data.st)
				  .arg(data.hp)
				  .arg(data.c)
				  .arg(data.pck.id)
				  .arg(data.ft)
				  ;

	if (data.p.size() > 1)
		txt +=  QStringLiteral("(%1, %2) ")
				.arg(data.p.at(0))
				.arg(data.p.at(1))
				;

	if (data.l) {
		txt += QStringLiteral(" LOCKED");
	}

	for (const RpgGameData::InventoryItem &it : data.inv.l) {
		txt += QStringLiteral(" %1 (c:%2)")
			   .arg(it.t)
			   .arg(it.c)
			   ;
	}

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
 * @brief RendererType::dumpAs
 * @param data
 * @param subData
 * @return
 */

QString RendererType::dumpAs(const RpgGameData::ControlCollection &data, const QList<RpgGameData::ControlCollection> &) const
{
	QString txt = QStringLiteral("%1 [%2] @%3")
				  .arg(data.f)
				  .arg(data.a ? '+' : ' ')
				  .arg(data.idx)
				  ;

	if (data.p.size() > 1)
		txt += QStringLiteral(" %1,%2").arg(data.p.at(0)).arg(data.p.at(1));

	if (data.own.isValid())
		txt += QStringLiteral(" (owner %1)").arg(data.own.o);

	if (data.lck)
		txt += QStringLiteral(" LOCKED");

	if (data.u.isValid())
		txt += QStringLiteral("  holder %1").arg(data.u.o);


	return txt;
}


/**
 * @brief RendererType::dumpAs
 * @param data
 * @param subData
 * @return
 */

QString RendererType::dumpAs(const RpgGameData::Pickable &data, const QList<RpgGameData::Pickable> &) const
{
	QString txt = QStringLiteral("%1 [%2 %3]")
				  .arg(data.f)
				  .arg(data.st)
				  .arg(data.a ? '+' : ' ')
				  ;

	if (data.u.isValid())
		txt += QStringLiteral(" holder %1")
			   .arg(data.u.o)
			   ;

	if (data.lck)
		txt += QStringLiteral(" LOCKED");

	if (data.own.isValid())
		txt += QStringLiteral(" owner %1")
			   .arg(data.own.o)
			   ;


	return txt;
}



/**
 * @brief RendererType::dumpAs
 * @param data
 * @param subData
 * @return
 */

QString RendererType::dumpAs(const RpgGameData::ControlGate &data, const QList<RpgGameData::ControlGate> &) const
{
	QString txt = QStringLiteral("%1 [%2] ")
				  .arg(data.f)
				  .arg(data.a ? '+' : ' ')
				  ;

	if (data.st == RpgGameData::ControlGate::GateClose)
		txt += QStringLiteral("closed");
	else if (data.st == RpgGameData::ControlGate::GateDamaged)
		txt += QStringLiteral("--damaged--");
	else
		txt += QStringLiteral("OPENED");

	if (data.lck)
		txt += QStringLiteral(" LOCKED");

	if (data.u.isValid())
		txt += QStringLiteral("  holder %1").arg(data.u.o);


	return txt;
}



/**
 * @brief RendererType::dumpAs
 * @param data
 * @param subData
 * @return
 */

QString RendererType::dumpAs(const RpgGameData::ControlTeleport &data, const QList<RpgGameData::ControlTeleport> &) const
{
	QString txt = QStringLiteral("%1 [%2] ")
				  .arg(data.f)
				  .arg(data.a ? '+' : ' ')
				  ;

	if (data.op)
		txt += QStringLiteral("operating");

	if (data.lck)
		txt += QStringLiteral(" LOCKED");

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
		ELOG_ERROR << "Invalid index" << index << "of size" << snap.size();
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
		RendererObject<T2> *o = findByBase<T2>(ptr.data);
		if (!o) {
			ELOG_DEBUG << "Missing object" << ptr.data.o << ptr.data.s << ptr.data.id;
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

		RendererObject<T2> *o = findByBase<T2>(ptr.data);

		if (!o) {
			ELOG_DEBUG << "Missing object" << ptr.data.o << ptr.data.s << ptr.data.id;
			success = false;
			continue;
		}

		o->overrideAuthSnap(ptr.list.cbegin()->second);
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
	Q_ASSERT(m_engine);
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
 * @brief RpgSnapshotStorage::collectionAdd
 * @param base
 * @param data
 */


void RpgSnapshotStorage::collectionAdd(const RpgGameData::ControlCollectionBaseData &base, const RpgGameData::ControlCollection &data)
{
	RpgGameData::SnapshotData<RpgGameData::ControlCollection, RpgGameData::ControlCollectionBaseData> snapdata;

	snapdata.data = base;
	snapdata.list.insert_or_assign(data.f, data);

	m_controls.collections.push_back(snapdata);
}



/**
 * @brief RpgSnapshotStorage::pickableAdd
 * @param base
 * @param data
 */

RpgGameData::PickableBaseData RpgSnapshotStorage::pickableAdd(const RpgGameData::PickableBaseData::PickableType &type,
															  const int &scene, const QPointF &pos)
{
	RpgGameData::PickableBaseData base;

	base.o = SERVER_OID;
	base.s = scene;
	base.id = m_engine->nextObjectId();
	base.pt = type;
	base.p = QList<float>{(float) pos.x(), (float) pos.y()};

	RpgGameData::SnapshotData<RpgGameData::Pickable, RpgGameData::PickableBaseData> snapdata;

	RpgGameData::Pickable data;
	data.st = RpgGameData::LifeCycle::StageCreate;
	data.sc = scene;
	data.f = 0;
	data.a = true;

	snapdata.data = base;
	snapdata.list.insert_or_assign(data.f, data);

	m_controls.pickables.push_back(snapdata);

	ELOG_WARNING << "PICKABLE ADD" << type << data.f << base.o << base.s << base.id;

	return base;
}



/**
 * @brief RpgSnapshotStorage::gateAdd
 * @param base
 * @param data
 */

void RpgSnapshotStorage::gateAdd(const RpgGameData::ControlGateBaseData &base, const RpgGameData::ControlGate &data)
{
	RpgGameData::SnapshotData<RpgGameData::ControlGate, RpgGameData::ControlGateBaseData> snapdata;

	snapdata.data = base;
	snapdata.list.insert_or_assign(data.f, data);

	m_controls.gates.push_back(snapdata);
}


/**
 * @brief RpgSnapshotStorage::teleportAdd
 * @param base
 * @param data
 */

void RpgSnapshotStorage::teleportAdd(const RpgGameData::ControlTeleportBaseData &base, const RpgGameData::ControlTeleport &data)
{
	RpgGameData::SnapshotData<RpgGameData::ControlTeleport, RpgGameData::ControlTeleportBaseData> snapdata;

	snapdata.data = base;
	snapdata.list.insert_or_assign(data.f, data);

	m_controls.teleports.push_back(snapdata);
}



/**
 * @brief RpgSnapshotStorage::lastLifeCycleId
 * @param base
 * @return
 */

int RpgSnapshotStorage::lastLifeCycleId(const RpgGameData::BaseData &base, std::vector<RpgGameData::BaseData>::iterator *ptr)
{
	return lastLifeCycleId(base.o, ptr);
}



/**
 * @brief RpgSnapshotStorage::lastLifeCycleId
 * @param owner
 * @param ptr
 * @return
 */

int RpgSnapshotStorage::lastLifeCycleId(const int &owner, std::vector<RpgGameData::BaseData>::iterator *ptr)
{
	const std::vector<RpgGameData::BaseData>::iterator &it = std::find_if(m_lastLifeCycleId.begin(), m_lastLifeCycleId.end(),
																		  [&owner](const RpgGameData::BaseData &d){
		return d.o == owner;
	});

	if (ptr)
		*ptr = it;

	if (it != m_lastLifeCycleId.end())
		return it->id;
	else
		return -1;
}



/**
 * @brief RpgSnapshotStorage::setLastLifeCycleId
 * @param base
 * @param id
 * @return
 */

bool RpgSnapshotStorage::setLastLifeCycleId(const RpgGameData::BaseData &base)
{
	const std::vector<RpgGameData::BaseData>::iterator &it = std::find_if(m_lastLifeCycleId.begin(), m_lastLifeCycleId.end(),
																		  [&base](const RpgGameData::BaseData &d){
		return d.o == base.o;
	});

	return setLastLifeCycleId(it, base);
}




/**
 * @brief RpgSnapshotStorage::setLastLifeCycleId
 * @param iterator
 * @param base
 * @return
 */

bool RpgSnapshotStorage::setLastLifeCycleId(const std::vector<RpgGameData::BaseData>::iterator &iterator,
											const RpgGameData::BaseData &base)
{
	if (iterator != m_lastLifeCycleId.end()) {
		if (iterator->id >= base.id) {
			ELOG_ERROR << "LifeCycle id error" << iterator->id << ">=" << base.id;
			return false;
		}
		iterator->id = base.id;
	} else {
		m_lastLifeCycleId.emplace_back(base.o, 0, base.id);
	}

	return true;
}




/**
 * @brief RpgSnapshotStorage::bulletAdd
 * @param base
 * @param data
 */

bool RpgSnapshotStorage::bulletAdd(const RpgGameData::BulletBaseData &base, const RpgGameData::Bullet &data)
{
	std::vector<RpgGameData::BaseData>::iterator iterator;
	const int lastId = lastLifeCycleId(base, &iterator);

	if (lastId != -1 && lastId >= base.id) {
		ELOG_WARNING << "Bullet already exists" << base.o << base.s << base.id;
		return false;
	}

	RpgGameData::SnapshotData<RpgGameData::Bullet, RpgGameData::BulletBaseData> snapdata;

	snapdata.data = base;
	snapdata.list.insert_or_assign(data.f, data);

	m_bullets.push_back(snapdata);

	if (!setLastLifeCycleId(iterator, base)) {
		ELOG_ERROR << "Bullet create failed" << base.o << base.s << base.id;
		return false;
	}

	ELOG_DEBUG << "Create bullet" << base.o << base.s << base.id << "owner" << base.own << "id:" << base.ownId.o;
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

	QString txt = QStringLiteral("RENDER %1 / %2\n---------------------------------------------\n").arg(tick).arg(m_engine->m_deadlineTick);

	int diff = 0;

	if (!renderer) {
		ELOG_ERROR << "INVALID RENDERER" << tick << m_lastAuthTick;
		return {};
	}


	QElapsedTimer t1;
	t1.start();

	renderer->render();
	saveRenderer(renderer.get(), 0);				// pass 0
	diff = saveRenderer(renderer.get(), 1);			// pass 1

	m_engine->renderTimerLog(t1.elapsed());


	for (const auto &ptr : m_engine->m_player) {
		const RpgGameData::CharacterSelect &c = ptr->config();

		txt.append(QStringLiteral("Player %1: %2 (%3) %4 XP, %5 curr\n")
				   .arg(ptr->playerId())
				   .arg(c.username)
				   .arg(c.finished)
				   .arg(c.xp)
				   .arg(c.cur)
				   );
	}

	txt.append(QStringLiteral("---------------------------------------\n\n"));

	txt.append(renderer->dump());

	txt.append(QStringLiteral("---------------------------------------\n\n"));


	m_lastAuthTick = tick + diff - RENDERER_SIZE;

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
	removeList(m_tmpSnapshot.controls.pickables, removeOutdated(m_controls.pickables, m_lastAuthTick-120));

	removeLessThan(m_tmpSnapshot.players, m_lastAuthTick-5);
	removeLessThan(m_tmpSnapshot.enemies, m_lastAuthTick-5);
	removeLessThan(m_tmpSnapshot.bullets, m_lastAuthTick-5);

	removeLessThan(m_tmpSnapshot.controls.lights, m_lastAuthTick-5);
	removeLessThan(m_tmpSnapshot.controls.containers, m_lastAuthTick-5);
	removeLessThan(m_tmpSnapshot.controls.collections, m_lastAuthTick-5);
	removeLessThan(m_tmpSnapshot.controls.pickables, m_lastAuthTick-5);
	removeLessThan(m_tmpSnapshot.controls.gates, m_lastAuthTick-5);
	removeLessThan(m_tmpSnapshot.controls.teleports, m_lastAuthTick-5);


	zapSnapshots(m_lastAuthTick-5);


#ifdef WITH_FTXUI
	QCborMap map;
	map.insert(QStringLiteral("mode"), QStringLiteral("SND"));
	map.insert(QStringLiteral("txt"), txt);
	m_engine->service()->writeToSocket(map.toCborValue());

#endif


	m_engine->checkPlayersCompleted();
}




/**
 * @brief RpgSnapshotStorage::_logger
 * @return
 */

Logger *RpgSnapshotStorage::_logger() const
{
	return m_engine->_logger();
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
		ELOG_WARNING << "Empty snapshot received";
		return false;
	}


	RpgGameData::PlayerBaseData pdata = *player;

	const auto &dstIt = RpgGameData::CurrentSnapshot::find(m_tmpSnapshot.players, pdata);
	const auto &srcIt = RpgGameData::CurrentSnapshot::find(snapshot.players, pdata);

	if (srcIt == snapshot.players.cend()) {
		//ELOG_ERROR << "Invalid player" << cbor;
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
			ELOG_WARNING << "Invalid bullet" << e.data.o << e.data.s << e.data.id;
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

	std::unique_ptr<Renderer> r = std::make_unique<Renderer>(m_lastAuthTick, tick-m_lastAuthTick+1, _logger());



	if (!r->addObjects(m_players))
		return {};

	if (!r->addObjects(m_enemies))
		return {};

	if (!r->addObjects(m_bullets))
		return {};



	if (!r->addObjects(m_controls.lights))
		return {};

	if (!r->addObjects(m_controls.containers))
		return {};

	if (!r->addObjects(m_controls.collections))
		return {};

	if (!r->addObjects(m_controls.pickables))
		return {};

	if (!r->addObjects(m_controls.gates))
		return {};

	if (!r->addObjects(m_controls.teleports))
		return {};


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

	if (!r->loadSnaps(m_controls.collections, true))
		return {};

	if (!r->loadSnaps(m_controls.pickables, true))
		return {};

	if (!r->loadSnaps(m_controls.gates, true))
		return {};

	if (!r->loadSnaps(m_controls.teleports, true))
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

	if (!r->loadAuthSnaps(snapshot.controls.collections))
		return {};

	if (!r->loadAuthSnaps(snapshot.controls.pickables))
		return {};

	if (!r->loadAuthSnaps(snapshot.controls.gates))
		return {};

	if (!r->loadAuthSnaps(snapshot.controls.teleports))
		return {};



	r->loadSnaps(m_tmpSnapshot.players, false);
	r->loadSnaps(m_tmpSnapshot.enemies, false);
	r->loadSnaps(m_tmpSnapshot.bullets, false);

	r->loadSnaps(m_tmpSnapshot.controls.lights, false);
	r->loadSnaps(m_tmpSnapshot.controls.containers, false);
	r->loadSnaps(m_tmpSnapshot.controls.collections, false);
	r->loadSnaps(m_tmpSnapshot.controls.pickables, false);
	r->loadSnaps(m_tmpSnapshot.controls.gates, false);
	r->loadSnaps(m_tmpSnapshot.controls.teleports, false);

	return r;

}


/**
 * @brief RpgSnapshotStorage::saveRenderer
 * @param renderer
 * @return
 */

int RpgSnapshotStorage::saveRenderer(Renderer *renderer, const uint &pass)
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
	diff = std::max(diff, renderer->saveObjects(m_controls.collections));
	diff = std::max(diff, renderer->saveObjects(m_controls.pickables));
	diff = std::max(diff, renderer->saveObjects(m_controls.gates));
	diff = std::max(diff, renderer->saveObjects(m_controls.teleports));


	// 0. lépés - elvégezzük az ütközéseket

	if (pass == 0) {
		renderer->generateSolverEvents(m_engine, diff);
		return -1;
	}

	// 1. lépés - végleges render



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

	for (const auto &p : m_controls.collections)
		generateEvents(p, t);

	for (const auto &p : m_controls.pickables)
		generateEvents(p, t);

	for (const auto &p : m_controls.gates)
		generateEvents(p, t);

	for (const auto &p : m_controls.teleports)
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
			ELOG_ERROR << "Snapshot out of time:" << ptr.first << "vs." << firstTick << "CURRENT" << m_engine->currentTick();
			continue;
		}

		qint64 f = ptr.first;

		const qint64 next = 1+(qint64)(f/10);
		bool ready = true;

		while (dest.contains(f)) {
			++f;

			if (f >= next) {
				ELOG_ERROR << "Snapshot storage full:" << ptr.first;
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

Renderer::Renderer(const qint64 &start, const int &size, Logger *logger)
	: m_startTick(std::max(start, 0LL))
	, m_size(start < 0 ? 1 : size)
	, m_solver(this)
	, m_logger(logger)
{
	if (size <= 0)
		ELOG_ERROR << "Renderer size <= 0";
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
 * @brief Renderer::dumpBaseDataAs
 * @param obj
 * @return
 */

QString Renderer::dumpBaseDataAs(const RendererObject<RpgGameData::PlayerBaseData> *obj)
{
	Q_ASSERT(obj);

	QString txt = QStringLiteral("[Player %1] - rq: %2")
				  .arg(obj->baseData.o)
				  .arg(obj->baseData.rq)
				  ;

	txt += QStringLiteral("\n-------------------------------------------\n");
	return txt;
}



/**
 * @brief Renderer::dumpBaseDataAs
 * @param obj
 * @return
 */

QString Renderer::dumpBaseDataAs(const RendererObject<RpgGameData::PickableBaseData> *obj)
{
	Q_ASSERT(obj);

	QString txt = QStringLiteral("Pickable %1 %2 %3  [%4]")
				  .arg(obj->baseData.o)
				  .arg(obj->baseData.s)
				  .arg(obj->baseData.id)
				  .arg(obj->baseData.pt)
				  ;


	if (obj->baseData.p.size() > 1)
		QString txt = QStringLiteral(" @(%1,%2)")
					  .arg(obj->baseData.p.at(0))
					  .arg(obj->baseData.p.at(1))
					  ;

	txt += QStringLiteral("\n-------------------------------------------\n");
	return txt;
}




/**
 * @brief Renderer::dumpBaseDataAs
 * @param obj
 * @return
 */

QString Renderer::dumpBaseDataAs(const RendererObject<RpgGameData::ControlTeleportBaseData> *obj)
{
	Q_ASSERT(obj);

	QString txt = QStringLiteral("Teleport %1 %2 %3  ")
				  .arg(obj->baseData.o)
				  .arg(obj->baseData.s)
				  .arg(obj->baseData.id)
				  ;

	if (obj->baseData.hd)
		txt += QStringLiteral(" HIDEOUT");

	txt += QStringLiteral(" ->@(%1,%2) %3")
		   .arg(obj->baseData.x)
		   .arg(obj->baseData.y)
		   .arg(obj->baseData.a)
		   ;

	txt += QStringLiteral("\n-------------------------------------------\n");
	return txt;
}


/**
 * @brief Renderer::dumpBaseDataAs
 * @param obj
 * @return
 */

QString Renderer::dumpBaseDataAs(const RendererObject<RpgGameData::ControlCollectionBaseData> *obj)
{
	Q_ASSERT(obj);

	QString txt = QStringLiteral("Collection %1 %2 %3 (img: %4) ")
				  .arg(obj->baseData.o)
				  .arg(obj->baseData.s)
				  .arg(obj->baseData.id)
				  .arg(obj->baseData.img)
				  ;

	txt += QStringLiteral("\n-------------------------------------------\n");
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
				ELOG_ERROR << "Extend failed" << src->baseData.o;
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

			ELOG_ERROR << "Update read only snap" << dst->data().f;
		}

		RpgGameData::Player::PlayerState specialSt = RpgGameData::Player::PlayerInvalid;
		RpgGameData::Weapon::WeaponType cw = RpgGameData::Weapon::WeaponInvalid;
		std::optional<RpgGameData::BaseData> clearPck;

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
					ELOG_WARNING << "State conflict" << specialSt << p.st;
				}
			} else if (!normalStates.contains(p.st)) {

				if (p.pck.isValid() && p.st != RpgGameData::Player::PlayerExit) {
					ELOG_WARNING << "Player in hideout, only Exit supported";
					continue;
				}

				// Ha ez az első különleges eset, akkor feldolgozzuk

				if (p.st == RpgGameData::Player::PlayerHit ||
						p.st == RpgGameData::Player::PlayerShot) {

					m_solver.add(m_current, src, p.arm.cw);

				} else if (p.st == RpgGameData::Player::PlayerAttack) {
					RendererObject<RpgGameData::EnemyBaseData> *tg = findByBase<RpgGameData::EnemyBaseData>(p.tg);

					m_solver.add(m_current, src, tg, p.arm.cw);
				} else if (p.st == RpgGameData::Player::PlayerLockControl) {
					if (RendererObjectType *tg = findByBase(p.tg)) {
						m_solver.addUnique(m_current, src, tg);
					} else {
						ELOG_ERROR << "Invalid control" << p.tg.o << p.tg.s << p.tg.id;
					}

				} else if (p.st == RpgGameData::Player::PlayerUseControl) {
					if (RendererObjectType *tg = findByBase(p.tg)) {
						if (auto *iface = m_solver.addUnique(m_current, src, tg)) {
							iface->releaseSuccess(m_current, src, p.x);
						}
					} else {
						ELOG_ERROR << "Invalid control" << p.tg.o << p.tg.s << p.tg.id;
					}

				} else if (p.st == RpgGameData::Player::PlayerUnlockControl) {
					if (RendererObjectType *tg = findByBase(p.tg)) {
						if (auto *iface = m_solver.addUnique(m_current, src, tg)) {
							iface->releaseFailed(m_current, src);
						}
					} else {
						ELOG_ERROR << "Invalid control" << p.tg.o << p.tg.s << p.tg.id;
					}
				}

				// A fegyvercsere, kilépés nem számít eseménynek

				if (specialSt == RpgGameData::Player::PlayerInvalid) {
					if (p.st == RpgGameData::Player::PlayerWeaponChange) {
						cw = p.arm.cw;
					} else if (p.st == RpgGameData::Player::PlayerExit) {
						clearPck = dst->m_data.pck;
					} else {
						specialSt = p.st;
					}
				} else {
					if (!src->carrySubSnap(p)) {
						ELOG_WARNING << "State conflict" << specialSt << p.st;
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

		if (clearPck.has_value()) {
			if (RendererObject<RpgGameData::ControlTeleportBaseData> *pck = findByBase<RpgGameData::ControlTeleportBaseData>(clearPck.value())) {
				m_solver.add(m_current, src, dst->m_data.sc, QPointF(pck->baseData.x, pck->baseData.y), pck->baseData.a);
			}

			dst->m_data.pck = {};
			dst->m_data.st = RpgGameData::Player::PlayerExit;
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
				ELOG_ERROR << "Extend failed" << src->baseData.o;
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
			ELOG_ERROR << "Update read only snap" << dst->data().f;
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
					ELOG_WARNING << "State conflict" << specialSt << p.st;
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
						ELOG_WARNING << "State conflict" << specialSt << p.st;
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
				ELOG_ERROR << "Extend failed" << src->baseData.o;
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
			ELOG_ERROR << "Update read only snap" << dst->data().f;
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

void Renderer::render(RendererItem<RpgGameData::Pickable> *dst, RendererObject<RpgGameData::PickableBaseData> *src)
{
	bool isLiving = true;

	const auto &ptr = extendFromLast<RpgGameData::Pickable>(src);

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
	dst->c = data.c;

	dst->arm = data.arm;
	dst->arm.cw = cw;

	dst->pck = data.pck;
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
 * @brief Renderer::restore
 * @param dst
 * @param data
 */

void Renderer::restore(RpgGameData::ControlCollection *dst, const RpgGameData::ControlCollection &data)
{
	Q_ASSERT(dst);

	dst->idx = data.idx;
	dst->p = data.p;
	dst->lck = data.lck;
	dst->a = data.a;
	dst->u = data.u;
	dst->own = data.own;
}


/**
 * @brief Renderer::restore
 * @param dst
 * @param data
 */

void Renderer::restore(RpgGameData::Pickable *dst, const RpgGameData::Pickable &data)
{
	Q_ASSERT(dst);

	dst->st = data.st;
	dst->lck = data.lck;
	dst->a = data.a;
	dst->u = data.u;
	dst->own = data.own;
}



/**
 * @brief Renderer::restore
 * @param dst
 * @param data
 */

void Renderer::restore(RpgGameData::ControlGate *dst, const RpgGameData::ControlGate &data)
{
	Q_ASSERT(dst);

	dst->st = data.st;
	dst->lck = data.lck;
	dst->a = data.a;
	dst->u = data.u;
}



/**
 * @brief Renderer::restore
 * @param dst
 * @param data
 */

void Renderer::restore(RpgGameData::ControlTeleport *dst, const RpgGameData::ControlTeleport &data)
{
	Q_ASSERT(dst);

	dst->lck = data.lck;
	dst->a = data.a;
	dst->u = data.u;
	dst->op = data.op;
}





/**
 * @brief Renderer::postRender
 * @param object
 */

void Renderer::postRender(RendererObject<RpgGameData::ControlCollectionBaseData> *object)
{
	bool clear = false;
	bool hasOwner = false;

	for (const auto &ptr : object->snap) {
		if (clear) {
			if (RendererItem<RpgGameData::ControlCollection> *item = dynamic_cast<RendererItem<RpgGameData::ControlCollection>*>(ptr.get())) {
				if (!hasOwner && item->data().own.isValid()) {
					ELOG_DEBUG << "HAS OWNER";
					hasOwner = true;
					continue;
				}
			}

			ptr->removeFlags();
			continue;
		}

		// Azért csak utána töröljük, hogy az első még megmaradjon

		if (!ptr->flags().testFlag(RendererType::ReadOnly) &&
				!ptr->flags().testFlags(RendererType::FromAuth))
			clear = true;
	}
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

	for (const auto &ptr : m_objects) {
		ptr->postRender(this);
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
	} else if (RendererObject<RpgGameData::ControlCollectionBaseData> *c = dynamic_cast<RendererObject<RpgGameData::ControlCollectionBaseData>*>(dest)) {
		return addUniqueData<ConflictCollection>(tick, c, dynamic_cast<RendererObject<RpgGameData::PlayerBaseData> *>(src));
	} else if (RendererObject<RpgGameData::PickableBaseData> *c = dynamic_cast<RendererObject<RpgGameData::PickableBaseData>*>(dest)) {
		return addUniqueData<ConflictPickable>(tick, c, dynamic_cast<RendererObject<RpgGameData::PlayerBaseData> *>(src));
	} else if (RendererObject<RpgGameData::ControlGateBaseData> *c = dynamic_cast<RendererObject<RpgGameData::ControlGateBaseData>*>(dest)) {
		return addUniqueData<ConflictGate>(tick, c, dynamic_cast<RendererObject<RpgGameData::PlayerBaseData> *>(src));
	} else if (RendererObject<RpgGameData::ControlTeleportBaseData> *c = dynamic_cast<RendererObject<RpgGameData::ControlTeleportBaseData>*>(dest)) {
		return addUniqueData<ConflictTeleport>(tick, c, dynamic_cast<RendererObject<RpgGameData::PlayerBaseData> *>(src));
	}else {
		ELOG_ERROR << "Invalid unique object type";
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

	/*std::erase_if(m_list, [](const auto &p){
		return p->solved;
	});*/

	return r;
}




/**
 * @brief ConflictSolver::generateEvents
 * @param engine
 */

void ConflictSolver::generateEvents(RpgEngine *engine, const int &tick)
{
	for (auto &ptr : m_list) {
		if (ptr->tick <= tick) {
			ptr->generateEvent(this, engine);
		}
	}
}



/**
 * @brief ConflictSolver::_logger
 * @return
 */

Logger *ConflictSolver::_logger() const
{
	return m_renderer->_logger();
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
		SLOG_ERROR(solver) << "Invalid item" << e << weaponType;
		return false;
	}

	T data = e->data();


	if (!e->flags().testFlag(RendererType::Storage)) {
		const auto &ptr = solver->m_renderer->extendFromLast<T>(src);

		if (!ptr) {
			SLOG_ERROR(solver) << "Extend failed";
			return false;
		}

		data = ptr.value();
	}


	auto it = data.arm.find(weaponType);

	if (it == data.arm.wl.end()) {
		SLOG_ERROR(solver) << "Missing weapon" << e << weaponType;
		return false;
	}

	if (it->b == 0) {
		SLOG_ERROR(solver) << "No bullet" << e << weaponType;
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
		SLOG_ERROR(solver) << "Invalid weapon";
		return false;
	}

	if (!src) {
		SLOG_ERROR(solver) << "Invalid source" << src;
		return false;
	}

	RendererType *t = src->get();

	if (!t) {
		SLOG_ERROR(solver) << "Invalid source item" << src;
		return false;
	}

	if (t->flags() & RendererType::ReadOnly) {
		SLOG_ERROR(solver) << "Read only storage item" << t;
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
		SLOG_ERROR(solver) << "Missing source";
		return false;
	}

	if (!dest) {
		SLOG_ERROR(solver) << "Missing destination";
		return false;
	}

	RendererItem<T> *p = solver->m_renderer->get<T>(src);
	RendererItem<T2> *other = solver->m_renderer->get<T2>(dest);

	if (!p || !other) {
		SLOG_ERROR(solver) << "Invalid data";
		return false;
	}

	T pData = p->data();

	if (!p->flags().testFlag(RendererType::Storage)) {
		const auto &ptr = solver->m_renderer->extendFromLast<T>(src);

		if (!ptr) {
			SLOG_ERROR(solver) << "Extend failed";
			return false;
		}

		pData = ptr.value();
	}


	T2 eData = other->data();

	if (!(other->flags() & RendererType::Storage)) {
		RendererItem<T2> *o = solver->m_renderer->forward<T2>(dest);

		if (o && (o->flags() & RendererType::Storage)) {
			eData = o->data();
		} else {
			SLOG_WARNING(solver) << "Missing snap" << src->baseData.o << src->baseData.id;

			o = solver->m_renderer->rewind<T2>(dest);

			if (!o || !(o->flags() & RendererType::Storage)) {
				SLOG_ERROR(solver) << "Snap create error" << src->baseData.o << src->baseData.id;
				return false;
			}

			eData = o->data();
		}
	}


	if (pData.hp <= 0) {
		SLOG_WARNING(solver) << "Attacker hp <= 0" << p;
		return true;
	}

	eData.attacked(dest->baseData, weaponType, src->baseData);

	if (!other->setData(eData)) {
		SLOG_WARNING(solver) << "Read only snap forced override" << src->baseData.o << src->baseData.id;
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
	: ConflictDataUnique(_tick, _dst, _src)
{
	Q_ASSERT(_src);
	Q_ASSERT(_dst);

	SLOG_DEBUG(_dst) << "CONTAINER SET" << _src->baseData.o << "->" << _dst->baseData.o << _dst->baseData.s << _dst->baseData.id;
}




/**
 * @brief ConflictSolver::ConflictContainer::generateEvent
 * @param solver
 * @param engine
 */

void ConflictSolver::ConflictContainer::generateEvent(ConflictSolver *solver, RpgEngine *engine)
{
	Q_ASSERT(solver);
	Q_ASSERT(m_unique);

	SLOG_ERROR(solver) << "!!!! CREATE CONTAINER EVENT";

	const ReleaseState &state = renderCurrentState(solver->m_renderer, tick);

	if (state.state == StateInvalid)
		return;

	RendererItem<RpgGameData::ControlContainer> *e = solver->m_renderer->snapAt<RpgGameData::ControlContainer>(m_unique, 0);

	if (!e) {
		SLOG_ERROR(solver) << "Invalid item" << e;
		return;
	}

	RpgGameData::ControlContainer data = e->data();

	if (state.state == StateSuccess) {
		data.st = RpgGameData::ControlContainer::ContainerOpen;
		data.a = false;
		data.u = {};
	} else if (state.state == StateFailed) {
		data.st = RpgGameData::ControlContainer::ContainerClose;
		data.a = true;
		data.u = {};
	} else if (state.state == StateHold && state.player) {
		data.a = false;
		data.u = state.player->asBaseData();
	}

	m_unique->overrideAuthSnap(data);

	if (state.state == StateSuccess || state.state == StateFailed) {
		RpgGameData::PlayerBaseData pd;

		if (state.player)
			pd = state.player->baseData;

		engine->playerAddXp(pd, state.xp);

		engine->addContainerUsed(solver->m_renderer->startTick()+tick, m_unique->baseData, pd,
								 state.state == StateSuccess);
	}
}



/**
 * @brief ConflictSolver::ConflictContainer::getInitialState
 * @param destPtr
 * @return
 */

bool ConflictSolver::ConflictContainer::getInitialState(Renderer *renderer, ReleaseState *destPtr) const
{
	Q_ASSERT(renderer);

	const auto *snap = renderer->snapAt<RpgGameData::ControlContainer>(m_unique, 0);

	Q_ASSERT(snap);

	const auto &data = snap->data();

	if (data.st == RpgGameData::ControlContainer::ContainerOpen) {
		SLOG_DEBUG(renderer) << "---- container already opened";
		return false;
	}

	if (data.u.isValid()) {
		setPlayer(renderer, data.u, destPtr);
	}

	return true;
}






/**
 * @brief ConflictSolver::ConflictUniqueIface::releaseSuccess
 * @param tick
 * @param src
 * @return
 */

bool ConflictSolver::ConflictUniqueIface::releaseSuccess(const int &tick, RendererObject<RpgGameData::PlayerBaseData> *src, const int &xp)
{
	Q_ASSERT(src);

	RendererObject<RpgGameData::PlayerBaseData> *p = dynamic_cast<RendererObject<RpgGameData::PlayerBaseData> *>(src);

	if (!p) {
		SLOG_ERROR(src) << "Invalid player data" << src->asBaseData().s << src->asBaseData().o << src->asBaseData().id;
		return false;
	}


	m_release.insert(tick, ReleaseState(p, StateSuccess, xp));

	return true;
}





/**
 * @brief ConflictSolver::ConflictUniqueIface::releaseFailed
 * @param tick
 * @param src
 * @return
 */

bool ConflictSolver::ConflictUniqueIface::releaseFailed(const int &tick, RendererObject<RpgGameData::PlayerBaseData> *src)
{
	Q_ASSERT(src);

	RendererObject<RpgGameData::PlayerBaseData> *p = dynamic_cast<RendererObject<RpgGameData::PlayerBaseData> *>(src);

	if (!p) {
		SLOG_ERROR(src) << "Invalid player data" << src->asBaseData().s << src->asBaseData().o << src->asBaseData().id;
		return false;
	}

	m_release.insert(tick, ReleaseState(p, StateFailed, 0));

	return true;
}



/**
 * @brief ConflictSolver::ConflictUniqueIface::setPlayer
 * @param renderer
 * @param base
 * @param destPtr
 * @return
 */

bool ConflictSolver::ConflictUniqueIface::setPlayer(Renderer *renderer, const RpgGameData::BaseData &base, ReleaseState *destPtr) const
{
	Q_ASSERT(renderer);

	RendererObject<RpgGameData::PlayerBaseData> *player = renderer->findByBase<RpgGameData::PlayerBaseData>(base);

	if (player) {
		if (auto *ptr = renderer->snapAt<RpgGameData::Player>(player, 0); ptr && ptr->data().hp <= 0) {
			SLOG_DEBUG(renderer) << "Player hp <= 0, remove from render";

			if (destPtr) {
				destPtr->player = nullptr;
				destPtr->state = StateInvalid;
			}

			return false;
		}
	}

	if (destPtr) {
		destPtr->player = player;
		destPtr->state = StateHold;
	}

	return player ? true : false;
}



/**
 * @brief ConflictSolver::ConflictUniqueIface::renderCurrentState
 * @return
 */

ConflictSolver::ConflictUniqueIface::ReleaseState ConflictSolver::ConflictUniqueIface::renderCurrentState(Renderer *renderer,
																										  const int &maxTick) const
{
	Q_ASSERT(renderer);

	ReleaseState st;

	if (!getInitialState(renderer, &st)) {
		SLOG_DEBUG(renderer) << "Initial state finished, render disabled";
		return {};
	}

	SLOG_INFO(renderer) << "---- INITIAL" << st.state << st.player << (st.player ? st.player->baseData.o : -1);

	for (const auto &[t, rs] : m_release.asKeyValueRange()) {
		SLOG_DEBUG(renderer) << "---- CHECK" << t << maxTick << rs.state << rs.player << (rs.player ? rs.player->baseData.o : -1);

		if (t > maxTick)
			break;

		if (!rs.player)
			continue;

		if (rs.state == StateHold) {
			if (!st.player) {
				st = rs;
				SLOG_INFO(renderer) << "---- SET" << t << maxTick << rs.state << rs.player << (rs.player ? rs.player->baseData.o : -1);
			}
		} else if (rs.state == StateSuccess || rs.state == StateFailed) {
			if (st.state == StateInvalid || st.player == rs.player) {
				st = rs;
				SLOG_INFO(renderer) << "---- SET" << t << maxTick << rs.state << rs.player << (rs.player ? rs.player->baseData.o : -1);
				break;
			}
		}
	}

	return st;
}








/**
 * @brief ConflictSolver::ConflictContainer::remove
 * @param _tick
 * @param src
 * @return
 */

ConflictSolver::ConflictUniqueIface::ConflictUniqueIface(const int &tick,
														 RendererObject<RpgGameData::PlayerBaseData> *player)
{
	m_release.insert(tick, ReleaseState(player, StateHold, 0));
}




/**
 * @brief ConflictSolver::ConflictUniqueIface::add
 * @param tick
 * @param src
 */

void ConflictSolver::ConflictUniqueIface::add(const int &tick, RendererObject<RpgGameData::PlayerBaseData> *src)
{
	Q_ASSERT(src);

	RendererObject<RpgGameData::PlayerBaseData> *p = dynamic_cast<RendererObject<RpgGameData::PlayerBaseData> *>(src);

	if (!p) {
		SLOG_ERROR(src) << "Invalid player data" << src->asBaseData().s << src->asBaseData().o << src->asBaseData().id;
		return;
	}

	m_release.insert(tick, ReleaseState(p, StateHold, 0));
	updateTick(tick);
}









/**
 * @brief ConflictSolver::ConflictCollection::ConflictCollection
 * @param _tick
 * @param _dst
 * @param _src
 */

ConflictSolver::ConflictCollection::ConflictCollection(const int &_tick,
													   RendererObject<RpgGameData::ControlCollectionBaseData> *_dst,
													   RendererObject<RpgGameData::PlayerBaseData> *_src)
	: ConflictDataUnique(_tick, _dst, _src)
{
	Q_ASSERT(_src);
	Q_ASSERT(_dst);

	SLOG_DEBUG(_dst) << "COLLECTION SET" << _src->baseData.o  << "->" << _dst->baseData.o << _dst->baseData.s << _dst->baseData.id;
}







/**
 * @brief ConflictSolver::ConflictCollection::generateEvent
 * @param engine
 */

void ConflictSolver::ConflictCollection::generateEvent(ConflictSolver *solver, RpgEngine *engine)
{
	Q_ASSERT(solver);
	Q_ASSERT(m_unique);

	SLOG_ERROR(solver) << "!!!! CREATE COLLECTION EVENT";

	const ReleaseState &state = renderCurrentState(solver->m_renderer, tick);

	if (state.state == StateInvalid || !state.player)
		return;

	RendererItem<RpgGameData::ControlCollection> *e = solver->m_renderer->snapAt<RpgGameData::ControlCollection>(m_unique, 0);

	if (!e) {
		SLOG_ERROR(solver) << "Invalid item" << e;
		return;
	}

	RpgGameData::ControlCollection data = e->data();

	if (state.state == StateSuccess) {
		data.own = state.player->asBaseData();
		data.a = false;
		data.u = {};
	} else if (state.state == StateFailed) {
		data.own = {};
		data.a = false;
		data.u = {};
	} else if (state.state == StateHold) {
		data.a = false;
		data.u = state.player->asBaseData();
	}

	m_unique->overrideAuthSnap(data);

	if (state.state == StateSuccess || state.state == StateFailed) {
		RpgGameData::PlayerBaseData pd;

		if (state.player)
			pd = state.player->baseData;

		engine->playerAddXp(pd, state.xp);

		engine->addRelocateCollection(solver->m_renderer->startTick()+tick, m_unique->baseData, pd,
									  state.state == StateSuccess);
	}

}





/**
 * @brief ConflictSolver::ConflictCollection::getInitialState
 * @param renderer
 * @param destPtr
 * @return
 */

bool ConflictSolver::ConflictCollection::getInitialState(Renderer *renderer, ReleaseState *destPtr) const
{
	Q_ASSERT(renderer);

	const auto *snap = renderer->snapAt<RpgGameData::ControlCollection>(m_unique, 0);

	Q_ASSERT(snap);

	const auto &data = snap->data();

	if (data.own.isValid()) {
		SLOG_DEBUG(renderer) << "---- collection already collected";
		return false;
	}

	if (data.u.isValid()) {
		setPlayer(renderer, data.u, destPtr);
	}

	return true;
}




/**
 * @brief ConflictSolver::ConflictPickable::solve
 * @param solver
 * @return
 */

ConflictSolver::ConflictPickable::ConflictPickable(const int &_tick,
												   RendererObject<RpgGameData::PickableBaseData> *_dst,
												   RendererObject<RpgGameData::PlayerBaseData> *_src)
	: ConflictDataUnique(_tick, _dst, _src)
{
	Q_ASSERT(_src);
	Q_ASSERT(_dst);

	SLOG_DEBUG(_dst) << "PICKABLE SET" << _src->baseData.o  << "->" << _dst->baseData.o << _dst->baseData.s << _dst->baseData.id;
}



/**
 * @brief ConflictSolver::ConflictPickable::generateEvent
 * @param solver
 * @param engine
 */

void ConflictSolver::ConflictPickable::generateEvent(ConflictSolver *solver, RpgEngine *engine)
{
	Q_ASSERT(solver);
	Q_ASSERT(m_unique);

	SLOG_ERROR(solver) << "!!!! CREATE PICKABLE EVENT";

	const ReleaseState &state = renderCurrentState(solver->m_renderer, tick);

	if (state.state == StateInvalid || !state.player)
		return;

	RendererItem<RpgGameData::Pickable> *e = solver->m_renderer->snapAt<RpgGameData::Pickable>(m_unique, 0);

	if (!e) {
		SLOG_ERROR(solver) << "Invalid item" << e;
		return;
	}

	RpgGameData::Pickable data = e->data();

	if (state.state == StateSuccess) {
		data.own = state.player->asBaseData();
		data.a = false;
		data.u = {};
	} /*else if (state.state == StateFailed) {
		data.own = {};
		data.a = false;
		data.u = {};
	}*/ else if (state.state == StateHold) {
		data.a = false;
		data.u = state.player->asBaseData();
	}

	m_unique->overrideAuthSnap(data);

	if (state.state == StateSuccess) {
		RpgGameData::PlayerBaseData pd;

		if (state.player)
			pd = state.player->baseData;

		engine->addPickablePicked(solver->m_renderer->startTick()+tick, m_unique->baseData, pd);
	}
}





/**
 * @brief ConflictSolver::ConflictPickable::getInitialState
 * @param renderer
 * @param destPtr
 * @return
 */

bool ConflictSolver::ConflictPickable::getInitialState(Renderer *renderer, ReleaseState *destPtr) const
{
	Q_ASSERT(renderer);

	const auto *snap = renderer->snapAt<RpgGameData::Pickable>(m_unique, 0);

	Q_ASSERT(snap);

	const auto &data = snap->data();

	if (data.own.isValid()) {
		SLOG_DEBUG(renderer) << "---- pickable already picked";
		return false;
	}

	if (data.u.isValid()) {
		setPlayer(renderer, data.u, destPtr);
	}

	return true;
}




/**
 * @brief ConflictSolver::ConflictGate::ConflictGate
 * @param _tick
 * @param _dst
 * @param _src
 */

ConflictSolver::ConflictGate::ConflictGate(const int &_tick, RendererObject<RpgGameData::ControlGateBaseData> *_dst,
										   RendererObject<RpgGameData::PlayerBaseData> *_src)
	: ConflictDataUnique(_tick, _dst, _src)
{
	Q_ASSERT(_src);
	Q_ASSERT(_dst);

	SLOG_DEBUG(_dst) << "GATE SET" << _src->baseData.o << "->" << _dst->baseData.o << _dst->baseData.s << _dst->baseData.id;
}





/**
 * @brief ConflictSolver::ConflictGate::generateEvent
 * @param solver
 * @param engine
 */

void ConflictSolver::ConflictGate::generateEvent(ConflictSolver *solver, RpgEngine */*engine*/)
{
	Q_ASSERT(solver);
	Q_ASSERT(m_unique);

	SLOG_ERROR(solver) << "!!!! CREATE GATE EVENT";

	const ReleaseState &state = renderCurrentState(solver->m_renderer, tick);

	SLOG_INFO(solver) << "CURRENT STATE" << state.state << state.player;

	if (state.state != StateSuccess)
		return;

	RendererItem<RpgGameData::ControlGate> *e = solver->m_renderer->snapAt<RpgGameData::ControlGate>(m_unique, 0);

	if (!e) {
		SLOG_ERROR(solver) << "Invalid item" << e;
		return;
	}

	RpgGameData::ControlGate data = e->data();

	data.st = (data.st == RpgGameData::ControlGate::GateOpen ?
				   RpgGameData::ControlGate::GateClose :
				   RpgGameData::ControlGate::GateOpen);
	data.a = true;

	SLOG_INFO(solver) << "OVERRIDE SNAP" << data.st;

	m_unique->overrideAuthSnap(data);
}






/**
 * @brief ConflictSolver::ConflictGate::getInitialState
 * @param renderer
 * @param destPtr
 * @return
 */

bool ConflictSolver::ConflictGate::getInitialState(Renderer *renderer, ReleaseState *destPtr) const
{
	Q_ASSERT(renderer);

	const auto *snap = renderer->snapAt<RpgGameData::ControlGate>(m_unique, 0);

	Q_ASSERT(snap);

	const auto &data = snap->data();

	if (data.st == RpgGameData::ControlGate::GateDamaged) {
		SLOG_DEBUG(renderer) << "---- gate already damaged";
		return false;
	}

	if (data.u.isValid()) {
		setPlayer(renderer, data.u, destPtr);
	}

	return true;
}



/**
 * @brief ConflictSolver::ConflictTeleport::ConflictTeleport
 * @param _tick
 * @param _dst
 * @param _src
 */

ConflictSolver::ConflictTeleport::ConflictTeleport(const int &_tick,
												   RendererObject<RpgGameData::ControlTeleportBaseData> *_dst,
												   RendererObject<RpgGameData::PlayerBaseData> *_src)
	: ConflictDataUnique(_tick, _dst, _src)
{
	Q_ASSERT(_src);
	Q_ASSERT(_dst);

	SLOG_DEBUG(_dst) << "TELEPORT SET" << _src->baseData.o << "->" << _dst->baseData.o << _dst->baseData.s << _dst->baseData.id;
}






/**
 * @brief ConflictSolver::ConflictTeleport::generateEvent
 * @param solver
 * @param engine
 */

void ConflictSolver::ConflictTeleport::generateEvent(ConflictSolver *solver, RpgEngine *engine)
{
	Q_ASSERT(solver);
	Q_ASSERT(m_unique);

	SLOG_ERROR(solver) << "!!!! CREATE TELEPORT EVENT";

	if (!getInitialState(solver->m_renderer, nullptr)) {
		SLOG_DEBUG(solver) << "Initial state finished, render disabled";
		return;
	}

	RendererItem<RpgGameData::ControlTeleport> *e = solver->m_renderer->snapAt<RpgGameData::ControlTeleport>(m_unique, 0);

	if (!e) {
		SLOG_ERROR(solver) << "Invalid item" << e;
		return;
	}

	const QPointF pos(m_unique->baseData.x, m_unique->baseData.y);

	for (const ReleaseState &st : m_release) {
		if (st.state != StateSuccess || !st.player)
			continue;

		engine->addTeleportUsed(solver->m_renderer->startTick()+tick, m_unique->baseData, st.player->baseData);
	}
}



/**
 * @brief ConflictSolver::ConflictTeleport::getInitialState
 * @param renderer
 * @param destPtr
 * @return
 */

bool ConflictSolver::ConflictTeleport::getInitialState(Renderer *renderer, ReleaseState */*destPtr*/) const
{
	Q_ASSERT(renderer);

	const auto *snap = renderer->snapAt<RpgGameData::ControlTeleport>(m_unique, 0);

	Q_ASSERT(snap);

	const auto &data = snap->data();

	if (!data.a) {
		SLOG_DEBUG(renderer) << "---- teleport inactive";
		return false;
	}

	return true;
}







/**
 * @brief ConflictSolver::ConflictRelocate::solve
 * @param solver
 * @return
 */

bool ConflictSolver::ConflictRelocate::solve(ConflictSolver *solver)
{
	Q_ASSERT(solver);

	if (!src) {
		SLOG_ERROR(solver) << "Invalid source" << src;
		return false;
	}

	RendererItem<RpgGameData::Player> *e = solver->m_renderer->get<RpgGameData::Player>(src);

	if (!e) {
		SLOG_ERROR(solver) << "Invalid item" << src;
		return false;
	}

	RpgGameData::Player data = e->data();

	if (e->flags() & RendererType::ReadOnly) {
		SLOG_ERROR(solver) << "Read only storage item" << src;
		return false;
	}


	if (!e->flags().testFlag(RendererType::Storage)) {
		const auto &ptr = solver->m_renderer->extendFromLast<RpgGameData::Player>(src);

		if (!ptr) {
			SLOG_ERROR(solver) << "Extend failed";
			return false;
		}

		data = ptr.value();
	}

	const QList<float> pos{(float) position.x(), (float) position.y()};

	data.p = pos;
	data.a = angle;
	data.sc = scene;


	e->setData(data);
	e->addFlags(RendererType::Modified);


	for (auto it = src->iterator(); it != src->snap.cend(); ++it) {
		RendererItem<RpgGameData::Player> *p = dynamic_cast<RendererItem<RpgGameData::Player> *>(it->get());

		if (!p || !p->hasContent() || p->flags().testFlags(RendererType::ReadOnly))
			continue;

		RpgGameData::Player pdata = p->data();

		pdata.p = pos;
		pdata.a = angle;
		pdata.sc = scene;

		p->setData(pdata);
		p->addFlags(RendererType::Modified);
	}


	return true;
}
