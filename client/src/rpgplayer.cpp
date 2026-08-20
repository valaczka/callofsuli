/*
 * ---- Call of Suli ----
 *
 * rpgplayer.cpp
 *
 * Created on: 2026. 05. 12.
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

#include "rpgchanger.h"
#include "rpgcontrol.h"
#include "rpgnpc.h"
#include "tiledspritehandler.h"
#include "rpgplayer.h"
#include "rpgmp.h"
#include "rpgdefender.h"
#include "rpgconfig.h"
#include "gamequestion.h"
#include "rpgstream.h"

#ifndef Q_OS_WASM
#include "standaloneclient.h"
#endif


#define SENSOR_LENGTH	400.




/**
 * @brief The RpgPlayerPrivate class
 */

class RpgPlayerPrivate
{
private:
	RpgPlayerPrivate(RpgPlayer *player) : q(player) {}

	void updateLock(const qint64 &tick, const quint32 &lockId);
	void resetLock(const RpgStream::EventPlayer &event);

	void applyKnockback();
	void vibrate();

	void updateJoystickMode();
	void updateJoystickIcon();

private:
	RpgPlayer *const q;

	std::vector<RpgStream::EventPlayer> m_eventList;
	quint32 m_lockId = 0;
	quint32 m_penalty = 0;
	std::optional<RpgStream::EventPlayer> m_lockedEvent;
	qint64 m_waitForLock = 0;
	bool m_gameQuestionLoaded = false;

	bool m_controlActionDisable = false;
	quint32 m_controlActionDisableLastNotification = 0;

	cpVect m_currentKnockback = cpvzero;

	QSet<cpShape*> m_groundCollision;

	bool m_joystickC_hasTouch = false;
	bool m_isTargetAuto = false;

	QSet<RpgStream::PlayerConfig::Utility> m_activeUtilities;


	friend class RpgPlayer;
	friend class RpgMotorPlayerControlled;
};




/**
 * @brief RpgPlayer::RpgPlayer
 * @param gameItem
 * @param center
 */

RpgPlayer::RpgPlayer(RpgGameItem *gameItem, const cpVect &center)
	: RpgEntity(gameItem, center, 25., CP_BODY_TYPE_DYNAMIC)
	, d(new RpgPlayerPrivate(this))
	, m_sfxPain(this)
	, m_sfxDead(this)
	, m_sfxFootStep(this)
	, m_sfxAccept(this)
	, m_sfxDecline(this)
	, m_effectHealed(this)
	, m_effectShield(this)
	, m_effectRing(this)
{
	m_defaultMotor = std::make_unique<RpgMotorPlayer>(this);

	filterSet(RpgGameItem::FixturePlayerBody,
			  RpgGameItem::FixtureGround | RpgGameItem::FixtureControl |
			  RpgGameItem::FixturePlayerTarget | RpgGameItem::FixtureNpcTarget);

	m_currentChunk.setX(-1);
	m_currentChunk.setY(-1);

	addTargetCircle(50, TiledObjectBody::getFilter(RpgGameItem::FixturePlayerTarget,
												   RpgGameItem::FixtureAll));


	m_sfxPain.setFollowPosition(false);
	m_sfxAccept.setFollowPosition(false);
	m_sfxDecline.setFollowPosition(false);


	connect(this, &RpgPlayer::healed, this, [this](){ m_effectHealed.play(); });
	connect(this, &RpgPlayer::hurt, this, [this]() { if (m_rpgGame->controlledPlayer() == this) m_sfxPain.playOne(); });
	connect(this, &RpgPlayer::becameDead, this, [this]() { if (m_rpgGame->controlledPlayer() == this) m_sfxDead.playOne(); });

	connect(this, &RpgPlayer::hasDefenderChanged, this, [this]() { d->updateJoystickMode(); });
	connect(this, &RpgPlayer::hasUtilityChanged, this, [this]() { d->updateJoystickMode(); });
	connect(this, &RpgPlayer::bulletChanged, this, [this]() { d->updateJoystickMode(); });
}


RpgPlayer::~RpgPlayer()
{
	delete d;
	d = nullptr;
}


/**
 * @brief RpgPlayer::initialize
 */

void RpgPlayer::initialize()
{
	Q_ASSERT(scene());

	setDefaultZ(1);
	setSubZ(0.5);

	createVisual();

	m_visualItem->setZ(1);
}



/**
 * @brief load
 * @param config
 */

void RpgPlayer::load(const RpgPlayerDefinition &config)
{
	setAvailableDirections(Direction_8);

	setConfig(config);
	setHp(config.hp);

	m_markerItem = createMarkerItem(QStringLiteral("qrc:/RpgPlayerMarker.qml"));

	if (QFile::exists(m_config.prefixPath+QStringLiteral("/input.txt"))) {
		//QHash<QString, RpgArmory::LayerData> layerData;
		QRect measure = RpgGameItem::loadTextureSprites(m_spriteHandler, m_config.prefixPath+QStringLiteral("/")/*, &layerData*/);

		Q_ASSERT(m_visualItem);

		m_visualItem->setWidth(measure.width());
		m_visualItem->setHeight(measure.height());
		setBodyOffset(measure.x(), measure.y());

		m_spriteHandler->setVisibleLayers({"default"});
	}


	// Default sound
	m_sfxDead.setSoundList({QStringLiteral(":/sound/sfx/dead.mp3")});

	// Can be overwritten here
	loadSfx();

	connect(m_spriteHandler, &TiledSpriteHandler::currentSpriteChanged, this, &RpgPlayer::onCurrentSpriteChanged);

	updateColor();

	onAlive();
}




/**
 * @brief RpgPlayer::setConfig
 * @param config
 */

void RpgPlayer::setConfig(const RpgPlayerDefinition &config)
{
	m_config = config;

	setMaxHp(m_config.hp);

	emit maxMpChanged();
	emit maxBulletChanged();
}


/**
 * @brief RpgPlayer::isRunning
 * @return
 */

bool RpgPlayer::isRunning() const
{
	// 60 FPS
	return currentSpeedSq() >= POW2(m_config.run)*0.9;
}


/**
 * @brief RpgPlayer::isWalking
 * @return
 */

bool RpgPlayer::isWalking() const
{
	// 60 FPS
	const float &l = currentSpeedSq();
	return l < POW2(m_config.run) && l > POW2(0.05);
}



/**
 * @brief RpgPlayer::useCurrentUtility
 */

void RpgPlayer::useCurrentUtility()
{
	if (RpgMotorPlayerControlled* motor = dynamic_cast<RpgMotorPlayerControlled*>(currentMotor())) {
		motor->useCurrentUtility();
	} else {
		LOG_CWARNING("game") << "Invalid player motor";
	}
}



/**
 * @brief RpgPlayer::currentChunk
 * @return
 */

QPoint RpgPlayer::currentChunk() const
{
	return m_currentChunk;
}

void RpgPlayer::setCurrentChunk(QPoint newCurrentChunk)
{
	if (m_currentChunk == newCurrentChunk)
		return;
	m_currentChunk = newCurrentChunk;
	emit currentChunkChanged();
}


QPointF RpgPlayer::currentChunkCenter() const
{
	return m_currentChunkCenter;
}

void RpgPlayer::setCurrentChunkCenter(QPointF newCurrentChunkCenter)
{
	if (m_currentChunkCenter == newCurrentChunkCenter)
		return;
	m_currentChunkCenter = newCurrentChunkCenter;
	emit currentChunkCenterChanged();
}



/**
 * @brief RpgMotorPlayer::RpgMotorPlayer
 * @param player
 */

RpgMotorPlayer::RpgMotorPlayer(RpgPlayer *player)
	: RpgMotorEntity(player)
	, RpgMotorPlayerEventIface()
	, m_player(player)
{
	Q_ASSERT(m_player);
}



/**
 * @brief RpgMotorPlayer::beforeWorldStep
 * @param tick
 * @param entity
 * @return
 */

bool RpgMotorPlayer::beforeWorldStep(const qint64 &tick, entt::entity &entity)
{
	Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();

	const Rpg::PlayerStateOutput *out = scope.try_get<Rpg::PlayerStateOutput>(entity);

	if (!out) {
		LOG_CERROR("game") << "!!!";
		return false;
	}


	if (tick < 0) {
		const RpgStream::PlayerState *state = out->at(0);

		if (!state) {
			LOG_CERROR("game") << "Missing tick 0" << this;
			return false;
		}

		m_current = *state;

		updateBody(nullptr);
		m_player->synchronize();

		return true;
	}

	const quint32 jittered = m_game->rpgLogicClient()->jitterTick(tick);

	if (jittered == 0)
		return false;


	if (!m_incomingEventList.empty()) {
		processEventAt(jittered);

		std::erase_if(m_incomingEventList,
					  [&jittered](const RpgStream::EventPlayer &e) {
			return e.tick() <= jittered;
		});
	}


	const RpgStream::PlayerState *state = out->at(jittered);

	if (!state) {
		//LOG_CERROR("game") << "!!! STATE" << tick << jittered;
		return false;
	}


	m_current = *state;

	m_player->setHp(state->hp());
	m_player->setMp(state->mp());
	m_player->setBullet(state->bullet());
	m_player->setDefender(state->defender(), state->hasDefender());

	m_player->setLocked(state->lock() > 0);

	bool invisible = false;

	for (const auto &u : state->activeUtilities()) {
		if (u == RpgStream::PlayerConfig::UtilityInvisible) {
			invisible = true;
			break;
		}
	}

	m_player->setInvisible(invisible);


	return true;
}



