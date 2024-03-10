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
#include "qelapsedtimer.h"
#include "tiledpathmotor.h"
#include "tiledreturnpathmotor.h"
#include <QQmlEngine>


/**
 * @brief The IsometricEnemyBase class
 */

class IsometricEnemyIface
{


public:
	IsometricEnemyIface() {}

	void loadPathMotor(const QPolygonF &polygon, const TiledPathMotor::Direction &direction = TiledPathMotor::Forward);

	AbstractTiledMotor *motor() const { return m_motor.get(); }
	TiledPathMotor *pathMotor() const { return m_motor ? dynamic_cast<TiledPathMotor*>(m_motor.get()) : nullptr; }

	IsometricPlayer *player() const;
	void setPlayer(IsometricPlayer *newPlayer);

	static bool checkPlayerVisibility(TiledObjectBody *body, TiledObjectBase *player);

	float playerDistance() const;
	void setPlayerDistance(float newPlayerDistance);

public:
	virtual void playerChanged() = 0;
	virtual void playerDistanceChanged() = 0;


protected:
	virtual bool enemyWorldStep() = 0;
	virtual void onPathMotorLoaded(const AbstractTiledMotor::Type &/*type*/) {};

	struct EnemyMetric {
		qreal speed = 3.0;						// <=0: no move
		qreal pursuitSpeed = 3.0;				// -1: =speed, 0: no pursuit, >0: pursuit speed
		qreal returnSpeed = -1.0;				// -1: =speed, 0: no return, >0: return speed
		float playerDistance = 50;				// stop at distance
		bool rotateToPlayer = true;
	};

	std::unique_ptr<AbstractTiledMotor> m_motor;
	std::unique_ptr<TiledReturnPathMotor> m_returnPathMotor;
	QPointer<IsometricPlayer> m_player;
	QPointer<IsometricPlayer> m_contactedPlayer;
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

public:
	explicit IsometricEnemy(QQuickItem *parent = nullptr);

	static IsometricEnemy* createEnemy(TiledScene *scene);


signals:
	void playerChanged() override final;
	void playerDistanceChanged() override final;

protected:
	virtual void entityWorldStep() override final;
	virtual bool enemyWorldStep() override;
	virtual void onPathMotorLoaded(const AbstractTiledMotor::Type &type) override;
	void onAlive() override;
	void onDead() override;

	void stepMotor();
	void rotateToPlayer(IsometricPlayer *player, float32 *anglePtr = nullptr, QPointF *vectorPtr = nullptr);

private:
	void load();
	void updateSprite() override;
	void nextAlteration();

	QString m_currentAlteration;
	QElapsedTimer m_hitTimer;

};

#endif // ISOMETRICENEMY_H
