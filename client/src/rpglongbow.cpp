/*
 * ---- Call of Suli ----
 *
 * rpglongbow.cpp
 *
 * Created on: 2024. 03. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgLongbow
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

#include "rpglongbow.h"
#include "rpgfireball.h"
#include "rpgplayer.h"

RpgLongbow::RpgLongbow(QObject *parent)
	: TiledWeapon{WeaponLongbow, parent}
{
	m_icon = QStringLiteral("qrc:/internal/medal/Icon.2_76.png");
}


/**
 * @brief RpgLongbow::createBullet
 * @return
 */

IsometricBullet *RpgLongbow::createBullet(const qreal &distance)
{
	if (!m_parentObject) {
		LOG_CERROR("game") << "Missing parent object" << this;
		return nullptr;
	}

	RpgFireball *fb = RpgFireball::createBullet(m_parentObject->game(), m_parentObject->scene());

	if (fb && distance > 0.) {
		if (distance < 1.)
			fb->setMaxDistance(fb->maxDistance() * distance);
		else
			fb->setMaxDistance(distance);
	}

	return fb;
}



/**
 * @brief RpgLongbow::eventAttack
 */

void RpgLongbow::eventAttack(TiledObject *)
{
	if (!m_parentObject) {
		LOG_CERROR("game") << "Missing parent object" << this;
		return;
	}

	if (m_disableTimerRepeater) {
		if (m_effectPlayed)
			return;

		m_effectPlayed = true;
	}

	TiledObject *p = m_parentObject.data();

	if (TiledGame *g = p->game()) {
		g->playSfx(QStringLiteral(":/rpg/longbow/fireball.mp3"), p->scene(), p->body()->bodyPosition());
	}
}



/**
 * @brief RpgLongbowPickable::RpgLongbowPickable
 * @param parent
 */

RpgLongbowPickable::RpgLongbowPickable(QQuickItem *parent)
	: RpgPickableObject(PickableLongbow, parent)
{
	m_activateEffect.reset(new TiledEffectSpark(TiledEffectSpark::SparkAllOrange, this));
}



/**
 * @brief RpgLongbowPickable::playerPick
 * @param player
 */

bool RpgLongbowPickable::playerPick(RpgPlayer *player)
{
	if (!player)
		return false;

	static const int num = 5;

	TiledWeapon *weapon = player->armory()->weaponFind(TiledWeapon::WeaponLongbow);

	if (!weapon)
		weapon = player->armory()->weaponAdd(new RpgLongbow);

	weapon->setBulletCount(weapon->bulletCount()+num);
	weapon->setPickedBulletCount(weapon->pickedBulletCount()+num);

	if (m_game)
		m_game->message(tr("1 longbow gained"));

	player->armory()->setCurrentWeaponIf(weapon, TiledWeapon::WeaponHand);

	return true;
}




/**
 * @brief RpgLongbowPickable::load
 */

void RpgLongbowPickable::load()
{
	loadDefault(QStringLiteral("longbow"));
}
