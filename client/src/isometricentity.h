/*
 * ---- Call of Suli ----
 *
 * isometricentity.h
 *
 * Created on: 2024. 03. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricEntity
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

#ifndef ISOMETRICENTITY_H
#define ISOMETRICENTITY_H

#include "isometricobject.h"
#include "tiledpathmotor.h"
#include <QQmlEngine>

class TiledGame;

/**
 * @brief The IsometricEntity class
 */

class IsometricEntity : public IsometricObject
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(int hp READ hp WRITE setHp NOTIFY hpChanged FINAL)
	Q_PROPERTY(int maxHp READ maxHp WRITE setMaxHp NOTIFY maxHpChanged FINAL)

public:
	explicit IsometricEntity(TiledGame *game, const qreal &radius = 10., const cpBodyType &type = CP_BODY_TYPE_DYNAMIC);

	int hp() const;
	void setHp(int newHp);

	int maxHp() const;
	void setMaxHp(int newMaxHp);

	bool isAlive() const { return m_hp > 0; }

	virtual void updateSprite() = 0;

	void setDestinationPoint(const QPolygonF &polygon);
	void setDestinationPoint(const cpVect &point);
	void clearDestinationPoint();

	TiledPathMotor *destinationMotor() const { return m_destinationMotor.get(); }

signals:
	void hurt();
	void healed();
	void hpChanged();
	void maxHpChanged();


protected:
	virtual void synchronize() override;

	virtual bool canSetDestinationPoint() const { return true; }

	virtual void onAlive() = 0;
	virtual void onDead() = 0;

protected:
	int m_hp = 1;
	int m_maxHp = 1;
	QStringList m_moveDisabledSpriteList;		// At these sprites move disabled

	std::unique_ptr<TiledPathMotor> m_destinationMotor;
	std::optional<cpVect> m_destinationPoint;
};



#endif // ISOMETRICENTITY_H
