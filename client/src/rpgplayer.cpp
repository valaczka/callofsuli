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

#include "tiledspritehandler.h"
#include "rpgplayer.h"
#include "rpgmp.h"
#include "rpgdefender.h"
#include "gamequestion.h"


#define SENSOR_LENGTH	400.




/**
 * @brief The RpgPlayerPrivate class
 */

class RpgPlayerPrivate
{
private:
	RpgPlayerPrivate(RpgPlayer *player) : q(player) {}

	void updateLock(const qint64 &tick);

private:
	RpgPlayer *const q;

	std::vector<RpgStream::EventPlayer> m_eventList;
	quint32 m_lockId = 0;
	std::optional<RpgStream::EventPlayer> m_lockedEvent;
	qint64 m_waitForLock = 0;
	bool m_gameQuestionLoaded = false;

	bool m_controlActionDisable = false;
	quint32 m_controlActionDisableLastNotification = 0;


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
	, m_sfxFootStep(this)
	, m_sfxAccept(this)
	, m_sfxDecline(this)
	, m_effectHealed(this)
	, m_effectShield(this)
	, m_effectRing(this)
{
	m_defaultMotor = std::make_unique<RpgMotorPlayer>(this);

	filterSet(RpgGameItem::FixturePlayerBody,
			  RpgGameItem::FixtureGround | RpgGameItem::FixtureControl);

	m_currentChunk.setX(-1);
	m_currentChunk.setY(-1);

	/*setSensorPolygon(SENSOR_LENGTH, M_PI * 0.5, RpgGameItem::FixtureSensor,
					 RpgGameItem::FixtureAll);

	addVirtualCircle(RpgGameItem::FixtureVirtualCircle,
					 RpgGameItem::FixtureAll, 220.);*/

	addTargetCircle(50, TiledObjectBody::getFilter(RpgGameItem::FixturePlayerTarget,
												   RpgGameItem::FixtureAll));




	m_sfxPain.setFollowPosition(false);
	m_sfxAccept.setFollowPosition(false);
	m_sfxDecline.setFollowPosition(false);


	connect(this, &RpgPlayer::healed, this, [this](){ m_effectHealed.play(); });

	/*
	m_speedLength = 108;
	m_speedRunLength = 210;
	m_moveDisabledSpriteList = QStringList{
							   QStringLiteral("attack"),
							   QStringLiteral("bow"),
							   QStringLiteral("cast"),
							   //QStringLiteral("hurt"),
							   QStringLiteral("death")
};

	*/
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

	m_markerItem = createMarkerItem(QStringLiteral("qrc:/RpgPlayerMarker.qml"));

	if (QFile::exists(m_config.prefixPath+QStringLiteral("/input.txt"))) {
		//QHash<QString, RpgArmory::LayerData> layerData;
		QRect measure = RpgGameItem::loadTextureSprites(m_spriteHandler, m_config.prefixPath+QStringLiteral("/")/*, &layerData*/);

		Q_ASSERT(m_visualItem);

		m_visualItem->setWidth(measure.width());
		m_visualItem->setHeight(measure.height());
		setBodyOffset(measure.x(), measure.y());

		m_spriteHandler->setVisibleLayers({"default"});

		/*m_armory->setLayers(layerData);

		d->idleLoad();*/

	} else {
		LOG_CWARNING("game") << "Deprecated character" << m_config.name;

		/*RpgGameItem::loadBaseTextureSprites(m_spriteHandler, m_config.prefixPath+QStringLiteral("/"));

		RpgGameItem::loadBaseTextureSprites(m_spriteHandler, QStringLiteral(":/rpg/shield/"), QStringLiteral("shield"));

		Q_ASSERT(m_visualItem);

		m_visualItem->setWidth(148);
		m_visualItem->setHeight(130);
		setBodyOffset(0, 0.45*64);*/

	}

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

	if (m_config.run <= 0)
		m_config.run = 300;

	if (m_config.walk <= 0)
		m_config.walk = 150;

	if (m_config.hp < 1)
		m_config.hp = 1;

	if (m_config.mp < 1)
		m_config.mp = 10;

	m_config.hp = 7;

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

bool RpgMotorPlayer::beforeWorldStep(const qint64 &, entt::entity &entity)
{
	const qint64 jittered = m_game->rpgLogicClient()->jitterTick();

	if (jittered == 0)
		return false;


	Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();

	auto [player, map] = scope.try_get<Rpg::Player, Rpg::PlayerStateOutput>(entity);


	if (!player || !map)
		return false;

	const RpgStream::PlayerState *last = map->last();

	if (!last) {
		LOG_CERROR("game") << "!!!";
		return false;
	}

	m_player->setHp(last->hp());
	m_player->setMp(last->mp());


	const RpgStream::PlayerState *st = map->at(jittered);

	if (!st)
		return false;


	m_current = *st;


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
		return;
	}

	cpVect to = cpv(m_current->entityState().posXAsFloat(),
					m_current->entityState().posYAsFloat());

	m_player->moveToPoint(to);

	m_current.reset();
}



/**
 * @brief RpgMotorPlayerControlled::RpgMotorPlayerControlled
 * @param player
 */

RpgMotorPlayerControlled::RpgMotorPlayerControlled(RpgPlayer *player)
	: RpgDestinationMotor(player)
	, m_player(player)
	, d(m_player->d)
{
	Q_ASSERT(m_player);

	m_player->setSensorPolygon(SENSOR_LENGTH, M_PI * 0.5, RpgGameItem::FixtureSensor,
							   RpgGameItem::FixtureAll);

	m_player->addVirtualCircle(RpgGameItem::FixtureVirtualCircle,
							   RpgGameItem::FixtureAll, 220.);
}






/**
 * @brief RpgMotorPlayerControlled::findNearestTarget
 * @return
 */

RpgEntity * RpgMotorPlayerControlled::findNearestTarget(const cpBitmask &category)
{
	QSet<TiledObjectBody*> list;

	RpgMotorEntity::queryContactedVisibleBodies(m_player, &list, category, RpgGameItem::FixtureGround);

	QMultiMap<float, TiledObjectBody *> bds = RpgMotorEntity::sort(m_player, list);

	for (TiledObjectBody *b : std::as_const(bds)) {
		RpgEntity *e = dynamic_cast<RpgEntity*>(b);

		if (!e || !e->isAlive())
			continue;

		if (e->team() == m_player->team())
			continue;

		return e;
	}

	return nullptr;
}





/**
 * @brief RpgMotorPlayerControlled::findNearestTarget
 * @param rayDest
 * @param category
 * @return
 */

RpgEntity *RpgMotorPlayerControlled::findNearestTarget(const cpVect &rayDest, const cpBitmask &category)
{
	RayCastInfo ray = m_player->rayCast(rayDest, RpgGameItem::FixtureGround, category, 2.);

	for (const RayCastInfoItem &i : ray) {
		if (!i.visible)
			continue;

		RpgEntity *e = dynamic_cast<RpgEntity*>(TiledObjectBody::fromShapeRef(i.shape));

		if (!e || !e->isAlive())
			continue;

		if (e->team() == m_player->team())
			continue;

		return e;
	}

	return nullptr;
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
				p->tower()->state().team() == m_player->team() && !p->defender();
	} else if (RpgDefender *p = dynamic_cast<RpgDefender*>(control)) {
		return p->team() != m_player->team() && p->hp() > 0;
	}

