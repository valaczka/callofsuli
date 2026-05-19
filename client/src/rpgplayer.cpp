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


/**
 * @brief RpgPlayer::RpgPlayer
 * @param gameItem
 * @param center
 */

RpgPlayer::RpgPlayer(RpgGameItem *gameItem, const cpVect &center)
	: RpgEntity(gameItem, center, 25., CP_BODY_TYPE_DYNAMIC)
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

	setSensorPolygon(300., M_PI * 2./3., RpgGameItem::FixtureSensor,
					 RpgGameItem::FixtureAll);

	addVirtualCircle(RpgGameItem::FixtureVirtualCircle,
					 RpgGameItem::FixtureAll, 300.);

	addTargetCircle(50, TiledObjectBody::getFilter(RpgGameItem::FixturePlayerTarget,
												   RpgGameItem::FixtureAll));




	m_sfxPain.setFollowPosition(false);
	m_sfxAccept.setFollowPosition(false);
	m_sfxDecline.setFollowPosition(false);


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

	m_castTimer.setInterval(100);

	connect(this, &RpgPlayer::hurt, this, &RpgPlayer::playHurtEffect);
	connect(this, &RpgPlayer::healed, this, &RpgPlayer::playHealedEffect);
	connect(this, &RpgPlayer::becameAlive, this, &RpgPlayer::playAliveEffect);
	connect(this, &RpgPlayer::becameDead, this, &RpgPlayer::playDeadEffect);
	connect(this, &RpgPlayer::isLockedChanged, this, &RpgPlayer::playShieldEffect);

	connect(m_armory.get(), &RpgArmory::currentWeaponChanged, this, &RpgPlayer::playWeaponChangedEffect);

	connect(&m_castTimer, &QTimer::timeout, this, &RpgPlayer::onCastTimerTimeout);

	*/
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

	m_visualItem->setProperty("ellipseColor", QColor::fromRgb(57,250,65,150));
	//m_visualItem->setProperty("ellipseSize", 1);
	m_visualItem->setProperty("ellipseWidth", 50.);

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

float RpgPlayer::chunkRadius() const
{
	return m_chunkRadius;
}

