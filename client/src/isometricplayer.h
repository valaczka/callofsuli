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


/**
 * @brief The IsometricPlayer class
 */

class IsometricPlayer : public IsometricCircleEntity
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(TiledTransport *currentTransport READ currentTransport WRITE setCurrentTransport NOTIFY currentTransportChanged FINAL)
	Q_PROPERTY(qreal currentAngle READ currentAngle WRITE setCurrentAngle NOTIFY currentAngleChanged FINAL)

public:
	explicit IsometricPlayer(QQuickItem *parent = nullptr);
	virtual ~IsometricPlayer();

	//static IsometricPlayer* createPlayer(TiledGame *game, TiledScene *scene);
	void onJoystickStateChanged(const TiledGame::JoystickState &state);

	virtual void entityWorldStep() override;

	Q_INVOKABLE void hit();
	Q_INVOKABLE void shot();

	void initialize();
	bool hasAbility() const;

	TiledTransport *currentTransport() const;
	void setCurrentTransport(TiledTransport *newCurrentTransport);

	qreal currentAngle() const;
	void setCurrentAngle(qreal newCurrentAngle);

signals:
	void becameAlive();
	void becameDead();

	void currentTransportChanged();
	void currentAngleChanged();

protected:
	//void updateSprite() override;
	void onAlive() override;
	void onDead() override;
	void startInabililty();

	virtual void createMarkerItem();

	virtual void load() = 0;
	virtual void attackedByEnemy(IsometricEnemy *enemy) = 0;
	virtual void onEnemyReached(IsometricEnemy *enemy) = 0;
	virtual void onEnemyLeft(IsometricEnemy *enemy) = 0;
	virtual void onTransportReached(TiledTransport *transport) = 0;
	virtual void onTransportLeft(TiledTransport *transport) = 0;


	QDeadlineTimer m_inabilityTimer;
	qreal m_sensorLength = 200.;
	qreal m_sensorRange = M_PI * 0.33;
	qreal m_targetCircleRadius = 50.;
	qreal m_speedLength = 6.;
	qint64 m_inabilityTime = 250;

private:
	void sensorBeginContact(Box2DFixture *other);
	void sensorEndContact(Box2DFixture *other);
	void fixtureBeginContact(Box2DFixture *other);
	void fixtureEndContact(Box2DFixture *other);

	TiledTransport *m_currentTransport = nullptr;
	qreal m_currentAngle = 0.;

	IsometricPlayerPrivate *d = nullptr;

	friend class TiledGame;
	friend class TiledRpgGame;
};

#endif // ISOMETRICPLAYER_H
