/*
 * ---- Call of Suli ----
 *
 * rpgplayer.cpp
 *
 * Created on: 2024. 03. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgPlayer
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

#include "rpgplayer.h"
#include "isometricbullet.h"
#include "rpgfirefog.h"
#include "rpglongsword.h"
#include "tiledspritehandler.h"
#include "rpggame.h"
#include <QDirIterator>
#include "application.h"

#ifndef Q_OS_WASM
#include "standaloneclient.h"
#endif



/**
 * @brief RpgPlayer::RpgPlayer
 * @param parent
 */

RpgPlayer::RpgPlayer(QQuickItem *parent)
	: IsometricPlayer(parent)
	, m_sfxPain(this)
	, m_sfxFootStep(this)
	, m_sfxAccept(this)
	, m_sfxDecline(this)
	, m_armory(new RpgArmory(this))
	, m_inventory(new RpgInventoryList)
	, m_effectHealed(this)
	, m_effectShield(this)
	, m_effectRing(this)
{
	m_sfxPain.setFollowPosition(false);
	m_sfxAccept.setFollowPosition(false);
	m_sfxDecline.setFollowPosition(false);

	m_moveDisabledSpriteList = QStringList{
							   QStringLiteral("attack"),
							   QStringLiteral("bow"),
							   QStringLiteral("cast"),
							   QStringLiteral("hurt"),
							   QStringLiteral("death")
};

	m_castTimer.setInterval(100);

	connect(this, &RpgPlayer::hurt, this, &RpgPlayer::playHurtEffect);
	connect(this, &RpgPlayer::healed, this, &RpgPlayer::playHealedEffect);
	connect(this, &RpgPlayer::becameAlive, this, &RpgPlayer::playAliveEffect);
	connect(this, &RpgPlayer::becameDead, this, &RpgPlayer::playDeadEffect);
	connect(this, &RpgPlayer::isLockedChanged, this, &RpgPlayer::playShieldEffect);
	connect(this, &RpgPlayer::currentTransportChanged, this, &RpgPlayer::onCurrentTransportChanged);

	connect(m_armory.get(), &RpgArmory::currentWeaponChanged, this, &RpgPlayer::playWeaponChangedEffect);

	connect(&m_castTimer, &QTimer::timeout, this, &RpgPlayer::onCastTimerTimeout);
}


/**
 * @brief RpgPlayer::~RpgPlayer
 */

RpgPlayer::~RpgPlayer()
{

}


/**
 * @brief RpgPlayer::createPlayer
 * @param game
 * @param scene
 * @param config
 * @return
 */

RpgPlayer *RpgPlayer::createPlayer(RpgGame *game, TiledScene *scene, const RpgPlayerCharacterConfig &config)
{
	RpgPlayer *player = nullptr;
	TiledObjectBase::createFromCircle<RpgPlayer>(&player, QPointF{}, 30, nullptr, game);

	if (player) {
		player->setParent(game);
		player->setGame(game);
		player->setScene(scene);
		player->setConfig(config);
		player->initialize();

		player->setMaxMp(config.mpMax);
	}

	return player;
}



/**
 * @brief RpgPlayer::attack
 * @param weapon
 */

void RpgPlayer::attack(TiledWeapon *weapon)
{
	if (!weapon || !isAlive())
		return;

	clearDestinationPoint();

	if (!hasAbility())
		return;

	if (weapon->weaponType() == TiledWeapon::WeaponMageStaff) {
		cast();
		return;
	}

	if (weapon->canShot()) {
		if (weapon->shot(IsometricBullet::TargetEnemy, m_body->bodyPosition(), currentAngle())) {
			playAttackEffect(weapon);

			if (weapon->pickedBulletCount() > 0) {
				weapon->setPickedBulletCount(weapon->pickedBulletCount()-1);
			} else if (RpgGame *g = qobject_cast<RpgGame*>(m_game)) {
				if (RpgPickableWeaponIface *iface = dynamic_cast<RpgPickableWeaponIface*>(weapon)) {
					g->useBullet(iface->toBulletPickable());
				} else {
					LOG_CERROR("game") << "Invalid weapon cast";
				}
			} else {
				LOG_CERROR("game") << "Invalid RpgGame cast";
			}
		}

		if (weapon->bulletCount() == 0)
			messageEmptyBullet(weapon->weaponType());

	} else if (weapon->canHit()) {
		if (!hasAbility())
			return;

		if (!enemy()) {
			const QList<IsometricEnemy*> &list = reachedEnemies();

			for (IsometricEnemy *e : list) {
				if (e && e->player() == this) {
					setDestinationPoint(e->body()->bodyPosition());
					break;
				}
			}
		} else {
			clearDestinationPoint();
			m_body->stop();
		}

		if (weapon->hit(enemy()))
			playAttackEffect(weapon);
	} else {
#ifndef Q_OS_WASM
		StandaloneClient *client = qobject_cast<StandaloneClient*>(Application::instance()->client());
		if (client)
			client->performVibrate();
#endif
		if (!m_sfxDecline.soundList().isEmpty()) m_sfxDecline.playOne();
		//m_game->messageColor(tr("Empty weapon"), QColor::fromRgbF(0.8, 0., 0.));
	}
}




