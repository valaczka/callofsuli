/*
 * ---- Call of Suli ----
 *
 * rpgfireball.cpp
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

#include "rpgfireball.h"
#include "rpgplayer.h"
#include "tiledscene.h"


/**
 * @brief RpgFireball::RpgFireball
 * @param parent
 */

RpgFireball::RpgFireball(TiledScene *scene)
	: RpgBullet(RpgGameData::Weapon::WeaponLongbow, scene)
{

}




/**
 * @brief RpgFireball::load
 */

void RpgFireball::load()
{
	createVisual();
	setAvailableDirections(Direction_8);

	static const QByteArray sprite = R"(
		{
			"name": "default",
			"directions": [ 270, 315, 360, 45, 90, 135, 180, 225 ],
			"x": 0,
			"y": 0,
			"count": 4,
			"width": 64,
			"height": 64,
			"duration": 60
		}
	)";


	IsometricObjectSprite json;
	json.fromJson(QJsonDocument::fromJson(sprite).object());

	appendSprite(QStringLiteral(":/rpg/fireball/fireball.png"), json);

	m_visualItem->setWidth(64);
	m_visualItem->setHeight(64);
	setBodyOffset(0, 50);

	jumpToSprite("default");
}


