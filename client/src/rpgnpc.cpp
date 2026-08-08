/*
 * ---- Call of Suli ----
 *
 * rpgnpc.cpp
 *
 * Created on: 2026. 07. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgNpc
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

#include "rpgnpc.h"
#include "rpgnpcmpleecher.h"
#include "rpgnpctowerattacker.h"
#include "rpgnpcplayerattacker.h"
#include "tiledspritehandler.h"



/**
 * @brief RpgNpc::RpgNpc
 * @param gameItem
 * @param center
 */

RpgNpc::RpgNpc(RpgGameItem *gameItem, const cpVect &center)
	: RpgEntity(gameItem, center, 25., CP_BODY_TYPE_DYNAMIC)
	, m_sfxPain(this)
	, m_sfxFootStep(this)
	, m_effectHealed(this)
{
	m_defaultMotor = std::make_unique<RpgMotorNpc>(this);

	static const cpBitmask defaultBitmask =
			RpgGameItem::FixturePlayerTarget |
			RpgGameItem::FixtureDefender |
			RpgGameItem::FixtureControl |
			RpgGameItem::FixtureNpcTarget;

	filterSet(RpgGameItem::FixtureNpcBody,
			  RpgGameItem::FixtureGround | defaultBitmask);

	addTargetCircle(50, TiledObjectBody::getFilter(RpgGameItem::FixtureNpcTarget,
												   defaultBitmask |
												   RpgGameItem::FixturePlayerBody |
												   RpgGameItem::FixtureNpcBody |
												   RpgGameItem::FixtureSensor));

	connect(this, &RpgNpc::healed, this, [this](){ m_effectHealed.play(); });
};




/**
 * @brief RpgNpc::~RpgNpc
 */

RpgNpc::~RpgNpc()
{

}




/**
 * @brief RpgNpc::createNpc
 * @param npc
 * @param gameItem
 * @param scene
 * @param pos
 * @return
 */

RpgNpc *RpgNpc::createNpc(const Rpg::Npc &npc, RpgGameItem *gameItem, TiledScene *scene, const cpVect &pos)
{
	Q_ASSERT(gameItem);
	Q_ASSERT(scene);


	switch (npc.data.type()) {
		case RpgStream::NpcData::TowerAttacker:
			return gameItem->createObject<RpgNpcTowerAttacker>(RpgLogicObjectMapper::toObjectId(npc.idTag),
															   scene, gameItem, pos);

		case RpgStream::NpcData::MpLeecher:
			return gameItem->createObject<RpgNpcMpLeecher>(RpgLogicObjectMapper::toObjectId(npc.idTag),
														   scene, gameItem, pos);

		case RpgStream::NpcData::PlayerAttacker:
			return gameItem->createObject<RpgNpcPlayerAttacker>(RpgLogicObjectMapper::toObjectId(npc.idTag),
																scene, gameItem, pos);

		case RpgStream::NpcData::None:
			return gameItem->createObject<RpgNpc>(RpgLogicObjectMapper::toObjectId(npc.idTag),
												  scene, gameItem, pos);
	}

	return nullptr;
}


/**
 * @brief RpgNpc::initialize
 */

void RpgNpc::initialize()
{
	Q_ASSERT(scene());

	setDefaultZ(1);
	setSubZ(0.5);

	createVisual();

	m_visualItem->setZ(1);
}



/**
 * @brief RpgNpc::updateSprite
 */

void RpgNpc::updateSprite()
{
	if (m_hp <= 0) {
		jumpToSprite("death", m_facingDirection);
		return;
	}

	const QString &sprite = m_spriteHandler->currentSprite();
	const QString &proxy = m_spriteHandler->proxySprite();

	const float &l = currentSpeedSq();

	if (sprite == QStringLiteral("attack") ||
			sprite == QStringLiteral("bow") ||
			sprite == QStringLiteral("hurt") ||
			sprite == QStringLiteral("cast"))
		jumpToSpriteLater("idle", m_facingDirection);
	else if (l >= POW2(m_config.run)*0.9 && m_facingDirection != Invalid)
		jumpToSprite("run", m_facingDirection);
	else if (l < POW2(m_config.run) && l > POW2(0.05) && m_facingDirection != Invalid)
		jumpToSprite("walk", m_facingDirection);
	else if (sprite == QStringLiteral("idle") && proxy != QStringLiteral("idle"))
		jumpToSpriteLater("idle", m_facingDirection);
	else if (sprite != QStringLiteral("idle"))
		jumpToSprite("idle", m_facingDirection);
	else if (m_facingDirection != m_spriteHandler->currentDirection())
		jumpToSprite(sprite.toLatin1(), m_facingDirection);
}




