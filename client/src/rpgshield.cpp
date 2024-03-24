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
			setBulletCount(m_bulletCount-1);
			eventProtect();
			return true;

		case TiledWeapon::WeaponLongbow:
			setBulletCount(m_bulletCount-3);
			eventProtect();
			return true;

		case TiledWeapon::WeaponGreatHand:
			setBulletCount(0);
			eventProtect();
			return true;

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
			return (m_bulletCount > 0);

		case TiledWeapon::WeaponLongbow:
			return (m_bulletCount > 2);

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
	m_activateEffect.reset(new TiledEffectSpark(TiledEffectSpark::SparkAllOrange, this));
}




/**
 * @brief RpgShieldPickable::playerPick
 * @param player
 */

void RpgShieldPickable::playerPick(RpgPlayer *player)
{
	if (!player)
		return;

	static const int num = 5;

	TiledWeapon *weapon = player->armory()->weaponFind(TiledWeapon::WeaponShield);

	if (!weapon)
		weapon = player->armory()->weaponAdd(new RpgShield);

	weapon->setBulletCount(weapon->bulletCount()+num);

	if (m_game)
		m_game->message(tr("%1 shield gained").arg(num));
}




/**
 * @brief RpgShieldPickable::playerThrow
 * @param player
 */

void RpgShieldPickable::playerThrow(RpgPlayer *player)
{

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