/**
 * @brief RpgPlayer::cast
 */

void RpgPlayer::cast()
{
	TiledWeapon *w = m_armory->weaponFind(TiledWeapon::WeaponMageStaff);
	RpgGame *g = qobject_cast<RpgGame*>(m_game);

	if (!m_timerRepeater.isForever() && !m_timerRepeater.hasExpired())
		return;

	if (m_mp > 0 && w && g) {
		if (g->playerUseCast(this)) {
			m_timerRepeater.setRemainingTime(125);
			return;
		}
	}

#ifndef Q_OS_WASM
	StandaloneClient *client = qobject_cast<StandaloneClient*>(Application::instance()->client());
	if (client)
		client->performVibrate();
#endif
	if (!m_sfxDecline.soundList().isEmpty()) m_sfxDecline.playOne();

}



/**
 * @brief RpgPlayer::attackToPoint
 * @param x
 * @param y
 */

void RpgPlayer::attackToPoint(const qreal &x, const qreal &y)
{
	if (!isAlive())
		return;

	clearDestinationPoint();
	m_pickAtDestination = false;

	if (m_isLocked) {
		m_body->stop();
		return;
	}

	QLineF l(m_body->bodyPosition(), QPointF{x,y});
	setCurrentAngle(toRadian(l.angle()));

	attackCurrentWeapon();
}





/**
 * @brief RpgPlayer::pick
 * @param object
 */

void RpgPlayer::pick(RpgPickableObject *object)
{
	if (!object || !isAlive())
		return;

	//clearDestinationPoint();

	if (!m_game->playerPickPickable(this, object)) {
		if (!m_sfxDecline.soundList().isEmpty()) m_sfxDecline.playOne();
	}
}



/**
 * @brief RpgPlayer::useContainer
 * @param container
 */

void RpgPlayer::useContainer(TiledContainer *container)
{
	if (!container || !isAlive())
		return;

	RpgGame *g = qobject_cast<RpgGame*>(m_game);

	clearDestinationPoint();

	if (!g || !container->isActive())
		return;

	g->playerTryUseContainer(this, container);
}


/**
 * @brief RpgPlayer::useCurrentObjects
 */

void RpgPlayer::useCurrentObjects()
{
	RpgGame *g = qobject_cast<RpgGame*>(m_game);

	if (currentContainer() && currentContainer()->isActive() && g) {
		g->playerTryUseContainer(this, currentContainer());
		return;
	}

	if (currentTransport() && g)
		g->transportPlayer();

}







/**
 * @brief RpgPlayer::load
 */

void RpgPlayer::load()
{
	setAvailableDirections(Direction_8);

	/*for (int i=0; i<=2; ++i)
	{
		IsometricObjectLayeredSprite json;
		json.fromJson(RpgGame::baseEntitySprite(i));
		//json.layers.insert(QStringLiteral("default"), QStringLiteral("_sprite%1.png").arg(i));
		json.layers.insert(QStringLiteral("default"), QStringLiteral("texture.png"));

		RpgArmory::fillAvailableLayers(&json, i);
		if (m_config.shield.isEmpty())
			RpgArmory::fillLayer(&json, QStringLiteral("shield"), i);
		else
			RpgArmory::fillLayer(&json, QStringLiteral("shield"), m_config.shield, i);

		appendSprite(json, m_config.prefixPath);
	}*/

	RpgGame::loadBaseTextureSprites(m_spriteHandler, m_config.prefixPath+QStringLiteral("/"));

	if (m_config.shield.isEmpty())
		RpgGame::loadBaseTextureSprites(m_spriteHandler, QStringLiteral(":/rpg/shield/"), QStringLiteral("shield"));
	else
		RpgGame::loadBaseTextureSprites(m_spriteHandler, QStringLiteral(":/rpg/")+m_config.shield+QStringLiteral("/"),
										QStringLiteral("shield"));

	setWidth(148);
	setHeight(130);
	setBodyOffset(0, 0.45*64);

	loadSfx();
	loadDefaultWeapons();

	connect(m_spriteHandler, &TiledSpriteHandler::currentSpriteChanged, this, &RpgPlayer::onCurrentSpriteChanged);

	m_armory->updateLayers();

	setShieldCount(m_armory->getShieldCount());
}





