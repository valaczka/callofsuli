/*
 * ---- Call of Suli ----
 *
 * rpgentity.h
 *
 * Created on: 2026. 05. 11.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgEntity
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

#ifndef RPGENTITY_H
#define RPGENTITY_H

#include "rpgobject.h"
#include "tiledpathmotor.h"
#include <QQmlEngine>



/**
 * @brief The RpgEntity class
 */

class RpgEntity : public RpgObject
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(int hp READ hp WRITE setHp NOTIFY hpChanged FINAL)
	Q_PROPERTY(int maxHp READ maxHp WRITE setMaxHp NOTIFY maxHpChanged FINAL)
	Q_PROPERTY(bool locked READ locked WRITE setLocked NOTIFY lockedChanged FINAL)

public:
	explicit RpgEntity(RpgGameItem *gameItem, const cpVect &center = cpvzero, const qreal &radius = 10.,
					   const cpBodyType &type = CP_BODY_TYPE_DYNAMIC);

	bool isAlive() const { return m_hp > 0; }

	RpgStream::Team team() const;
	void setTeam(RpgStream::Team newTeam);

	int hp() const;
	void setHp(int newHp);

	int maxHp() const;
	void setMaxHp(int newMaxHp);

	bool locked() const;
	void setLocked(bool newLocked);

signals:
	void hurt();
	void healed();
	void becameDead();
	void becameAlive();

	void hpChanged();
	void maxHpChanged();
	void lockedChanged();

protected:
	virtual void onAlive() {};
	virtual void onDead() {};
	virtual void updateColor() {};

protected:
	int m_hp = 1;
	int m_maxHp = 1;

	RpgStream::Team m_team = RpgStream::TeamNone;
	bool m_locked = false;;
};





/**
 * @brief The RpgMotorEntity class
 */

class RpgMotorEntity : public AbstractRpgMotor
{
public:
	RpgMotorEntity(RpgEntity *entity);

	enum QueryFlag {
		QueryNone =				0,
		QueryBody =				1,
		QuerySensorPolygon =	1 << 1,
		QueryTarget =			1 << 2,
		QueryVirtualCircle =	1 << 3,

		QueryAll = QueryBody | QuerySensorPolygon | QueryTarget | QueryVirtualCircle
	};

	Q_DECLARE_FLAGS(QueryFlags, QueryFlag);

	static void queryContactedBodies(cpSpace *space, cpShape *shape, QSet<TiledObjectBody *> *dst,
									 const cpBitmask &categories);

	static void queryContactedBodies(TiledObjectBody *body, QSet<TiledObjectBody *> *dst,
									 const cpBitmask &categories, const QueryFlags &flags = QueryAll);

	static void queryContactedVisibleBodies(TiledObjectBody *body, QSet<TiledObjectBody *> *dst,
											const cpBitmask &categories, const cpBitmask &ground,
											const float &maxDist = 0.,
											const QueryFlags &flags = QueryAll);

	static TiledObjectBody *getNearest(TiledObjectBody *body, const QSet<TiledObjectBody *> &dst);

	static QMultiMap<float, TiledObjectBody *> sort(TiledObjectBody *body, const QSet<TiledObjectBody *> &dst);

protected:
	RpgEntity *const m_entity;
};





/**
 * @brief The RpgDestinationMotor class
 */

class RpgDestinationMotor : public RpgMotorEntity
{
public:
	RpgDestinationMotor(RpgEntity *entity);

	void setDestination(const QPolygonF &polygon);
	void setDestination(const cpVect &point);
	void clearDestination();
	std::optional<QPolygonF> destination() const;

protected:
	std::optional<cpVect> m_destinationPoint = std::nullopt;
	std::unique_ptr<TiledPathMotor> m_destinationMotor;

};



#endif // RPGENTITY_H
