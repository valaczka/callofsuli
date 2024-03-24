/*
 * ---- Call of Suli ----
 *
 * rpgarrow.h
 *
 * Created on: 2024. 03. 18.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgArrow
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

#ifndef RPGARROW_H
#define RPGARROW_H

#include "isometricbullet.h"
#include "rpgpickableobject.h"
#include "tiledscene.h"
#include <QQmlEngine>


/**
 * @brief The RpgArrow class
 */

class RpgArrow : public IsometricBullet
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgArrow(QQuickItem *parent = nullptr);
	virtual ~RpgArrow() {}

	static RpgArrow* createBullet(TiledGame *game, TiledScene *scene);

protected:
	void load() override final;
	//void impactEvent(TiledObjectBase *base) override final;
};



/**
 * @brief The RpgArrowPickable class
 */

class RpgArrowPickable : public RpgPickableObject
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgArrowPickable(QQuickItem *parent = nullptr);

	void playerPick(RpgPlayer *player) override final;
	void playerThrow(RpgPlayer *player) override final;

protected:
	void load() override final;

};



#endif // RPGARROW_H