/**
 * @brief RpgPlayer::updateSprite
 */

void RpgPlayer::updateSprite()
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
		if (m_body->isRunning())
			jumpToSprite("run", m_movingDirection);
		else
			jumpToSprite("walk", m_movingDirection);
	} else
		jumpToSprite("idle", m_currentDirection);
}



/**
 * @brief RpgPlayer::protectWeapon
 * @param weaponType
 * @return
 */

bool RpgPlayer::protectWeapon(const TiledWeapon::WeaponType &weaponType)
{
	if (m_config.cast == RpgPlayerCharacterConfig::CastProtect && m_castTimer.isActive()) {
		return true;
	}

	const bool r = IsometricPlayer::protectWeapon(m_armory->weaponList(), weaponType);

	if (r)
		m_armory->updateLayers();

	setShieldCount(m_armory->getShieldCount());

	return r;
}





/**
 * @brief RpgPlayer::attackedByEnemy
 */

void RpgPlayer::attackedByEnemy(IsometricEnemy *, const TiledWeapon::WeaponType &weaponType, const bool &isProtected)
{
	if (!isAlive())
		return;

	if (isProtected) {
		QTimer::singleShot(200, Qt::PreciseTimer, this, [this](){ jumpToSprite("hurt", m_currentDirection); });
		return;
	}


	int hp = m_hp-1;

	if (hp <= 0) {
		setHp(0);
		jumpToSprite("death", m_currentDirection);
	} else {
		setHp(hp);
		if (m_spriteHandler->currentSprite() != "attack" &&
				m_spriteHandler->currentSprite() != "bow" &&
				m_spriteHandler->currentSprite() != "cast") {
			QTimer::singleShot(200, Qt::PreciseTimer, this, [this](){
				jumpToSprite("hurt", m_currentDirection);
			});
		}

		if (weaponType == TiledWeapon::WeaponGreatHand)
			startInability(4*m_inabilityTime);
		else if (m_armory->currentWeapon() && m_armory->currentWeapon()->weaponType() == TiledWeapon::WeaponHand)
			startInability(3*m_inabilityTime);
		else
			startInability();
	}

	m_armory->updateLayers();
}



/**
 * @brief RpgPlayer::onPickableReached
 * @param object
 */

void RpgPlayer::onPickableReached(TiledObject *object)
{
	RpgPickableObject *pickable = qobject_cast<RpgPickableObject*>(object);
	if (pickable)
		pick(pickable);
}


/**
 * @brief RpgPlayer::onEnemyReached
 * @param enemy
 */

void RpgPlayer::onEnemyReached(IsometricEnemy *enemy)
{
	if (m_config.cast == RpgPlayerCharacterConfig::CastFireFog && m_castTimer.isActive()) {
		RpgFireFogWeapon w;
		w.setParentObject(this);
		w.setBulletCount(-1);

		if (w.hit(enemy))
			playAttackEffect(&w);
	}
}



/**
 * @brief RpgPlayer::atDestinationPointEvent
 */

void RpgPlayer::atDestinationPointEvent()
{
	if (m_pickAtDestination)
		useCurrentObjects();

	m_pickAtDestination = false;
}





/**
 * @brief RpgPlayer::onCurrentTransportChanged
 */

void RpgPlayer::onCurrentTransportChanged()
{
	auto t = currentTransport();

	if (!t)
		return;

	if (!t->isOpen() && !t->lockName().isEmpty()) {
		if (inventoryContains(RpgPickableObject::PickableKey, t->lockName()))
			t->setIsOpen(true);
	}
}



