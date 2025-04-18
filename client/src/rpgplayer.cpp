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
#include "rpgfirefog.h"
#include "rpglongsword.h"
#include "tiledspritehandler.h"
#include "rpggame.h"
#include "rpgcontainer.h"
#include <QDirIterator>
#include "application.h"
#include "rpggamedataiface_t.h"

#ifndef Q_OS_WASM
#include "standaloneclient.h"
#endif



/**
 * @brief RpgPlayer::RpgPlayer
 * @param parent
 */

RpgPlayer::RpgPlayer(RpgGame *game, const qreal &radius, const cpBodyType &type)
	: IsometricPlayer(game, radius, type)
	, RpgGameDataInterface<RpgGameData::Player, RpgGameData::PlayerBaseData>()
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
 * @brief RpgPlayer::attack
 * @param weapon
 */

void RpgPlayer::attack(RpgWeapon *weapon)
{
	RpgGame *g = qobject_cast<RpgGame*>(m_game);

	if (!weapon || !isAlive() || !g)
		return;

	clearDestinationPoint();

	if (!hasAbility())
		return;

	if (weapon->weaponType() == RpgGameData::Weapon::WeaponMageStaff) {
		cast();
		return;
	}

	if (weapon->canShot()) {
		g->playerShot(this, weapon, currentAngle());

	} else if (weapon->canHit()) {
		if (!m_enemy) {
			const QList<IsometricEnemy*> &list = reachedEnemies();

			for (IsometricEnemy *e : list) {
				if (e && e->player() == this) {
					//if (const auto &ptr = m_scene->findShortestPath(m_body->bodyPosition(), e->bodyPosition()))
					//	setDestinationPoint(ptr.value());
					setDestinationPoint(e->bodyPosition());
					break;
				}
			}
		} else {
			clearDestinationPoint();
			stop();
			rotateBody(angleToPoint(m_enemy->bodyPosition()));
		}

		g->playerHit(this, qobject_cast<RpgEnemy*>(m_enemy), weapon);

	} else {
#ifndef Q_OS_WASM
		StandaloneClient *client = qobject_cast<StandaloneClient*>(Application::instance()->client());
		if (client)
			client->performVibrate();
#endif
		if (!m_sfxDecline.soundList().isEmpty()) m_sfxDecline.playOne();
		//m_game->messageColor(tr("Empty weapon"), QColor::fromRgbF(0.8, 0., 0.));

		return;
	}

	if (weapon->bulletCount() == 0)
		messageEmptyBullet(weapon->weaponType());
}




/**
 * @brief RpgPlayer::cast
 */

void RpgPlayer::cast()
{
	LOG_CERROR("game") << "Missing implementation";
	/*RpgWeapon *w = m_armory->weaponFind(RpgGameData::Weapon::WeaponMageStaff);
	RpgGame *g = qobject_cast<RpgGame*>(m_game);

	if (!m_game->tickTimer())
		return;

	const auto tick = m_game->tickTimer()->currentTick();

	if (m_timerRepeater >= 0 && m_timerRepeater > tick)
		return;

	if (m_mp > 0 && w && g) {
		if (g->playerUseCast(this)) {
			m_timerRepeater = tick + 125;
			return;
		}
	}

#ifndef Q_OS_WASM
	StandaloneClient *client = qobject_cast<StandaloneClient*>(Application::instance()->client());
	if (client)
		client->performVibrate();
#endif
	if (!m_sfxDecline.soundList().isEmpty()) m_sfxDecline.playOne();*/

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
		stop();
		return;
	}

	rotateBody(angleToPoint(QVector2D(x,y)), true);

	/////synchronize();

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

	/*if (!m_game->playerPickPickable(this, object)) {
		if (!m_sfxDecline.soundList().isEmpty()) m_sfxDecline.playOne();
	}*/
}



/**
 * @brief RpgPlayer::useContainer
 * @param container
 */

void RpgPlayer::useContainer(RpgContainer *container)
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

	Q_ASSERT(m_visualItem);

	m_visualItem->setWidth(148);
	m_visualItem->setHeight(130);
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
 * @brief RpgPlayer::attackedByEnemy
 */

