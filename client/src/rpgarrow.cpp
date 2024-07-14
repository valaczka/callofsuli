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
#include "rpgplayer.h"


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

	static const QByteArray sprite = R"(
		{
			"name": "default",
			"directions": [ 270, 315, 360, 45, 90, 135, 180, 225 ],
			"x": 0,
			"y": 0,
			"count": 1,
			"width": 64,
			"height": 64,
			"duration": 1,
			"loops": 1
		}
	)";


	IsometricObjectSprite json;
	json.fromJson(QJsonDocument::fromJson(sprite).object());

	appendSprite(QStringLiteral(":/rpg/arrow/arrow.png"), json);

	setWidth(64);
	setHeight(64);
	setBodyOffset(0, 50);

	jumpToSprite("default");
}





/**
 * @brief RpgArrowPickable::RpgArrowPickable
 * @param parent
 */

RpgArrowPickable::RpgArrowPickable(QQuickItem *parent)
	: RpgPickableObject(PickableArrow, parent)
{
	m_activateEffect.reset(new TiledEffectSpark(TiledEffectSpark::SparkAllOrange, this));
}



/**
 * @brief RpgArrowPickable::playerPick
 * @param player
 */

bool RpgArrowPickable::playerPick(RpgPlayer *player)
{
	if (!player)
		return false;

	static const int num = 10;

	TiledWeapon *weapon = player->armory()->weaponFind(TiledWeapon::WeaponShortbow);

	if (!weapon) {
		if (m_game)
			m_game->messageColor(tr("Shortbow missing"), QColor::fromRgbF(0.8, 0., 0.));
		return false;
	}
		//weapon = player->armory()->weaponAdd(new RpgShortbow);

	weapon->setBulletCount(weapon->bulletCount()+num);
	weapon->setPickedBulletCount(weapon->pickedBulletCount()+num);

	if (m_game)
		m_game->message(tr("%1 arrows gained").arg(num));

	//player->armory()->setCurrentWeapon(weapon);

	return true;
}





/**
 * @brief RpgArrowPickable::load
 */

void RpgArrowPickable::load()
{
	loadDefault(QStringLiteral("arrow"));
}