/**
 * @brief RpgPlayer::loadDefaultWeapons
 */

void RpgPlayer::loadDefaultWeapons()
{
	m_armory->setCurrentWeapon(m_armory->weaponAdd(new TiledWeaponHand));
}




/**
 * @brief RpgPlayer::loadSfx
 */

void RpgPlayer::loadSfx()
{
	if (m_config.sfxPain.isEmpty()) {
		m_sfxPain.setSoundList({
								   QStringLiteral(":/sound/sfx/pain1.mp3"),
								   QStringLiteral(":/sound/sfx/pain2.mp3"),
								   QStringLiteral(":/sound/sfx/pain3.mp3"),
							   });
	} else {
		m_sfxPain.setSoundList(m_config.sfxPain);
	}

	m_sfxPain.setPlayOneDeadline(600);


	if (m_config.sfxFootStep.isEmpty()) {
		m_sfxFootStep.setSoundList({
									   QStringLiteral(":/sound/sfx/run1.mp3"),
									   QStringLiteral(":/sound/sfx/run2.mp3"),
								   });
	} else {
		m_sfxFootStep.setSoundList(m_config.sfxFootStep);
	}

	m_sfxAccept.setSoundList(m_config.sfxAccept);
	m_sfxAccept.setPlayOneDeadline(1500);
	m_sfxDecline.setSoundList(m_config.sfxDecline);
	m_sfxDecline.setPlayOneDeadline(1500);

	m_sfxFootStep.setInterval(350);
}






/**
 * @brief RpgPlayer::onCurrentSpriteChanged
 */

void RpgPlayer::onCurrentSpriteChanged()
{
	const QString &sprite = m_spriteHandler->currentSprite();

	if (sprite == QStringLiteral("run"))
		m_sfxFootStep.startFromBegin();
	else if (sprite != QStringLiteral("run"))
		m_sfxFootStep.stop();
}



/**
 * @brief RpgPlayer::playAliveEffect
 */

void RpgPlayer::playAliveEffect()
{
	playHealedEffect();
}


/**
 * @brief RpgPlayer::playHurtEffect
 */

void RpgPlayer::playHurtEffect()
{
	m_sfxPain.playOne();
}


/**
 * @brief RpgPlayer::playHealedEffect
 */

void RpgPlayer::playHealedEffect()
{
	m_effectHealed.play();
}


/**
 * @brief RpgPlayer::playDeadEffect
 */

void RpgPlayer::playDeadEffect()
{
	m_game->playSfx(m_config.sfxDead.isEmpty() ? QStringLiteral(":/sound/sfx/dead.mp3")
											   : m_config.sfxDead
												 , m_scene /*, m_body->bodyPosition()*/);
	m_effectShield.stop();
}





/**
 * @brief RpgPlayer::playAttackEffect
 * @param weapon
 */

void RpgPlayer::playAttackEffect(TiledWeapon *weapon)
{
	if (!weapon)
		return;

	switch (weapon->weaponType()) {
		case TiledWeapon::WeaponHand:
		case TiledWeapon::WeaponGreatHand:
		case TiledWeapon::WeaponLongsword:
		case TiledWeapon::WeaponBroadsword:
		case TiledWeapon::WeaponAxe:
		case TiledWeapon::WeaponMace:
		case TiledWeapon::WeaponHammer:
		case TiledWeapon::WeaponDagger:
			jumpToSprite("attack", m_currentDirection);
			break;

		case TiledWeapon::WeaponLongbow:
		case TiledWeapon::WeaponShortbow:
			jumpToSprite("bow", m_currentDirection);
			break;

		case TiledWeapon::WeaponMageStaff:
			jumpToSprite("cast", m_currentDirection);
			return;											// Nem kell az attack!

		case TiledWeapon::WeaponShield:
		case TiledWeapon::WeaponLightningWeapon:
		case TiledWeapon::WeaponFireFogWeapon:
		case TiledWeapon::WeaponInvalid:
			break;
	}

	emit attackDone();
}




/**
 * @brief RpgPlayer::playWeaponChangedEffect
 */

void RpgPlayer::playWeaponChangedEffect()
{
	RpgGame *g = qobject_cast<RpgGame*>(m_game);

	if (!g || g->controlledPlayer() != this)
		return;

	if (!armory()->currentWeapon())
		return m_game->messageColor(tr("Without weapon"), QStringLiteral("#EF5350"));


	m_game->message(armory()->currentWeapon()->weaponNameEn().append(tr(" activated")));

	if (!m_sfxAccept.soundList().isEmpty())
		m_sfxAccept.playOne();
}



