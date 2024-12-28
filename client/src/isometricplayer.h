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
#include "tiledcontainer.h"
#include "tiledtransport.h"
#include "tiledgame.h"
#include <QQmlEngine>

class IsometricEnemy;
class IsometricPlayerPrivate;
class TiledPathMotor;

#ifndef OPAQUE_PTR_IsometricEnemy
#define OPAQUE_PTR_IsometricEnemy
Q_DECLARE_OPAQUE_POINTER(IsometricEnemy*)
#endif


/**
 * @brief The IsometricPlayer class
 */

class IsometricPlayer : public IsometricEntity
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(TiledTransport *currentTransport READ currentTransport WRITE setCurrentTransport NOTIFY currentTransportChanged FINAL)
	Q_PROPERTY(TiledContainer *currentContainer READ currentContainer WRITE setCurrentContainer NOTIFY currentContainerChanged FINAL)
	Q_PROPERTY(IsometricEnemy* enemy READ enemy WRITE setEnemy NOTIFY enemyChanged FINAL)
	Q_PROPERTY(QVector2D currentVelocity READ currentVelocity WRITE setCurrentVelocity NOTIFY currentVelocityChanged FINAL)
	Q_PROPERTY(bool isLocked READ isLocked WRITE setIsLocked NOTIFY isLockedChanged FINAL)

public:
	explicit IsometricPlayer(TiledScene *scene);
	virtual ~IsometricPlayer();

	void onJoystickStateChanged(const TiledGame::JoystickState &state);

	void setDestinationPoint(const QPolygonF &polygon);
	void setDestinationPoint(const QPointF &point);
	void clearDestinationPoint();

	void initialize();
	bool hasAbility();

	TiledTransport *currentTransport() const;
	void setCurrentTransport(TiledTransport *newCurrentTransport);

	TiledObject *currentTransportBase() const { return m_currentTransportBase; }

	void removeEnemy(IsometricEnemy *enemy);

	IsometricEnemy *enemy() const;
	void setEnemy(IsometricEnemy *newEnemy);

	QVector2D currentVelocity() const;
	void setCurrentVelocity(QVector2D newCurrentVelocity);

	bool isLocked() const;
	void setIsLocked(bool newIsLocked);

	TiledContainer *currentContainer() const;
	void setCurrentContainer(TiledContainer *newCurrentContainer);

	virtual void worldStep() override;

	bool isRunning() const;
	bool isWalking() const;

signals:
	void becameAlive();
	void becameDead();

	void currentTransportChanged();
	void enemyChanged();
	void currentVelocityChanged();
	void isLockedChanged();
	void currentContainerChanged();

protected:
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

	qint64 m_inabilityTimer = -1;
	qreal m_sensorLength = 200.;
	qreal m_sensorRange = M_PI_2;
	qreal m_targetCircleRadius = 50.;
	qreal m_speedLength = 3.;
	qreal m_speedRunLength = 7.;
	qint64 m_inabilityTime = 200;
	bool m_isLocked = false;

private:
	void clearData();
	TiledPathMotor *destinationMotor() const;

	/*void sensorBeginContact(Box2DFixture *other);
	void sensorEndContact(Box2DFixture *other);
	void fixtureBeginContact(Box2DFixture *other);
	void fixtureEndContact(Box2DFixture *other);*/

	QPointer<TiledTransport> m_currentTransport = nullptr;
	QPointer<TiledObject> m_currentTransportBase;
	QPointer<TiledContainer> m_currentContainer = nullptr;

	QVector2D m_currentVelocity;

	IsometricPlayerPrivate *d = nullptr;

	friend class TiledGame;
	friend class RpgGame;
};

#endif // ISOMETRICPLAYER_H
