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
#include "rpgcontrol.h"
#include "actionrpggame.h"
#include "actionrpgmultiplayergame.h"
#include "rpgfirefog.h"
#include "rpglongsword.h"
#include "tiledspritehandler.h"
#include "rpggame.h"
#include <QDirIterator>
#include "application.h"
#include "rpggamedataiface_t.h"
#include <tilelayeritem.h>

#ifndef Q_OS_WASM
#include "standaloneclient.h"
#endif



class RpgPlayerExitControl : public RpgActiveIface
{
public:
	RpgPlayerExitControl() : RpgActiveIface() {
		setIsActive(true);
		setIsLocked(false);
		setQuestionLock(false);
	}

	virtual const RpgConfig::ControlType &activeType() const override final { return m_type; };

	virtual RpgGameData::BaseData pureBaseData() const override final { return RpgGameData::BaseData(); };

	virtual bool loadFromGroupLayer(RpgGame *, TiledScene *,
									Tiled::GroupLayer *, Tiled::MapRenderer * = nullptr) override final {
		return false;
	}

protected:
	virtual bool loadFromLayer(RpgGame *, TiledScene *,
							   Tiled::Layer *, Tiled::MapRenderer * = nullptr) override final {
		return false;
	};

	virtual void refreshVisualItem() override final {};

private:
	const RpgConfig::ControlType m_type = RpgConfig::ControlExit;
};





class RpgPlayerPrivate
{
private:
	RpgPlayerPrivate(RpgPlayer *player)
		: q(player)
		, m_exitControl(new RpgPlayerExitControl)
	{}

	RpgPlayer *const q;

	QPointer<RpgActiveControlObject> m_currentControl;
	std::unique_ptr<RpgPlayerExitControl> m_exitControl;


	friend class RpgPlayer;
};




/**
 * @brief RpgPlayer::RpgPlayer
 * @param parent
 */

RpgPlayer::RpgPlayer(RpgGame *game, const qreal &radius, const cpBodyType &type)
	: IsometricPlayer(game, radius, type)
	, RpgGameDataInterface<RpgGameData::Player, RpgGameData::PlayerBaseData>()
	, d(new RpgPlayerPrivate(this))
	, m_sfxPain(this)
	, m_sfxFootStep(this)
	, m_sfxAccept(this)
	, m_sfxDecline(this)
	, m_armory(new RpgArmory(this))
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
							   //QStringLiteral("hurt"),
							   QStringLiteral("death")
};

	m_castTimer.setInterval(100);

	connect(this, &RpgPlayer::hurt, this, &RpgPlayer::playHurtEffect);
	connect(this, &RpgPlayer::healed, this, &RpgPlayer::playHealedEffect);
	connect(this, &RpgPlayer::becameAlive, this, &RpgPlayer::playAliveEffect);
	connect(this, &RpgPlayer::becameDead, this, &RpgPlayer::playDeadEffect);
	connect(this, &RpgPlayer::isLockedChanged, this, &RpgPlayer::playShieldEffect);

	connect(m_armory.get(), &RpgArmory::currentWeaponChanged, this, &RpgPlayer::playWeaponChangedEffect);

	connect(&m_castTimer, &QTimer::timeout, this, &RpgPlayer::onCastTimerTimeout);
}


/**
 * @brief RpgPlayer::~RpgPlayer
 */

RpgPlayer::~RpgPlayer()
{
	delete d;
}




/**
 * @brief RpgPlayer::attack
 * @param weapon
 */