void RpgPlayer::attackedByEnemy(RpgEnemy *, const RpgGameData::Weapon::WeaponType &weaponType, const bool &isProtected)
{
	m_armory->updateLayers();

	if (!isAlive())
		return;

	if (isProtected) {
		QTimer::singleShot(200, Qt::PreciseTimer, this, [this](){ jumpToSprite("hurt", m_facingDirection); });
		return;
	}

	if (m_spriteHandler->currentSprite() != "attack" &&
			m_spriteHandler->currentSprite() != "bow" &&
			m_spriteHandler->currentSprite() != "cast") {
		QTimer::singleShot(200, Qt::PreciseTimer, this, [this](){
			jumpToSprite("hurt", m_facingDirection);
		});
	}

	if (weaponType == RpgGameData::Weapon::WeaponGreatHand)
		startInability(4*m_inabilityTime);
	else if (m_armory->currentWeapon() && m_armory->currentWeapon()->weaponType() == RpgGameData::Weapon::WeaponHand)
		startInability(3*m_inabilityTime);
	else
		startInability();


}



/**
 * @brief RpgPlayer::onPickableReached
 * @param object
 */

void RpgPlayer::onPickableReached(TiledObjectBody *object)
{
	RpgPickableObject *pickable = dynamic_cast<RpgPickableObject*>(object);
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
		if (inventoryContains(RpgGameData::PickableBaseData::PickableKey, t->lockName()))
			t->setIsOpen(true);
	}
}



/**
 * @brief RpgPlayer::updateConfig
 */

void RpgPlayer::updateConfig()
{
	if (m_config.inability >= 0)
		m_inabilityTime = m_config.inability;
}



/**
 * @brief RpgPlayer::loadDefaultWeapons
 */

void RpgPlayer::loadDefaultWeapons()
{
	m_armory->setCurrentWeapon(m_armory->weaponAdd(RpgGameData::Weapon::WeaponHand));
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
												 , scene() /*, m_body->bodyPosition()*/);
	m_effectShield.stop();
}




/**
 * @brief RpgPlayer::playAttackEffect
 * @param weaponType
 */

void RpgPlayer::playAttackEffect(const RpgGameData::Weapon::WeaponType &weaponType)
{
	switch (weaponType) {
		case RpgGameData::Weapon::WeaponHand:
		case RpgGameData::Weapon::WeaponGreatHand:
		case RpgGameData::Weapon::WeaponLongsword:
		case RpgGameData::Weapon::WeaponBroadsword:
		case RpgGameData::Weapon::WeaponAxe:
		case RpgGameData::Weapon::WeaponMace:
		case RpgGameData::Weapon::WeaponHammer:
		case RpgGameData::Weapon::WeaponDagger:
			jumpToSprite("attack", m_facingDirection);
			break;

		case RpgGameData::Weapon::WeaponLongbow:
		case RpgGameData::Weapon::WeaponShortbow:
			jumpToSprite("bow", m_facingDirection);
			break;

		case RpgGameData::Weapon::WeaponMageStaff:
			jumpToSprite("cast", m_facingDirection);
			return;											// Nem kell az attack!

		case RpgGameData::Weapon::WeaponShield:
		case RpgGameData::Weapon::WeaponLightningWeapon:
		case RpgGameData::Weapon::WeaponFireFogWeapon:
		case RpgGameData::Weapon::WeaponInvalid:
			break;
	}

	emit attackDone();
}





/**
 * @brief RpgPlayer::playAttackEffect
 * @param weapon
 */

