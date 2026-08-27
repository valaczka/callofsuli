/*
 * ---- Call of Suli ----
 *
 * rpgcontrol.cpp
 *
 * Created on: 2026. 07. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControl
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

#include "rpgcontrol.h"
#include "tiledspritehandler.h"





/**
 * @brief RpgControl::RpgControl
 * @param gameItem
 * @param config
 */

RpgControl::RpgControl(RpgGameItem *gameItem, const Rpg::Control &config)
	: RpgObject(gameItem, config.pos, 20., CP_BODY_TYPE_STATIC)
	, m_config(config)
{
	m_defaultMotor = std::make_unique<RpgControlMotor>(this);

	filterSet(RpgGameItem::FixtureControl, RpgGameItem::FixtureAll);
	setSensor(true);

	this->setGlowColor(RpgGame::colorGlow());

	setSubZ(0.5);
}



/**
 * @brief RpgControl::~RpgControl
 */

RpgControl::~RpgControl()
{
	if (!m_scene) {
		LOG_CERROR("game") << "Missing scene" << this;
		return;
	}

	for (TiledQuick::TileLayerItem *layer : std::as_const(m_layerItems)) {
		m_scene->removeVisualItem(layer);
		layer->deleteLater();
	}

	m_layerItems.clear();
}




/**
 * @brief RpgControl::createControl
 * @param config
 * @param gameItem
 * @param scene
 * @return
 */

RpgControl *RpgControl::createControl(const Rpg::Control &config, RpgGameItem *gameItem, TiledScene *scene)
{
	Q_ASSERT(gameItem);
	Q_ASSERT(scene);

	QString common;
	QHash<StateCommon, QString> baseImage;

	QString spriteSource;
	TiledObjectSpriteList spriteList;
	RpgControlCommon::SpriteAnimations animations = RpgControlCommon::AnimationNone;
	QPointF offset;
	QString title;
	int height = 0;
	RpgGameItem::ProxyDirections proxy;

	switch (config.type) {
		case RpgStream::ControlData::Chest:
			spriteSource = QStringLiteral(":/rpg/chest");
			title = tr("Upgrade danger");
			animations = RpgControlCommon::AnimationNormalToActive;
			height = 80;
			proxy = {
				{ SouthWest, { SouthWest, West, NorthWest, North } },
				{ East, { NorthEast, East, SouthEast, South } }
			};
			break;

		case RpgStream::ControlData::None:
			LOG_CERROR("game") << "Invalid control type" << config.type;
			break;
	}



	if (!common.isEmpty())
		return gameItem->createObject<RpgControlCommon>(RpgLogicObjectMapper::toObjectId(config.idTag),
														scene,
														common, gameItem, config, baseImage,
														title);
	else if (!spriteSource.isEmpty() && proxy.isEmpty()) {
		RpgControlCommon *c = gameItem->createObject<RpgControlCommon>(RpgLogicObjectMapper::toObjectId(config.idTag),
														scene,
														gameItem, config,
														spriteSource, spriteList, animations,
														title);
		if (!spriteList.sprites.isEmpty()) {
			c->visualItem()->setSize(QSizeF(spriteList.sprites.first().width, spriteList.sprites.first().height));
		}

		if (height > 0 && c->m_markerItem)
			c->m_markerItem->setProperty("entityHeight", height);

		c->setBodyOffset(offset);

		return c;
	} else if (!spriteSource.isEmpty() && !proxy.empty()) {
		RpgControlCommon *c = gameItem->createObject<RpgControlCommon>(RpgLogicObjectMapper::toObjectId(config.idTag),
														scene,
														gameItem, config, spriteSource, proxy, animations,
														title);

		if (height > 0 && c->m_markerItem)
			c->m_markerItem->setProperty("entityHeight", height);

		return c;
	}

	return nullptr;
}





/**
 * @brief RpgControl::setMarked
 * @param marked
 */

void RpgControl::setMarked(const bool &marked)
{
	if (m_markerItem)
		m_markerItem->setVisible(marked && m_isAlive);

	this->setGlowEnabled(marked && m_isAlive);

	RpgObject::setMarked(marked && m_isAlive);
}





/**
 * @brief RpgControl::config
 * @return
 */

const Rpg::Control &RpgControl::config() const
{
	return m_config;
}


