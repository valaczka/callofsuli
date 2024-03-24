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
#include "rpglongbow.h"


/**
 * @brief RpgFireball::RpgFireball
 * @param parent
 */

RpgFireball::RpgFireball(QQuickItem *parent)
	: IsometricBullet(parent)
{

}


/**
 * @brief RpgFireball::createBullet
 * @param game
 * @param scene
 * @return
 */

RpgFireball *RpgFireball::createBullet(TiledGame *game, TiledScene *scene)
{
	RpgFireball *bullet = nullptr;
	TiledObjectBase::createFromCircle<RpgFireball>(&bullet, QPointF{}, 20, nullptr, scene);

	if (bullet) {
		bullet->m_body->setBullet(true);
		bullet->setGame(game);
		bullet->setScene(scene);
		bullet->initialize();
	}

	return bullet;
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

	setWidth(64);
	setHeight(64);
	setBodyOffset(0, 50);

	jumpToSprite("default");
}



/**
 * @brief RpgFireballPickable::RpgFireballPickable
 * @param parent
 */

RpgFireballPickable::RpgFireballPickable(QQuickItem *parent)
	: RpgPickableObject(PickableFireball, parent)
{
	m_activateEffect.reset(new TiledEffectSpark(TiledEffectSpark::SparkAllOrange, this));
}



/**
 * @brief RpgFireballPickable::playerPick
 * @param player
 */

void RpgFireballPickable::playerPick(RpgPlayer *player)
{
	if (!player)
		return;

	static const int num = 35;

	TiledWeapon *weapon = player->armory()->weaponFind(TiledWeapon::WeaponLongbow);

	if (!weapon)
		weapon = player->armory()->weaponAdd(new RpgLongbow);

	weapon->setBulletCount(weapon->bulletCount()+num);

	if (m_game)
		m_game->message(tr("%1 fireballs gained").arg(num));

	player->armory()->setCurrentWeapon(weapon);
}



/**
 * @brief RpgFireballPickable::playerThrow
 * @param player
 */

void RpgFireballPickable::playerThrow(RpgPlayer *player)
{

}



/**
 * @brief RpgFireballPickable::load
 */

void RpgFireballPickable::load()
{
	loadDefault(QStringLiteral("fireball"));
}