/**
 * @brief RpgMotorPlayer::updateBody
 * @param object
 */

void RpgMotorPlayer::updateBody(TiledObject *)
{
	if (!m_current) {
		m_player->stop();

		if (m_player->m_sfxFootStep.isActive())
			m_player->m_sfxFootStep.stop();
		return;
	}

	updateBody(m_player, m_current.value(), false);

	m_current.reset();

	if (m_player->isRunning())
		m_player->m_sfxFootStep.startFromBegin();
	else if (m_player->m_sfxFootStep.isActive())
		m_player->m_sfxFootStep.stop();
}



/**
 * @brief RpgMotorPlayer::processEvent
 * @param event
 */

void RpgMotorPlayer::processEvent(const RpgStream::EventPlayer &event)
{
	m_incomingEventList.push_back(event);
}



/**
 * @brief RpgMotorPlayer::updateBody
 * @param player
 * @param state
 */

void RpgMotorPlayer::updateBody(RpgPlayer *player, const RpgStream::PlayerState &state, const bool &isEmplace)
{
	Q_ASSERT(player);

	cpVect to = cpv(state.entityState().posXAsFloat(),
					state.entityState().posYAsFloat());

	if (isEmplace)
		player->emplace(to);
	else
		player->TiledObjectBody::moveToPoint(to);

	player->rotateBody(state.entityState().angleAsFloat(), true);
	player->setFacingDirection(TiledObject::Direction(state.entityState().facing()));
	player->overrideCurrentSpeedSq(state.entityState().velSq());
}



/**
 * @brief RpgMotorPlayer::onAttack
 * @param player
 */

void RpgMotorPlayer::onAttack(RpgPlayer *player)
{
	Q_ASSERT(player);

	player->jumpToSprite("attack", player->facingDirection());

	player->game()->playSfx(QStringLiteral(":/rpg/common/hit.mp3"),
							player->scene(), player->bodyPositionF());

}



/**
 * @brief RpgMotorPlayer::onUseUtility
 * @param player
 */

void RpgMotorPlayer::onUseUtility(RpgPlayer *player, const bool isControlled)
{
	Q_ASSERT(player);

	if (player->currentUtility() == RpgStream::PlayerConfig::UtilitySniper) {
		player->jumpToSprite("attack", player->facingDirection());

		if (isControlled)
			player->game()->playSfx(QStringLiteral(":/rpg/broadsword/broadsword2.mp3"),
									player->scene(), player->bodyPositionF());
		return;
	} else if (player->currentUtility() == RpgStream::PlayerConfig::UtilityMissionary) {
		player->jumpToSprite("cast", player->facingDirection());
	} else if (player->currentUtility() == RpgStream::PlayerConfig::UtilityBoostAttackTower) {
		if (isControlled)
			player->game()->message(QObject::tr("Tower attack boost activated"), true);
	}

	if (isControlled)
		player->game()->playSfx(QStringLiteral(":/rpg/common/click.mp3"), player->scene());
}



/**
 * @brief RpgMotorPlayer::processEventAt
 * @param tick
 */

void RpgMotorPlayer::processEventAt(const qint64 &/*tick*/)
{
	for (const RpgStream::EventPlayer &e : m_incomingEventList) {
		if (e.type() == RpgStream::EventPlayer::EventAttackPlayer) {
			onAttack(m_player);
		} else if (e.type() == RpgStream::EventPlayer::EventUseUtility) {
			onUseUtility(m_player, false);
		}
	}
}



/**
 * @brief RpgMotorPlayerControlled::RpgMotorPlayerControlled
 * @param player
 */

RpgMotorPlayerControlled::RpgMotorPlayerControlled(RpgPlayer *player)
	: RpgDestinationMotor(player)
	, RpgMotorPlayerEventIface()
	, m_player(player)
	, d(m_player->d)
{
	Q_ASSERT(m_player);

	m_player->setSensorPolygon(SENSOR_LENGTH, M_PI * 0.5,
							   TiledObjectBody::getFilter(RpgGameItem::FixtureSensor, RpgGameItem::FixtureAll));

	m_player->addVirtualCircle(TiledObjectBody::getFilter(RpgGameItem::FixtureVirtualCircle, RpgGameItem::FixtureAll), 220.);

}










/**
 * @brief RpgMotorPlayerControlled::findNearestControl
 * @param category
 * @return
 */

TiledObjectBody *RpgMotorPlayerControlled::findNearestControl(const float &maxDist, const cpBitmask &category)
{
	QSet<TiledObjectBody*> list;

	RpgMotorEntity::queryContactedVisibleBodies(m_player, &list, category, RpgGameItem::FixtureGround,
												maxDist, QueryTarget);

	QMultiMap<float, TiledObjectBody *> bds = RpgMotorEntity::sort(m_player, list);

	for (TiledObjectBody *b : std::as_const(bds)) {
		if (checkControl(b))
			return b;
	}

	return nullptr;
}



/**
 * @brief RpgMotorPlayerControlled::findNearestControl
 * @param rayDest
 * @param category
 * @return
 */

TiledObjectBody *RpgMotorPlayerControlled::findNearestControl(const cpVect &rayDest, const cpBitmask &category)
{
	RayCastInfo ray = m_player->rayCast(rayDest, RpgGameItem::FixtureGround, category, 2.);

	for (const RayCastInfoItem &i : ray) {
		if (!i.visible)
			continue;

		TiledObjectBody *b = TiledObjectBody::fromShapeRef(i.shape);

		if (checkControl(b))
			return b;
	}

	return nullptr;
}


/**
 * @brief RpgMotorPlayerControlled::checkControl
 * @param control
 * @return
 */

bool RpgMotorPlayerControlled::checkControl(TiledObjectBody *control) const
{
	if (RpgTower *p = dynamic_cast<RpgTower*>(control)) {
		return p->canAttack() && (p->state().team() != m_player->team() || p->load() < 100);
	} else if (RpgDefenderPoint *p = dynamic_cast<RpgDefenderPoint*>(control)) {
		return p->tower() && p->tower()->state().active() &&
				m_player->hasDefender() &&
				p->tower()->state().team() == m_player->team() && !p->defender();
	} else if (RpgDefender *p = dynamic_cast<RpgDefender*>(control)) {
		return p->team() != m_player->team() && p->hp() > 0;
	} else if (RpgControl *p = dynamic_cast<RpgControl*>(control)) {
		return p->isAlive() && p->canTargeting();
	}

	return false;
}

const std::optional<cpVect> &RpgMotorPlayerControlled::targetAhead() const
{
	return m_targetAhead;
}




/**
 * @brief RpgMotorPlayerControlled::updateBody
 * @param object
 */