/**
 * @brief RpgNpc::getControlledMotor
 * @return
 */

std::unique_ptr<RpgMotorNpcControlled> RpgNpc::getControlledMotor()
{
	return std::make_unique<RpgMotorNpcControlled>(this);
}




/**
 * @brief RpgNpc::load
 * @param config
 */

void RpgNpc::load(const RpgNpcDefinition &config)
{
	setAvailableDirections(Direction_8);

	setConfig(config);

	m_markerItem = createMarkerItem(QStringLiteral("qrc:/RpgNpcMarker.qml"));

	if (QFile::exists(m_config.prefixPath+QStringLiteral("/input.txt"))) {
		//QHash<QString, RpgArmory::LayerData> layerData;
		QRect measure = RpgGameItem::loadTextureSprites(m_spriteHandler, m_config.prefixPath+QStringLiteral("/")/*, &layerData*/);

		Q_ASSERT(m_visualItem);

		m_visualItem->setWidth(measure.width());
		m_visualItem->setHeight(measure.height());
		setBodyOffset(measure.x(), measure.y());

	} else {
		RpgGameItem::loadTextureSprites(m_spriteHandler, RpgGameItem::baseSpriteMapper(), m_config.prefixPath+QStringLiteral("/"));

		m_visualItem->setWidth(148);
		m_visualItem->setHeight(130);
		setBodyOffset(0, 32);
	}

	//m_spriteHandler->setVisibleLayers({QStringLiteral("default")});

	loadSfx();

	///connect(m_spriteHandler, &TiledSpriteHandler::currentSpriteChanged, this, &RpgPlayer::onCurrentSpriteChanged);

	updateColor();

	onAlive();
}



/**
 * @brief RpgNpc::setConfig
 * @param config
 */

void RpgNpc::setConfig(const RpgNpcDefinition &config)
{
	m_config = config;

	setMaxHp(m_config.hp);
}




/**
 * @brief RpgNpc::targetEntity
 * @return
 */

RpgEntity *RpgNpc::targetEntity() const
{
	return m_targetEntity;
}

void RpgNpc::setTargetEntity(RpgEntity *newTargetEntity)
{
	if (m_targetEntity == newTargetEntity)
		return;
	m_targetEntity = newTargetEntity;
	emit targetEntityChanged();
}



/**
 * @brief RpgNpc::synchronize
 */

void RpgNpc::synchronize()
{
	RpgEntity::synchronize();

	if (m_scatterPoint.isValid()) {
		QPointF p = bodyPositionF();
		p.setY(scene()->height() - p.y());
		m_scatterPoint.scatter->replace(m_scatterPoint.index, p);
		m_scatterPoint.scatter->setPointConfiguration(m_scatterPoint.index,
													  QXYSeries::PointConfiguration::Visibility, isAlive());
	}
}



/**
 * @brief RpgNpc::onAlive
 */

void RpgNpc::onAlive()
{
	setSubZ(0.5);
}



/**
 * @brief RpgNpc::onDead
 */

void RpgNpc::onDead()
{
	setSubZ(0.0);
}




/**
 * @brief RpgNpc::updateColor
 */

void RpgNpc::updateColor()
{
	const QColor color = getColor();

	if (m_markerItem) {
		m_markerItem->setProperty("progressBarColor", color);
		m_markerItem->setProperty("labelColor", color);
	}

	if (m_scatterPoint.isValid()) {
		m_scatterPoint.scatter->setPointConfiguration(m_scatterPoint.index,
													  QXYSeries::PointConfiguration::Color,
													  color);
	}
}



/**
 * @brief RpgNpc::loadSfx
 */

void RpgNpc::loadSfx()
{
	if (!m_config.sfxPain.isEmpty()) {
		m_sfxPain.setSoundList(m_config.sfxPain);
	}

	m_sfxPain.setPlayOneDeadline(600);

	if (!m_config.sfxFootStep.isEmpty()) {
		m_sfxFootStep.setSoundList(m_config.sfxFootStep);
	}

	m_sfxFootStep.setInterval(350);

	if (!m_config.sfxDead.isEmpty()) {
		m_sfxDead.setSoundList({m_config.sfxDead});
	}
}





