/*
 * ---- Call of Suli ----
 *
 * rpgfireball.h
 *
 * Created on: 2024. 03. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgFireball
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

#ifndef RPGFIREBALL_H
#define RPGFIREBALL_H

#include "isometricbullet.h"
#include "rpgpickableobject.h"
#include <QQmlEngine>


/**
 * @brief The RpgFireball class
 */

class RpgFireball : public IsometricBullet
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgFireball(QQuickItem *parent = nullptr);
	virtual ~RpgFireball() {}

	static RpgFireball* createBullet(TiledGame *game, TiledScene *scene);

protected:
	void load() override final;
};



/**
 * @brief The RpgArrowPickable class
 */

class RpgFireballPickable : public RpgPickableObject
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgFireballPickable(QQuickItem *parent = nullptr);

	bool playerPick(RpgPlayer *player) override final;

protected:
	void load() override final;

};

#endif // RPGFIREBALL_H
