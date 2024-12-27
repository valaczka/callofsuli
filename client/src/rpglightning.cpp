/*
 * ---- Call of Suli ----
 *
 * rpglightning.cpp
 *
 * Created on: 2024. 07. 24.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgLightning
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

#include "rpglightning.h"
#include "rpgplayer.h"
#include "rpggame.h"
#include "tiledscene.h"
#include "isometricbullet_p.h"

/**
 * @brief RpgLightning::RpgLightning
 * @param parent
 */

RpgLightning::RpgLightning(TiledScene *scene)
	: IsometricBullet(scene)
{
	m_maxDistance = 500.;
	m_speed = 30.;
}


/**
 * @brief RpgLightning::createBullet
 * @param game
 * @param scene
 * @return
 */

RpgLightning *RpgLightning::createBullet(TiledScene *scene)
{
	RpgLightning *bullet = new RpgLightning(scene);
	bullet->initialize(20.);
	return bullet;
}


/**
 * @brief RpgLightning::load
 */

void RpgLightning::load()
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

	appendSprite(QStringLiteral(":/rpg/lightning/lightning.png"), json);

	setWidth(64);
	setHeight(64);
	setBodyOffset(0, 50);

	jumpToSprite("default");
}



/**
 * @brief RpgLightning::impactEvent
 * @param base
 */

void RpgLightning::impactEvent(TiledObject *base)
{
	RpgGame *game = d->owner() ? qobject_cast<RpgGame*>(d->owner()->game()) : nullptr;

	if (!game) {
		LOG_CWARNING("game") << "Missing owner, bullet automatic impact event failed";
		return;
	}

	IsometricEnemy *enemy = qobject_cast<IsometricEnemy*>(base);

	if (!enemy)
		return;

	// Ha kérdés van, akkor megsemmisül, egyébként megy tovább

	if (!game->playerAttackEnemy(d->owner(), enemy, d->fromWeaponType()))
	{
		setImpacted(true);
		stop();
		setCurrentDirection(Invalid);
		doAutoDelete();
	}
}




/**
 * @brief RpgLightningWeapon::RpgLightningWeapon
 * @param parent
 */

RpgLightningWeapon::RpgLightningWeapon(QObject *parent)
	: TiledWeapon{WeaponLightningWeapon, parent}
{

}



/**
 * @brief RpgLightningWeapon::createBullet
 * @param distance
 * @return
 */

IsometricBullet *RpgLightningWeapon::createBullet(const qreal &distance)
{
	if (!m_parentObject) {
		LOG_CERROR("game") << "Missing parent object" << this;
		return nullptr;
	}

	RpgLightning *fb = RpgLightning::createBullet(m_parentObject->scene());

	if (fb && distance > 0.) {
		if (distance < 1.)
			fb->setMaxDistance(fb->maxDistance() * distance);
		else
			fb->setMaxDistance(distance);
	}

	return fb;
}




/**
 * @brief RpgLightningWeapon::eventAttack
 * @param target
 */

void RpgLightningWeapon::eventAttack(TiledObject */*target*/)
{
	if (!m_sfx && m_parentObject) {
		m_sfx.reset(new TiledGameSfx(m_parentObject));
		m_sfx->setSoundList({
								QStringLiteral(":/rpg/lightning/lightning1.mp3"),
								QStringLiteral(":/rpg/lightning/lightning2.mp3"),
							});
	}

	if (m_sfx)
		m_sfx->playOne();
}