/**
 * @brief RpgControl::isAlive
 * @return
 */

bool RpgControl::isAlive() const
{
	return m_isAlive;
}

void RpgControl::setIsAlive(bool newIsAlive)
{
	if (m_isAlive == newIsAlive)
		return;
	m_isAlive = newIsAlive;
	emit isAliveChanged();

	setSubZ(m_isAlive ? 0.5 : 0.0);

	setMarked(false);
}




/**
 * @brief RpgControlMotor::beforeWorldStep
 * @param tick
 * @param entity
 * @return
 */

bool RpgControlMotor::beforeWorldStep(const qint64 &/*tick*/, entt::entity &entity)
{
	Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();

	const RpgStream::ControlState *state = scope.getCurrentState<RpgStream::ControlState>(entity);

	if (!state) {
		LOG_CERROR("game") << "Invalid control state";
		return false;
	}

	m_control->setIsAlive(state->isAlive());

	m_control->loadCurrentState(*state);

	return true;
}






/**
 * @brief RpgControlCommon::RpgControlCommon
 * @param name
 * @param gameItem
 * @param config
 * @param baseImageHash
 * @param displayName
 */

RpgControlCommon::RpgControlCommon(const QString &name, RpgGameItem *gameItem, const Rpg::Control &config,
								   const QHash<StateCommon, QString> &baseImageHash, const QString &displayName)
	: RpgControl(gameItem, config)
	, RpgControlCommonIface(name, config, StateNormal)
	, m_baseImageHash(baseImageHash)
	, m_spriteSource()
	, m_spriteList()
{
	Q_ASSERT(!m_name.isEmpty());

	if (!displayName.isEmpty())
		setDisplayName(displayName);
}




/**
 * @brief RpgControlCommon::RpgControlCommon
 * @param name
 * @param gameItem
 * @param config
 * @param spriteSource
 * @param spriteList
 * @param animations
 * @param displayName
 */

RpgControlCommon::RpgControlCommon(RpgGameItem *gameItem, const Rpg::Control &config,
								   const QString &spriteSource, const TiledObjectSpriteList &spriteList,
								   const SpriteAnimations &animations, const QString &displayName)
	: RpgControl(gameItem, config)
	, RpgControlCommonIface(QString(), config, StateNormal)
	, m_baseImageHash()
	, m_spriteSource(spriteSource)
	, m_spriteList(spriteList)
	, m_animations(animations)
{
	if (!displayName.isEmpty())
		setDisplayName(displayName);
}



/**
 * @brief RpgControlCommon::RpgControlCommon
 * @param gameItem
 * @param config
 * @param texturePath
 * @param animations
 * @param displayName
 */

RpgControlCommon::RpgControlCommon(RpgGameItem *gameItem, const Rpg::Control &config,
								   const QString &texturePath, const RpgGameItem::ProxyDirections &proxy,
								   const SpriteAnimations &animations, const QString &displayName)
	: RpgControl(gameItem, config)
	, RpgControlCommonIface(QString(), config, StateNormal)
	, m_baseImageHash()
	, m_proxy(proxy)
	, m_spriteSource(texturePath)
	, m_animations(animations)
{
	if (!displayName.isEmpty())
		setDisplayName(displayName);
}



/**
 * @brief RpgControlCommon::initialize
 */

void RpgControlCommon::initialize()
{
	m_scene = scene();

	Q_ASSERT(m_scene);

	if (!m_name.isEmpty()) {
		TiledVisualItem* item = loadFromCommonMap(m_rpgGame, m_scene, &m_layerItems, m_name, stateHash(), baseImageHash());

		if (!item) {
			LOG_CERROR("game") << "Common control load failed" << m_name;
			return;
		}

		m_visualItem = item;
	} else if (m_proxy) {
		createVisual();
		setAvailableDirections(Direction_8);

		QRect measure = RpgGameItem::loadTextureSprites(m_spriteHandler, m_spriteSource+QStringLiteral("/"), nullptr, m_proxy.value());

		m_visualItem->setWidth(measure.width());
		m_visualItem->setHeight(measure.height());
		setBodyOffset(measure.x(), measure.y());

		m_spriteHandler->setVisibleLayers({"default"});

		synchronize();
		jumpToSprite("normal");

	} else {
		createVisual();
		appendSprite(m_spriteSource, m_spriteList);

		if (!m_spriteList.sprites.isEmpty())
			jumpToSprite(m_spriteList.sprites.first().name.toLatin1());
	}

	setState(m_visual.state());

	resetMarkerDisplay(m_displayName);
}