void RpgPlayer::attack(RpgWeapon *weapon)
{
	RpgGame *g = qobject_cast<RpgGame*>(m_game);

	if (!weapon || !isAlive() || !g || isHiding())
		return;

	clearDestinationPoint();

	if (!hasAbility())
		return;

	if (weapon->canShot()) {
		g->playerShot(this, weapon, desiredBodyRotation());

	} else if (weapon->canHit()) {
		if (!m_enemy) {
			const QList<IsometricEnemy*> &list = reachedEnemies();

			for (IsometricEnemy *e : list) {
				if (e && e->player() == this) {
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

	rotateBody(angleToPoint(cpv(x,y)), true);

	synchronize();

	attackCurrentWeapon();
}







/**
 * @brief RpgPlayer::useCurrentObjects
 */

void RpgPlayer::useCurrentControl()
{
	RpgGame *g = qobject_cast<RpgGame*>(m_game);

	if (d->m_currentControl && d->m_currentControl->isActive() && !isHiding() && isAlive()) {
		g->playerTryUseControl(this, d->m_currentControl->activeControl());
	}
}






/**
 * @brief RpgPlayer::exitHiding
 */

void RpgPlayer::exitHiding()
{
	if (RpgGame *g = qobject_cast<RpgGame*>(m_game))
		g->playerTryUseControl(this, d->m_exitControl.get());
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

	m_visualItem->setProperty("ellipseColor", QColor::fromRgb(57,250,65,150));
	//m_visualItem->setProperty("ellipseSize", 2);
	m_visualItem->setProperty("ellipseWidth", 50.);


	/*
	TiledVisualLight {
		readonly property RpgPlayer _pl : baseObject && (baseObject instanceof RpgPlayer) ? baseObject : null

		parent: _pl ? _pl.scene : null
		x: root.x - root.width/2
		y: root.y - root.height/2
		sourceZ: Math.floor(root.z)
		z: currentZ
		color: Client.Utils.colorSetAlpha("#ffffff", 0.8)

		width: 250
		height: 350
	}*/

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

void RpgPlayer::attackedByEnemy(RpgEnemy *enemy, const RpgGameData::Weapon::WeaponType &weaponType, const bool &isProtected,
								const bool &immediately)
{
	m_armory->updateLayers();

	if (!isAlive())
		return;

	if (isProtected) {
		if (immediately)
			jumpToSprite("hurt", m_facingDirection);
		else
			QTimer::singleShot(200, Qt::PreciseTimer, this, [this](){ jumpToSprite("hurt", m_facingDirection); });
		return;
	}

	if (m_spriteHandler->currentSprite() != "attack" &&
			m_spriteHandler->currentSprite() != "bow" &&
			m_spriteHandler->currentSprite() != "cast") {
		if (immediately)
			jumpToSprite("hurt", m_facingDirection);
		else
			QTimer::singleShot(200, Qt::PreciseTimer, this, [this](){
				jumpToSprite("hurt", m_facingDirection);
			});
	}

	if (enemy)
		return;

	if (weaponType == RpgGameData::Weapon::WeaponGreatHand)
		startInability(4*m_inabilityTime);
	else if (m_armory->currentWeapon() && m_armory->currentWeapon()->weaponType() == RpgGameData::Weapon::WeaponHand)
		startInability(3*m_inabilityTime);
	else
		startInability();


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
		useCurrentControl();

	m_pickAtDestination = false;
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

			/*case RpgGameData::Weapon::WeaponMageStaff:
			jumpToSprite("cast", m_facingDirection);
			return;											// Nem kell az attack! */

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

int RpgPlayer::collectionRq() const
{
	return m_collectionRq;
}

void RpgPlayer::setCollectionRq(int newCollectionRq)
{
	if (m_collectionRq == newCollectionRq)
		return;
	m_collectionRq = newCollectionRq;
	emit collectionRqChanged();
}

int RpgPlayer::collection() const
{
	return m_collection;
}

void RpgPlayer::setCollection(int newCollection)
{
	if (m_collection == newCollection)
		return;

	m_collection = newCollection;
	emit collectionChanged();
}


/**
 * @brief RpgPlayer::currentControl
 * @return
 */

RpgActiveControlObject *RpgPlayer::currentControl() const
{
	return d->m_currentControl;
}



/**
 * @brief RpgPlayer::setCurrentControl
 * @param newCurrentControl
 */

void RpgPlayer::setCurrentControl(RpgActiveControlObject *newCurrentControl)
{
	if (d->m_currentControl == newCurrentControl)
		return;
	d->m_currentControl = newCurrentControl;
	emit currentControlChanged();
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

	if (std::abs(p.a) < 0.0000001)
		p.a = 0.;

	p.hp = hp();
	p.l = m_isLocked;
	p.c = m_collection;
	p.pck = m_hidingObject;

	if (RpgGame *g = dynamic_cast<RpgGame*>(m_game); g && g->actionRpgGame())
		p.xp = g->actionRpgGame()->xp();

	if (TiledScene *s = scene())
		p.sc = s->sceneId();

	const cpVect vel = cpBodyGetVelocity(body());

	if (vel.x != 0. || vel.y != 0.)
		p.st = RpgGameData::Player::PlayerMoving;
	else
		p.st = RpgGameData::Player::PlayerIdle;

	p.cv = toPosList(vel);

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
	return !m_hidingObject.isValid() && !(m_config.cast == RpgPlayerCharacterConfig::CastInvisible && m_castTimer.isActive());
}


/**
 * @brief RpgPlayer::worldStep
 */

void RpgPlayer::worldStep()
{
	if (m_hidingObject.isValid()) {
		setCurrentVelocity(cpvzero);
		fnStop(this, desiredBodyRotation());
		return;
	}

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

	TiledObjectBody *base = TiledObjectBody::fromShapeRef(other);

	if (!base)
		return;

	const FixtureCategories categories = FixtureCategories::fromInt(cpShapeGetFilter(other).categories);

	if (isBodyShape(self) && categories.testFlag(TiledObjectBody::FixtureControl)) {
		RpgActiveControlObject *control = dynamic_cast<RpgActiveControlObject*>(base);

		if (control && control->isActive()) {
			RpgGame *g = qobject_cast<RpgGame*>(m_game);
			RpgPickable *pickable = dynamic_cast<RpgPickable*>(control->activeControl());

			if (g && pickable)
				g->playerTryUseControl(this, pickable);
			else
				setCurrentControl(control);
		}
	}
}



/**
 * @brief RpgPlayer::onShapeContactEnd
 * @param self
 * @param other
 */

void RpgPlayer::onShapeContactEnd(cpShape *self, cpShape *other)
{
	IsometricPlayer::onShapeContactEnd(self, other);

	TiledObjectBody *base = TiledObjectBody::fromShapeRef(other);

	if (!base)
		return;

	const FixtureCategories categories = FixtureCategories::fromInt(cpShapeGetFilter(other).categories);

	if (isBodyShape(self) && categories.testFlag(TiledObjectBody::FixtureControl)) {
		RpgActiveControlObject *control = dynamic_cast<RpgActiveControlObject*>(base);

		if (d->m_currentControl == control && control)
			setCurrentControl(nullptr);
	}
}




#ifdef WITH_FTXUI
#include "desktopapplication.h"



class FtxWriter
{
public:
	FtxWriter(const int &player, const int &myPlayer, const QString &mode = QStringLiteral("SND"))
		: m_app(dynamic_cast<DesktopApplication*>(Application::instance()))
		, m_mode(mode)
	{
		Q_ASSERT(m_app);
		m_player = player;
		m_myPlayer = myPlayer;
	}

	~FtxWriter()
	{
		if (m_txt.isEmpty())
			return;

		m_lines.append(m_txt);

		for (int i=m_lines.size() - 480; i>0; --i)
			m_lines.removeFirst();


		QCborMap map;
		map.insert(QStringLiteral("mode"), m_mode);

		QString tt = QStringLiteral("PLAYER %1 (vs %2)\n----------------------------------------------------\n").arg(m_player).arg(m_myPlayer);

		for (auto it = m_lines.crbegin(); it != m_lines.crend(); ++it)
			tt += *it + QStringLiteral("\n");

		map.insert(QStringLiteral("txt"), tt);
		m_app->writeToSocket(map.toCborValue());
	}


	FtxWriter& operator += (const QString &w1) {
		m_txt.append(w1);
		return *this;
	}

	FtxWriter& operator << (const QString &w1) {
		m_txt.append(w1);
		return *this;
	}

private:
	DesktopApplication *const m_app;
	const QString m_mode;
	inline static QStringList m_lines = {};
	QString m_txt;

	int m_player = 0;
	int m_myPlayer = -1;
};




#endif






/**
 * @brief RpgPlayer::updateFromSnapshot
 * @param data
 */

void RpgPlayer::updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::Player> &snapshot)
{
#ifdef WITH_FTXUI
	RpgGame *g = dynamic_cast<RpgGame*>(m_game);
	ActionRpgMultiplayerGame *mg = g ? dynamic_cast<ActionRpgMultiplayerGame*>(g->actionRpgGame()) : nullptr;

	FtxWriter writer(baseData().o, mg ? mg->playerId() : -1);

	writer += QStringLiteral("%1|%2: ")
			  .arg(m_game->tickTimer()->currentTick())
			  .arg(snapshot.current)
			  ;

	writer += QStringLiteral("[%1 - %2 - %3] ")
			  .arg(snapshot.s1.f, 4)
			  .arg(snapshot.s2.f, 4)
			  .arg(snapshot.last.f, 4)
			  ;


	writer += QStringLiteral("%1-%2-%3 ")
			  .arg(snapshot.s1.st)
			  .arg(snapshot.s2.st)
			  .arg(snapshot.last.st)
			  ;


	const auto &to = snapshot.s2.f >= 0 ? snapshot.s2 : snapshot.last;

	if (to.p.size() > 1)
		writer += QStringLiteral("=> (%1,%2)")
				  .arg(to.p.at(0))
				  .arg(to.p.at(1))
				  ;
	else
		writer += QStringLiteral("???????????");

#endif


	if (snapshot.s1.f < 0 && snapshot.last.f < 0) {
		LOG_CERROR("scene") << "Invalid tick" << snapshot.s1.f << snapshot.s2.f << snapshot.last.f << snapshot.current;

#ifdef WITH_FTXUI
		writer += QStringLiteral(" ### INVALID TICK ###");
#endif

		stop();
		IsometricEntity::worldStep();
		return;
	}




	QString msg;
	cpVect speed = cpvzero;

	try {
		if (snapshot.s1.st == RpgGameData::Player::PlayerHit ||
				snapshot.s1.st == RpgGameData::Player::PlayerShot ||
				snapshot.s1.st == RpgGameData::Player::PlayerExit) {
			if (const qint64 t = m_stateLastRenderedTicks.value(snapshot.s1.st); t >= snapshot.s1.f)
				throw 1;

			if (!snapshot.s1.p.isEmpty()) {
				fnEmplace(this, cpv(snapshot.s1.p.at(0), snapshot.s1.p.at(1)), snapshot.s1.a);
			} else {
				LOG_CERROR("scene") << "Missing hitpoint" << snapshot.s1.f;
			}

			m_stateLastRenderedTicks.insert(snapshot.s1.st, snapshot.s1.f);
		}


		if (snapshot.s1.st == RpgGameData::Player::PlayerHit) {
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

			throw -1;
		} else if (snapshot.s1.st == RpgGameData::Player::PlayerShot) {
			auto wptr = RpgArmory::weaponCreate(snapshot.s1.arm.cw);

			if (wptr) {
				wptr->setParentObject(this);
				playAttackEffect(wptr.get());
				wptr->playAttack(nullptr);
			}

			throw -1;
		} else {
			throw 1;
		}
	} catch (int e) {

		if (e > 0) {
			speed = entityMove(this, snapshot,
							   RpgGameData::Player::PlayerIdle, RpgGameData::Player::PlayerMoving,
							   m_speedLength, 2*m_speedRunLength,
							   &msg);
		} else {
			msg = QStringLiteral("SKIPPED");
		}
	}

	if (snapshot.s1.f >= 0) {
		setIsLocked(snapshot.s1.l);
		updateFromSnapshot(snapshot.s1);
	} else {
		setIsLocked(snapshot.last.l);
		updateFromSnapshot(snapshot.last);
	}

	/*if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite())) {
		stop();
	} else if (!hasAbility()) {
		stop();
	}*/

	IsometricEntity::worldStep();

	if (!(speed == cpvzero))
		overrideCurrentSpeed(speed);


#ifdef WITH_FTXUI
	if (!msg.isEmpty())
		writer += QStringLiteral("   ")+msg;

	const auto &b = bodyPosition();
	const auto &v = cpvmult(cpBodyGetVelocity(body()), 1/60.);

	writer += QStringLiteral("   @[%1,%2] +(%3,%4)").arg(b.x).arg(b.y).arg(v.x).arg(v.y);
#endif
}



/**
 * @brief RpgPlayer::updateFromSnapshot
 * @param snap
 * @return
 */

void RpgPlayer::updateFromSnapshot(const RpgGameData::Player &snap)
{
	setHp(snap.hp);
	setCollection(snap.c);
	setHidingObject(snap.pck);

	if (snap.st == RpgGameData::Player::PlayerAttack) {
		if (RpgGame *g = qobject_cast<RpgGame*>(m_game); g && g->actionRpgGame()) {
			if (TiledObject *target = dynamic_cast<TiledObject*>(g->findBody(
																	 TiledObjectBody::ObjectId{
																	 .ownerId = snap.tg.o,
																	 .sceneId = snap.tg.s,
																	 .id = snap.tg.id
		}))) {
				if (RpgEnemy *enemy = dynamic_cast<RpgEnemy*>(target))
					enemy->attackedByPlayer(g->actionRpgGame()->gameMode() == ActionRpgGame::MultiPlayerHost ? this : nullptr,
											snap.arm.cw);
			}

		}

		// Ne cserélje ki a fegyvert

		RpgGameData::Armory arm = snap.arm;
		arm.cw = RpgGameData::Weapon::WeaponInvalid;
		m_armory->updateFromSnapshot(arm);

	} else if (snap.st == RpgGameData::Player::PlayerExit) {
		LOG_CINFO("game") << "EXIT PLAYER" << snap.p << snap.a;

		if (const qint64 t = m_stateLastRenderedTicks.value(snap.st); t < snap.f) {
			LOG_CINFO("game") << "EXIT PLAYER REAL" << snap.f << snap.p << snap.a;

			if (!snap.p.isEmpty()) {
				emplace(cpv(snap.p.at(0), snap.p.at(1)));
				if (snap.a >= 0)
					setCurrentAngleForced(snap.a);
			} else {
				LOG_CERROR("scene") << "Missing hitpoint" << snap.f;
			}

			m_stateLastRenderedTicks.insert(snap.st, snap.f);

			setLastSnapshot(snap);
		}

	} else {
		m_armory->updateFromSnapshot(snap.arm);
	}

	setShieldCount(m_armory->getShieldCount());
}



/**
 * @brief RpgPlayer::isLastSnapshotValid
 * @param snap
 * @return
 */

bool RpgPlayer::isLastSnapshotValid(const RpgGameData::Player &snap, const RpgGameData::Player &lastSnap) const
{
	if (lastSnap.f < 0)
		return false;

	if (lastSnap.hp != snap.hp)
		return false;

	if (lastSnap.c != snap.c)
		return false;

	if (lastSnap.xp != snap.xp)
		return false;

	if (lastSnap.pck != snap.pck)
		return false;

	if (snap.st == RpgGameData::Player::PlayerAttack || snap.st == RpgGameData::Player::PlayerExit) {
		return false;
	}

	return true;
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




/**
 * @brief RpgPlayer::isHiding
 * @return
 */


bool RpgPlayer::isHiding() const
{
	return m_hidingObject.isValid();
}


/**
 * @brief RpgPlayer::setHidingObject
 * @param baseData
 */

void RpgPlayer::setHidingObject(const RpgGameData::BaseData &baseData)
{
	if (m_hidingObject.isEqual(baseData))
		return;
	m_hidingObject = baseData;
	emit isHidingChanged();

	LOG_CINFO("game") << "------------------- HIDING" << m_hidingObject.id;

	if (m_visualItem)
		m_visualItem->setVisible(!m_hidingObject.isValid());
}


/**
 * @brief RpgPlayer::isGameCompleted
 * @return
 */

bool RpgPlayer::isGameCompleted() const
{
	return m_isGameCompleted;
}

void RpgPlayer::setIsGameCompleted(bool newIsGameCompleted)
{
	if (m_isGameCompleted == newIsGameCompleted)
		return;
	m_isGameCompleted = newIsGameCompleted;
	emit isGameCompletedChanged();

	LOG_CINFO("game") << "PLAYER COMPLETED!!!!!!!!!!!!!!!!" << this << m_isGameCompleted;
}