void RpgMotorPlayerControlled::updateBody(TiledObject *)
{
	if (!m_player->isAlive() || m_game->gameState() != RpgGame::GameStatePlay) {
		m_player->stop();
		d->applyKnockback();
		m_player->setCurrentChunk({-1,-1});
		m_player->setCurrentChunkCenter({-1,-1});

		m_player->setTargetControl(nullptr);
		m_player->setTargetEntity(nullptr);
		m_player->setUtilityEntity(nullptr);
		return;
	}

	// Moving (JoystickA)

	if (d->m_lockId > 0) {
		m_player->stop();
		d->applyKnockback();
		m_player->setCurrentChunk({-1,-1});
		m_player->setCurrentChunkCenter({-1,-1});
		return;
	}


	const QString &sprite = m_player->m_spriteHandler->currentSprite();

	static const QStringList disabledList = {
		QStringLiteral("attack"),
		QStringLiteral("bow"),
		QStringLiteral("cast"),
		//QStringLiteral("hurt"),
		QStringLiteral("death")
	};

	if (disabledList.contains(sprite)) {
		m_player->stop();
		d->applyKnockback();
		return;
	}

	if (m_currentJoystickState.distance >= 1.0) {
		m_targetAngle = std::nullopt;

		m_destinationMotor.reset();
		m_destinationPoint = std::nullopt;

		m_player->setSpeedFromAngle(m_currentJoystickState.angle, m_player->m_config.run);
		m_player->rotateBody(m_currentJoystickState.angle);
	} else if (m_currentJoystickState.distance > 0.5) {
		m_targetAngle = std::nullopt;

		m_destinationMotor.reset();
		m_destinationPoint = std::nullopt;

		m_player->setSpeedFromAngle(m_currentJoystickState.angle, m_player->m_config.walk);
		m_player->rotateBody(m_currentJoystickState.angle);
	} else if (m_destinationPoint) {
		m_targetAngle = std::nullopt;

		if (!m_player->moveTowardsLimited(m_destinationPoint.value(),
										  m_player->m_config.walk,
										  m_player->m_config.run*0.5,
										  m_player->m_config.run)) {
			m_player->stop();
			m_player->emplace(m_destinationPoint.value());
			m_destinationPoint = std::nullopt;
		}
	} else if (m_destinationMotor) {
		m_targetAngle = std::nullopt;

		if (m_destinationMotor->atEnd(m_player)) {
			m_player->stop();
			m_destinationMotor.reset();
		} else if (const QPolygonF &polygon = m_destinationMotor->polygon(); !polygon.isEmpty()) {
			const float distance = m_player->distanceToPointSq(polygon.last());

			if (distance >= POW2(m_player->m_config.run*0.5)) {				// Hogy a végén szépen lassan gyalogoljon csak
				m_destinationMotor->setSpeed(m_player->m_config.run);
				m_destinationMotor->updateBody(m_player);
			} else {
				m_destinationMotor->setSpeed(m_player->m_config.walk);
				m_destinationMotor->updateBody(m_player);
			}
		} else {
			m_player->stop();
			m_destinationMotor.reset();
		}

	} else {
		m_player->stop();
		if (m_currentJoystickState.distance > 0.1)
			m_player->rotateBody(m_currentJoystickState.angle);
	}


	// Manage ground collision

	if (auto ptr = destination(); ptr && !d->m_groundCollision.empty()) {
		const auto path = m_gameItem->findShortestPath(m_player, ptr->last().x(), ptr->last().y());

		if (!path) {
			LOG_CERROR("game") << "No available path";
		} else {
			setDestination(path.value());
		}
	}


	/// Workaround
	///
	/// a worldStep() még az updateBody() előtt frissíti a currentSpeedSq-t,	az updateBody() után viszont nem
	/// a saveState() emiatt az első ticknél nem mutat különbséget az előzővel (mivel a sebesség 0), ezért nem küldi el
	/// itt elvégezzük ezt a műveletet a saveState() előtt

	m_player->overrideCurrentSpeedSq(cpvlengthsq(cpBodyGetVelocity(m_player->body())));

	d->applyKnockback();



	/// Joystick B


	if (m_player->joystickMode() == RpgPlayer::JoystickModeControl) {

		// Joystick - control

		m_player->setTargetEntity(nullptr);
		m_player->setUtilityEntity(nullptr);
		d->m_isTargetAuto = false;

		if (d->m_controlActionDisable) {
			m_player->setTargetControl(nullptr);
		} else {
			float targetDist = 250;			// TODO:

			if (m_targetJoystickState.hasTouch) {
				if (m_targetJoystickState.distance > 0.1)
					m_targetAngle = m_targetJoystickState.angle;
				else if (!m_player->targetControl() && !m_targetAngle.has_value())
					m_targetAngle = m_player->desiredBodyRotation();

				targetDist *= std::clamp(m_targetJoystickState.distance, 0.3, 1.0);
			}

			if (m_targetAngle.has_value()) {
				cpVect ahead = m_player->bodyPosition()+TiledObjectBody::vectorFromAngle(m_targetAngle.value(), targetDist);

				m_player->setTargetControl(findNearestControl(ahead));
			} else {
				m_player->setTargetControl(findNearestControl(targetDist));
			}
		}

		if (m_player->targetControl() || !m_targetJoystickState.hasTouch
				|| m_targetJoystickState.distance <= 0.1
				|| !m_targetAngle.has_value()
				|| !m_player->hasDefender()) {
			m_player->setCurrentChunk({-1,-1});
			m_player->setCurrentChunkCenter({-1,-1});
		} else {
			cpVect center;


			const QPoint ch = m_game->rpgLogicClient()->getChunkFromVector(m_player->bodyPosition(),
																		   m_targetAngle.value(),
																		   &center);

			m_player->setCurrentChunk(ch);
			m_player->setCurrentChunkCenter(TiledObjectBody::toPointF(center));

		}


	} else if (m_player->joystickMode() == RpgPlayer::JoystickModeTarget) {

		// Joystick target

		m_player->setUtilityEntity(nullptr);

		if (m_player->bullet() <= 0) {
			m_player->setTargetEntity(nullptr);

			m_player->setTargetControl(findNearestControl(0.));
			return;
		}


		const float dist = std::max(SENSOR_LENGTH, 450.);			// TODO: weapon length

		if (m_targetJoystickState.hasTouch) {
			if (m_targetJoystickState.distance > 0.2) {
				m_targetAngle = m_targetJoystickState.angle;
				d->m_joystickC_hasTouch = true;
			} else {
				m_targetAngle = std::nullopt;

				if (d->m_joystickC_hasTouch)
					m_player->setTargetEntity(nullptr);
			}


			if (m_targetAngle.has_value()) {
				cpVect ahead = m_player->bodyPosition()+TiledObjectBody::vectorFromAngle(m_targetAngle.value(), dist);

				RpgEntity *next = findNearestTarget(ahead,
													RpgGameItem::FixturePlayerBody | RpgGameItem::FixturePlayerTarget |
													RpgGameItem::FixtureNpcBody | RpgGameItem::FixtureNpcTarget
													);

				if (!m_player->targetEntity() || next) {
					m_player->setTargetEntity(next);
					d->m_isTargetAuto = false;
				}

				m_targetAhead = ahead;

				m_player->setTargetControl(nullptr);

				return;
			}

		} else {
			d->m_joystickC_hasTouch = false;
		}

		m_targetAhead = std::nullopt;

		if (m_player->targetEntity()) {
			if (!m_player->targetEntity()->isAlive()) {
				m_player->setTargetEntity(nullptr);
			} else {
				RayCastInfo ray = m_player->rayCast(m_player->targetEntity()->bodyPosition(),
													RpgGameItem::FixtureGround,
													RpgGameItem::FixturePlayerBody | RpgGameItem::FixturePlayerTarget |
													RpgGameItem::FixtureNpcBody | RpgGameItem::FixtureNpcTarget,
													2.);


				if (!ray.isVisible(m_player->targetEntity()) ||
						m_player->distanceToPointSq(m_player->targetEntity()->bodyPosition()) > POW2(dist))
					m_player->setTargetEntity(nullptr);
			}
		} else if (!m_targetJoystickState.hasTouch) {
			RpgEntity *next = findNearestTarget(
								  RpgGameItem::FixturePlayerBody | RpgGameItem::FixturePlayerTarget |
								  RpgGameItem::FixtureNpcBody | RpgGameItem::FixtureNpcTarget
								  );

			if (next) {
				m_player->setTargetEntity(next);
				d->m_isTargetAuto = true;
			}
		}


		if (m_player->targetEntity())
			m_player->setTargetControl(nullptr);
		else
			m_player->setTargetControl(findNearestControl(0.));


	} else if (m_player->joystickMode() == RpgPlayer::JoystickModeUtility) {

		// Joystick utility

		m_player->setTargetEntity(nullptr);
		m_player->setTargetControl(nullptr);

		cpBitmask utilityBitmask = RpgGameItem::FixtureInvalid;
		const float utilityTarget = utilityRequireTarget(&utilityBitmask);


		if (utilityTarget == 0.f) {
			m_player->setUtilityEntity(nullptr);
			m_player->setTargetControl(findNearestControl(0.));
			return;
		}


		const float dist = std::max(SENSOR_LENGTH, 450.);			// TODO: weapon length

		if (m_targetJoystickState.hasTouch) {
			if (m_targetJoystickState.distance > 0.2) {
				m_targetAngle = m_targetJoystickState.angle;
				d->m_joystickC_hasTouch = true;
			} else {
				m_targetAngle = std::nullopt;

				if (d->m_joystickC_hasTouch)
					m_player->setUtilityEntity(nullptr);
			}


			if (m_targetAngle.has_value()) {
				cpVect ahead = m_player->bodyPosition()+TiledObjectBody::vectorFromAngle(m_targetAngle.value(), dist);

				RpgEntity *next = findNearestTarget(ahead, utilityBitmask);

				if (!m_player->utilityEntity() || next) {
					m_player->setUtilityEntity(next);
					d->m_isTargetAuto = false;
				}

				m_targetAhead = ahead;

				m_player->setTargetControl(nullptr);

				return;
			}

		} else {
			d->m_joystickC_hasTouch = false;
		}

		m_targetAhead = std::nullopt;

		if (m_player->utilityEntity()) {
			if (!m_player->utilityEntity()->isAlive()) {
				m_player->setUtilityEntity(nullptr);
			} else {
				RayCastInfo ray = m_player->rayCast(m_player->utilityEntity()->bodyPosition(),
													RpgGameItem::FixtureGround,
													utilityBitmask,
													2.);


				if (!ray.isVisible(m_player->utilityEntity()) ||
						m_player->distanceToPointSq(m_player->utilityEntity()->bodyPosition()) > POW2(dist))
					m_player->setUtilityEntity(nullptr);
			}
		} else if (!m_targetJoystickState.hasTouch) {
			if (RpgEntity *next = findNearestTarget(utilityBitmask)) {
				m_player->setUtilityEntity(next);
				d->m_isTargetAuto = true;
			}
		}

		if (m_player->utilityEntity())
			m_player->setTargetControl(nullptr);
		else
			m_player->setTargetControl(findNearestControl(0.));
	}

}









