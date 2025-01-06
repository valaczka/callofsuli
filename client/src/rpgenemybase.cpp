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
#include "utils_.h"

#include "rpgaxe.h"
#include "rpghammer.h"
#include "rpgmace.h"

/**
 * @brief RpgEnemyBase::RpgEnemyBase
 * @param parent
 */

RpgEnemyBase::RpgEnemyBase(const RpgGameData::Enemy::EnemyType &type, TiledScene *scene)
	: IsometricEnemy(scene)
	, RpgEnemyIface(type)
	, m_effectHealed(this)
	, m_effectFire(this)
	, m_effectSleep(this)
{
	m_armory.reset(new RpgArmory(this));

	setMetric(RpgGame::defaultEnemyMetric().soldier.value(QStringLiteral("default")));

	if (m_enemyType == RpgGameData::Enemy::EnemyArcher || m_enemyType == RpgGameData::Enemy::EnemyArcherFix) {
		setMetric(RpgGame::defaultEnemyMetric().archer.value(QStringLiteral("default")));
	}

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

	connect(this, &RpgEnemyBase::playerChanged, this, [this]() {
		if (m_player && !m_sfxRoar.soundList().isEmpty())
			m_sfxRoar.playOne();
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
	if (!isAlive() || isSleeping()) {
		jumpToSprite("death", m_facingDirection);
		return;
	}

	if (m_spriteHandler->currentSprite() == QStringLiteral("attack") ||
			m_spriteHandler->currentSprite() == QStringLiteral("bow") ||
			m_spriteHandler->currentSprite() == QStringLiteral("hurt") ||
			m_spriteHandler->currentSprite() == QStringLiteral("cast") )
		jumpToSpriteLater("idle", m_facingDirection);
	else if (isRunning() && m_facingDirection != Invalid)
		jumpToSprite("run", m_facingDirection);
	else if (isWalking() && m_facingDirection != Invalid)
		jumpToSprite("walk", m_facingDirection);
	else
		jumpToSprite("idle", m_facingDirection);
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

	RpgGame::loadBaseTextureSprites(m_spriteHandler, QStringLiteral(":/enemy/")+m_directory+QStringLiteral("/"));

	m_spriteHandler->setStartFrameSeed();

	setWidth(148);
	setHeight(130);
	setBodyOffset(0, 0.45*64);

	QStringList soundList;

	for (int i=0; i<10; ++i) {
		const QString fname = i == 0 ? QStringLiteral(":/enemy/")+m_directory+QStringLiteral("/roar.mp3")
									 : QStringLiteral(":/enemy/")+m_directory+QStringLiteral("/roar-%1.mp3").arg(i);

		if (QFile::exists(fname))
			soundList.append(fname);
	}

	m_sfxRoar.setSoundList(soundList);
	m_sfxRoar.setPlayOneDeadline(600);

	const auto ptr = Utils::fileToJsonObject(QStringLiteral(":/enemy/")+m_directory+QStringLiteral("/metric.json"));

	if (ptr) {
		LOG_CTRACE("game") << "Enemy metric override" << m_directory;

		m_metric.fromJson(ptr.value());
		if (sensorPolygon())
			setSensorPolygon(m_metric.sensorLength, m_metric.sensorRange);
	}

}



/**
 * @brief RpgEnemyBase::eventPlayerReached
 * @param player
 */


void RpgEnemyBase::eventPlayerReached(IsometricPlayer *player)
{
	RpgPlayer *p = qobject_cast<RpgPlayer*>(player);

	// Skip rotate on dagger

	if (p && p->armory()->currentWeapon() &&
			p->armory()->currentWeapon()->weaponType() == TiledWeapon::WeaponDagger) {
		return;
	}

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

	if (weaponType == TiledWeapon::WeaponLongbow || weaponType == TiledWeapon::WeaponFireFogWeapon)
		m_effectFire.play();

	if (!isAlive() || isSleeping())
		return;

	int newHp = getNewHpAfterAttack(m_hp, weaponType, player);

	if (newHp == m_hp)
		return;

	setHp(std::max(0, newHp));

	if (newHp <= 0) {
		jumpToSprite("death", m_facingDirection);
		eventKilledByPlayer(player);
	} else {
		jumpToSprite("hurt", m_facingDirection);

		if (weaponType == TiledWeapon::WeaponBroadsword || weaponType == TiledWeapon::WeaponAxe)
			startInability();
	}

}




/**
 * @brief RpgEnemyBase::getNewHpAfterAttack
 * @param origHp
 * @param weaponType
 * @param player
 * @return
 */

int RpgEnemyBase::getNewHpAfterAttack(const int &origHp, const TiledWeapon::WeaponType &weaponType, IsometricPlayer *player) const
{
	int hp = origHp;

	switch (weaponType) {
		case TiledWeapon::WeaponDagger:
			if (!m_player || m_player != player) {
				hp = 0;
			} else {
				hp -= 1;
			}
			break;

		case TiledWeapon::WeaponLongsword:
			hp -= 2;
			break;

		case TiledWeapon::WeaponShortbow:
			hp -= 3;
			break;

		case TiledWeapon::WeaponAxe:
			hp -= 3;
			break;

		case TiledWeapon::WeaponMace:
			hp -= 1;
			break;

		case TiledWeapon::WeaponHammer:
			hp -= 2;
			break;

		case TiledWeapon::WeaponLongbow:
		case TiledWeapon::WeaponLightningWeapon:
		case TiledWeapon::WeaponFireFogWeapon:
		case TiledWeapon::WeaponGreatHand:
			hp = 0;
			break;

		case TiledWeapon::WeaponBroadsword:
			hp = hp > 1 ? 1 : 0;
			break;

		case TiledWeapon::WeaponMageStaff:
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
		jumpToSprite(sprite.toLatin1(), m_facingDirection);
	}
}




/**
 * @brief RpgEnemyBase::getPickablePosition
 * @return
 */

QPointF RpgEnemyBase::getPickablePosition(const int &num) const
{
	return bodyPosition() - TiledObject::vectorFromAngle(directionToIsometricRadian(m_facingDirection), 75. *num).toPointF();
}





/**
 * @brief RpgEnemyBase::canBulletImpact
 * @param type
 * @return
 */

bool RpgEnemyBase::canBulletImpact(const TiledWeapon::WeaponType &type) const
{
	if (m_enemyType == RpgGameData::Enemy::EnemySkeleton && type == TiledWeapon::WeaponShortbow)
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
 * @brief RpgEnemyBase::serialize
 * @return
 */

std::unique_ptr<RpgGameData::Body> RpgEnemyBase::serialize() const
{
	RpgGameData::Enemy *enemy = new RpgGameData::Enemy(m_enemyType, objectId().sceneId, objectId().id);


	enemy->a = currentAngle();
	enemy->p = toPosList(bodyPosition());
	enemy->hp = m_hp;
	enemy->mhp = m_maxHp;


	return std::unique_ptr<RpgGameData::Body>(std::move(enemy));
}


/**
 * @brief RpgEnemyBase::deserialize
 * @param from
 * @return
 */

bool RpgEnemyBase::deserialize(const RpgGameData::Body *from) const
{

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
	m_directory = directoryBaseName(m_enemyType, m_subType);

	TiledWeapon *w = nullptr;

	if (m_enemyType == RpgGameData::Enemy::EnemySoldier || m_enemyType == RpgGameData::Enemy::EnemySoldierFix)
		w = m_armory->weaponAdd(new RpgLongsword);
	else if (m_enemyType == RpgGameData::Enemy::EnemyArcher || m_enemyType == RpgGameData::Enemy::EnemyArcherFix)
		w = m_armory->weaponAdd(new RpgShortbow);
	else if (m_enemyType == RpgGameData::Enemy::EnemySkeleton)
		w = m_armory->weaponAdd(new RpgLongsword);
	else if (m_enemyType == RpgGameData::Enemy::EnemySmith || m_enemyType == RpgGameData::Enemy::EnemySmithFix)
		w = m_armory->weaponAdd(new RpgHammer);
	else if (m_enemyType == RpgGameData::Enemy::EnemyButcher || m_enemyType == RpgGameData::Enemy::EnemyButcherFix)
		w = m_armory->weaponAdd(new RpgAxe);
	else if (m_enemyType == RpgGameData::Enemy::EnemyBarbarian || m_enemyType == RpgGameData::Enemy::EnemyBarbarianFix)
		w = m_armory->weaponAdd(new RpgMace);


	if (w) {
		w->setExcludeFromLayers(true);
		w->setBulletCount(-1);
		m_armory->setCurrentWeapon(w);
	}
}
