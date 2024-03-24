/*
 * ---- Call of Suli ----
 *
 * rpgenemybase.cpp
 *
 * Created on: 2024. 03. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgEnemyBase
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

#include "rpgenemybase.h"
#include "rpggame.h"
#include "tiledspritehandler.h"

/**
 * @brief RpgEnemyBase::RpgEnemyBase
 * @param parent
 */

RpgEnemyBase::RpgEnemyBase(const RpgEnemyType &type, QQuickItem *parent)
	: IsometricEnemy(parent)
	, RpgEnemyIface(type)
{
	m_armory.reset(new RpgArmory(this));

	m_metric.speed = 2.;
	m_metric.runSpeed = 3.;
	m_metric.returnSpeed = 4.;
	m_metric.pursuitSpeed = 6.;

	/*m_sfxPain.setSoundList({
							   QStringLiteral(":/rpg/werebear/monster-5.mp3"),
						   });

	m_sfxPain.setPlayOneDeadline(600);


	m_sfxFootStep.setSoundList({
								   QStringLiteral(":/rpg/werebear/stepdirt_7.mp3"),
								   QStringLiteral(":/rpg/werebear/stepdirt_8.mp3"),
							   });
	m_sfxFootStep.setVolume(0.4);
	m_sfxFootStep.setInterval(450);*/

	m_moveDisabledSpriteList = {
		QStringLiteral("attack"),
		QStringLiteral("bow"),
		QStringLiteral("cast"),
		QStringLiteral("hurt"),
		QStringLiteral("death")
	};


	/*
	connect(this, &RpgWerebear::hurt, &m_sfxPain, &TiledGameSfx::playOne);
	//connect(this, &RpgPlayer::healed, this, &RpgPlayer::playHealedEffect);
	//connect(this, &RpgPlayer::becameAlive, this, &RpgPlayer::playAliveEffect);
	connect(this, &RpgWerebear::becameDead, this, &RpgWerebear::playDeadEffect);
	connect(this, &RpgWerebear::playerChanged, this, &RpgWerebear::playSeeEffect);*/

}

RpgEnemyBase::~RpgEnemyBase()
{

}



/**
 * @brief RpgEnemyBase::defaultWeapon
 * @return
 */

TiledWeapon *RpgEnemyBase::defaultWeapon() const
{
	return m_armory->currentWeapon();
}




/**
 * @brief RpgEnemyBase::updateSprite
 */

void RpgEnemyBase::updateSprite()
{
	if (m_hp <= 0) {
		jumpToSprite("death", m_currentDirection);
		return;
	}

	if (m_spriteHandler->currentSprite() == "attack" ||
			m_spriteHandler->currentSprite() == "bow" ||
			m_spriteHandler->currentSprite() == "hurt" ||
			m_spriteHandler->currentSprite() == "cast")
		jumpToSpriteLater("idle", m_currentDirection);
	else if (m_movingDirection != Invalid) {
		if (m_metric.runSpeed >= 0. && m_movingSpeed >= m_metric.runSpeed)
			jumpToSprite("run", m_movingDirection);
		else
			jumpToSprite("walk", m_movingDirection);
	} else
		jumpToSprite("idle", m_currentDirection);
}



/**
 * @brief RpgEnemyBase::load
 */

void RpgEnemyBase::load()
{
	setMaxHp(5);
	setHp(5);

	setAvailableDirections(Direction_8);

	for (int i=0; i<=2; ++i)
	{
		IsometricObjectLayeredSprite json;
		json.fromJson(RpgGame::baseEntitySprite(i));
		json.layers.insert(QStringLiteral("default"), QStringLiteral("_sprite%1.png").arg(i));
		RpgArmory::fillAvailableLayers(&json, i);
		appendSprite(json, QStringLiteral(":/rpg/")+m_directory+QStringLiteral("/"));
	}


	setWidth(148);
	setHeight(130);
	setBodyOffset(0, 0.45*64);

	//connect(m_spriteHandler, &TiledSpriteHandler::currentSpriteChanged, this, &RpgWerebear::onCurrentSpriteChanged);
}



/**
 * @brief RpgEnemyBase::attackedByPlayer
 * @param player
 * @param weaponType
 */

void RpgEnemyBase::attackedByPlayer(IsometricPlayer *player, const TiledWeapon::WeaponType &weaponType)
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
		startInabililty();
	}
}




/**
 * @brief RpgEnemyBase::getNewHpAfterAttack
 * @param origHp
 * @param weaponType
 * @param player
 * @return
 */

int RpgEnemyBase::getNewHpAfterAttack(const int &origHp, const TiledWeapon::WeaponType &weaponType, IsometricPlayer */*player*/) const
{
	int hp = origHp;

	switch (weaponType) {
		case TiledWeapon::WeaponLongsword:
			hp -= 1;
			break;

		case TiledWeapon::WeaponShortbow:
			hp -= 2;
			break;

		case TiledWeapon::WeaponLongbow:
			hp -= 3;
			break;

		case TiledWeapon::WeaponHand:
		case TiledWeapon::WeaponGreatHand:
		case TiledWeapon::WeaponShield:
		case TiledWeapon::WeaponInvalid:
			break;
	}

	return hp;
}



/**
 * @brief RpgEnemyBase::playAttackEffect
 * @param weapon
 */

void RpgEnemyBase::playAttackEffect(TiledWeapon *weapon)
{
	if (!weapon)
		return;

	if (const QString &sprite = RpgGame::getAttackSprite(weapon->weaponType()); !sprite.isEmpty()) {
		jumpToSprite(sprite.toLatin1(), m_currentDirection);
	}
}




/**
 * @brief RpgEnemyBase::getPickablePosition
 * @return
 */

QPointF RpgEnemyBase::getPickablePosition() const
{
	QLineF line = QLineF::fromPolar(75., toDegree(directionToIsometricRaidan(m_currentDirection)));
	line.translate(m_body->bodyPosition()-line.p2());
	return line.p1();
}





/**
 * @brief RpgEnemyBase::throwableWeapons
 * @return
 */

QList<TiledWeapon *> RpgEnemyBase::throwableWeapons() const
{
	QList<TiledWeapon*> list;

	for (TiledWeapon *w : std::as_const(*m_armory->weaponList())) {
		if (w->canThrow() || w->canThrowBullet())
			list.append(w);
	}

	return list;
}



/**
 * @brief RpgEnemyBase::throwWeapon
 * @param weapon
 */

void RpgEnemyBase::throwWeapon(TiledWeapon *weapon)
{
	m_armory->weaponRemove(weapon);
}



/**
 * @brief RpgEnemyBase::protectWeapon
 * @param weaponType
 * @return
 */

bool RpgEnemyBase::protectWeapon(const TiledWeapon::WeaponType &weaponType)
{
	for (TiledWeapon *w : std::as_const(*m_armory->weaponList())) {
		if (w->canProtect(weaponType) && w->protect(weaponType))
			return true;
	}

	return false;
}


/**
 * @brief RpgEnemyBase::onCurrentSpriteChanged
 */

void RpgEnemyBase::onCurrentSpriteChanged()
{

}
