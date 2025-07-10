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

/**
 * @brief RpgEnemyBase::RpgEnemyBase
 * @param parent
 */

RpgEnemyBase::RpgEnemyBase(const RpgGameData::EnemyBaseData::EnemyType &type, RpgGame *game, const qreal &radius)
	: RpgEnemy(type, game, radius)
	, m_effectHealed(this)
	, m_effectFire(this)
	, m_effectSleep(this)
{
	m_armory.reset(new RpgArmory(this));

	setMetric(RpgGame::defaultEnemyMetric().soldier.value(QStringLiteral("default")));

	if (m_enemyType == RpgGameData::EnemyBaseData::EnemyArcher || m_enemyType == RpgGameData::EnemyBaseData::EnemyArcherFix) {
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

RpgWeapon *RpgEnemyBase::defaultWeapon() const
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
	setMaxHp(30);
	setHp(30);

	setAvailableDirections(Direction_8);

	m_directory = directoryBaseName(m_enemyType, m_subType);

	RpgGame::loadBaseTextureSprites(m_spriteHandler, QStringLiteral(":/enemy/")+m_directory+QStringLiteral("/"));

	m_spriteHandler->setStartFrameSeed();

	Q_ASSERT(m_visualItem);

	m_visualItem->setWidth(148);
	m_visualItem->setHeight(130);
	setBodyOffset(0, 0.45*64);


	m_visualItem->setProperty("ellipseColor", QColor::fromRgb(245,51,51,150));
	//m_visualItem->setProperty("ellipseSize", 2);
	m_visualItem->setProperty("ellipseWidth", 50);

	QStringList soundList;

	for (int i=0; i<10; ++i) {
		const QString fname = i == 0 ? QStringLiteral(":/enemy/")+m_directory+QStringLiteral("/roar.mp3")
									 : QStringLiteral(":/enemy/")+m_directory+QStringLiteral("/roar-%1.mp3").arg(i);

		if (QFile::exists(fname))
			soundList.append(fname);
	}

	m_sfxRoar.setSoundList(soundList);
	m_sfxRoar.setPlayOneDeadline(600);

	const auto ptr = Utils::fileToJsonObject(QStringLiteral(":/enemy/")+m_directory+QStringLiteral("/config.json"));

	if (ptr) {
		if (ptr->contains(QStringLiteral("metric"))) {
			LOG_CTRACE("game") << "Enemy metric override" << m_directory;

			m_metric.fromJson(ptr->value(QStringLiteral("metric")).toObject());
			if (sensorPolygon())
				setSensorPolygon(m_metric.sensorLength, m_metric.sensorRange);
		}

		loadConfig(ptr->value(QStringLiteral("config")).toObject());
		return;
	}


	loadConfig();
}



/**
 * @brief RpgEnemyBase::serializeThis
 * @return
 */

RpgGameData::Enemy RpgEnemyBase::serializeThis() const
{
	return serializeEnemy();
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
			p->armory()->currentWeapon()->weaponType() == RpgGameData::Weapon::WeaponDagger) {
		return;
	}

	rotateToPlayer(player);
}



/**
 * @brief RpgEnemyBase::attackedByPlayer
 * @param player
 * @param weaponType
 */

void RpgEnemyBase::attackedByPlayer(RpgPlayer *player, const RpgGameData::Weapon::WeaponType &weaponType)
{
	if (player && isAlive() && weaponType == RpgGameData::Weapon::WeaponHand) {
		if (startSleeping())
			return;
	}

	if (weaponType == RpgGameData::Weapon::WeaponLongbow || weaponType == RpgGameData::Weapon::WeaponFireFogWeapon)
		m_effectFire.play();

	if (isSleeping())
		return;

	if (!isAlive()) {
		jumpToSprite("death", m_facingDirection);

		if (player)
			eventKilledByPlayer(player);
		return;
	}

	jumpToSprite("hurt", m_facingDirection);

	if (!player)
		return;

	if (weaponType == RpgGameData::Weapon::WeaponBroadsword || weaponType == RpgGameData::Weapon::WeaponAxe)
		startInability();

	if (!m_contactedPlayers.contains(player))
		m_contactedPlayers.append(QPointer(player));

	setPlayer(player);
	rotateToPlayer(player);
}






/**
 * @brief RpgEnemyBase::playAttackEffect
 * @param weapon
 */

void RpgEnemyBase::playAttackEffect(RpgWeapon *weapon)
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

cpVect RpgEnemyBase::getPickablePosition(const int &num) const
{
	return cpvsub(bodyPosition(),
				  TiledObject::vectorFromAngle(directionToIsometricRadian(m_facingDirection), 75. *num)
				  );
}





/**
 * @brief RpgEnemyBase::canBulletImpact
 * @param type
 * @return
 */

bool RpgEnemyBase::canBulletImpact(const RpgGameData::Weapon::WeaponType &type) const
{
	if (m_enemyType == RpgGameData::EnemyBaseData::EnemySkeleton && type == RpgGameData::Weapon::WeaponShortbow)
		return false;

	return true;
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
 * @brief RpgEnemyBase::loadConfig
 */

void RpgEnemyBase::loadConfig(const QJsonObject &config)
{
	RpgEnemyConfig cfg;
	cfg.fromJson(config);

	if (cfg.weapon == RpgGameData::Weapon::WeaponInvalid) {
		if (m_enemyType == RpgGameData::EnemyBaseData::EnemySoldier || m_enemyType == RpgGameData::EnemyBaseData::EnemySoldierFix)
			cfg.weapon = RpgGameData::Weapon::WeaponLongsword;
		else if (m_enemyType == RpgGameData::EnemyBaseData::EnemyArcher || m_enemyType == RpgGameData::EnemyBaseData::EnemyArcherFix)
			cfg.weapon = RpgGameData::Weapon::WeaponShortbow;
		else if (m_enemyType == RpgGameData::EnemyBaseData::EnemySkeleton)
			cfg.weapon = RpgGameData::Weapon::WeaponLongsword;
		else if (m_enemyType == RpgGameData::EnemyBaseData::EnemySmith || m_enemyType == RpgGameData::EnemyBaseData::EnemySmithFix)
			cfg.weapon = RpgGameData::Weapon::WeaponHammer;
		else if (m_enemyType == RpgGameData::EnemyBaseData::EnemyButcher || m_enemyType == RpgGameData::EnemyBaseData::EnemyButcherFix)
			cfg.weapon = RpgGameData::Weapon::WeaponAxe;
		else if (m_enemyType == RpgGameData::EnemyBaseData::EnemyBarbarian || m_enemyType == RpgGameData::EnemyBaseData::EnemyBarbarianFix)
			cfg.weapon = RpgGameData::Weapon::WeaponMace;
	}


	if (RpgWeapon *w = m_armory->weaponAdd(cfg.weapon)) {
		w->setExcludeFromLayers(true);
		w->setBulletCount(-1);
		m_armory->setCurrentWeapon(w);
	} else {
		LOG_CERROR("game") << "Invalid enemy config";
	}

	setConfig(cfg);
}
