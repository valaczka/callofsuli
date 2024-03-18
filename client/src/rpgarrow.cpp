/*
 * ---- Call of Suli ----
 *
 * rpgarrow.cpp
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

#include "rpgarrow.h"
#include "tiledspritehandler.h"


RpgArrow::RpgArrow(QQuickItem *parent)
	: IsometricBullet(parent)
{

}



/**
 * @brief RpgArrow::createBullet
 * @param game
 * @param scene
 * @return
 */

RpgArrow *RpgArrow::createBullet(TiledGame *game, TiledScene *scene)
{
	RpgArrow *bullet = nullptr;
	TiledObjectBase::createFromCircle<RpgArrow>(&bullet, QPointF{}, 20, nullptr, scene);

	if (bullet) {
		bullet->m_body->setBullet(true);
		bullet->setGame(game);
		bullet->setScene(scene);
		bullet->initialize();
	}

	return bullet;
}



/**
 * @brief RpgArrow::load
 */

void RpgArrow::load()
{
	createVisual();
	setAvailableDirections(Direction_8);

	QString test = R"({
					   "layers": {
		"none": "lightning.png",
		"arrow": "arrows.png"
	},
	"sprites": [
		{
			"name": "base",
			"directions": [ 270, 315, 360, 45, 90, 135, 180, 225 ],
			"x": 0,
			"y": 0,
			"count": 4,
			"width": 64,
			"height": 64,
			"duration": 30
		}
	]
	})";


	IsometricObjectLayeredSprite json;
	json.fromJson(QJsonDocument::fromJson(test.toUtf8()).object());

	appendSprite(json, ":/");

	setWidth(64);
	setHeight(64);
	setBodyOffset(0, 25);

	m_spriteHandler->setVisibleLayers({"none"});
}