/**
 * @brief RpgMotorPlayerControlled::beforeWorldStep
 * @param tick
 * @param entity
 * @return
 */

bool RpgMotorPlayerControlled::beforeWorldStep(const qint64 &tick, entt::entity &entity)
{
	d->m_currentKnockback = cpvzero;

	{
		Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();

		const Rpg::PlayerStateOutput *out = scope.try_get<Rpg::PlayerStateOutput>(entity);

		if (!out) {
			LOG_CERROR("game") << "!!!";
			return false;
		}


		if (tick < 0) {
			const RpgStream::PlayerState *state = out->at(0);

			if (!state) {
				LOG_CERROR("game") << "Missing tick 0" << this;
				return false;
			}

			RpgMotorPlayer::updateBody(m_player, *state, true);

			saveCurrentState(0);

			m_player->synchronize();

			return true;
		}

		if (const quint32 diff = m_game->rpgLogicClient()->lastAuthDiff() +
				m_game->rpgLogicClient()->jitterDiff()/2; tick >= diff)
		{
			const quint32 slideTick = tick-diff;
			const RpgStream::PlayerState *state = out->at(slideTick);

			if (state && (state->entityState().slideX() != 0 || state->entityState().slideY() != 0))
				d->m_currentKnockback = cpv(state->entityState().slideXAsFloat(), state->entityState().slideYAsFloat());
		}

	}



	const RpgStream::PlayerState *latest = nullptr;

	const std::map<quint32, RpgStream::PlayerState> sim = m_game->rpgLogicClient()->getSimulatedStates(entity, m_statePull, &latest);

	if (!latest) {
		LOG_CERROR("game") << "!!!" << tick;
		return false;
	}

	const RpgStream::PlayerState *state = nullptr;


	if (!sim.empty() && sim.cbegin()->second == *latest) {
		state = &(sim.rbegin()->second);
	} else {
		bool reqEmplace = true;

		if (!sim.empty()) {
			// Csak akkor helyezzük vissza, ha a pozíció sem stimmel (pl. ha mp-t vett fel, és emiatt változott a státusz, akkor nem bántjuk
			// Később (pl. hp == 0) lekezeljük újra

			if (sim.cbegin()->first > latest->tick()) {
				LOG_CTRACE("game") << "State gap" << latest->tick() << sim.cbegin()->first;
				reqEmplace = false;
			} else if (sim.cbegin()->second.entityState().isEqualWithoutSlide(latest->entityState()))
				reqEmplace = false;
		}

		if (reqEmplace && !cpveql(d->m_currentKnockback, cpvzero)) {
			reqEmplace = false;
		}

		state = latest;

		if (reqEmplace)
			RpgMotorPlayer::updateBody(m_player, *state, true);
	}



	// Ha most vált halottra vagy éppen most támad fel

	if (state->hp() == 0 || m_player->hp() == 0) {
		RpgMotorPlayer::updateBody(m_player, *state, true);
		//m_player->emplace(state->entityState().posXAsFloat(), state->entityState().posYAsFloat());
	}


	bool nextControlState = (state->penalty() >= tick);

	if (!nextControlState && d->m_controlActionDisable && m_player->isAlive()) {
		m_game->gameItem()->message(QObject::tr("Penalty time over"));
	}


	m_player->setHp(state->hp());
	m_player->setMp(state->mp());
	m_player->setBullet(state->bullet());
	m_player->setDefender(state->defender(), state->hasDefender());
	m_player->setUtility(state->utility(), state->hasUtility());

	m_game->setQuestQuestion(state->question());
	m_game->setQuestStreak(state->streak());

	bool oldLock = m_player->locked();

	m_player->setLocked(state->lock() > 0 || d->m_lockedEvent);
	//d->m_lockId = state->lock();
	d->m_penalty = state->penalty();
	d->updateLock(tick, state->lock());

	// Ha a locked status váltott

	if (m_player->locked() != oldLock) {
		RpgMotorPlayer::updateBody(m_player, *state, true);
		//m_player->emplace(state->entityState().posXAsFloat(), state->entityState().posYAsFloat());
	}


	d->m_controlActionDisable = nextControlState;

	if (d->m_controlActionDisable) {
		if (state->penalty() > d->m_controlActionDisableLastNotification && m_player->isAlive()) {
			int sec = std::ceil(AbstractGame::TickTimer::tickToMsec(state->penalty() - tick)/1000.);
			m_game->gameItem()->messageColor(QObject::tr("%1 sec penalty").arg(sec),
											 QColorConstants::Svg::red);

			if (GameQuestion *gq = m_game->gameQuestion()) {
				gq->setProperty("progressColor", QColorConstants::Svg::red);
				gq->setProperty("msecLeft", m_game->msecLeft()
								-AbstractGame::TickTimer::tickToMsec(state->penalty() - tick));
			}
		}

		d->m_controlActionDisableLastNotification = state->penalty();
	}



	// Load utilities

	QSet<RpgStream::PlayerConfig::Utility> tmp = d->m_activeUtilities;

	for (const RpgStream::PlayerConfig::Utility &u : state->activeUtilities()) {
		tmp.remove(u);

		if (!d->m_activeUtilities.contains(u)) {
			d->m_activeUtilities.insert(u);

			if (u == RpgStream::PlayerConfig::UtilityInvisible) {
				m_game->gameItem()->message(QObject::tr("You are invisible now"), true);
			}
		}
	}

	for (const RpgStream::PlayerConfig::Utility &u : tmp) {
		// TODO: message...
		d->m_activeUtilities.remove(u);

		if (u == RpgStream::PlayerConfig::UtilityInvisible) {
			m_game->gameItem()->message(QObject::tr("You are visible now"));
		}
	}


	m_player->setInvisible(d->m_activeUtilities.contains(RpgStream::PlayerConfig::UtilityInvisible));



	if (!m_incomingEventList.empty()) {
		processEventAt(state->tick());

		std::erase_if(m_incomingEventList,
					  [jittered = state->tick()](const RpgStream::EventPlayer &e) {
			return e.tick() <= jittered;
		});
	}

	return true;
}





/**
 * @brief RpgMotorPlayerControlled::afterWorldStep
 * @param tick
 * @param state
 * @return
 */

bool RpgMotorPlayerControlled::afterWorldStep(const qint64 &tick, RpgStream::FullState *state)
{
	if (m_player->isRunning())
		m_player->m_sfxFootStep.startFromBegin();
	else if (m_player->m_sfxFootStep.isActive())
		m_player->m_sfxFootStep.stop();

	if (!state)
		return false;

	const quint32 tagId = RpgLogicObjectMapper::getId(m_player->objectId());


	saveCurrentState(tick);

	std::vector<RpgStream::PlayerState> list =
			m_statePull.extractAtLeast(m_game->gameMode() == RpgGame::MultiPlayer ? state->serverTick() : 0,
									   m_game->gameMode() == RpgGame::MultiPlayer ? 6 : 1);			// SINGLE PLAYER: 1



	RpgStream::PlayerStateList sl;
	sl.setTagId(tagId);
	sl.setIsDeltaMode(state->isDeltaMode());
	if (state->isDeltaMode())
		sl.compressStateVector(std::move(list));
	else
		sl.setState(std::move(list));

	state->flags().setFlag(RpgStream::FullState::Player);
	state->players().push_back(std::move(sl));


	if (!d->m_eventList.empty()) {
		for (RpgStream::EventPlayer &e : d->m_eventList) {
			e.setTagId(tagId);
			// Nem kell tick, a szerver az RpgStream::Events-ét nézi
		}

		RpgStream::Events events;
		events.setTick(m_gameItem->tickTimer()->currentTick());					// Ide a render lag miatt nem a <tick>-et tesszük!
		events.flags().setFlag(RpgStream::Events::Player);
		events.setPlayer(d->m_eventList);

		state->flags().setFlag(RpgStream::FullState::Event);
		state->events().emplace_back(std::move(events));

		d->m_eventList.clear();
	}


	return true;
}



/**
 * @brief RpgMotorPlayerControlled::saveCurrentState
 * @return
 */

const RpgStream::PlayerState *RpgMotorPlayerControlled::saveCurrentState(const qint64 &tick)
{
	RpgStream::PlayerState st;
	st.setTick(tick);
	st.entityState().setPosXAsFloat(m_player->bodyPosition().x);
	st.entityState().setPosYAsFloat(m_player->bodyPosition().y);
	st.entityState().setAngleAsFloat(m_player->desiredBodyRotation());


	static const quint32 streamMaxSpeed = (quint32){1} << SPEEDSQ_SIZE_BITS;

	if (m_player->currentSpeedSq() > streamMaxSpeed) {
		LOG_CERROR("engine") << "SpeedSq size error" << m_player->currentSpeedSq() << ">" << streamMaxSpeed;
	}

	st.entityState().setFacing(m_player->facingDirection());
	st.entityState().setVelSq(m_player->currentSpeedSq());
	//st.entityState().setSlideXAsFloat(m_player->d->m_knockbackVelocity.x);
	//st.entityState().setSlideYAsFloat(m_player->d->m_knockbackVelocity.y);


	st.setHp(m_player->hp());
	st.setMp(m_player->mp());
	st.setBullet(m_player->bullet());
	st.setLock(d->m_lockId);
	st.setPenalty(d->m_penalty);
	st.setDefender(m_player->m_defender);
	st.setHasDefender(m_player->m_hasDefender);
	st.setUtility(m_player->m_utility);
	st.setHasUtility(m_player->m_hasUtility);
	st.setStreak(m_game->questStreak());
	st.setQuestion(m_game->questQuestion());


	m_statePull.append(std::move(st));

	return m_statePull.last();
}