/**
 * @brief RpgMotorNpc::RpgMotorNpc
 * @param npc
 */

RpgMotorNpc::RpgMotorNpc(RpgNpc *npc)
	: RpgMotorEntity(npc)
	, RpgMotorNpcEventIface()
	, m_npc(npc)
{
	Q_ASSERT(m_npc);
}





/**
 * @brief RpgMotorNpc::beforeWorldStep
 * @param tick
 * @param entity
 * @return
 */

bool RpgMotorNpc::beforeWorldStep(const qint64 &tick, entt::entity &entity)
{
	Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();

	const Rpg::NpcStateOutput *out = scope.try_get<Rpg::NpcStateOutput>(entity);

	if (!out) {
		LOG_CERROR("game") << "!!!";
		return false;
	}


	if (tick < 0) {
		const RpgStream::NpcState *state = out->at(0);

		if (!state) {
			LOG_CERROR("game") << "Missing tick 0" << this;
			return false;
		}

		m_current = *state;

		updateBody(nullptr);
		m_npc->synchronize();

		return true;
	}

	const quint32 jittered = m_game->rpgLogicClient()->jitterTick(tick);

	if (jittered == 0)
		return false;

	if (!m_incomingEventList.empty()) {
		processEventAt(jittered);

		std::erase_if(m_incomingEventList,
					  [&jittered](const RpgStream::EventNpc &e) {
			return e.tick() <= jittered;
		});
	}


	const RpgStream::NpcState *state = out->at(jittered);

	if (!state) {
		//LOG_CERROR("game") << "!!! STATE" << tick << jittered;
		m_current.reset();
		return false;
	}


	m_current = *state;

	m_npc->setHp(state->hp());

	return true;
}





/**
 * @brief RpgMotorNpc::updateBody
 */

void RpgMotorNpc::updateBody(TiledObject *)
{
	if (!m_current) {
		m_npc->stop();
		return;
	}

	updateBody(m_npc, m_current.value(), false);

	m_current.reset();
}




/**
 * @brief RpgMotorNpc::processEvent
 * @param event
 */

void RpgMotorNpc::processEvent(const RpgStream::EventNpc &event)
{
	m_incomingEventList.push_back(event);
}




/**
 * @brief RpgMotorNpc::updateBody
 * @param npc
 * @param state
 * @param isEmplace
 */

void RpgMotorNpc::updateBody(RpgNpc *npc, const RpgStream::NpcState &state, const bool &isEmplace)
{
	Q_ASSERT(npc);

	cpVect to = cpv(state.entityState().posXAsFloat(),
					state.entityState().posYAsFloat());

	if (isEmplace)
		npc->emplace(to);
	else
		npc->TiledObjectBody::moveToPoint(to);

	npc->rotateBody(state.entityState().angleAsFloat(), true);
	npc->setFacingDirection(TiledObject::Direction(state.entityState().facing()));
	npc->overrideCurrentSpeedSq(state.entityState().velSq());
}






/**
 * @brief RpgMotorNpcControlled::RpgMotorNpcControlled
 * @param npc
 */

RpgMotorNpcControlled::RpgMotorNpcControlled(RpgNpc *npc)
	: RpgDestinationMotor(npc)
	, RpgMotorNpcEventIface()
	, m_npc(npc)
{
	Q_ASSERT(m_npc);

	m_npc->setSensorPolygon(400., M_PI * 0.5,
							TiledObjectBody::getFilter(RpgGameItem::FixtureSensor,
													   RpgGameItem::FixturePlayerBody |
													   RpgGameItem::FixturePlayerTarget |
													   RpgGameItem::FixtureControl |
													   RpgGameItem::FixtureDefender |
													   RpgGameItem::FixtureNpcBody |
													   RpgGameItem::FixtureNpcTarget)
							);
}



/**
 * @brief RpgMotorNpcControlled::updateBody
 */

