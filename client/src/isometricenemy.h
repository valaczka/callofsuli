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
 * @brief The EnemyMetric class
 */

class EnemyMetric : public QSerializer
{
	Q_GADGET

public:
	EnemyMetric()
	{
		speed = 3.0;					// <=0: no move
		pursuitSpeed = 3.0;				// -1: =speed, 0: no pursuit, >0: pursuit speed
		returnSpeed = -1.0;				// -1: =speed, 0: no return, >0: return speed
		rotateToPlayer = true;
		inabilityTime = 1250;			// Inability after hit
		firstAttackTime = 350;			// First attack to player
		autoAttackTime = 750;			// Repeated attack to player
		sleepingTime = 10000;			// Sleeping after hit (hand), 0: no sleeping

		sensorLength = 620.;
		sensorRange = M_PI*2./3.;
		targetCircleRadius = 0.;
	}

	QS_SERIALIZABLE

	QS_FIELD(qreal, speed)
	QS_FIELD(qreal, pursuitSpeed)
	QS_FIELD(qreal, returnSpeed)
	QS_FIELD(bool, rotateToPlayer)
	QS_FIELD(qint64, inabilityTime)
	QS_FIELD(qint64, firstAttackTime)
	QS_FIELD(qint64, autoAttackTime)
	QS_FIELD(qint64, sleepingTime)

	QS_FIELD(qreal, sensorLength)
	QS_FIELD(qreal, sensorRange)
	QS_FIELD(qreal, targetCircleRadius)
};




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

	void removeContactedPlayer(IsometricPlayer *player);

	float playerDistance() const;
	void setPlayerDistance(float newPlayerDistance);

	const EnemyMetric &metric() const { return m_metric; }
	void setMetric(const EnemyMetric &metric) { m_metric = metric; }

public:
	virtual void playerChanged() = 0;
	virtual void playerDistanceChanged() = 0;


protected:
	virtual bool enemyWorldStep() = 0;
	virtual bool enemyWorldStepOnVisiblePlayer() = 0;
	virtual void onPathMotorLoaded(const AbstractTiledMotor::Type &/*type*/) {};

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

class IsometricEnemy : public IsometricEntity, public IsometricEnemyIface
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(IsometricPlayer *player READ player WRITE setPlayer NOTIFY playerChanged FINAL)
	Q_PROPERTY(qreal playerDistance READ playerDistance WRITE setPlayerDistance NOTIFY playerDistanceChanged FINAL)
	Q_PROPERTY(int enemyType READ enemyType CONSTANT FINAL)

public:
	explicit IsometricEnemy(TiledScene *scene);

	void initialize();
	bool hasAbility();
	bool isSleeping();
	bool isRunning() const;
	bool isWalking() const;

	void rotateToPlayer(IsometricPlayer *player, const bool &forced = false);

	virtual int enemyType() const = 0;

	virtual void onShapeContactBegin(b2::ShapeRef self, b2::ShapeRef other) override;
	virtual void onShapeContactEnd(b2::ShapeRef self, b2::ShapeRef other) override;

	virtual void worldStep() override;

signals:
	void becameAlive();
	void becameDead();
	void becameAsleep();
	void becameAwake();

	void playerChanged() override final;
	void playerDistanceChanged() override final;

protected:
	virtual void onPathMotorLoaded(const AbstractTiledMotor::Type &type) override;

	virtual void synchronize() override;

	virtual void load() = 0;
	void onAlive() override;
	void onDead() override;
	void onSleepingBegin();
	void onSleepingEnd();

	void startInability();
	bool startSleeping();

	virtual void eventPlayerReached(IsometricPlayer *player) = 0;
	virtual void eventPlayerLeft(IsometricPlayer *player) = 0;
	virtual void eventPlayerContacted(IsometricPlayer *player) { Q_UNUSED(player); }
	virtual void eventPlayerDiscontacted(IsometricPlayer *player) { Q_UNUSED(player); }
	virtual void eventKilledByPlayer(IsometricPlayer *player);

	void stepMotor();

protected:
	qint64 m_inabilityTimer = -1;
	qint64 m_autoHitTimer = -1;
	qint64 m_sleepingTimer = -1;
	bool m_isSleeping = false;

	friend class TiledGame;
	friend class RpgGame;
};

#endif // ISOMETRICENEMY_H