	return false;
}



/**
 * @brief RpgMotorPlayerControlled::updateBody
 * @param object
 */

void RpgMotorPlayerControlled::updateBody(TiledObject *)
{
	if (!m_player->isAlive()) {
		m_player->stop();
		m_player->setCurrentChunk({-1,-1});
		m_player->setCurrentChunkCenter({-1,-1});

		m_player->setTargetControl(nullptr);
		m_player->setTargetEntity(nullptr);
		return;
	}

	// Moving (JoystickA)

	if (d->m_lockedEvent) {
		m_player->stop();
		m_player->setCurrentChunk({-1,-1});
		m_player->setCurrentChunkCenter({-1,-1});
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




	// Using control (JoystickB)

	if (d->m_controlActionDisable || !m_player->hasDefender()) {
		m_player->setTargetControl(nullptr);
	} else {
		float targetDist = 250;			// TODO:

		if (m_controlJoystickState.hasTouch) {
			if (m_controlJoystickState.distance > 0.1)
				m_targetAngle = m_controlJoystickState.angle;
			else if (!m_player->targetControl() && !m_targetAngle.has_value())
				m_targetAngle = m_player->desiredBodyRotation();

			targetDist *= std::clamp(m_controlJoystickState.distance, 0.3, 1.0);
		}

		if (!m_targetJoystickState.hasTouch) {
			if (m_targetAngle.has_value()) {
				cpVect ahead = m_player->bodyPosition()+TiledObjectBody::vectorFromAngle(m_targetAngle.value(), targetDist);

				m_player->setTargetControl(findNearestControl(ahead));
			} else {
				m_player->setTargetControl(findNearestControl(targetDist));
			}
		}
	}



	if (m_player->targetControl() || !m_controlJoystickState.hasTouch
			|| m_controlJoystickState.distance <= 0.1
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





	// Attack enemy (JoystickC)

	if (m_controlJoystickState.hasTouch || m_player->bullet() <= 0) {
		m_player->setTargetEntity(nullptr);
		return;
	}


	const float dist = std::max(SENSOR_LENGTH, 450.);			// TODO: weapon length

	if (m_targetJoystickState.hasTouch) {
		if (m_targetJoystickState.distance > 0.1)
			m_targetAngle = m_targetJoystickState.angle;
		else if (!m_player->targetEntity() && !m_targetAngle.has_value())
			m_targetAngle = m_player->desiredBodyRotation();


		if (m_targetAngle.has_value()) {
			cpVect ahead = m_player->bodyPosition()+TiledObjectBody::vectorFromAngle(m_targetAngle.value(), dist);

			m_player->setTargetEntity(findNearestTarget(ahead, RpgGameItem::FixturePlayerBody | RpgGameItem::FixturePlayerTarget));
		}

	} else if (m_player->targetEntity()) {
		if (!m_player->targetEntity()->isAlive()) {
			m_player->setTargetEntity(nullptr);
		} else {
			RayCastInfo ray = m_player->rayCast(m_player->targetEntity()->bodyPosition(),
												RpgGameItem::FixtureGround,
												RpgGameItem::FixturePlayerBody | RpgGameItem::FixturePlayerTarget,
												2.);


			if (!ray.isVisible(m_player->targetEntity()) ||
					m_player->distanceToPointSq(m_player->targetEntity()->bodyPosition()) > POW2(dist))
				m_player->setTargetEntity(nullptr);
		}
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
	Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();

	const RpgStream::PlayerState *state = scope.getCurrentState<RpgStream::PlayerState>(entity);

	if (!state) {
		LOG_CERROR("game") << "!!!";
		return false;
	}


	// Ha most vált halottra vagy éppen most támad fel

	if (state->hp() == 0 || m_player->hp() == 0) {
		m_player->emplace(state->entityState().posXAsFloat(), state->entityState().posYAsFloat());
	}


	bool nextControlState = (state->penalty() >= tick);

	if (!nextControlState && d->m_controlActionDisable && m_player->isAlive()) {
		m_game->gameItem()->message(QObject::tr("Penalty time over"));
	}


	m_player->setHp(state->hp());
	m_player->setMp(state->mp());
	m_player->setBullet(state->bullet());
	m_player->setDefender(state->defender());
	m_player->setLocked(state->lock() > 0 || d->m_lockedEvent);
	d->m_lockId = state->lock();
	d->updateLock(tick);


	d->m_controlActionDisable = nextControlState;

	if (d->m_controlActionDisable) {
		if (state->penalty() > d->m_controlActionDisableLastNotification && m_player->isAlive()) {
			int sec = std::ceil(AbstractGame::TickTimer::tickToMsec(state->penalty() - tick)/1000.);
			m_game->gameItem()->message(QObject::tr("%1 sec penalty").arg(sec));
		}

		d->m_controlActionDisableLastNotification = state->penalty();
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
	if (!state)
		return false;

	const quint32 tagId = RpgLogicObjectMapper::getId(m_player->objectId());

	if (m_currentJoystickState.distance > 0.1) {
		/*Rpg::RpgLogicScope scope = m_game->rpgLogicClient().getScope();

		auto [player, map] = scope.try_get<Rpg::Player, Rpg::PlayerTickMap>(entity);

		LOG_CINFO("game") << "CURR" << tick << m_player->bodyPositionF()
						  << (player ? player ->playerData.playerId() : -1)
						  << "---" << (map ? map->map.size() : -1)
						  << "LAST"
						  << (map && !map->map.isEmpty() ? map->map.last().entityState().posXAsFloat() : -1);
						  */

		RpgStream::PlayerState st;
		st.setTick(tick);
		st.entityState().setPosXAsFloat(m_player->bodyPosition().x);
		st.entityState().setPosYAsFloat(m_player->bodyPosition().y);

		m_statePull.append(std::move(st));

		std::vector<RpgStream::PlayerState> list = m_statePull.extract(m_game->gameMode() == RpgGame::MultiPlayer ? 6 : 1);			// SINGLE PLAYER: 1


		/*LOG_CINFO("game") << "---------------------------";

		for (const RpgStream::PlayerState &s : list) {
			LOG_CDEBUG("game") << s.tick() << "POS" << s.entityState().posXAsFloat() << s.entityState().posYAsFloat();
		}*/


		RpgStream::PlayerStateList sl;
		sl.setTagId(tagId);
		sl.setIsDeltaMode(state->isDeltaMode());
		if (state->isDeltaMode())
			sl.compressStateVector(std::move(list));
		else
			sl.setState(std::move(list));

		state->flags().setFlag(RpgStream::FullState::Player);
		state->players().push_back(std::move(sl));



		/*	for (const RpgStream::PlayerStateList &l : state->players().list()) {
			LOG_CDEBUG("game") << "####" << l.tagId() << l.isDeltaMode();

			for (const RpgStream::PlayerState &s : l.state()) {
				LOG_CDEBUG("game") << "#" << s.tick() << "POS" << s.entityState().posXAsFloat() << s.entityState().posYAsFloat()
								   << "|" << s.entityState().deltaMask() << s.entityState().hasPosXDeltaMask() << s.entityState().hasPosYDeltaMask();
			}
		}*/

		/*
			RpgStream::PlayerStateList stream;
			stream.setIsDeltaMode(true);
			stream.compressStateVector(list, out);

			LOG_CWARNING("game") << "---------------------------";

			LOG_CDEBUG("game") << out.entityState().tick() << "POS" << out.entityState().posXAsFloat() << out.entityState().posYAsFloat();

			for (const RpgStream::PlayerState &s : stream.state()) {
				LOG_CDEBUG("game") << s.entityState().tick() << "POS" << s.entityState().posXAsFloat() << s.entityState().posYAsFloat()
								   << "|" << s.entityState().deltaMask() << s.entityState().hasPosXDeltaMask() << s.entityState().hasPosYDeltaMask();
			}

			LOG_CERROR("game") << "---------------------------";

			std::vector<RpgStream::PlayerState> test = stream.extractStateVector(out);

			LOG_CDEBUG("game") << out.entityState().tick() << "POS" << out.entityState().posXAsFloat() << out.entityState().posYAsFloat();

			for (const RpgStream::PlayerState &s : test) {
				LOG_CDEBUG("game") << s.entityState().tick() << "POS" << s.entityState().posXAsFloat() << s.entityState().posYAsFloat();
			}
			*/

		//map->map.insert(tick, std::move(st));
	}


	if (!d->m_eventList.empty()) {

		for (RpgStream::EventPlayer &e : d->m_eventList) {
			e.setTagId(tagId);
			e.setTick(tick);
		}

		RpgStream::Events events;
		events.setTick(tick);
		events.setPlayer(d->m_eventList);

		state->flags().setFlag(RpgStream::FullState::Event);
		state->events().emplace_back(std::move(events));

		d->m_eventList.clear();
	}


	return true;
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
	if (RpgPlayer *player = qobject_cast<RpgPlayer*>(m_player->targetEntity())) {
		RpgStream::EventPlayer e(RpgStream::EventPlayer::EventAttackPlayer);
		e.setTarget(RpgLogicObjectMapper::getId(player->objectId()));

		LOG_CWARNING("game") << "ATTACK PLAYER" << e.target();

		d->m_eventList.emplace_back(std::move(e));

		return;
	}

	LOG_CWARNING("game") << "Invalid target";
}





/**
 * @brief RpgMotorPlayerControlled::useCurrentControl
 */

void RpgMotorPlayerControlled::useCurrentControl()
{
	if (RpgTower *tower = dynamic_cast<RpgTower*>(m_player->targetControl())) {
		RpgStream::EventPlayer e(RpgStream::EventPlayer::EventTower);
		e.setTarget(RpgLogicObjectMapper::getId(tower->objectId()));

		LOG_CWARNING("game") << "ATTACK TOWER" << e.target() << d->m_lockId;

		d->m_lockedEvent = e;
		d->m_waitForLock = m_gameItem->tickTimer()->currentTick() + 5*60;			// Wait for lockId from server

		d->m_eventList.emplace_back(std::move(e));

		return;
	}


	if (RpgDefender *p = dynamic_cast<RpgDefender*>(m_player->targetControl())) {
		RpgStream::EventPlayer e(RpgStream::EventPlayer::EventAttackDefender);
		e.setTarget(RpgLogicObjectMapper::getId(p->objectId()));

		LOG_CWARNING("game") << "ATTACK DEFENDER" << e.target() << d->m_lockId;

		d->m_lockedEvent = e;
		d->m_waitForLock = m_gameItem->tickTimer()->currentTick() + 5*60;			// Wait for lockId from server

		d->m_eventList.emplace_back(std::move(e));

		return;
	}


	if (dynamic_cast<RpgDefenderPoint*>(m_player->targetControl()))
		return putDefender(true);

}



/**
 * @brief RpgMotorPlayerControlled::putDefender
 */

void RpgMotorPlayerControlled::putDefender(const bool &click)
{
	if (RpgDefenderPoint *p = dynamic_cast<RpgDefenderPoint*>(m_player->targetControl())) {
		RpgStream::EventPlayer e(RpgStream::EventPlayer::EventDefender);
		e.setTarget(RpgLogicObjectMapper::getId(p->objectId()));

		LOG_CWARNING("game") << "PUT DEFENDER" << e.target();

		d->m_eventList.emplace_back(std::move(e));

		return;
	}

	if (click) {
		LOG_CWARNING("game") << "Just click";
		useCurrentControl();
		return;
	}

	if (const QPoint &ch = m_player->currentChunk(); ch.x() >= 0 && ch.y() >= 0) {
		RpgStream::EventPlayer e(RpgStream::EventPlayer::EventDefender);
		e.chunk().setX(ch.x());
		e.chunk().setY(ch.y());

		LOG_CWARNING("game") << "PUT DEFENDER ON CHUNK" << ch;

		d->m_eventList.emplace_back(std::move(e));
	}

	LOG_CWARNING("game") << "Missing point";
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

	if (self == m_player->virtualCircle()) {
		TiledVisualItem *item = nullptr;

		if (RpgTower *o = dynamic_cast<RpgTower*>(otherBody)) {
			item = qobject_cast<TiledVisualItem*>(o->visualItem());
			o->markerItem()->setVisible(true);
			if (o->state().team() == m_player->team() || o->state().team() == RpgStream::TeamNone)
				o->setDefenderLayersVisible(true);
		} else if (RpgMp *o = dynamic_cast<RpgMp*>(otherBody))
			item = qobject_cast<TiledVisualItem*>(o->visualItem());
		else if (RpgDefender *o = dynamic_cast<RpgDefender*>(otherBody))
			item = qobject_cast<TiledVisualItem*>(o->visualItem());


		if (item)
			item->setGlowEnabled(true);
	}

	if (m_player->isBodyShape(self)) {
		if (RpgMp *mp = dynamic_cast<RpgMp*>(otherBody)) {
			LOG_CINFO("game") << "CONTACT MP" << RpgLogicObjectMapper::getId(mp->objectId());
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

	if (self == m_player->virtualCircle()) {
		TiledVisualItem *item = nullptr;

		if (RpgTower *o = dynamic_cast<RpgTower*>(otherBody)) {
			item = qobject_cast<TiledVisualItem*>(o->visualItem());
			o->markerItem()->setVisible(false);
			o->setDefenderLayersVisible(false);
		} else if (RpgMp *o = dynamic_cast<RpgMp*>(otherBody))
			item = qobject_cast<TiledVisualItem*>(o->visualItem());
		else if (RpgDefender *o = dynamic_cast<RpgDefender*>(otherBody))
			item = qobject_cast<TiledVisualItem*>(o->visualItem());


		if (item)
			item->setGlowEnabled(false);
	}

	/*if (m_player->isBodyShape(self)) {
		if (RpgMp *mp = dynamic_cast<RpgMp*>(otherBody)) {
			LOG_CINFO("game") << "CONTACT MP END" << RpgLogicObjectMapper::getId(mp->objectId());
			return;
		}
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

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventMpPick);
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
	if (!d->m_lockedEvent) {
		LOG_CERROR("game") << "Missing locked event";
		return;
	}

	if (d->m_lockId == 0) {
		LOG_CERROR("game") << "Invalid lock id";
		d->m_lockedEvent = std::nullopt;
		return;
	}


	if (success) {
		RpgStream::EventPlayer e = d->m_lockedEvent.value();
		e.setLockId(d->m_lockId);

		LOG_CWARNING("game") << "RESEND ATTACK" << e.target() << d->m_lockId;

		d->m_eventList.emplace_back(std::move(e));
	} else {
		RpgStream::EventPlayer e(RpgStream::EventPlayer::EventFailed);
		e.setLockId(d->m_lockId);
		e.setTarget(d->m_lockedEvent->target());

		LOG_CWARNING("game") << "***FAIL****" << e.tagId() << "--->" << e.target() << d->m_lockId;

		d->m_eventList.emplace_back(std::move(e));
	}

	d->m_lockedEvent = std::nullopt;
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
	const QString &sprite = m_spriteHandler->currentSprite();
	const QString &proxy = m_spriteHandler->proxySprite();

	LOG_CDEBUG("game") << "CURRENT" << sprite;

	if (sprite == QStringLiteral("run"))
		m_sfxFootStep.startFromBegin();
	else if (sprite != QStringLiteral("run"))
		m_sfxFootStep.stop();

	/*if (!m_specialState.isEmpty() && m_specialState != proxy)
		setSpecialState(QString());

	if (sprite == QStringLiteral("idle") && proxy == QStringLiteral("idle"))
		d->idleSet();
	else
		d->idleClear();*/
}


/**
 * @brief RpgPlayer::hasDefender
 * @return
 */

bool RpgPlayer::hasDefender() const
{
	return m_defender != RpgStream::BaseDefenderObject::None;
}


void RpgPlayer::setDefender(const RpgStream::BaseDefenderObject::Type &type)
{
	if (m_defender == type)
		return;
	m_defender = type;
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
 * @brief RpgPlayer::targetControl
 * @return
 */

TiledObjectBody *RpgPlayer::targetControl() const
{
	return m_targetControl;
}

void RpgPlayer::setTargetControl(TiledObjectBody *newTargetControl)
{
	m_targetControl = newTargetControl;
}


/**
 * @brief RpgPlayer::updateColor
 */

void RpgPlayer::updateColor()
{
	LOG_CDEBUG("game") << "Update colors" << this << m_team;

	if (m_markerItem) {
		m_markerItem->setProperty("progressBarColor", RpgGameItem::teamColor().value(m_team));
		m_markerItem->setProperty("labelColor", RpgGameItem::teamColor().value(m_team));
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

	LOG_CINFO("game") << "TARGET" << m_targetEntity;
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
	QColor color = RpgGameItem::teamColor().value(m_team);


	if (RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(currentMotor())) {
		stroke = 1.;

		if (m_targetControl) {
			QRectF rect = m_targetControl->bodyAABB();

			offset = rect.center() - bodyPositionF();
			width = std::max(rect.width(), rect.height());			// = *0.5*2

			if (dynamic_cast<RpgDefenderPoint*>(m_targetControl))
				color = QColorConstants::Svg::orange;
			else if (dynamic_cast<RpgDefender*>(m_targetControl))
				color = QColorConstants::Svg::red;
			else
				color = QColorConstants::Svg::lightgreen;

		} else if (m_targetEntity) {
			QRectF rect = m_targetEntity->bodyAABB();

			offset = rect.center() - bodyPositionF();
			width = std::max(rect.width(), rect.height()) * 1.5;
			color = QColorConstants::Svg::red;

		} else if (motor->controlJoystickState().distance > 0.1) {
			const QPointF p = currentChunkCenter();

			if (p.x() >= 0 && p.y() >= 0) {
				offset = p - bodyPositionF();
				width = 25.;
				stroke = 2.;
				color = QColorConstants::Svg::lightgreen;
			}

		}


		/*else {
			const QPointF p = currentChunkCenter();

			if (p.x() >= 0 && p.y() >= 0) {
				offset = p - bodyPositionF();
				width = 25.;
				stroke = 2.;
				color = QColor::fromRgb(57,250,65,150);
			}
		}*/

		if (m_scatterPoint.isValid())
			m_scatterPoint.scatter->setPointConfiguration(m_scatterPoint.index, QXYSeries::PointConfiguration::Size, 14);
	} else {
		if (m_scatterPoint.isValid())
			m_scatterPoint.scatter->setPointConfiguration(m_scatterPoint.index, QXYSeries::PointConfiguration::Size, 10);
	}



	if (m_scatterPoint.isValid()) {
		QPointF p = bodyPositionF();
		p.setY(scene()->height() - p.y());
		m_scatterPoint.scatter->replace(m_scatterPoint.index, p);
	}


	if (!isAlive())
		stroke = 0;

	m_visualItem->setProperty("ellipseOffset", offset);
	m_visualItem->setProperty("ellipseWidth", width);
	m_visualItem->setProperty("ellipseColor", color);
	m_visualItem->setProperty("ellipseSize", stroke);
}





/**
 * @brief RpgPlayerPrivate::updateLock
 */

void RpgPlayerPrivate::updateLock(const qint64 &tick)
{
	if (m_lockedEvent && m_lockId == 0 && !m_gameQuestionLoaded && m_waitForLock < tick) {
		LOG_CERROR("game") << "Wait for lockId timeout";
		m_lockedEvent = std::nullopt;
	}


	if (m_lockId > 0 && !m_gameQuestionLoaded) {
		q->m_rpgGame->loadNextQuestion();
		m_gameQuestionLoaded = true;
		return;
	}

	if (m_lockId == 0 && m_gameQuestionLoaded) {
		if (m_lockedEvent)
			q->m_rpgGame->gameQuestion()->finish();
		else {
			q->m_rpgGame->gameQuestion()->forceDestroy();
			m_lockedEvent = std::nullopt;
		}

		m_gameQuestionLoaded = false;
	}
}