/**
 * @brief RpgMotorPlayerControlled::currentJoystickState
 * @return
 */

TiledGame::JoystickState RpgMotorPlayerControlled::currentJoystickState() const
{
	return m_currentJoystickState;
}

void RpgMotorPlayerControlled::setCurrentJoystickState(const TiledGame::JoystickState &newCurrentJoystickState)
{
	m_currentJoystickState = newCurrentJoystickState;
}





/**
 * @brief RpgMotorPlayerControlled::attackCurrentTarget
 */

void RpgMotorPlayerControlled::attackCurrentTarget()
{
	if (!m_player->targetEntity() || !m_player->targetEntity()->isAlive()) {
		LOG_CWARNING("game") << "Invalid target";
		m_player->m_sfxDecline.playOne();
		return;
	}

	if (m_player->bullet() <= 0) {
		LOG_CWARNING("game") << "Missing bullet";
		m_player->m_sfxDecline.playOne();
		return;
	}

	m_player->rotateToPoint(m_player->targetEntity()->bodyPosition());

	if (m_game->hasActiveTargetUtility(RpgStream::PlayerConfig::UtilityBlockAttack, m_player->team())) {
		//m_game->gameItem()->message(QObject::tr("Attack blocked"));
		m_player->m_sfxDecline.playOne();
		return;
	}

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventAttackPlayer);
	e.setSeq(m_player->nextEventId());
	e.setTarget(RpgLogicObjectMapper::getId(m_player->targetEntity()->objectId()));

	RpgMotorPlayer::onAttack(m_player);

	d->m_eventList.emplace_back(std::move(e));

}





/**
 * @brief RpgMotorPlayerControlled::useCurrentControl
 */

void RpgMotorPlayerControlled::useCurrentControl()
{
	const qint64 tick = m_gameItem->tickTimer()->currentTick();

	if (RpgTower *tower = dynamic_cast<RpgTower*>(m_player->targetControl())) {
		if (m_game->hasActiveTargetUtility(RpgStream::PlayerConfig::UtilityBlockAttack, m_player->team())) {
			//m_game->gameItem()->message(QObject::tr("Attack blocked"));
			m_player->m_sfxDecline.playOne();
			return;
		}

		RpgStream::EventPlayer e(RpgStream::EventPlayer::EventTower);
		e.setSeq(m_player->nextEventId());
		e.setTarget(RpgLogicObjectMapper::getId(tower->objectId()));

		LOG_CDEBUG("game") << "Lock request" << e.seq();

		d->m_lockedEvent = e;
		d->m_waitForLock = tick + 5*60;			// Wait for lockId from server

		d->m_eventList.emplace_back(std::move(e));

		return;
	}


	if (RpgDefender *p = dynamic_cast<RpgDefender*>(m_player->targetControl())) {
		if (m_game->hasActiveTargetUtility(RpgStream::PlayerConfig::UtilityBlockAttack, m_player->team())) {
			//m_game->gameItem()->message(QObject::tr("Attack blocked"));
			m_player->m_sfxDecline.playOne();
			return;
		}

		RpgStream::EventPlayer e(RpgStream::EventPlayer::EventAttackDefender);
		e.setSeq(m_player->nextEventId());
		e.setTarget(RpgLogicObjectMapper::getId(p->objectId()));

		LOG_CDEBUG("game") << "Lock request" << e.seq();

		d->m_lockedEvent = e;
		d->m_waitForLock = tick + 5*60;			// Wait for lockId from server

		d->m_eventList.emplace_back(std::move(e));

		return;
	}


	if (RpgControl *p = dynamic_cast<RpgControl*>(m_player->targetControl())) {
		RpgStream::EventPlayer e(RpgStream::EventPlayer::EventUseControl);
		e.setSeq(m_player->nextEventId());
		e.setTarget(RpgLogicObjectMapper::getId(p->objectId()));

		LOG_CDEBUG("game") << "Lock request" << e.seq();

		d->m_lockedEvent = e;
		d->m_waitForLock = tick + 5*60;			// Wait for lockId from server

		d->m_eventList.emplace_back(std::move(e));

		return;
	}



	if (dynamic_cast<RpgDefenderPoint*>(m_player->targetControl()))
		return putDefender(true);

}





/**
 * @brief RpgMotorPlayerControlled::useCurrentUtility
 */

void RpgMotorPlayerControlled::useCurrentUtility()
{
	if (!m_player->m_hasUtility || !m_player->m_canUseUtility) {
		LOG_CWARNING("game") << "Invalid utility";
		m_player->m_sfxDecline.playOne();
		return;
	}

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventUseUtility);
	e.setSeq(m_player->nextEventId());

	if (m_player->utilityEntity())
		e.setTarget(RpgLogicObjectMapper::getId(m_player->utilityEntity()->objectId()));

	RpgMotorPlayer::onUseUtility(m_player, true);

	d->m_eventList.emplace_back(std::move(e));
}



/**
 * @brief RpgMotorPlayerControlled::updateUseUtility
 */

void RpgMotorPlayerControlled::updateUseUtility()
{
	if (!m_player->hasUtility())
		return m_player->setCanUseUtility(false);

	if (m_player->m_utility == RpgStream::PlayerConfig::UtilityMissionary) {
		RpgNpc *target = qobject_cast<RpgNpc*>(m_player->utilityEntity());

		if (!target)
			return m_player->setCanUseUtility(false);

		return m_player->setCanUseUtility(target->team() != m_player->team());

	} else if (m_player->m_utility == RpgStream::PlayerConfig::UtilitySniper) {
		RpgDefender *defender = qobject_cast<RpgDefender*>(m_player->utilityEntity());
		RpgNpc *target = qobject_cast<RpgNpc*>(m_player->utilityEntity());

		if (!target && !defender)
			return m_player->setCanUseUtility(false);

		if (target)
			return m_player->setCanUseUtility(target->team() != m_player->team());

		if (defender)
			return m_player->setCanUseUtility(defender->team() != m_player->team());
	} else if (m_player->m_utility == RpgStream::PlayerConfig::UtilityBoostAttackTower) {
		// Csak a normál módon használhatjuk (tower attack)
		m_player->setCanUseUtility(false);

		return;
	}

	m_player->setCanUseUtility(true);
}




/**
 * @brief RpgMotorPlayerControlled::utilityRequireTarget
 * @return
 */

float RpgMotorPlayerControlled::utilityRequireTarget(cpBitmask *categoryPtr) const
{
	if (categoryPtr)
		*categoryPtr = RpgGameItem::FixtureInvalid;

	if (!m_player->hasUtility())
		return 0.f;

	if (m_player->m_utility == RpgStream::PlayerConfig::UtilityMissionary) {
		if (categoryPtr)
			*categoryPtr = RpgGameItem::FixtureNpcBody | RpgGameItem::FixtureNpcTarget;

		return cfgUtilityMissionary.dist;
	}

	if (m_player->m_utility == RpgStream::PlayerConfig::UtilitySniper) {
		if (categoryPtr)
			*categoryPtr = RpgGameItem::FixtureNpcBody | RpgGameItem::FixtureNpcTarget |
						   RpgGameItem::FixtureDefender;

		return cfgUtilitySniper.dist;
	}

	return 0.f;
}



/**
 * @brief RpgMotorPlayerControlled::putDefender
 */

void RpgMotorPlayerControlled::putDefender(const bool &click)
{
	if (RpgDefenderPoint *p = dynamic_cast<RpgDefenderPoint*>(m_player->targetControl())) {
		if (!RpgStream::BaseDefenderObject::placementFlags(m_player->currentDefender())
				.testFlags(RpgStream::BaseDefenderObject::PlacementTower)) {
			m_player->m_sfxDecline.playOne();
			return;
		}

		RpgStream::EventPlayer e(RpgStream::EventPlayer::EventDefender);
		e.setSeq(m_player->nextEventId());
		e.setTarget(RpgLogicObjectMapper::getId(p->objectId()));

		d->m_eventList.emplace_back(std::move(e));

		return;
	}

	if (click) {
		LOG_CWARNING("game") << "Just click";
		//useCurrentControl();
		//return;
	}

	if (const QPoint &ch = m_player->currentChunk(); ch.x() >= 0 && ch.y() >= 0) {
		if (!RpgStream::BaseDefenderObject::placementFlags(m_player->currentDefender())
				.testFlags(RpgStream::BaseDefenderObject::PlacementChunk)) {
			m_player->m_sfxDecline.playOne();
		}

		RpgStream::EventPlayer e(RpgStream::EventPlayer::EventDefender);
		e.setSeq(m_player->nextEventId());
		e.chunk().setX(ch.x());
		e.chunk().setY(ch.y());

		d->m_eventList.emplace_back(std::move(e));
	}

	LOG_CWARNING("game") << "Missing point";
}




