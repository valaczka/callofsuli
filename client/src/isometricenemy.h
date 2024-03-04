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
	IsometricEnemyIface() {}

	void loadPathMotor(const QPolygonF &polygon, const TiledPathMotor::Direction &direction = TiledPathMotor::Forward);

	AbstractTiledMotor *motor() const { return m_motor.get(); }
	TiledPathMotor *pathMotor() const { return m_motor ? dynamic_cast<TiledPathMotor*>(m_motor.get()) : nullptr; }

	IsometricPlayer *player() const;
	void setPlayer(IsometricPlayer *newPlayer);

	static bool checkPlayerVisibility(TiledObjectBody *body, TiledObjectBase *player);

public:
	virtual void playerChanged() = 0;

protected:
	virtual void onPathMotorLoaded(const AbstractTiledMotor::Type &/*type*/) {};

	std::unique_ptr<AbstractTiledMotor> m_motor;
	std::unique_ptr<TiledReturnPathMotor> m_returnPathMotor;
	QPointer<IsometricPlayer> m_player;
	QPointer<IsometricPlayer> m_contactedPlayer;
};




/**
 * @brief The IsometricEnemy class
 */

class IsometricEnemy : public IsometricCircleEntity, public IsometricEnemyIface
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(IsometricPlayer *player READ player WRITE setPlayer NOTIFY playerChanged FINAL)

public:
	explicit IsometricEnemy(QQuickItem *parent = nullptr);

	static IsometricEnemy* createEnemy(QQuickItem *parent = nullptr);

	virtual void entityWorldStep() override;

signals:
	void playerChanged() override;

protected:
	virtual void onPathMotorLoaded(const AbstractTiledMotor::Type &type) override;

	void stepMotor();
	void rotateToPlayer(IsometricPlayer *player, float32 *anglePtr = nullptr, QPointF *vectorPtr = nullptr);

private:
	void load();
	void updateSprite() override;
	void nextAlteration();

	QString m_currentAlteration;

};

#endif // ISOMETRICENEMY_H
