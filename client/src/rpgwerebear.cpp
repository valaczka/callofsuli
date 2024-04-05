/*
 * ---- Call of Suli ----
 *
 * isometricwerebear.cpp
 *
 * Created on: 2024. 03. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgWerebear
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

#include "rpgwerebear.h"
#include "tiledspritehandler.h"
#include "utils_.h"




/**
 * @brief RpgWerebear::RpgWerebear
 * @param parent
 */

RpgWerebear::RpgWerebear(QQuickItem *parent)
	: IsometricEnemy(parent)
	, RpgEnemyIface(EnemyWerebear)
	, m_sfxFootStep(this)
	, m_sfxPain(this)
	, m_effectHealed(this)
	, m_weaponHand(new RpgWerebearWeaponHand)
{
	m_metric.speed = 2.;
	m_metric.returnSpeed = 4.;
	m_metric.pursuitSpeed = 4.;

	m_sfxPain.setSoundList({
							   QStringLiteral(":/rpg/werebear/monster-5.mp3"),
						   });

	m_sfxPain.setPlayOneDeadline(600);


	m_sfxFootStep.setSoundList({
								   QStringLiteral(":/rpg/werebear/stepdirt_7.mp3"),
								   QStringLiteral(":/rpg/werebear/stepdirt_8.mp3"),
							   });
	m_sfxFootStep.setVolume(0.4);
	m_sfxFootStep.setInterval(450);

	m_moveDisabledSpriteList = {
		QStringLiteral("hurt"),
		QStringLiteral("death")
	};

	m_weaponHand->setParentObject(this);

	connect(this, &RpgWerebear::hurt, &m_sfxPain, &TiledGameSfx::playOne);
	//connect(this, &RpgWerebear::healed, this, [this]() { m_effectHealed.play(); });
	connect(this, &RpgWerebear::becameAlive, this, [this]() {
		m_effectHealed.play();
	});
	connect(this, &RpgWerebear::becameDead, this, &RpgWerebear::playDeadEffect);
	connect(this, &RpgWerebear::playerChanged, this, &RpgWerebear::playSeeEffect);
}


/**
 * @brief RpgWerebear::~RpgWerebear
 */

RpgWerebear::~RpgWerebear()
{

}






/**
 * @brief RpgWerebear::load
 */