/**
 * @brief RpgMotorPlayerControlled::changeMpToBullet
 */

void RpgMotorPlayerControlled::changeMpToBullet(const bool &force)
{
	if (m_player->bullet() > 0 && !force) {
		m_player->setJoystickMode(RpgPlayer::JoystickModeTarget);
		return;
	}

	if (m_player->mp() < CFG_MP_CHANGE_BULLET) {
		m_player->game()->message(QObject::tr("Not enough MP"));
		emit m_player->m_rpgGame->gameItem()->mpMarkerRequest();
		d->vibrate();
		return;
	}

	if (m_game->hasActiveTargetUtility(RpgStream::PlayerConfig::UtilityBlockMpConvert, m_player->team())) {
		m_game->gameItem()->message(QObject::tr("MP conversion blocked"));
		m_player->m_sfxDecline.playOne();
		d->vibrate();
		return;
	}

	const qint64 tick = m_gameItem->tickTimer()->currentTick();

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventChangeBullet);

	e.setSeq(m_player->nextEventId());

	LOG_CDEBUG("game") << "Lock request" << e.seq();

	d->m_lockedEvent = e;
	d->m_waitForLock = tick + 5*60;			// Wait for lockId from server

	d->m_eventList.emplace_back(std::move(e));
}



/**
 * @brief RpgMotorPlayerControlled::changeMpToDefender
 */

void RpgMotorPlayerControlled::changeMpToDefender()
{
	if (m_player->m_defender == RpgStream::BaseDefenderObject::None) {
		m_player->game()->message(QObject::tr("Select defender"));
		d->vibrate();
		return;
	}

	if (m_player->m_hasDefender) {
		m_player->setJoystickMode(RpgPlayer::JoystickModeControl);
		//m_player->game()->message(QObject::tr("Already have a defender"));
		//d->vibrate();
		return;
	}

	if (m_player->mp() < cfgRequiredMpDefender.value(m_player->m_defender)) {
		m_player->game()->message(QObject::tr("Not enough MP"));
		emit m_player->m_rpgGame->gameItem()->mpMarkerRequest();
		d->vibrate();
		return;
	}

	if (m_game->hasActiveTargetUtility(RpgStream::PlayerConfig::UtilityBlockMpConvert, m_player->team())) {
		m_game->gameItem()->message(QObject::tr("MP conversion blocked"));
		m_player->m_sfxDecline.playOne();
		d->vibrate();
		return;
	}

	const qint64 tick = m_gameItem->tickTimer()->currentTick();

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventChangeDefender);

	e.setSeq(m_player->nextEventId());

	LOG_CDEBUG("game") << "Lock request" << e.seq();

	d->m_lockedEvent = e;
	d->m_waitForLock = tick + 5*60;			// Wait for lockId from server

	d->m_eventList.emplace_back(std::move(e));
}



/**
 * @brief RpgMotorPlayerControlled::changeMpToUtility
 */

void RpgMotorPlayerControlled::changeMpToUtility()
{
	if (m_player->m_utility == RpgStream::PlayerConfig::UtilityNone) {
		m_player->game()->message(QObject::tr("Select utility"));
		d->vibrate();
		return;
	}

	if (m_player->m_hasUtility) {
		m_player->setJoystickMode(RpgPlayer::JoystickModeUtility);
		//m_player->game()->message(QObject::tr("Already have an utility"));
		//d->vibrate();
		return;
	}

	if (m_player->mp() < cfgRequiredMpUtility.value(m_player->m_utility)) {
		m_player->game()->message(QObject::tr("Not enough MP"));
		emit m_player->m_rpgGame->gameItem()->mpMarkerRequest();
		d->vibrate();
		return;
	}

	if (m_game->hasActiveTargetUtility(RpgStream::PlayerConfig::UtilityBlockMpConvert, m_player->team())) {
		m_game->gameItem()->message(QObject::tr("MP conversion blocked"));
		m_player->m_sfxDecline.playOne();
		d->vibrate();
		return;
	}

	const qint64 tick = m_gameItem->tickTimer()->currentTick();

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventChangeUtility);

	e.setSeq(m_player->nextEventId());

	LOG_CDEBUG("game") << "Lock request" << e.seq();

	d->m_lockedEvent = e;
	d->m_waitForLock = tick + 5*60;			// Wait for lockId from server

	d->m_eventList.emplace_back(std::move(e));
}





/**
 * @brief RpgMotorPlayerControlled::replaceDefender
 * @param type
 */

void RpgMotorPlayerControlled::replaceDefender(const RpgStream::BaseDefenderObject::Type &type)
{
	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventReplaceDefender);

	e.setSeq(m_player->nextEventId());
	e.setAt(type);

	d->m_eventList.emplace_back(std::move(e));
}




/**
 * @brief RpgMotorPlayerControlled::replaceUtility
 * @param type
 */

void RpgMotorPlayerControlled::replaceUtility(const RpgStream::PlayerConfig::Utility &type)
{
	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventReplaceUtility);

	e.setSeq(m_player->nextEventId());
	e.setAt(type);

	d->m_eventList.emplace_back(std::move(e));
}





/**
 * @brief RpgMotorPlayerControlled::processEvent
 * @param event
 */

void RpgMotorPlayerControlled::processEvent(const RpgStream::EventPlayer &event)
{
	m_incomingEventList.push_back(event);

	if (event.lockId() == 0)
		d->resetLock(event);


	if (event.type() == RpgStream::EventPlayer::EventChangeBullet) {
		m_player->setJoystickMode(RpgPlayer::JoystickModeTarget);
	} else if (event.type() == RpgStream::EventPlayer::EventChangeDefender) {
		m_player->setJoystickMode(RpgPlayer::JoystickModeControl);
	} else if (event.type() == RpgStream::EventPlayer::EventChangeUtility) {
		m_player->setJoystickMode(RpgPlayer::JoystickModeUtility);
	}
}




/**
 * @brief RpgMotorPlayerControlled::onShapeContactBegin
 * @param self
 * @param other
 */

void RpgMotorPlayerControlled::onShapeContactBegin(cpShape *self, cpShape *other)
{
	TiledObjectBody *otherBody = TiledObjectBody::fromShapeRef(other);

	if (!otherBody) {
		LOG_CERROR("game") << "****ERR";
		return;
	}

	const cpShapeFilter &filter = cpShapeGetFilter(other);

	if (self == m_player->virtualCircle()) {
		if (RpgTower *o = dynamic_cast<RpgTower*>(otherBody))
			o->setDefenderLayersVisible(m_player->team());

		if (RpgObject *o = dynamic_cast<RpgObject*>(otherBody))
			o->setMarked(true);
		else if (TiledVisualItem *item = qobject_cast<TiledVisualItem*>(otherBody->visualItem()))
			item->setGlowEnabled(true);
	}

	/*if (self == m_player->targetCircle() || m_player->isBodyShape(self) ||
			self == m_player->sensorPolygon()) {

		addContactedTarget(self);
	}*/

	if (m_player->isBodyShape(self)) {
		if (filter.categories & RpgGameItem::FixtureGround) {
			d->m_groundCollision.insert(other);
		}

		if (RpgMp *mp = dynamic_cast<RpgMp*>(otherBody)) {
			eventMpPick(mp);
			return;
		}
	}

}




/**
 * @brief RpgMotorPlayerControlled::onShapeContactEnd
 * @param self
 * @param other
 */

void RpgMotorPlayerControlled::onShapeContactEnd(cpShape *self, cpShape *other)
{
	TiledObjectBody *otherBody = TiledObjectBody::fromShapeRef(other);

	if (!otherBody) {
		LOG_CERROR("game") << "****ERR";
		return;
	}

	const cpShapeFilter &filter = cpShapeGetFilter(other);

	if (self == m_player->virtualCircle()) {
		if (RpgTower *o = dynamic_cast<RpgTower*>(otherBody))
			o->setDefenderLayersVisible(RpgStream::TeamNone);

		if (RpgObject *o = dynamic_cast<RpgObject*>(otherBody))
			o->setMarked(false);
		else if (TiledVisualItem *item = qobject_cast<TiledVisualItem*>(otherBody->visualItem()))
			item->setGlowEnabled(false);
	}


	if (m_player->isBodyShape(self) && (filter.categories & RpgGameItem::FixtureGround)) {
		d->m_groundCollision.remove(other);
	}

	/*if (self == m_player->targetCircle() || m_player->isBodyShape(self) ||
			self == m_player->sensorPolygon()) {

		removeContactedTarget(self);
	}*/

}



/**
 * @brief RpgMotorPlayerControlled::eventMpPick
 * @param mp
 */

