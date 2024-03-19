/*
 * ---- Call of Suli ----
 *
 * isometricplayer.h
 *
 * Created on: 2024. 03. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricPlayer
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

#ifndef ISOMETRICPLAYER_H
#define ISOMETRICPLAYER_H

#include "isometricentity.h"
#include "qdeadlinetimer.h"
#include "tiledtransport.h"
#include "tiledgame.h"
#include <QQmlEngine>

class IsometricEnemy;
class IsometricPlayerPrivate;


#if QT_VERSION >= 0x060000

#ifndef OPAQUE_PTR_IsometricEnemy
#define OPAQUE_PTR_IsometricEnemy
  Q_DECLARE_OPAQUE_POINTER(IsometricEnemy*)
#endif

#endif


/**
 * @brief The IsometricPlayer class
 */

class IsometricPlayer : public IsometricCircleEntity
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(TiledTransport *currentTransport READ currentTransport WRITE setCurrentTransport NOTIFY currentTransportChanged FINAL)
	Q_PROPERTY(qreal currentAngle READ currentAngle WRITE setCurrentAngle NOTIFY currentAngleChanged FINAL)
	Q_PROPERTY(IsometricEnemy* enemy READ enemy WRITE setEnemy NOTIFY enemyChanged FINAL)
	Q_PROPERTY(QPointF currentVelocity READ currentVelocity WRITE setCurrentVelocity NOTIFY currentVelocityChanged FINAL)
	Q_PROPERTY(TiledObject *currentPickable READ currentPickable WRITE setCurrentPickable NOTIFY currentPickableChanged FINAL)

public:
	explicit IsometricPlayer(QQuickItem *parent = nullptr);
	virtual ~IsometricPlayer();

	//static IsometricPlayer* createPlayer(TiledGame *game, TiledScene *scene);
	void onJoystickStateChanged(const TiledGame::JoystickState &state);

	virtual void entityWorldStep() override;

	void initialize();
	bool hasAbility() const;

	TiledTransport *currentTransport() const;
	void setCurrentTransport(TiledTransport *newCurrentTransport);

	qreal currentAngle() const;
	void setCurrentAngle(qreal newCurrentAngle);

	IsometricEnemy *enemy() const;
	void setEnemy(IsometricEnemy *newEnemy);

	QPointF currentVelocity() const;
	void setCurrentVelocity(QPointF newCurrentVelocity);

	TiledObject *currentPickable() const;
	void setCurrentPickable(TiledObject *newCurrentPickable);
	void removePickable(TiledObject *pickable);

signals:
	void becameAlive();
	void becameDead();

	void currentTransportChanged();
	void currentAngleChanged();
	void enemyChanged();
	void currentVelocityChanged();
	void currentPickableChanged();

protected:
	//void updateSprite() override;
	void onAlive() override;
	void onDead() override;
	void startInabililty();

	virtual void createMarkerItem();

	virtual void load() = 0;
	virtual bool protectWeapon(const TiledWeapon::WeaponType &weaponType) = 0;
	virtual void attackedByEnemy(IsometricEnemy *enemy, const TiledWeapon::WeaponType &weaponType) = 0;
	virtual void onEnemyReached(IsometricEnemy *enemy) = 0;
	virtual void onEnemyLeft(IsometricEnemy *enemy) = 0;
	virtual void onTransportReached(TiledTransport *transport) = 0;
	virtual void onTransportLeft(TiledTransport *transport) = 0;

	bool protectWeapon(TiledWeaponList *weaponList, const TiledWeapon::WeaponType &weaponType);


	QDeadlineTimer m_inabilityTimer;
	qreal m_sensorLength = 200.;
	qreal m_sensorRange = M_PI * 0.33;
	qreal m_targetCircleRadius = 50.;
	qreal m_speedLength = 6.;
	qint64 m_inabilityTime = 1000;


private:
	void sensorBeginContact(Box2DFixture *other);
	void sensorEndContact(Box2DFixture *other);
	void fixtureBeginContact(Box2DFixture *other);
	void fixtureEndContact(Box2DFixture *other);

	QPointer<TiledTransport> m_currentTransport = nullptr;
	QPointer<TiledObject> m_currentPickable = nullptr;
	qreal m_currentAngle = 0.;
	QPointF m_currentVelocity;

	IsometricPlayerPrivate *d = nullptr;

	friend class TiledGame;
	friend class TiledRpgGame;
};

#endif // ISOMETRICPLAYER_H
