/*
 * ---- Call of Suli ----
 *
 * rpgshield.cpp
 *
 * Created on: 2024. 03. 19.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgShield
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

#include "rpgshield.h"
#include "rpgplayer.h"
#include "utils_.h"

RpgShield::RpgShield(QObject *parent)
	: TiledWeapon{WeaponShield, parent}
{
	m_icon = QStringLiteral("qrc:/Qaterial/Icons/shield.svg");
}


/**
 * @brief RpgShield::protect
 * @param weapon
 * @return
 */

bool RpgShield::protect(const WeaponType &weapon)
{
	switch (weapon) {
		case TiledWeapon::WeaponHand:
			eventProtect();
			return true;

		case TiledWeapon::WeaponShortbow:
		case TiledWeapon::WeaponLongsword:
		case TiledWeapon::WeaponDagger:
			setBulletCount(m_bulletCount-1);
			eventProtect();
			return true;

		case TiledWeapon::WeaponAxe:
			setBulletCount(m_bulletCount-3);
			eventProtect();
			return true;

		case TiledWeapon::WeaponMace:
		case TiledWeapon::WeaponHammer:
			setBulletCount(m_bulletCount-2);
			eventProtect();
			return true;

		case TiledWeapon::WeaponLongbow:
			setBulletCount(m_bulletCount-5);
			eventProtect();
			return true;

		case TiledWeapon::WeaponGreatHand:
		case TiledWeapon::WeaponBroadsword:
			setBulletCount(0);
			eventProtect();
			return true;

		case TiledWeapon::WeaponMageStaff:
		case TiledWeapon::WeaponLightningWeapon:
		case TiledWeapon::WeaponFireFogWeapon:
		case TiledWeapon::WeaponShield:
		case TiledWeapon::WeaponInvalid:
			break;
	}

	return false;
}



/**
 * @brief RpgShield::canProtect
 * @param weapon
 * @return
 */

bool RpgShield::canProtect(const WeaponType &weapon) const
{
	switch (weapon) {
		case TiledWeapon::WeaponHand:
		case TiledWeapon::WeaponShortbow:
		case TiledWeapon::WeaponGreatHand:
		case TiledWeapon::WeaponLongsword:
		case TiledWeapon::WeaponDagger:
			return (m_bulletCount > 0);

		case TiledWeapon::WeaponLongbow:
		case TiledWeapon::WeaponBroadsword:
		case TiledWeapon::WeaponAxe:
			return (m_bulletCount > 2);

		case TiledWeapon::WeaponHammer:
		case TiledWeapon::WeaponMace:
			return (m_bulletCount > 1);

		case TiledWeapon::WeaponMageStaff:
		case TiledWeapon::WeaponLightningWeapon:
		case TiledWeapon::WeaponFireFogWeapon:
		case TiledWeapon::WeaponShield:
		case TiledWeapon::WeaponInvalid:
			break;
	}

	return false;
}





/**
 * @brief RpgShieldPickable::RpgShieldPickable
 * @param parent
 */

RpgShieldPickable::RpgShieldPickable(QQuickItem *parent)
	: RpgPickableObject(PickableShield, parent)
{
	m_activateEffect.reset(new TiledEffectSpark(TiledEffectSpark::SparkAllBlue, this));
}




/**
 * @brief RpgShieldPickable::playerPick
 * @param player
 */

bool RpgShieldPickable::playerPick(RpgPlayer *player)
{
	if (!player)
		return false;

	pick(player, m_game);

	return true;
}





/**
 * @brief RpgShieldPickable::pick
 * @param player
 * @param game
 */

void RpgShieldPickable::pick(RpgPlayer *player, TiledGame *game)
{
	if (!player)
		return;

	static const int num = 2;

	TiledWeapon *weapon = player->armory()->weaponFind(TiledWeapon::WeaponShield);

	if (!weapon)
		weapon = player->armory()->weaponAdd(new RpgShield);

	weapon->setBulletCount(weapon->bulletCount()+num);
	player->armory()->updateLayers();
	player->setShieldCount(player->armory()->getShieldCount());

	if (game)
		game->message(tr("%1 shields gained").arg(num));
}





/**
 * @brief RpgShieldPickable::load
 */

void RpgShieldPickable::load()
{
	const auto &ptr = Utils::fileToJsonObject(QStringLiteral(":/rpg/shield/pickable.json"));

	if (!ptr) {
		LOG_CERROR("game") << "Resource load error";
		return;
	}

	TiledObjectSpriteList json;
	json.fromJson(*ptr);

	appendSprite(QStringLiteral(":/rpg/shield/pickable.png"), json);

	setWidth(json.sprites.first().width);
	setHeight(json.sprites.first().height);
	setBodyOffset(0, 0.75 * json.sprites.first().height/2.);

	//connect(m_spriteHandler, &TiledSpriteHandler::currentSpriteChanged, this, &RpgWerebear::onCurrentSpriteChanged);

	jumpToSprite("default");
}


/**
 * @brief RpgShieldPickable::onActivated
 */

void RpgShieldPickable::onActivated()
{
	RpgPickableObject::onActivated();
	jumpToSprite("spring");
	jumpToSpriteLater("default");
}