/**
 * @brief RpgControlCommon::initControl
 */

void RpgControlCommon::initControl()
{
	if (m_config.type == RpgStream::ControlData::Chest) {
		rotateBody(TiledObject::directionToIsometricRadian(m_config.data == 1 ? SouthWest : SouthEast), true);
		jumpToSprite("normal");
	}
}


/**
 * @brief RpgControlCommon::setMarked
 * @param marked
 */

void RpgControlCommon::setMarked(const bool &marked)
{
	if (m_config.type == RpgStream::ControlData::Chest) {
		return RpgControl::setMarked(marked && m_visual.state() == StateNormal);
	}

	return RpgControl::setMarked(marked);
}






/**
 * @brief RpgControlCommon::canTargeting
 * @return
 */

bool RpgControlCommon::canTargeting() const
{
	if (m_config.type == RpgStream::ControlData::Chest) {
		return m_visual.state() == StateNormal;
	}

	return RpgControl::canTargeting();
}





/**
 * @brief RpgControlCommon::resetMarkerDisplay
 * @param displayName
 */

void RpgControlCommon::resetMarkerDisplay(const QString &displayName)
{
	if (displayName.isEmpty())
		return;

	if (!m_markerItem)
		m_markerItem = createMarkerItem();

	setDisplayName(displayName);
}


/**
 * @brief RpgControlCommon::stateHash
 * @return
 */

QHash<QString, RpgControl::StateCommon> RpgControlCommon::stateHash() const
{
	static const QHash<QString, RpgControl::StateCommon> hash = {
		{ "active", StateActive },
		{ "destroyed", StateDestroyed },
		{ "normal", StateNormal }
	};

	return hash;
}


/**
 * @brief RpgControlCommon::stateChange
 * @param from
 * @param to
 */

void RpgControlCommon::stateChange(const StateCommon &from, const StateCommon &to)
{
	if (m_spriteSource.isEmpty())
		return;

	struct Anim {
		RpgControlCommon::StateCommon from;
		RpgControlCommon::StateCommon to;
		SpriteAnimation flag;
		const char* sprite;
	};

	static const std::vector<Anim> animations = {
		{ StateNormal, StateActive, AnimationNormalToActive, "activating" },
		{ StateActive, StateNormal, AnimationActiveToNormal, "deactivating" },
		{ StateNormal, StateDestroyed, AnimationNormalToDestroyed, "destroying" },
		{ StateDestroyed, StateNormal, AnimationDestroyedToNormal, "reloading" },
		{ StateActive, StateDestroyed, AnimationActiveToDestroyed, "unloading" },
		{ StateDestroyed, StateActive, AnimationDestroyedToActive, "reactivating" },
	};

	for (const Anim &a : animations) {
		if (from == a.from && to == a.to && m_animations.testFlag(a.flag)) {
			jumpToSprite(a.sprite);
			break;
		}
	}

	jumpToSpriteLater(stateHash().key(to).toLatin1());

	if (m_config.type == RpgStream::ControlData::Chest) {
		if (to == StateDestroyed) {
			m_visualItem->setVisible(false);
			setMarked(false);
		} else {
			if (to == StateActive)
				setMarked(false);

			m_visualItem->setVisible(true);
		}
	}
}



/**
 * @brief RpgControlCommon::loadCurrentState
 * @param state
 */

void RpgControlCommon::loadCurrentState(const RpgStream::ControlState &state)
{
	if (m_config.type == RpgStream::ControlData::Chest) {
		if (state.state() == Rpg::Chest::StateDisabled)
			setState(StateDestroyed);
		else if (state.state() == Rpg::Chest::StateActivated)
			setState(StateActive);
		else
			setState(StateNormal);
	}
}


/**
 * @brief RpgControlCommon::animations
 * @return
 */

const RpgControlCommon::SpriteAnimations &RpgControlCommon::animations() const
{
	return m_animations;
}

void RpgControlCommon::setAnimations(const SpriteAnimations &newAnimations)
{
	m_animations = newAnimations;
}