void RpgMotorPlayerControlled::eventMpPick(RpgMp *mp)
{
	if (!mp)
		return;

	if (m_game->hasActiveTargetUtility(RpgStream::PlayerConfig::UtilityBlockMpPick, m_player->team())) {
		//m_game->gameItem()->message(QObject::tr("MP pick blocked"));
		m_player->m_sfxDecline.playOne();
		return;
	}

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventMpPick);
	e.setSeq(m_player->nextEventId());
	e.setTarget(RpgLogicObjectMapper::getId(mp->objectId()));

	d->m_eventList.emplace_back(std::move(e));
}








/**
 * @brief RpgMotorPlayerControlled::targetJoystickState
 * @return
 */

TiledGame::JoystickState RpgMotorPlayerControlled::targetJoystickState() const
{
	return m_targetJoystickState;
}

void RpgMotorPlayerControlled::setTargetJoystickState(const TiledGame::JoystickState &newTargetJoystickState)
{
	m_targetJoystickState = newTargetJoystickState;
}



/**
 * @brief RpgMotorPlayerControlled::questionFinished
 * @param success
 */

void RpgMotorPlayerControlled::questionFinished(const bool &success)
{
	/*if (!d->m_lockedEvent) {
		LOG_CERROR("game") << "Missing locked event";
		return;
	}*/

	if (d->m_lockId == 0) {
		LOG_CERROR("game") << "Invalid lock id";
		d->m_lockedEvent = std::nullopt;
		return;
	}


	if (success) {
		if (d->m_lockedEvent) {
			RpgStream::EventPlayer e = d->m_lockedEvent.value();
			e.setSeq(m_player->nextEventId());
			e.setLockId(d->m_lockId);

			d->m_eventList.emplace_back(std::move(e));
		} else {
			LOG_CWARNING("game") << "Missing locked event" << d->m_lockId;

			RpgStream::EventPlayer e(RpgStream::EventPlayer::EventUnlock);
			e.setSeq(m_player->nextEventId());
			e.setLockId(d->m_lockId);

			d->m_eventList.emplace_back(std::move(e));
		}
	} else {
		LOG_CERROR("game") << "Event failed" << d->m_lockId;

		RpgStream::EventPlayer e(RpgStream::EventPlayer::EventFailed);
		e.setSeq(m_player->nextEventId());
		e.setLockId(d->m_lockId);

		if (d->m_lockedEvent)
			e.setTarget(d->m_lockedEvent->target());

		d->m_eventList.emplace_back(std::move(e));
	}

	d->m_lockedEvent = std::nullopt;
}





/**
 * @brief RpgMotorPlayerControlled::questionLoadedChanged
 * @param loaded
 */

void RpgMotorPlayerControlled::questionLoadedChanged(const bool &loaded)
{
	if (loaded)
		d->m_gameQuestionLoaded = true;
}




/**
 * @brief RpgMotorPlayerControlled::controlJoystickState
 * @return
 */

TiledGame::JoystickState RpgMotorPlayerControlled::controlJoystickState() const
{
	return m_controlJoystickState;
}

void RpgMotorPlayerControlled::setControlJoystickState(const TiledGame::JoystickState &newControlJoystickState)
{
	m_controlJoystickState = newControlJoystickState;
}


/**
 * @brief RpgPlayer::mp
 * @return
 */

int RpgPlayer::mp() const
{
	return m_mp;
}

void RpgPlayer::setMp(int newMp)
{
	if (m_mp == newMp)
		return;
	m_mp = newMp;
	emit mpChanged();
}


/**
 * @brief RpgPlayer::maxMp
 * @return
 */

int RpgPlayer::maxMp() const
{
	return m_config.mp;
}



/**
 * @brief RpgPlayer::onAlive
 */

void RpgPlayer::onAlive()
{
	setSubZ(0.5);

	/*filterSet(TiledObjectBody::FixturePlayerBody,
					  TiledObjectBody::FixtureCategories(TiledObjectBody::FixtureAll)
					  .setFlag(TiledObjectBody::FixturePlayerBody, false)
					  .setFlag(TiledObjectBody::FixtureVirtualCircle, false)
					  //.setFlag(TiledObjectBody::FixtureSensor, false)
					  );*/
}


/**
 * @brief RpgPlayer::onDead
 */

void RpgPlayer::onDead()
{
	setSubZ(0.0);


	//filterSet(TiledObjectBody::FixtureInvalid, TiledObjectBody::FixtureInvalid);
}



/**
 * @brief RpgPlayer::onCurrentSpriteChanged
 */


void RpgPlayer::onCurrentSpriteChanged()
{
	/*const QString &sprite = m_spriteHandler->currentSprite();
	const QString &proxy = m_spriteHandler->proxySprite();

	if (sprite == QStringLiteral("run"))
		m_sfxFootStep.startFromBegin();
	else if (sprite != QStringLiteral("run"))
		m_sfxFootStep.stop();*/

	/*if (!m_specialState.isEmpty() && m_specialState != proxy)
		setSpecialState(QString());

	if (sprite == QStringLiteral("idle") && proxy == QStringLiteral("idle"))
		d->idleSet();
	else
		d->idleClear();*/
}

const RpgPlayerDefinition &RpgPlayer::config() const
{
	return m_config;
}


/**
 * @brief RpgPlayer::hasDefender
 * @return
 */

bool RpgPlayer::hasDefender() const
{
	return m_defender != RpgStream::BaseDefenderObject::None && m_hasDefender;
}


void RpgPlayer::setDefender(const RpgStream::BaseDefenderObject::Type &type, const bool &hasDefender)
{
	if (m_defender == type && m_hasDefender == hasDefender)
		return;
	m_defender = type;
	m_hasDefender = hasDefender;

	emit hasDefenderChanged();
}



/**
 * @brief RpgPlayer::maxBullet
 * @return
 */

int RpgPlayer::maxBullet() const
{
	return m_config.bullet;
}



/**
 * @brief RpgPlayer::bullet
 * @return
 */

int RpgPlayer::bullet() const
{
	return m_bullet;
}

void RpgPlayer::setBullet(int newBullet)
{
	if (m_bullet == newBullet)
		return;
	m_bullet = newBullet;
	emit bulletChanged();

	if (m_bullet > m_config.bullet) {
		m_config.bullet = m_bullet;
		emit maxBulletChanged();
	}
}



/**
 * @brief RpgPlayer::setUtility
 * @param type
 * @param hasUtility
 */

void RpgPlayer::setUtility(const RpgStream::PlayerConfig::Utility &type, const bool &hasUtility)
{
	if (m_utility == type && m_hasUtility == hasUtility)
		return;

	m_utility = type;
	m_hasUtility = hasUtility;

	emit hasUtilityChanged();

	if (RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(currentMotor())) {
		motor->updateUseUtility();
	}
}




/**
 * @brief RpgPlayer::targetControl
 * @return
 */

TiledObjectBody *RpgPlayer::targetControl() const
{
	return m_targetControl;
}

void RpgPlayer::setTargetControl(TiledObjectBody *newTargetControl)
{
	if (m_targetControl== newTargetControl)
		return;
	m_targetControl= newTargetControl;
	emit targetControlChanged();
}


/**
 * @brief RpgPlayer::updateColor
 */

void RpgPlayer::updateColor()
{
	if (m_markerItem) {
		m_markerItem->setProperty("progressBarColor", m_rpgGame->getColor(m_team));
		m_markerItem->setProperty("labelColor", m_rpgGame->getColor(m_team));
	}
}




RpgEntity *RpgPlayer::targetEntity() const
{
	return m_targetEntity;
}

