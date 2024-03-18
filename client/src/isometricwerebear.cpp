/*
 * ---- Call of Suli ----
 *
 * isometricwerebear.cpp
 *
 * Created on: 2024. 03. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricWerebear
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

#include "isometricwerebear.h"
#include "tiledspritehandler.h"
#include "utils_.h"





/**
 * @brief IsometricWerebear::IsometricWerebear
 * @param parent
 */

IsometricWerebear::IsometricWerebear(QQuickItem *parent)
	: IsometricEnemy(EnemyWerebear, parent)
	, m_sfxFootStep(this)
{
	m_metric.speed = 2.;
	m_metric.returnSpeed = 3.;
	m_metric.pursuitSpeed = 3.;
	m_autoAttackSprite = "hit";

	m_sfxFootStep.setSoundList({
								   QStringLiteral(":/enemies/werebear/stepdirt_7.wav"),
								   QStringLiteral(":/enemies/werebear/stepdirt_8.wav"),
							   });
	m_sfxFootStep.setVolume(0.4);
	m_sfxFootStep.setInterval(450);

	m_defaultWeapon.reset(new IsometricWerebearWeaponHand);
	m_defaultWeapon->setParentObject(this);
}


/**
 * @brief IsometricWerebear::~IsometricWerebear
 */

IsometricWerebear::~IsometricWerebear()
{

}






/**
 * @brief IsometricWerebear::load
 */

void IsometricWerebear::load()
{
	setMaxHp(5);
	setHp(5);

	setAvailableDirections(Direction_8);

	const char *file = nullptr;
	const char *data = "data.json";

	switch (m_werebearType) {
		case WerebearBrownArmor:
			file = "werebear_brown_armor.png";
			break;
		case WerebearBrownBare:
			file = "werebear_brown_bare.png";
			break;
		case WerebearBrownShirt:
			file = "werebear_brown_shirt.png";
			break;
		case WerebearWhiteArmor:
			file = "werebear_white_armor.png";
			break;
		case WerebearWhiteBare:
			file = "werebear_white_bare.png";
			break;
		case WerebearWhiteShirt:
			file = "werebear_white_shirt.png";
			break;
		case WerebearDefault:
			file = "werebear_0.png";
			data = "data0.json";
			break;
	}

	const auto &ptr = Utils::fileToJsonObject(QStringLiteral(":/enemies/werebear/").append(data));

	if (!ptr) {
		LOG_CERROR("game") << "Resource load error";
		return;
	}

	IsometricObjectSpriteList json;
	json.fromJson(*ptr);

	appendSprite(QStringLiteral(":/enemies/werebear/").append(file), json);

	setWidth(128);
	setHeight(128);
	setBodyOffset(0, 0.45*64);


	connect(m_spriteHandler, &TiledSpriteHandler::currentSpriteChanged, this, &IsometricWerebear::onCurrentSpriteChanged);
}





/**
 * @brief IsometricWerebear::attackedByPlayer
 * @param player
 * @param weaponType
 */

void IsometricWerebear::attackedByPlayer(IsometricPlayer *player, const TiledWeapon::WeaponType &weaponType)
{
	IsometricEnemy::attackedByPlayer(player, weaponType);

	if (!isAlive())
		return;

	int newHp = m_hp;

	switch (weaponType) {
		case TiledWeapon::WeaponSword:
			newHp -= 1;
			break;

		case TiledWeapon::WeaponShortbow:
			newHp -= 2;
			break;

		case TiledWeapon::WeaponHand:
		case TiledWeapon::WeaponShield:
		case TiledWeapon::WeaponInvalid:
			break;
	}

	if (newHp == m_hp)
		return;

	setHp(std::max(0, newHp));

	if (m_hp <= 0) {
		jumpToSprite("die", m_currentDirection);
	} else {
		jumpToSprite("hurt", m_currentDirection);
		startInabililty();
	}
}






/**
 * @brief IsometricWerebear::onCurrentSpriteChanged
 */

void IsometricWerebear::onCurrentSpriteChanged()
{
	const QString &sprite = m_spriteHandler->currentSprite();

	if (sprite == QStringLiteral("run"))
		m_sfxFootStep.startFromBegin();
	else if (sprite != QStringLiteral("run"))
		m_sfxFootStep.stop();
}





IsometricWerebear::WerebearType IsometricWerebear::werebearType() const
{
	return m_werebearType;
}

void IsometricWerebear::setWerebearType(const WerebearType &newWerebearType)
{
	if (m_werebearType == newWerebearType)
		return;
	m_werebearType = newWerebearType;
	emit werebearTypeChanged();
}


/**
 * @brief IsometricWerebear::setWerebearType
 * @param text
 */

void IsometricWerebear::setWerebearType(const QString &text)
{
	static const QMap<QString, WerebearType> map = {
		{ QStringLiteral("brownArmor"),	WerebearBrownArmor },
		{ QStringLiteral("brownBare"),	WerebearBrownBare },
		{ QStringLiteral("brownShirt"),	WerebearBrownShirt },
		{ QStringLiteral("whiteArmor"),	WerebearWhiteArmor },
		{ QStringLiteral("whiteBare"),	WerebearWhiteBare },
		{ QStringLiteral("whiteShirt"),	WerebearWhiteShirt },
	};

	setWerebearType(map.value(text, WerebearDefault));
}



/**
 * @brief IsometricWerebearWeaponHand::eventAttack
 */

void IsometricWerebearWeaponHand::eventAttack()
{
	IsometricWerebear *wb = qobject_cast<IsometricWerebear*>(m_parentObject.get());
	if (wb && wb->game())
		wb->game()->playSfx(QStringLiteral(":/enemies/werebear/big_punch.wav"),
							wb->scene(), wb->body()->bodyPosition());
}