void RpgPlayer::setChunkRadius(float newChunkRadius)
{
	if (qFuzzyCompare(m_chunkRadius, newChunkRadius))
		return;
	m_chunkRadius = newChunkRadius;
	emit chunkRadiusChanged();
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
{
	Q_ASSERT(m_player);
}




/**
 * @brief RpgMotorPlayerControlled::updateBody
 * @param object
 */

void RpgMotorPlayerControlled::updateBody(TiledObject *)
{
	if (m_currentJoystickState.distance >= 1.0) {
		m_destinationMotor.reset();
		m_destinationPoint = std::nullopt;

		m_player->setSpeedFromAngle(m_currentJoystickState.angle, m_player->m_config.run);
		m_player->rotateBody(m_currentJoystickState.angle);
	} else if (m_currentJoystickState.distance > 0.5) {
		m_destinationMotor.reset();
		m_destinationPoint = std::nullopt;

		m_player->setSpeedFromAngle(m_currentJoystickState.angle, m_player->m_config.walk);
		m_player->rotateBody(m_currentJoystickState.angle);
	} else if (m_destinationPoint) {
		if (!m_player->moveTowardsLimited(m_destinationPoint.value(),
										  m_player->m_config.walk,
										  m_player->m_config.run*0.5,
										  m_player->m_config.run)) {
			m_player->stop();
			m_player->emplace(m_destinationPoint.value());
			m_destinationPoint = std::nullopt;
		}
	} else if (m_destinationMotor) {
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

	cpVect ahead = m_player->bodyPosition()+TiledObjectBody::vectorFromAngle(m_player->desiredBodyRotation(), m_player->chunkRadius());
	cpVect center;

	const QPoint ch = m_game->rpgLogicClient()->getChunkFromVector(ahead, &center);
	m_player->setCurrentChunk(ch);
	m_player->setCurrentChunkCenter(TiledObjectBody::toPointF(center));

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

	const quint32 myId = RpgLogicObjectMapper::getId(m_player->objectId());

	if (Rpg::EventsOutput *events = scope.getCtx<Rpg::EventsOutput>()) {
		if (const RpgStream::Events *e = events->at(tick)) {
			for (const RpgStream::EventPlayer &p : e->player()) {
				if (p.tagId() != myId)
					continue;

				if (p.type() == RpgStream::EventPlayer::EventMpPick) {
					LOG_CINFO("game") << "**************************************** MP PICKED *****************";
					m_gameItem->playSfx(QStringLiteral(":/rpg/common/leather_inventory.mp3"), m_player->scene(),
										m_player->bodyPositionF());
				}
			}
		}
	} else {
		LOG_CERROR("game") << "NO EVENTS CTX";
	}

	if (!state) {
		LOG_CERROR("game") << "!!!";
		return false;
	}

	m_player->setHp(state->hp());
	m_player->setMp(state->mp());

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


	if (!m_eventList.empty()) {

		for (RpgStream::EventPlayer &e : m_eventList) {
			e.setTagId(tagId);
			e.setTick(tick);
		}

		RpgStream::Events events;
		events.setTick(tick);
		events.setPlayer(m_eventList);

		state->flags().setFlag(RpgStream::FullState::Event);
		state->events().emplace_back(std::move(events));

		m_eventList.clear();
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
 * @brief RpgMotorPlayerControlled::eventTest
 */

void RpgMotorPlayerControlled::eventTest()
{
	LOG_CINFO("game") << "EVENT TEST";

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventTest);

	m_eventList.emplace_back(std::move(e));
}



/**
 * @brief RpgMotorPlayerControlled::useCurrentControl
 */

void RpgMotorPlayerControlled::useCurrentControl()
{
	if (RpgTower *tower = m_player->tower()) {
		RpgStream::EventPlayer e(RpgStream::EventPlayer::EventTower);
		e.setTarget(RpgLogicObjectMapper::getId(tower->objectId()));
		e.setSuccess(true);

		LOG_CWARNING("game") << "ADD TOWER" << e.target();

		m_eventList.emplace_back(std::move(e));

		return;
	}

	LOG_CWARNING("game") << "Missing control";
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
		if (RpgTower *tower = dynamic_cast<RpgTower*>(otherBody)) {
			if (TiledVisualItem *item = qobject_cast<TiledVisualItem*>(tower->visualItem()))
				item->setGlowEnabled(true);
		}
	}

	if (m_player->isBodyShape(self)) {
		if (RpgMp *mp = dynamic_cast<RpgMp*>(otherBody)) {
			LOG_CINFO("game") << "CONTACT MP" << RpgLogicObjectMapper::getId(mp->objectId());
			eventMpPick(mp);
			return;
		}
	}

	if (m_player->isBodyShape(self) || self == m_player->targetCircle()) {
		if (RpgTower *tower = dynamic_cast<RpgTower*>(otherBody)) {
			LOG_CINFO("game") << "CONTACT TOWER" << RpgLogicObjectMapper::getId(tower->objectId());
			m_player->setTower(tower);
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
		if (RpgTower *tower = dynamic_cast<RpgTower*>(otherBody)) {
			if (TiledVisualItem *item = qobject_cast<TiledVisualItem*>(tower->visualItem()))
				item->setGlowEnabled(false);
		}
	}

	if (m_player->isBodyShape(self)) {
		if (RpgMp *mp = dynamic_cast<RpgMp*>(otherBody)) {
			LOG_CINFO("game") << "CONTACT MP END" << RpgLogicObjectMapper::getId(mp->objectId());
			return;
		}
	}

	if (m_player->isBodyShape(self) || self == m_player->targetCircle()) {
		if (RpgTower *tower = dynamic_cast<RpgTower*>(otherBody)) {
			LOG_CINFO("game") << "CONTACT TOWER END" << RpgLogicObjectMapper::getId(tower->objectId());
			m_player->setTower(nullptr);
			return;
		}
	}
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

	m_eventList.emplace_back(std::move(e));
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


	if (m_visualItem)
		m_visualItem->setProperty("ellipseSize", 1);

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

	if (m_visualItem)
		m_visualItem->setProperty("ellipseSize", 0);

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
 * @brief RpgPlayer::updateColor
 */

void RpgPlayer::updateColor()
{
	LOG_CDEBUG("game") << "Update colors" << this << m_team;

	if (m_visualItem) {
		m_visualItem->setProperty("ellipseColor", RpgGameItem::teamColor().value(m_team));
	}

	if (m_markerItem) {
		m_markerItem->setProperty("progressBarColor", RpgGameItem::teamColor().value(m_team));
		m_markerItem->setProperty("labelColor", RpgGameItem::teamColor().value(m_team));
	}
}



/**
 * @brief RpgPlayer::team
 * @return
 */

RpgStream::Team RpgPlayer::team() const
{
	return m_team;
}

void RpgPlayer::setTeam(RpgStream::Team newTeam)
{
	m_team = newTeam;

	updateColor();
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
 * @brief RpgPlayer::tower
 * @return
 */

RpgTower *RpgPlayer::tower() const
{
	return m_tower;
}

void RpgPlayer::setTower(RpgTower *newTower)
{
	if (m_tower == newTower)
		return;
	m_tower = newTower;
	emit towerChanged();
}