void RpgMotorNpcControlled::updateBody(TiledObject *)
{
	if (!m_npc->isAlive() || m_game->gameState() != RpgGame::GameStatePlay) {
		m_npc->stop();

		applyKnockback();

		m_npc->setTargetEntity(nullptr);
		return;
	}

	if (!m_groundCollision.isEmpty())
		onGroundCollision();

	updateTarget();
	const int speed = getMovementSpeed();
	updateMovement(speed);

	applyKnockback();

	updateMotor();
}




/**
 * @brief RpgMotorNpcControlled::beforeWorldStep
 * @param tick
 * @param entity
 * @return
 */

bool RpgMotorNpcControlled::beforeWorldStep(const qint64 &tick, entt::entity &entity)
{
	m_currentTick = tick;

	m_currentKnockback = cpvzero;

	{
		Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();

		const Rpg::NpcStateOutput *out = scope.try_get<Rpg::NpcStateOutput>(entity);

		if (!out) {
			LOG_CERROR("game") << "!!!";
			return false;
		}


		if (tick < 0) {
			const RpgStream::NpcState *state = out->at(0);

			if (!state) {
				LOG_CERROR("game") << "Missing tick 0" << this;
				return false;
			}

			RpgMotorNpc::updateBody(m_npc, *state, true);

			saveCurrentState(0);

			m_npc->synchronize();

			return true;
		}

		if (const quint32 diff = m_game->rpgLogicClient()->lastAuthDiff() +
				m_game->rpgLogicClient()->jitterDiff()/2; tick >= diff)
		{
			const quint32 slideTick = tick-diff;
			const RpgStream::NpcState *state = out->at(slideTick);

			if (state && (state->entityState().slideX() != 0 || state->entityState().slideY() != 0))
				m_currentKnockback = cpv(state->entityState().slideXAsFloat(), state->entityState().slideYAsFloat());
		}

	}



	const RpgStream::NpcState *latest = nullptr;

	const std::map<quint32, RpgStream::NpcState> sim = m_game->rpgLogicClient()->getSimulatedStates(entity, m_statePull, &latest);

	if (!latest) {
		LOG_CERROR("game") << "!!!" << tick;
		return false;
	}

	const RpgStream::NpcState *state = nullptr;


	if (!sim.empty() && sim.cbegin()->second == *latest) {
		state = &(sim.rbegin()->second);
	} else {
		bool reqEmplace = true;

		if (!sim.empty()) {
			// Csak akkor helyezzük vissza, ha a pozíció sem stimmel (pl. ha mp-t vett fel, és emiatt változott a státusz, akkor nem bántjuk
			// Később (pl. hp == 0) lekezeljük újra

			if (sim.cbegin()->first > latest->tick()) {
				LOG_CWARNING("game") << "State gap" << latest->tick() << sim.cbegin()->first;
				reqEmplace = false;
			} else if (sim.cbegin()->second.entityState().isEqualWithoutSlide(latest->entityState()))
				reqEmplace = false;
		}

		if (reqEmplace && !cpveql(m_currentKnockback, cpvzero)) {
			reqEmplace = false;
		}

		state = latest;

		if (reqEmplace)
			RpgMotorNpc::updateBody(m_npc, *state, true);
	}

	// Ha most vált halottra vagy éppen most támad fel

	if (state->hp() == 0 || m_npc->hp() == 0) {
		RpgMotorNpc::updateBody(m_npc, *state, true);
	}

	m_npc->setHp(state->hp());


	if (!m_incomingEventList.empty()) {
		processEventAt(state->tick());

		std::erase_if(m_incomingEventList,
					  [jittered = state->tick()](const RpgStream::EventNpc &e) {
			return e.tick() <= jittered;
		});
	}

	return true;
}




/**
 * @brief RpgMotorNpcControlled::afterWorldStep
 * @param tick
 * @param state
 * @return
 */


