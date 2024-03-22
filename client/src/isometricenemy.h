/*
 * ---- Call of Suli ----
 *
 * isometricenemy.h
 *
 * Created on: 2024. 03. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricEnemy
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

#ifndef ISOMETRICENEMY_H
#define ISOMETRICENEMY_H

#include "isometricentity.h"
#include "isometricplayer.h"
#include "tiledpathmotor.h"
#include "tiledreturnpathmotor.h"
#include <QQmlEngine>


/**
 * @brief The IsometricEnemyBase class
 */

class IsometricEnemyIface
{
public:
	IsometricEnemyIface()
	{}


	void loadPathMotor(const QPolygonF &polygon, const TiledPathMotor::Direction &direction = TiledPathMotor::Forward);
	void loadFixPositionMotor(const QPointF &point, const TiledObject::Direction &direction = TiledObject::Invalid);

	AbstractTiledMotor *motor() const { return m_motor.get(); }
	TiledPathMotor *pathMotor() const { return m_motor ? dynamic_cast<TiledPathMotor*>(m_motor.get()) : nullptr; }

	IsometricPlayer *player() const;
	void setPlayer(IsometricPlayer *newPlayer);


	float playerDistance() const;
	void setPlayerDistance(float newPlayerDistance);

	virtual void attackedByPlayer(IsometricPlayer *player, const TiledWeapon::WeaponType &weaponType) = 0;
	virtual bool protectWeapon(const TiledWeapon::WeaponType &weaponType) = 0;

public:
	virtual void playerChanged() = 0;
	virtual void playerDistanceChanged() = 0;


protected:
	virtual bool enemyWorldStep() = 0;
	virtual void onPathMotorLoaded(const AbstractTiledMotor::Type &/*type*/) {};


	struct EnemyMetric {
		qreal speed = 3.0;						// <=0: no move
		qreal runSpeed = -1.0;					// Over runSpeed activate "run" sprite
		qreal pursuitSpeed = 3.0;				// -1: =speed, 0: no pursuit, >0: pursuit speed
		qreal returnSpeed = -1.0;				// -1: =speed, 0: no return, >0: return speed
		bool rotateToPlayer = true;
		qint64 inabilityTime = 1250;			// Inability after hit
		qint64 autoAttackTime = 1500;			// Repeated attack to player

		qreal sensorLength = 450.;
		qreal sensorRange = M_PI*2./3.;
		qreal targetCircleRadius = 0.;
	};

	std::unique_ptr<AbstractTiledMotor> m_motor;
	std::unique_ptr<TiledReturnPathMotor> m_returnPathMotor;
	QPointer<IsometricPlayer> m_player;
	QList<QPointer<IsometricPlayer>> m_contactedPlayers;
	QList<QPointer<IsometricPlayer>> m_reachedPlayers;
	EnemyMetric m_metric;
	float m_playerDistance = -1;
};




/**
 * @brief The IsometricEnemy class
 */

class IsometricEnemy : public IsometricCircleEntity, public IsometricEnemyIface
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(IsometricPlayer *player READ player WRITE setPlayer NOTIFY playerChanged FINAL)
	Q_PROPERTY(qreal playerDistance READ playerDistance WRITE setPlayerDistance NOTIFY playerDistanceChanged FINAL)
	Q_PROPERTY(TiledWeapon* defaultWeapon READ defaultWeapon CONSTANT FINAL)

public:
	explicit IsometricEnemy(QQuickItem *parent = nullptr);

	virtual TiledWeapon *defaultWeapon() const = 0;

	void initialize();
	bool hasAbility() const;

signals:
	void becameAlive();
	void becameDead();

	void playerChanged() override final;
	void playerDistanceChanged() override final;

protected:
	virtual void entityWorldStep() override final;
	virtual bool enemyWorldStep() override;
	virtual void onPathMotorLoaded(const AbstractTiledMotor::Type &type) override;

	virtual void load() = 0;
	void onAlive() override;
	void onDead() override;

	bool protectWeapon(const TiledWeapon::WeaponType &weaponType) override;
	void attackedByPlayer(IsometricPlayer *player, const TiledWeapon::WeaponType &weaponType) override;
	void startInabililty();

	virtual void eventPlayerReached(IsometricPlayer *player) = 0;
	virtual void eventPlayerLeft(IsometricPlayer *player) = 0;
	virtual void eventPlayerContacted(IsometricPlayer *player) { Q_UNUSED(player); }
	virtual void eventPlayerDiscontacted(IsometricPlayer *player) { Q_UNUSED(player); }

	virtual void attackPlayer(IsometricPlayer *player, TiledWeapon *weapon);
	virtual void playAttackEffect(TiledWeapon *weapon) { Q_UNUSED(weapon); }

	void stepMotor();
	void rotateToPlayer(IsometricPlayer *player, float32 *anglePtr = nullptr, qreal *distancePtr = nullptr);
	void rotateToPoint(const QPointF &point, float32 *anglePtr = nullptr, qreal *distancePtr = nullptr);
	float32 angleToPoint(const QPointF &point) const;
	qreal distanceToPoint(const QPointF &point) const;

private:
	void sensorBeginContact(Box2DFixture *other);
	void sensorEndContact(Box2DFixture *other);
	void fixtureBeginContact(Box2DFixture *other);
	void fixtureEndContact(Box2DFixture *other);

protected:
	QDeadlineTimer m_inabilityTimer;
	QDeadlineTimer m_autoHitTimer;
	std::vector<std::unique_ptr<TiledWeapon>> m_weapons;

	friend class TiledGame;
	friend class RpgGame;
};

#endif // ISOMETRICENEMY_H
