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

	T serialize(const qint64 tick = -1) const;

	virtual TiledObjectBody::ObjectId objectId() const = 0;
	virtual T2 baseData() const;

protected:
	static QList<float> toPosList(const QVector2D &pos) { return { pos.x(), pos.y() }; }
	static QList<float> toPosList(const QPointF &pos) { return { (float) pos.x(), (float) pos.y() }; }

	virtual std::unique_ptr<RpgGameData::Body> serializeThis() const = 0;
};



/**
 * @brief RpgGameDataInterface::serialize
 * @param tick
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
inline T RpgGameDataInterface<T, T2, T3, T4>::serialize(const qint64 tick) const
{
	std::unique_ptr<RpgGameData::Body> ptr = serializeThis();

	T p = *dynamic_cast<T*>(ptr.get());

	if (tick >= 0)
		p.f = tick;

	return p;
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






#endif // RPGGAMEDATAIFACE_H