/**
 * @brief RpgPlayer::playShieldEffect
 */

void RpgPlayer::playShieldEffect()
{
	if (m_isLocked && m_hp > 0) {
		if (!m_effectRing.active())
			m_effectShield.play();
	} else
		m_effectShield.stop();
}




/**
 * @brief RpgPlayer::messageEmptyBullet
 * @param weaponType
 */

void RpgPlayer::messageEmptyBullet(const TiledWeapon::WeaponType &weaponType)
{
	QString msg;

	switch (weaponType) {
		case TiledWeapon::WeaponLongbow:
			msg = tr("All fireballs lost");
			break;
		case TiledWeapon::WeaponShortbow:
			msg = tr("All arrows lost");
			break;

		case TiledWeapon::WeaponShield:
			msg = tr("All shields lost");
			break;

		case TiledWeapon::WeaponMageStaff:
			msg = tr("Missing MP");
			break;

		case TiledWeapon::WeaponHand:
		case TiledWeapon::WeaponGreatHand:
		case TiledWeapon::WeaponLongsword:
		case TiledWeapon::WeaponBroadsword:
		case TiledWeapon::WeaponDagger:
		case TiledWeapon::WeaponAxe:
		case TiledWeapon::WeaponMace:
		case TiledWeapon::WeaponHammer:
		case TiledWeapon::WeaponFireFogWeapon:
		case TiledWeapon::WeaponLightningWeapon:
		case TiledWeapon::WeaponInvalid:
			break;
	}

	if (msg.isEmpty())
		return;

	m_game->messageColor(msg, QColor::fromRgbF(0.8, 0., 0.));
}




/**
 * @brief RpgPlayer::onCastTimerTimeout
 */

void RpgPlayer::onCastTimerTimeout()
{
	if (m_isLocked)
		return;

	if (RpgGame *g = qobject_cast<RpgGame*>(m_game))
		g->onPlayerCastTimeout(this);
}


/**
 * @brief RpgPlayer::attackReachedEnemies
 * @param weaponType
 */

void RpgPlayer::attackReachedEnemies(const TiledWeapon::WeaponType &weaponType)
{
	if (weaponType == TiledWeapon::WeaponFireFogWeapon) {
		if (m_config.cast == RpgPlayerCharacterConfig::CastFireFog && m_castTimer.isActive()) {
			RpgFireFogWeapon w;
			w.setParentObject(this);
			w.setBulletCount(-1);
			w.setDisableTimerRepeater(true);

			for (IsometricEnemy *e : reachedEnemies()) {
				if (!e)
					continue;
				if (w.hit(e))
					playAttackEffect(&w);
			}
		}
	} else {
		LOG_CERROR("game") << "Weapon not supported:" << weaponType;
	}
}



int RpgPlayer::maxMp() const
{
	return m_maxMp;
}

void RpgPlayer::setMaxMp(int newMaxMp)
{
	if (m_maxMp == newMaxMp)
		return;
	m_maxMp = newMaxMp;
	emit maxMpChanged();
}



/**
 * @brief RpgPlayer::isDiscoverable
 * @return
 */

bool RpgPlayer::isDiscoverable() const
{
	return !(m_config.cast == RpgPlayerCharacterConfig::CastInvisible && m_castTimer.isActive());
}


int RpgPlayer::mp() const
{
	return m_mp;
}

void RpgPlayer::setMp(int newMp)
{
	newMp = std::min(newMp, m_maxMp);
	if (m_mp == newMp)
		return;
	m_mp = newMp;
	emit mpChanged();
}



void RpgPlayer::setConfig(const RpgPlayerCharacterConfig &newConfig)
{
	m_config = newConfig;
	emit configChanged();
}



/**
 * @brief RpgPlayer::inventory
 * @return
 */

RpgInventoryList*RpgPlayer::inventory() const
{
	return m_inventory.get();
}






/**
 * @brief RpgPlayer::inventoryAdd
 * @param object
 */

void RpgPlayer::inventoryAdd(RpgPickableObject *object)
{
	if (!object)
		return;

	inventoryAdd(object->pickableType(), object->name());
}


/**
 * @brief RpgPlayer::inventoryAdd
 * @param type
 * @param name
 */