void RpgPlayer::setTargetEntity(RpgEntity *newTargetEntity)
{
	if (m_targetEntity == newTargetEntity)
		return;
	m_targetEntity = newTargetEntity;
	emit targetEntityChanged();
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


	if (!m_config.sfxDead.isEmpty()) {
		if (m_config.sfxDead == QStringLiteral("-"))
			m_sfxDead.setSoundList({});
		else
			m_sfxDead.setSoundList({m_config.sfxDead});
	}


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
 * @brief RpgPlayer::updateSprite
 */

void RpgPlayer::updateSprite()
{
	if (m_locked && m_hp > 0) {
		if (!m_effectShield.active())
			m_effectShield.play();
	} else
		m_effectShield.stop();

	if (m_hp <= 0) {
		jumpToSprite("death", m_facingDirection);
		return;
	}

	const QString &sprite = m_spriteHandler->currentSprite();
	const QString &proxy = m_spriteHandler->proxySprite();

	if (sprite == QStringLiteral("attack") ||
			sprite == QStringLiteral("bow") ||
			sprite == QStringLiteral("hurt") ||
			sprite == QStringLiteral("cast"))
		jumpToSpriteLater("idle", m_facingDirection);
	else if (isRunning() && m_facingDirection != Invalid)
		jumpToSprite("run", m_facingDirection);
	else if (isWalking() && m_facingDirection != Invalid)
		jumpToSprite("walk", m_facingDirection);
	else if (sprite == QStringLiteral("idle") && proxy != QStringLiteral("idle"))
		jumpToSpriteLater("idle", m_facingDirection);
	else if (sprite != QStringLiteral("idle"))
		jumpToSprite("idle", m_facingDirection);
	else if (m_facingDirection != m_spriteHandler->currentDirection())
		jumpToSprite(sprite.toLatin1(), m_facingDirection);

}



/**
 * @brief RpgPlayer::synchronize
 */

void RpgPlayer::synchronize()
{
	RpgEntity::synchronize();

	QPointF offset(0.,0.);
	qreal width = 50.;
	qreal stroke = 0.;
	QColor color = m_rpgGame->getColor(m_team);


	if (RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(currentMotor())) {
		stroke = 1.;

		if (m_utilityEntity && m_canUseUtility) {
			QRectF rect = m_utilityEntity->bodyAABB();

			offset = rect.center();
			width = std::max(rect.width(), rect.height()) * 1.5;
			color = QColorConstants::Svg::salmon;

		} else if (m_targetEntity) {
			QRectF rect = m_targetEntity->bodyAABB();

			offset = rect.center();
			width = std::max(rect.width(), rect.height()) * 1.5;
			color = QColorConstants::Svg::red;

		} else if (m_targetControl) {
			QRectF rect = m_targetControl->bodyAABB();

			offset = rect.center();
			width = std::max(rect.width(), rect.height());			// = *0.5*2

			if (dynamic_cast<RpgDefenderPoint*>(m_targetControl))
				color = QColorConstants::Svg::orange;
			else if (dynamic_cast<RpgDefender*>(m_targetControl))
				color = QColorConstants::Svg::red;
			else
				color = QColorConstants::Svg::limegreen;

		} else if (motor->targetJoystickState().distance > 0.1) {
			const QPointF p = currentChunkCenter();

			if (p.x() >= 0 && p.y() >= 0) {
				offset = p;
				width = 25.;
				stroke = 2.;
				color = QColorConstants::Svg::limegreen;
			}

		}

		///////motor->targetAhead()

		if (m_scatterPoint.isValid())
			m_scatterPoint.scatter->setPointConfiguration(m_scatterPoint.index, QXYSeries::PointConfiguration::Size, 14);

		m_visualItem->setOpacity(m_invisible ? 0.4 : 1.0);
	} else {
		if (m_scatterPoint.isValid())
			m_scatterPoint.scatter->setPointConfiguration(m_scatterPoint.index, QXYSeries::PointConfiguration::Size, 10);

		m_visualItem->setVisible(!m_invisible);
		m_markerItem->setVisible(!m_invisible);
	}



	if (m_scatterPoint.isValid()) {
		QPointF p = bodyPositionF();
		p.setY(scene()->height() - p.y());
		m_scatterPoint.scatter->replace(m_scatterPoint.index, p);
		m_scatterPoint.scatter->setPointConfiguration(m_scatterPoint.index, QXYSeries::PointConfiguration::Visibility,
													  !m_invisible);
	}



	if (!isAlive())
		stroke = 0;


	if (!offset.isNull()) {
		m_visualItem->setProperty("ellipseTarget", offset);
		m_visualItem->setProperty("ellipseZ", scene()->getDynamicZ(offset));
	} else {
		// Nem vesszük le a targetet, mert glitchel az animation
		m_visualItem->setProperty("ellipseZ", 0);
	}

	m_visualItem->setProperty("ellipseWidth", width);
	m_visualItem->setProperty("ellipseColor", color);
	m_visualItem->setProperty("ellipseSize", stroke);
}





/**
 * @brief RpgPlayerPrivate::updateLock
 */

void RpgPlayerPrivate::updateLock(const qint64 &tick, const quint32 &lockId)
{
	if (m_lockedEvent && m_lockId == 0 && !m_gameQuestionLoaded && m_waitForLock < tick) {
		LOG_CERROR("game") << "Wait for lockId timeout";
		m_lockedEvent = std::nullopt;
	}

	if (m_lockedEvent && m_lockId > 0 && lockId > 0 && lockId != m_lockId) {
		LOG_CERROR("game") << "LockId mismatch" << lockId << "!=" << m_lockId;
		m_lockedEvent = std::nullopt;
	}

	m_lockId = lockId;


	if (m_lockId > 0 && !m_gameQuestionLoaded) {
		if (q->m_rpgGame->loadNextQuestion()) {
			////m_gameQuestionLoaded = true;
			///

			static const QColor iconColor = QColorConstants::Svg::cyan;
			if (GameQuestion *gq = q->m_rpgGame->gameQuestion()) {
				gq->setProperty("progressColor", iconColor);
				gq->setProperty("msecLeft", q->m_rpgGame->msecLeft()
								-AbstractGame::TickTimer::tickToMsec(CFG_QUESTION_MAX_DURATION));
			}
		}

		return;
	}

	if (m_lockId == 0 && m_gameQuestionLoaded) {
		if (m_lockedEvent)
			q->m_rpgGame->gameQuestion()->finish();
		else {
			q->m_rpgGame->gameQuestion()->forceDestroy();
			m_lockedEvent = std::nullopt;
		}

		if (GameQuestion *gq = q->m_rpgGame->gameQuestion())
			gq->setProperty("msecLeft", 0);

		m_gameQuestionLoaded = false;
	}
}



/**
 * @brief RpgPlayerPrivate::resetLock
 */

void RpgPlayerPrivate::resetLock(const RpgStream::EventPlayer &event)
{
	if (m_lockedEvent && m_lockId == 0 && !m_gameQuestionLoaded && m_lockedEvent->type() == event.type()) {
		m_lockedEvent = std::nullopt;
	}
}




/**
 * @brief RpgPlayerPrivate::applyKnockback
 * @param knockback
 */

void RpgPlayerPrivate::applyKnockback()
{
	if (cpveql(m_currentKnockback, cpvzero))
		return;

	const cpVect current = cpBodyGetVelocity(q->body());

	q->setSpeed(cpvadd(current, m_currentKnockback));

	q->overrideCurrentSpeed(current);
}



/**
 * @brief RpgPlayerPrivate::vibrate
 */

void RpgPlayerPrivate::vibrate()
{
#ifndef Q_OS_WASM
	StandaloneClient *client = qobject_cast<StandaloneClient*>(q->m_rpgGame->client());
	if (client)
		client->performVibrate();
#endif

}



/**
 * @brief RpgPlayerPrivate::updateJoystickMode
 */

void RpgPlayerPrivate::updateJoystickMode()
{
	if (q->m_joystickMode == RpgPlayer::JoystickModeUtility && !q->hasUtility()) {
		if (q->bullet() > 0)
			q->setJoystickMode(RpgPlayer::JoystickModeTarget);
		else
			q->setJoystickMode(RpgPlayer::JoystickModeControl);

	} else if (q->m_joystickMode == RpgPlayer::JoystickModeControl && !q->hasDefender()) {
		if (q->bullet() > 0)
			q->setJoystickMode(RpgPlayer::JoystickModeTarget);
	}

	updateJoystickIcon();
}



/**
 * @brief RpgPlayerPrivate::updateJoystickIcon
 */

void RpgPlayerPrivate::updateJoystickIcon()
{
	if (q->m_joystickMode == RpgPlayer::JoystickModeTarget)
		q->setCurrentJoystickIcon(QUrl());
	else if (q->m_joystickMode == RpgPlayer::JoystickModeControl)
		q->setCurrentJoystickIcon(RpgChanger::dataDefenders().value(q->m_defender).value(QStringLiteral("icon")).toUrl());
	else if (q->m_joystickMode == RpgPlayer::JoystickModeUtility)
		q->setCurrentJoystickIcon(RpgChanger::dataUtilities().value(q->m_utility).value(QStringLiteral("icon")).toUrl());

}


bool RpgPlayer::hasUtility() const
{
	return m_utility != RpgStream::PlayerConfig::UtilityNone && m_hasUtility;
}

bool RpgPlayer::canUseUtility() const
{
	return m_canUseUtility;
}

void RpgPlayer::setCanUseUtility(bool newCanUseUtility)
{
	if (m_canUseUtility == newCanUseUtility)
		return;
	m_canUseUtility = newCanUseUtility;
	emit canUseUtilityChanged();
}

RpgEntity *RpgPlayer::utilityEntity() const
{
	return m_utilityEntity;
}

void RpgPlayer::setUtilityEntity(RpgEntity *newUtilityEntity)
{
	if (m_utilityEntity == newUtilityEntity)
		return;
	m_utilityEntity = newUtilityEntity;
	emit utilityEntityChanged();

	if (RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(currentMotor())) {
		motor->updateUseUtility();
	}
}



/**
 * @brief RpgPlayer::joystickMode
 * @return
 */

RpgPlayer::JoystickMode RpgPlayer::joystickMode() const
{
	return m_joystickMode;
}

void RpgPlayer::setJoystickMode(JoystickMode newJoystickMode)
{
	if (m_joystickMode == newJoystickMode)
		return;
	m_joystickMode = newJoystickMode;
	emit joystickModeChanged();

	d->updateJoystickIcon();
}

QUrl RpgPlayer::currentJoystickIcon() const
{
	return m_currentJoystickIcon;
}

void RpgPlayer::setCurrentJoystickIcon(const QUrl &newCurrentJoystickIcon)
{
	if (m_currentJoystickIcon == newCurrentJoystickIcon)
		return;
	m_currentJoystickIcon = newCurrentJoystickIcon;
	emit currentJoystickIconChanged();
}