bool RpgMotorNpcControlled::afterWorldStep(const qint64 &tick, RpgStream::FullState *state)
{
	if (!state)
		return false;

	const quint32 tagId = RpgLogicObjectMapper::getId(m_npc->objectId());

	saveCurrentState(tick);

	std::vector<RpgStream::NpcState> list =
			m_statePull.extractAtLeast(m_game->gameMode() == RpgGame::MultiPlayer ? state->serverTick() : 0,
									   m_game->gameMode() == RpgGame::MultiPlayer ? 6 : 1);			// SINGLE PLAYER: 1


	if (!list.empty()) {
		RpgStream::NpcStateList sl;
		sl.setTagId(tagId);
		sl.setIsDeltaMode(state->isDeltaMode());
		if (state->isDeltaMode())
			sl.compressStateVector(std::move(list));
		else
			sl.setState(std::move(list));

		state->flags().setFlag(RpgStream::FullState::Npc);
		state->npcs().push_back(std::move(sl));
	}

	if (!m_eventList.empty()) {
		for (RpgStream::EventNpc &e : m_eventList) {
			e.setTagId(tagId);
			e.setTick(tick);
		}

		RpgStream::Events events;
		events.setTick(m_gameItem->tickTimer()->currentTick());					// Ide a render lag miatt nem a <tick>-et tesszük!
		events.flags().setFlag(RpgStream::Events::Npc);
		events.setNpc(m_eventList);

		LOG_CINFO("game") << "EVENT" << tick << events.npc().size();

		state->flags().setFlag(RpgStream::FullState::Event);
		state->events().emplace_back(std::move(events));

		m_eventList.clear();
	}


	return true;
}




/**
 * @brief RpgMotorNpcControlled::saveCurrentState
 * @param tick
 * @return
 */

const RpgStream::NpcState *RpgMotorNpcControlled::saveCurrentState(const qint64 &tick)
{
	RpgStream::NpcState st;
	st.setTick(tick);
	st.entityState().setPosXAsFloat(m_npc->bodyPosition().x);
	st.entityState().setPosYAsFloat(m_npc->bodyPosition().y);
	st.entityState().setAngleAsFloat(m_npc->desiredBodyRotation());

	static const quint32 streamMaxSpeed = (quint32){1} << SPEEDSQ_SIZE_BITS;

	if (m_npc->currentSpeedSq() > streamMaxSpeed) {
		LOG_CERROR("engine") << "SpeedSq size error" << m_npc->currentSpeedSq() << ">" << streamMaxSpeed;
	}

	st.entityState().setFacing(m_npc->facingDirection());
	st.entityState().setVelSq(m_npc->currentSpeedSq());
	//st.entityState().setSlideXAsFloat(m_player->d->m_knockbackVelocity.x);
	//st.entityState().setSlideYAsFloat(m_player->d->m_knockbackVelocity.y);


	st.setHp(m_npc->hp());

	saveState(st);

	m_statePull.append(std::move(st));

	return m_statePull.last();
}




/**
 * @brief RpgMotorNpcControlled::processEvent
 * @param event
 */

void RpgMotorNpcControlled::processEvent(const RpgStream::EventNpc &event)
{
	m_incomingEventList.push_back(event);
}




/**
 * @brief RpgMotorNpcControlled::onShapeContactBegin
 * @param self
 * @param other
 */

void RpgMotorNpcControlled::onShapeContactBegin(cpShape *self, cpShape *other)
{
	TiledObjectBody *otherBody = TiledObjectBody::fromShapeRef(other);

	if (!otherBody) {
		LOG_CERROR("game") << "****ERR";
		return;
	}

	const cpShapeFilter &filter = cpShapeGetFilter(other);

	if (m_npc->isBodyShape(self) && (filter.categories & RpgGameItem::FixtureGround)) {
		m_groundCollision.insert(other);
	}
}


/**
 * @brief RpgMotorNpcControlled::onShapeContactEnd
 * @param self
 * @param other
 */

void RpgMotorNpcControlled::onShapeContactEnd(cpShape *self, cpShape *other)
{
	TiledObjectBody *otherBody = TiledObjectBody::fromShapeRef(other);

	if (!otherBody) {
		LOG_CERROR("game") << "****ERR";
		return;
	}

	const cpShapeFilter &filter = cpShapeGetFilter(other);

	if (m_npc->isBodyShape(self) && (filter.categories & RpgGameItem::FixtureGround)) {
		m_groundCollision.remove(other);
	}
}



/**
 * @brief RpgMotorNpcControlled::updateTarget
 */

void RpgMotorNpcControlled::updateTarget()
{

}



/**
 * @brief RpgMotorNpcControlled::getMovementSpeed
 * @return
 */

int RpgMotorNpcControlled::getMovementSpeed()
{
	return m_npc->m_targetEntity ?
				m_npc->m_config.run :
				m_npc->m_config.walk;
}