void RpgPlayer::playAttackEffect(RpgWeapon *weapon)
{
	if (!weapon)
		return;

	playAttackEffect(weapon->weaponType());
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

void RpgPlayer::messageEmptyBullet(const RpgGameData::Weapon::WeaponType &weaponType)
{
	QString msg;

	switch (weaponType) {
		case RpgGameData::Weapon::WeaponLongbow:
			msg = tr("All fireballs lost");
			break;
		case RpgGameData::Weapon::WeaponShortbow:
			msg = tr("All arrows lost");
			break;

		case RpgGameData::Weapon::WeaponShield:
			msg = tr("All shields lost");
			break;

		case RpgGameData::Weapon::WeaponMageStaff:
			msg = tr("Missing MP");
			break;

		case RpgGameData::Weapon::WeaponHand:
		case RpgGameData::Weapon::WeaponGreatHand:
		case RpgGameData::Weapon::WeaponLongsword:
		case RpgGameData::Weapon::WeaponBroadsword:
		case RpgGameData::Weapon::WeaponDagger:
		case RpgGameData::Weapon::WeaponAxe:
		case RpgGameData::Weapon::WeaponMace:
		case RpgGameData::Weapon::WeaponHammer:
		case RpgGameData::Weapon::WeaponFireFogWeapon:
		case RpgGameData::Weapon::WeaponLightningWeapon:
			msg = tr("Weapon lost");
			break;

		case RpgGameData::Weapon::WeaponInvalid:
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

	/*if (RpgGame *g = qobject_cast<RpgGame*>(m_game))
		g->onPlayerCastTimeout(this);*/
}


/**
 * @brief RpgPlayer::attackReachedEnemies
 * @param weaponType
 */

void RpgPlayer::attackReachedEnemies(const RpgGameData::Weapon::WeaponType &weaponType)
{
	if (weaponType == RpgGameData::Weapon::WeaponFireFogWeapon) {
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




/**
 * @brief RpgPlayer::serializeThis
 * @return
 */

RpgGameData::Player RpgPlayer::serializeThis() const
{
	RpgGameData::Player p;

	p.p = toPosList(bodyPosition());
	p.a = currentAngle();
	p.hp = hp();

	if (TiledScene *s = scene())
		p.sc = s->sceneId();

	const cpVect vel = cpBodyGetVelocity(body());

	if (vel.x != 0. || vel.y != 0.)
		p.st = RpgGameData::Player::PlayerMoving;
	else
		p.st = RpgGameData::Player::PlayerIdle;

	p.cv = { (float) vel.x, (float) vel.y };

	p.arm = m_armory->serialize();

	if (RpgEnemy *enemy = qobject_cast<RpgEnemy*>(m_enemy)) {
		p.tg = enemy->baseData();
	}

	return p;
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


/**
 * @brief RpgPlayer::worldStep
 */

void RpgPlayer::worldStep()
{
	IsometricPlayer::worldStep();

	if (RpgWeapon *w = m_armory->currentWeapon();
			w && (w->canShot() || w->canCast())) {
		updateEnemies(w->bulletDistance());
	}
	else
		updateEnemies(0.);
}


/**
 * @brief RpgPlayer::onShapeContactBegin
 * @param self
 * @param other
 */

void RpgPlayer::onShapeContactBegin(cpShape *self, cpShape *other)
{
	IsometricPlayer::onShapeContactBegin(self, other);

	/************************
	TiledObjectBody *base = TiledObjectBody::fromBodyRef(other.GetBody());

	if (!base)
		return;

	const FixtureCategories categories = FixtureCategories::fromInt(other.GetFilter().categoryBits);

	if (isAny(bodyShapes(), self) && categories.testFlag(TiledObjectBody::FixtureContainer)) {
		RpgContainer *container = dynamic_cast<RpgContainer*>(base);

		if (!m_currentContainer && container)
			setCurrentContainer(container);
	}
	****************************/
}



/**
 * @brief RpgPlayer::onShapeContactEnd
 * @param self
 * @param other
 */

void RpgPlayer::onShapeContactEnd(cpShape *self, cpShape *other)
{
	IsometricPlayer::onShapeContactEnd(self, other);

	/**********************************
	TiledObjectBody *base = TiledObjectBody::fromBodyRef(other.GetBody());

	if (!base)
		return;

	const FixtureCategories categories = FixtureCategories::fromInt(other.GetFilter().categoryBits);

	if (isAny(bodyShapes(), self) && categories.testFlag(TiledObjectBody::FixtureContainer)) {
		RpgContainer *container = dynamic_cast<RpgContainer*>(base);

		if (m_currentContainer == container && container)
			setCurrentContainer(nullptr);
	}
	**************************************/
}






/**
 * @brief RpgPlayer::updateFromSnapshot
 * @param data
 */

void RpgPlayer::updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::Player> &snapshot)
{
	if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite())) {
		stop();
	} else if (!hasAbility()) {
		stop();
	}

	if (snapshot.s1.f < 0) {
		LOG_CERROR("game") << "Invalid tick" << snapshot.s1.f << snapshot.s2.f << snapshot.current;
		stop();
		IsometricEntity::worldStep();
		return;
	}

	if (m_lastSnap >= 0 && snapshot.s1.f > m_lastSnap) {
		LOG_CERROR("game") << "SNAP ERROR" << m_lastSnap << snapshot.s1.f << snapshot.current << snapshot.s2.f;
	}
	m_lastSnap = snapshot.s2.f;


	QVector2D speed;

	if (snapshot.s1.st == RpgGameData::Player::PlayerHit) {
		LOG_CINFO("game") << "HIT" << snapshot.current << snapshot.s1.f << snapshot.s1.p << snapshot.s1.a << snapshot.s2.f;

		auto wptr = RpgArmory::weaponCreate(snapshot.s1.arm.cw);

		if (wptr) {
			TiledObject *target = nullptr;

			if (RpgGame *g = qobject_cast<RpgGame*>(m_game)) {
				target = dynamic_cast<TiledObject*>(g->findBody(
														TiledObjectBody::ObjectId{
															.ownerId = snapshot.s1.tg.o,
															.sceneId = snapshot.s1.tg.s,
															.id = snapshot.s1.tg.id
														}));
			}

			wptr->setParentObject(this);
			playAttackEffect(wptr.get());
			wptr->playAttack(target);
		}
	} else if (snapshot.s1.st == RpgGameData::Player::PlayerShot) {
		LOG_CINFO("game") << "SHOT" << snapshot.current << snapshot.s1.f << snapshot.s1.p << snapshot.s1.a << snapshot.s2.f;

		auto wptr = RpgArmory::weaponCreate(snapshot.s1.arm.cw);

		if (wptr) {
			wptr->setParentObject(this);
			playAttackEffect(wptr.get());
			wptr->playAttack(nullptr);
		}
	} else {
		speed = entityMove(this, snapshot,
						   RpgGameData::Player::PlayerIdle, RpgGameData::Player::PlayerMoving,
						   m_speedLength);
	}

	updateFromSnapshot(snapshot.s1);

	if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite())) {
		stop();
	} else if (!hasAbility()) {
		stop();
	}

	IsometricEntity::worldStep();

	if (!speed.isNull())
		overrideCurrentSpeed(speed);

}



