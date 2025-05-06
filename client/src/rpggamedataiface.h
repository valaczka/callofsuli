/*
 * ---- Call of Suli ----
 *
 * rpggamedataiface.h
 *
 * Created on: 2025. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#ifndef RPGGAMEDATAIFACE_H
#define RPGGAMEDATAIFACE_H

#include "isometricentity.h"
#include "tiledobject.h"
#include "rpgconfig.h"
#include <QVector2D>
#include <QCborMap>



/**
 * @brief The RpgGameDataInterface class
 */



template <typename T, typename T2,
		  typename = std::enable_if<std::is_base_of<RpgGameData::Body, T>::value>::type,
		  typename = std::enable_if<std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
class RpgGameDataInterface
{
public:
	RpgGameDataInterface() {}

	T serialize(const qint64 tick = -1);
	std::optional<T> serializeCmp(const qint64 tick = -1);

	virtual TiledObjectBody::ObjectId objectId() const = 0;
	virtual T2 baseData() const;

	virtual void updateFromSnapshot(const RpgGameData::SnapshotInterpolation<T> &snapshot) = 0;
	virtual void updateFromSnapshot(const T &snap) = 0;
	virtual bool isLastSnapshotValid(const T &snap, const T &lastSnap) const;
	void updateFromLastSnapshot(const T &snap);

protected:
	static QList<float> toPosList(const cpVect &pos) { return { (float) pos.x, (float) pos.y }; }

	virtual T serializeThis() const = 0;

	const T &lastSnapshot() const { return m_lastSnapShot; }
	void setLastSnapshot(const T &snap) { m_lastSnapShot = snap; }

	template <typename E>
	static cpVect entityMove(IsometricEntity *entity,
							 const RpgGameData::SnapshotInterpolation<T> &snapshot,
							 const E &idle,
							 const E &moving,
							 const qreal &speed,
							 const qreal &maxSpeed,
							 QString *msg
							 );

	static bool fnStop(IsometricEntity *entity, const float &angle);
	static bool fnEmplace(IsometricEntity *entity, const cpVect &dst, const float &angle);
	static cpVect fnMove (IsometricEntity *entity, const cpVect &final,
						  const int &inFrame, const float &maxSpeed, const QList<float> &cv);

private:
	T m_lastSnapShot;
	T m_lastSerialized;
};





/**
 * @brief RpgGameDataInterface::serialize
 * @param tick
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
inline T RpgGameDataInterface<T, T2, T3, T4>::serialize(const qint64 tick)
{
	m_lastSerialized = serializeThis();

	if (tick >= 0)
		m_lastSerialized.f = tick;

	return m_lastSerialized;
}



/**
 * @brief RpgGameDataInterface::serializeCmp
 * @param tick
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
inline std::optional<T> RpgGameDataInterface<T, T2, T3, T4>::serializeCmp(const qint64 tick)
{
	T p = serializeThis();

	if (m_lastSerialized.f != -1) {
		if (p.canInterpolateFrom(m_lastSerialized))
			return std::nullopt;

		const qint64 f = p.f;
		p.f = m_lastSerialized.f;
		if (p == m_lastSerialized)
			return std::nullopt;
		p.f = f;
	}

	m_lastSerialized = p;

	if (tick >= 0)
		m_lastSerialized.f = tick;

	return m_lastSerialized;
}


/**
 * @brief RpgGameDataInterface::baseData
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
inline T2 RpgGameDataInterface<T, T2, T3, T4>::baseData() const
{
	TiledObjectBody::ObjectId oid = objectId();
	T2 r;
	r.o = oid.ownerId;
	r.s = oid.sceneId;
	r.id = oid.id;
	return r;
}


/**
 * @brief RpgGameDataInterface::isLastSnapshotValid
 * @param snap
 * @param lastSnap
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
inline bool RpgGameDataInterface<T, T2, T3, T4>::isLastSnapshotValid(const T &snap, const T &lastSnap) const
{
	Q_UNUSED(lastSnap);
	Q_UNUSED(snap);
	return true;
}





/**
 * @brief RpgGameDataInterface::updateFromLastSnapshot
 * @param snap
 * @param last
 */

template<typename T, typename T2, typename T3, typename T4>
inline void RpgGameDataInterface<T, T2, T3, T4>::updateFromLastSnapshot(const T &snap)
{
	if (m_lastSnapShot.f >= 0) {
		if (snap.f < m_lastSnapShot.f) {
			if (isLastSnapshotValid(snap, m_lastSnapShot))
				updateFromSnapshot(m_lastSnapShot);
			else
				updateFromSnapshot(snap);
		} else {
			updateFromSnapshot(snap);
			m_lastSnapShot.f = -1;
		}
	} else {
		updateFromSnapshot(snap);
	}
}





#endif // RPGGAMEDATAIFACE_H