/**
 * @brief RpgMotorNpcControlled::updateMovement
 * @param speed
 */

void RpgMotorNpcControlled::updateMovement(const float &speed)
{
	const QString &sprite = m_npc->m_spriteHandler->currentSprite();

	static const QStringList disabledList = {
		QStringLiteral("attack"),
		QStringLiteral("bow"),
		QStringLiteral("cast"),
		//QStringLiteral("hurt"),
		QStringLiteral("death")
	};

	if (disabledList.contains(sprite)) {
		m_npc->stop();
		return;
	}


	if (m_destinationPoint) {
		if (!m_npc->moveTowards(m_destinationPoint.value(), speed)) {
			m_npc->stop();
			m_npc->emplace(m_destinationPoint.value());
			m_destinationPoint = std::nullopt;
		}
	} else if (m_destinationMotor) {
		if (m_destinationMotor->atEnd(m_npc)) {
			m_npc->stop();
			m_destinationMotor.reset();
		} else if (const QPolygonF &polygon = m_destinationMotor->polygon(); !polygon.isEmpty()) {
			m_destinationMotor->setSpeed(speed);
			m_destinationMotor->updateBody(m_npc);
		} else {
			m_npc->stop();
			m_destinationMotor.reset();
		}
	} else {
		m_npc->stop();
	}

	/// Workaround
	///
	/// a worldStep() még az updateBody() előtt frissíti a currentSpeedSq-t,	az updateBody() után viszont nem
	/// a saveState() emiatt az első ticknél nem mutat különbséget az előzővel (mivel a sebesség 0), ezért nem küldi el
	/// itt elvégezzük ezt a műveletet a saveState() előtt

	m_npc->overrideCurrentSpeedSq(cpvlengthsq(cpBodyGetVelocity(m_npc->body())));
}




/**
 * @brief RpgMotorNpcControlled::updateMotor
 */

void RpgMotorNpcControlled::updateMotor()
{
	if (m_destinationPoint || m_destinationMotor)
		return;

	if (!m_game || !m_game->rpgLogicClient()) {
		LOG_CERROR("game") << "Missing game or client";
		return;
	}

	Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();

	Rpg::ChunkGrid *grid = scope.getCtx<Rpg::ChunkGrid>();

	if (!grid) {
		LOG_CERROR("game") << "Missing grid";
		return;
	}

	const auto ptr = grid->getRandomChunk(scope.logic()->rnd());

	if (!ptr) {
		LOG_CERROR("game") << "No available chunk";
		return;
	}

	const auto path = m_gameItem->findShortestPath(m_npc, grid->chunkCenter(ptr.value()));

	if (!path) {
		LOG_CERROR("game") << "No available path";
		return;
	}

	setDestination(path.value());
}



/**
 * @brief RpgMotorNpcControlled::onGroundCollision
 */

void RpgMotorNpcControlled::onGroundCollision()
{
	auto ptr = destination();

	if (!ptr)
		return;

	if (m_groundCollisionCounter > 0) {
		--m_groundCollisionCounter;
		return;
	}

	const auto path = m_gameItem->findShortestPath(m_npc, ptr->last().x(), ptr->last().y());

	if (!path) {
		LOG_CERROR("game") << "No available path";
		clearDestination();
		return;
	}

	setDestination(path.value());

	m_groundCollisionCounter = 5;
}



/**
 * @brief RpgMotorNpcControlled::applyKnockback
 */

void RpgMotorNpcControlled::applyKnockback()
{
	if (cpveql(m_currentKnockback, cpvzero))
		return;

	const cpVect current = cpBodyGetVelocity(m_npc->body());

	m_npc->setSpeed(cpvadd(current, m_currentKnockback));

	m_npc->overrideCurrentSpeed(current);
}



bool RpgNpc::isFriend() const
{
	return m_isFriend;
}

void RpgNpc::setIsFriend(bool newIsFriend)
{
	if (m_isFriend == newIsFriend)
		return;
	m_isFriend = newIsFriend;
	emit isFriendChanged();

	updateColor();
}



/**
 * @brief RpgNpc::getColor
 * @return
 */

QColor RpgNpc::getColor() const
{
	return m_rpgGame->getColor(m_team,
							   m_isFriend ? RpgGame::colorNeutral() :
											RpgGame::colorOpponent());
}
