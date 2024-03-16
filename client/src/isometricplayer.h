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

	static IsometricPlayer* createPlayer(TiledGame *game, TiledScene *scene);
	void onJoystickStateChanged(const TiledGame::JoystickState &state);

	virtual void entityWorldStep() override;

	void hurt();
	Q_INVOKABLE void hit();

	Q_INVOKABLE void shot();

	TiledTransport *currentTransport() const;
	void setCurrentTransport(TiledTransport *newCurrentTransport);

	qreal currentAngle() const;
	void setCurrentAngle(qreal newCurrentAngle);

signals:
	void currentTransportChanged();
	void currentAngleChanged();

protected:
	void updateSprite() override;
	void onAlive() override;
	void onDead() override;
	virtual void createMarkerItem();

private:
	void load();			/// virtual


	TiledTransport *m_currentTransport = nullptr;
	QString m_currentAlteration;
	qreal m_currentAngle = 0.;

	IsometricPlayerPrivate *d = nullptr;

	friend class TiledGame;
};

#endif // ISOMETRICPLAYER_H
