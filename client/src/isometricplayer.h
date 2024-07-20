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
#include "tiledcontainer.h"
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
	Q_PROPERTY(TiledContainer *currentContainer READ currentContainer WRITE setCurrentContainer NOTIFY currentContainerChanged FINAL)
	Q_PROPERTY(qreal currentAngle READ currentAngle WRITE setCurrentAngle NOTIFY currentAngleChanged FINAL)
	Q_PROPERTY(IsometricEnemy* enemy READ enemy WRITE setEnemy NOTIFY enemyChanged FINAL)
	Q_PROPERTY(QPointF currentVelocity READ currentVelocity WRITE setCurrentVelocity NOTIFY currentVelocityChanged FINAL)
	Q_PROPERTY(bool isLocked READ isLocked WRITE setIsLocked NOTIFY isLockedChanged FINAL)

public:
	explicit IsometricPlayer(QQuickItem *parent = nullptr);
	virtual ~IsometricPlayer();

	void onJoystickStateChanged(const TiledGame::JoystickState &state);

	void setDestinationPoint(const qreal &x, const qreal &y);
	void setDestinationPoint(const QPointF &point) { setDestinationPoint(point.x(), point.y()); }
	void clearDestinationPoint();

	virtual void entityWorldStep(const qreal &factor) override;

	void initialize();
	bool hasAbility() const;

	TiledTransport *currentTransport() const;
	void setCurrentTransport(TiledTransport *newCurrentTransport);

	TiledObjectBase *currentTransportBase() const { return m_currentTransportBase; }

	qreal currentAngle() const;
	void setCurrentAngle(qreal newCurrentAngle);

	void removeEnemy(IsometricEnemy *enemy);

	IsometricEnemy *enemy() const;
	void setEnemy(IsometricEnemy *newEnemy);

	QPointF currentVelocity() const;
	void setCurrentVelocity(QPointF newCurrentVelocity);

	bool isLocked() const;
	void setIsLocked(bool newIsLocked);

	TiledContainer *currentContainer() const;
	void setCurrentContainer(TiledContainer *newCurrentContainer);

signals:
	void becameAlive();
	void becameDead();

	void currentTransportChanged();
	void currentAngleChanged();
	void enemyChanged();
	void currentVelocityChanged();
	void isLockedChanged();
	void currentContainerChanged();

protected:
	//void updateSprite() override;
	void onAlive() override;
	void onDead() override;
	void startInability();
	void startInability(const int &msec);

	virtual void load() = 0;
	virtual bool protectWeapon(const TiledWeapon::WeaponType &weaponType) = 0;
	virtual void attackedByEnemy(IsometricEnemy *enemy, const TiledWeapon::WeaponType &weaponType, const bool &isProtected) = 0;
	virtual void onPickableReached(TiledObject *object) = 0;
	virtual void onPickableLeft(TiledObject *object) = 0;
	virtual void onEnemyReached(IsometricEnemy *enemy) = 0;
	virtual void onEnemyLeft(IsometricEnemy *enemy) = 0;
	virtual void onTransportReached(TiledTransport *transport) = 0;
	virtual void onTransportLeft(TiledTransport *transport) = 0;

	virtual void atDestinationPointEvent() {}

	bool protectWeapon(TiledWeaponList *weaponList, const TiledWeapon::WeaponType &weaponType);

	QList<IsometricEnemy*> reachedEnemies() const;
	QList<IsometricEnemy*> contactedAndReachedEnemies() const;

	QDeadlineTimer m_inabilityTimer;
	qreal m_sensorLength = 200.;
	qreal m_sensorRange = M_PI_2;
	qreal m_targetCircleRadius = 50.;
	qreal m_speedLength = 3.;
	qreal m_speedRunLength = 7.;
	qreal m_runSpeed = toMovingSpeed(5.);
	qint64 m_inabilityTime = 200;
	bool m_isLocked = false;


private:
	void clearData();
	void sensorBeginContact(Box2DFixture *other);
	void sensorEndContact(Box2DFixture *other);
	void fixtureBeginContact(Box2DFixture *other);
	void fixtureEndContact(Box2DFixture *other);

	QPointer<TiledTransport> m_currentTransport = nullptr;
	QPointer<TiledObjectBase> m_currentTransportBase;
	QPointer<TiledContainer> m_currentContainer = nullptr;

	qreal m_currentAngle = 0.;
	QPointF m_currentVelocity;

	IsometricPlayerPrivate *d = nullptr;

	friend class TiledGame;
	friend class RpgGame;
};

#endif // ISOMETRICPLAYER_H
