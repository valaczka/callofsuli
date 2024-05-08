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
#include "rpglongsword.h"
#include "tiledspritehandler.h"

/**
 * @brief RpgEnemyBase::RpgEnemyBase
 * @param parent
 */

RpgEnemyBase::RpgEnemyBase(const RpgEnemyType &type, QQuickItem *parent)
	: IsometricEnemy(parent)
	, RpgEnemyIface(type)
	, m_effectHealed(this)
	, m_effectFire(this)
	, m_effectSleep(this)
{
	m_armory.reset(new RpgArmory(this));

	m_metric.speed = 2.;
	m_metric.runSpeed = 3.;
	m_metric.returnSpeed = 4.;
	m_metric.pursuitSpeed = 6.;

	m_moveDisabledSpriteList = QStringList{
							   QStringLiteral("attack"),
							   QStringLiteral("bow"),
							   QStringLiteral("cast"),
							   QStringLiteral("hurt"),
							   QStringLiteral("death")
};


	connect(this, &RpgEnemyBase::becameAlive, this, [this]() {
		m_effectHealed.play();
	});

	connect(this, &RpgEnemyBase::becameAsleep, this, [this]() {
		m_effectSleep.play();
	});

	connect(this, &RpgEnemyBase::becameAwake, this, [this]() {
		m_effectSleep.clear();
	});
}


/**
 * @brief RpgEnemyBase::~RpgEnemyBase
 */

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
	setMaxHp(9);
	setHp(9);

	setAvailableDirections(Direction_8);

	loadType();

	for (int i=0; i<=2; ++i)
	{
		IsometricObjectLayeredSprite json;
		json.fromJson(RpgGame::baseEntitySprite(i));
		json.layers.insert(QStringLiteral("default"), QStringLiteral("_sprite%1.png").arg(i));

		// Ez nem biztos, hogy kell, ha nincs elvehető fegyver

		//RpgArmory::fillAvailableLayers(&json, i);
		appendSprite(json, QStringLiteral(":/rpg/")+m_directory+QStringLiteral("/"));
	}


	setWidth(148);
	setHeight(130);
	setBodyOffset(0, 0.45*64);

	//connect(m_spriteHandler, &TiledSpriteHandler::currentSpriteChanged, this, &RpgWerebear::onCurrentSpriteChanged);

}



void RpgEnemyBase::eventPlayerReached(IsometricPlayer *player)
{
	rotateToPlayer(player);
}



/**
 * @brief RpgEnemyBase::attackedByPlayer
 * @param player
 * @param weaponType
 */

void RpgEnemyBase::attackedByPlayer(IsometricPlayer *player, const TiledWeapon::WeaponType &weaponType)
{
	if (isAlive() && weaponType == TiledWeapon::WeaponHand) {
		if (startSleeping())
			return;
	}

	IsometricEnemy::attackedByPlayer(player, weaponType);

	if (weaponType == TiledWeapon::WeaponLongbow)
		m_effectFire.play();

	if (!isAlive() || isSleeping())
		return;

	int newHp = getNewHpAfterAttack(m_hp, weaponType, player);

	if (newHp == m_hp)
		return;

	setHp(std::max(0, newHp));

	if (m_hp <= 0) {
		jumpToSprite("death", m_currentDirection);

		if (weaponType == TiledWeapon::WeaponLongbow) {
			if (RpgGame *game = qobject_cast<RpgGame*>(m_game))
				game->enemySetDieForever(this, true);
		}

		eventKilledByPlayer(player);
	} else {
		jumpToSprite("hurt", m_currentDirection);
		//if (weaponType != TiledWeapon::WeaponHand)
		//	startInabililty();
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
			hp -= 3;
			break;

		case TiledWeapon::WeaponShortbow:
			hp -= 3;
			break;

		case TiledWeapon::WeaponLongbow:
		case TiledWeapon::WeaponGreatHand:
			hp = 0;
			break;

		case TiledWeapon::WeaponHand:
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

QPointF RpgEnemyBase::getPickablePosition(const int &num) const
{
	QLineF line = QLineF::fromPolar(75. * num, toDegree(directionToIsometricRaidan(m_currentDirection)));
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
 * @brief RpgEnemyBase::canBulletImpact
 * @param type
 * @return
 */

bool RpgEnemyBase::canBulletImpact(const TiledWeapon::WeaponType &type) const
{
	if (m_enemyType == EnemySkeleton && type == TiledWeapon::WeaponShortbow)
		return false;

	return true;
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

QString RpgEnemyBase::subType() const
{
	return m_subType;
}

void RpgEnemyBase::setSubType(const QString &newSubType)
{
	m_subType = newSubType;
}


/**
 * @brief RpgEnemyBase::onCurrentSpriteChanged
 */

void RpgEnemyBase::onCurrentSpriteChanged()
{

}



/**
 * @brief RpgEnemyBase::loadType
 */

void RpgEnemyBase::loadType()
{
	if (m_enemyType == EnemySoldier || m_enemyType == EnemySoldierFix) {
		static const QString &strSoldier = QStringLiteral("soldier");

		if (m_subType.startsWith(strSoldier)) {
			m_directory = m_subType;
			m_directory.replace(strSoldier, QStringLiteral("enemySoldier"));
		}

		auto w = m_armory->weaponAdd(new RpgLongsword);
		w->setExcludeFromLayers(true);
		m_armory->setCurrentWeapon(w);
	} else if (m_enemyType == EnemyArcher || m_enemyType == EnemyArcherFix) {
		static const QString &strSoldier = QStringLiteral("archer");

		if (m_subType.startsWith(strSoldier)) {
			m_directory = m_subType;
			m_directory.replace(strSoldier, QStringLiteral("enemyArcher"));
		}

		auto w = m_armory->weaponAdd(new RpgShortbow);
		w->setExcludeFromLayers(true);
		w->setBulletCount(-1);
		m_armory->setCurrentWeapon(w);
		m_metric.firstAttackTime = 500;
		m_metric.autoAttackTime = 1250;
	} else if (m_enemyType == EnemySkeleton) {
		static const QString &strSoldier = QStringLiteral("skeleton");

		if (m_subType.startsWith(strSoldier)) {
			m_directory = m_subType;
			m_directory.replace(strSoldier, QStringLiteral("enemySkeleton"));
		}

		auto w = m_armory->weaponAdd(new RpgLongsword);
		w->setExcludeFromLayers(true);
		m_armory->setCurrentWeapon(w);
	}
}