void RpgPlayer::inventoryAdd(const RpgPickableObject::PickableType &type, const QString &name)
{
	switch (type) {
		case RpgPickableObject::PickableKey:
			m_inventory->append(new RpgInventory(type, name));
			break;

		case RpgPickableObject::PickableHp:
		case RpgPickableObject::PickableMp:
		case RpgPickableObject::PickableCoin:
		case RpgPickableObject::PickableShortbow:
		case RpgPickableObject::PickableLongbow:
		case RpgPickableObject::PickableArrow:
		case RpgPickableObject::PickableFireball:
		case RpgPickableObject::PickableLightning:
		case RpgPickableObject::PickableLongsword:
		case RpgPickableObject::PickableDagger:
		case RpgPickableObject::PickableShield:
		case RpgPickableObject::PickableTime:
			break;

		case RpgPickableObject::PickableInvalid:
			LOG_CWARNING("game") << "Invalid inventory type";
			break;
	}

	return;
}


/**
 * @brief RpgPlayer::inventoryRemove
 * @param type
 */

void RpgPlayer::inventoryRemove(const RpgPickableObject::PickableType &type)
{
	QList<RpgInventory*> list;

	for (RpgInventory *i : *m_inventory) {
		if (i->pickableType() == type)
			list.append(i);
	}

	m_inventory->remove(list);
}



/**
 * @brief RpgPlayer::inventoryRemove
 * @param type
 * @param name
 */

void RpgPlayer::inventoryRemove(const RpgPickableObject::PickableType &type, const QString &name)
{
	QList<RpgInventory*> list;

	for (RpgInventory *i : *m_inventory) {
		if (i->pickableType() == type && i->name() == name)
			list.append(i);
	}

	m_inventory->remove(list);
}



/**
 * @brief RpgPlayer::inventoryContains
 * @param type
 * @return
 */

bool RpgPlayer::inventoryContains(const RpgPickableObject::PickableType &type) const
{
	for (RpgInventory *i : *m_inventory) {
		if (i->pickableType() == type)
			return true;
	}

	return false;
}


/**
 * @brief RpgPlayer::inventoryContains
 * @param type
 * @param name
 * @return
 */

bool RpgPlayer::inventoryContains(const RpgPickableObject::PickableType &type, const QString &name) const
{
	for (RpgInventory *i : *m_inventory) {
		if (i->pickableType() == type && i->name() == name)
			return true;
	}

	return false;
}


/**
 * @brief RpgPlayer::shieldCount
 * @return
 */

int RpgPlayer::shieldCount() const
{
	return m_shieldCount;
}

void RpgPlayer::setShieldCount(int newShieldCount)
{
	if (m_shieldCount == newShieldCount)
		return;
	m_shieldCount = newShieldCount;
	emit shieldCountChanged();

	if (m_shieldCount == 0)
		messageEmptyBullet(TiledWeapon::WeaponShield);
}


/**
 * @brief RpgPlayer::config
 * @return
 */

const RpgPlayerCharacterConfig &RpgPlayer::config() const
{
	return m_config;
}


/**
 * @brief RpgPlayer::currentSceneStartPosition
 * @return
 */

QPointF RpgPlayer::currentSceneStartPosition() const
{
	return m_currentSceneStartPosition;
}

void RpgPlayer::setCurrentSceneStartPosition(QPointF newCurrentSceneStartPosition)
{
	m_currentSceneStartPosition = newCurrentSceneStartPosition;
}





/**
 * @brief RpgPlayer::armory
 * @return
 */

RpgArmory *RpgPlayer::armory() const
{
	return m_armory.get();
}







/**
 * @brief RpgPlayerCharacterConfig::updateSfxPath
 * @param prefix
 */

void RpgPlayerCharacterConfig::updateSfxPath(const QString &prefix)
{
	for (QList<QString> *ptr : std::vector<QList<QString>*>{
		 &sfxAccept,
		 &sfxDecline,
		 &sfxFootStep,
		 &sfxPain,
}) {
		for (QString &s : *ptr) {
			if (!s.isEmpty() && !s.startsWith(QStringLiteral(":/")))
				s.prepend(prefix);
		}
	}

	if (!sfxDead.isEmpty() && !sfxDead.startsWith(QStringLiteral(":/")))
		sfxDead.prepend(prefix);
}