void RpgWerebear::load()
{
	setAvailableDirections(Direction_8);

	const char *file = nullptr;
	const char *data = "data.json";
	int hp = 9;

	switch (m_werebearType) {
		case WerebearBrownArmor:
			file = "werebear_brown_armor.png";
			hp = 12;
			break;
		case WerebearBrownBare:
			file = "werebear_brown_bare.png";
			break;
		case WerebearBrownShirt:
			file = "werebear_brown_shirt.png";
			break;
		case WerebearWhiteArmor:
			file = "werebear_white_armor.png";
			hp = 12;
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

	setMaxHp(hp);
	setHp(hp);

	const auto &ptr = Utils::fileToJsonObject(QStringLiteral(":/rpg/werebear/").append(data));

	if (!ptr) {
		LOG_CERROR("game") << "Resource load error";
		return;
	}

	IsometricObjectSpriteList json;
	json.fromJson(*ptr);

	appendSprite(QStringLiteral(":/rpg/werebear/").append(file), json);

	setWidth(128);
	setHeight(128);
	setBodyOffset(0, 0.45*64);

	connect(m_spriteHandler, &TiledSpriteHandler::currentSpriteChanged, this, &RpgWerebear::onCurrentSpriteChanged);
}





/**
 * @brief RpgWerebear::attackedByPlayer
 * @param player
 * @param weaponType
 */

void RpgWerebear::attackedByPlayer(IsometricPlayer *player, const TiledWeapon::WeaponType &weaponType)
{
	IsometricEnemy::attackedByPlayer(player, weaponType);

	if (!isAlive())
		return;

	int newHp = getNewHpAfterAttack(m_hp, weaponType, player);

	if (newHp == m_hp)
		return;

	setHp(std::max(0, newHp));

	if (m_hp <= 0) {
		jumpToSprite("death", m_currentDirection);
	} else {
		jumpToSprite("hurt", m_currentDirection);
		if (weaponType != TiledWeapon::WeaponHand)
			startInabililty();
	}
}



/**
 * @brief RpgWerebear::getNewHpAfterAttack
 * @param origHp
 * @param weaponType
 * @param player
 * @return
 */

int RpgWerebear::getNewHpAfterAttack(const int &origHp, const TiledWeapon::WeaponType &weaponType, IsometricPlayer */*player*/) const
{
	int newHp = origHp;

	switch (weaponType) {
		case TiledWeapon::WeaponLongsword:
			newHp -= 1;
			break;

		case TiledWeapon::WeaponLongbow:
			newHp -= 3;
			break;

		case TiledWeapon::WeaponShortbow:
			newHp -= 2;
			break;

		case TiledWeapon::WeaponHand:
		case TiledWeapon::WeaponGreatHand:
		case TiledWeapon::WeaponShield:
		case TiledWeapon::WeaponInvalid:
			break;
	}

	return newHp;
}



/**
 * @brief RpgWerebear::playAttackEffect
 * @param weapon
 */

void RpgWerebear::playAttackEffect(TiledWeapon *weapon)
{
	if (!weapon)
		return;

	if (weapon->weaponType() == TiledWeapon::WeaponGreatHand || weapon->weaponType() == TiledWeapon::WeaponHand) {
		jumpToSprite("hit", m_currentDirection);
	}
}




/**
 * @brief RpgWerebear::playDeadEffect
 */

void RpgWerebear::playDeadEffect()
{
	m_game->playSfx(QStringLiteral(":/rpg/werebear/monster-6.mp3"), m_scene, m_body->bodyPosition());
}



/**
 * @brief RpgWerebear::playSeeEffect
 */

void RpgWerebear::playSeeEffect()
{
	if (m_player)
		m_game->playSfx(QStringLiteral(":/rpg/werebear/monster-1.mp3"), m_scene, m_body->bodyPosition());
}




/**
 * @brief RpgWerebear::getPickablePosition
 * @return
 */

QPointF RpgWerebear::getPickablePosition(const int &num) const
{
	QLineF line = QLineF::fromPolar(50. * num, toDegree(directionToIsometricRaidan(m_currentDirection)));
	line.translate(m_body->bodyPosition()-line.p2());
	return line.p1();
}



/**
 * @brief RpgWerebear::protectWeapon
 * @param weaponType
 * @return
 */

bool RpgWerebear::protectWeapon(const TiledWeapon::WeaponType &weaponType)
{
	if (m_weaponHand->canProtect(weaponType) && m_weaponHand->protect(weaponType))
		return true;

	return false;
}






/**
 * @brief RpgWerebear::onCurrentSpriteChanged
 */

void RpgWerebear::onCurrentSpriteChanged()
{
	const QString &sprite = m_spriteHandler->currentSprite();

	if (sprite == QStringLiteral("run"))
		m_sfxFootStep.startFromBegin();
	else if (sprite != QStringLiteral("run"))
		m_sfxFootStep.stop();
}





RpgWerebear::WerebearType RpgWerebear::werebearType() const
{
	return m_werebearType;
}

void RpgWerebear::setWerebearType(const WerebearType &newWerebearType)
{
	if (m_werebearType == newWerebearType)
		return;
	m_werebearType = newWerebearType;
	emit werebearTypeChanged();
}


/**
 * @brief RpgWerebear::setWerebearType
 * @param text
 */

void RpgWerebear::setWerebearType(const QString &text)
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
 * @brief RpgWerebear::defaultWeapon
 * @return
 */

TiledWeapon *RpgWerebear::defaultWeapon() const
{
	return m_weaponHand.get();
}



/**
 * @brief RpgWerebear::updateSprite
 */

void RpgWerebear::updateSprite()
{
	if (m_hp <= 0)
		jumpToSprite("death", m_currentDirection);
	else if (m_spriteHandler->currentSprite() == QStringLiteral("hit") || m_spriteHandler->currentSprite() == QStringLiteral("hurt"))
		jumpToSpriteLater("idle", m_currentDirection);
	else if (m_movingDirection != Invalid)
		jumpToSprite("run", m_movingDirection);
	else
		jumpToSprite("idle", m_currentDirection);
}



/**
 * @brief RpgWerebearWeaponHand::eventAttack
 */

RpgWerebearWeaponHand::RpgWerebearWeaponHand(QObject *parent)
	: TiledWeapon(TiledWeapon::WeaponGreatHand, parent)
{
	m_canHit = true;
	m_icon = QStringLiteral("qrc:/Qaterial/Icons/hand.svg");
}



/**
 * @brief RpgWerebearWeaponHand::eventAttack
 */

void RpgWerebearWeaponHand::eventAttack(TiledObject *)
{
	RpgWerebear *wb = qobject_cast<RpgWerebear*>(m_parentObject.get());
	if (wb && wb->game())
		wb->game()->playSfx(QStringLiteral(":/rpg/werebear/big_punch.mp3"),
							wb->scene(), wb->body()->bodyPosition());
}