/**
 * @brief RpgPlayer::updateFromSnapshot
 * @param snap
 * @return
 */

void RpgPlayer::updateFromSnapshot(const RpgGameData::Player &snap)
{
	setHp(snap.hp);
	m_armory->updateFromSnapshot(snap.arm);
	setShieldCount(m_armory->getShieldCount());
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
	updateConfig();
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

void RpgPlayer::inventoryAdd(const RpgGameData::PickableBaseData::PickableType &type, const QString &name)
{
	switch (type) {
		case RpgGameData::PickableBaseData::PickableKey:
			m_inventory->append(new RpgInventory(type, name));
			break;

		case RpgGameData::PickableBaseData::PickableHp:
		case RpgGameData::PickableBaseData::PickableMp:
		case RpgGameData::PickableBaseData::PickableCoin:
		case RpgGameData::PickableBaseData::PickableShortbow:
		case RpgGameData::PickableBaseData::PickableLongbow:
		case RpgGameData::PickableBaseData::PickableLongsword:
		case RpgGameData::PickableBaseData::PickableDagger:
		case RpgGameData::PickableBaseData::PickableShield:
		case RpgGameData::PickableBaseData::PickableTime:
			break;

		case RpgGameData::PickableBaseData::PickableInvalid:
			LOG_CWARNING("game") << "Invalid inventory type";
			break;
	}

	return;
}


/**
 * @brief RpgPlayer::inventoryRemove
 * @param type
 */

void RpgPlayer::inventoryRemove(const RpgGameData::PickableBaseData::PickableType &type)
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

void RpgPlayer::inventoryRemove(const RpgGameData::PickableBaseData::PickableType &type, const QString &name)
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

bool RpgPlayer::inventoryContains(const RpgGameData::PickableBaseData::PickableType &type) const
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

bool RpgPlayer::inventoryContains(const RpgGameData::PickableBaseData::PickableType &type, const QString &name) const
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
		messageEmptyBullet(RpgGameData::Weapon::WeaponShield);
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




RpgContainer *RpgPlayer::currentContainer() const
{
	return m_currentContainer;
}

void RpgPlayer::setCurrentContainer(RpgContainer *newCurrentContainer)
{
	if (m_currentContainer == newCurrentContainer)
		return;
	m_currentContainer = newCurrentContainer;
	emit currentContainerChanged();
}
