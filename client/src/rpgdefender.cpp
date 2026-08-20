/*
 * ---- Call of Suli ----
 *
 * rpgdefender.cpp
 *
 * Created on: 2026. 05. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgDefender
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

#include <libtiled/map.h>
#include <libtiled/objectgroup.h>
#include "rpgdefender.h"
#include "rpgdefenderfog.h"
#include "rpgplayer.h"
#include "rpgcontrol.h"





// Pulse

static struct {
	QSize size = {192, 210};
	int height = 0;

	TiledObjectSprite spriteNormal = {
		"normal",
		2,
		0, 630, size.width(), size.height(),
		30,
		0,
		true
	};

	TiledObjectSprite spriteActive = {
		"active",
		20,
		0, 0, size.width(), size.height(),
		15,
		0,
		true,
		true			// reverse
	};

	TiledObjectSprite spriteDestroyed = {
		"destroyed",
		1,
		384, 630, size.width(), size.height(),
		30,
		1,
		true
	};

	QString source = ":/rpg/defenderPulse/spritesheet.png";
	RpgControlCommon::SpriteAnimations animations = RpgControlCommon::AnimationNone;
	QPointF offset = { 100-((float) size.width()/2),
					   153-((float) size.height()/2) };

	TiledObjectSpriteList spriteList() const {
		TiledObjectSpriteList l;
		l.sprites << spriteNormal << spriteActive << spriteDestroyed;
		return l;
	};
} defenderPulse;




// Multiplier1

static struct {
	QSize size = {192, 192};
	int height = 0;

	TiledObjectSprite spriteNormal = {
		"normal",
		12,
		0, 0, size.width(), size.height(),
		50,
		0,
		true
	};

	TiledObjectSprite spriteActive = {
		"active",
		12,
		0, 0, size.width(), size.height(),
		50,
		0,
		true
	};

	TiledObjectSprite spriteDestroyed = {
		"destroyed",
		1,
		384, 384, size.width(), size.height(),
		30,
		1,
		true
	};

	QString source = ":/rpg/defenderMultiplier1/spritesheet.png";
	RpgControlCommon::SpriteAnimations animations = RpgControlCommon::AnimationNone;
	QPointF offset = { 95-((float) size.width()/2),
					   145-((float) size.height()/2) };

	TiledObjectSpriteList spriteList() const {
		TiledObjectSpriteList l;
		l.sprites << spriteNormal << spriteActive << spriteDestroyed;
		return l;
	};
} defenderMultiplier1;





// Electric

static struct {
	QSize size = {192, 192};
	int height = 0;

	TiledObjectSprite spriteNormal = {
		"normal",
		10,
		0, 0, size.width(), size.height(),
		40,
		0,
		true
	};

	TiledObjectSprite spriteActive = {
		"active",
		13,
		768, 192, size.width(), size.height(),
		15,
		1,
		true
	};

	TiledObjectSprite spriteDestroyed = {
		"destroyed",
		1,
		960, 576, size.width(), size.height(),
		30,
		1,
		true
	};

	QString source = ":/rpg/defenderElectric/spritesheet.png";
	RpgControlCommon::SpriteAnimations animations = RpgControlCommon::AnimationNone;
	QPointF offset = { 96-((float) size.width()/2),
					   184-((float) size.height()/2) };

	TiledObjectSpriteList spriteList() const {
		TiledObjectSpriteList l;
		l.sprites << spriteNormal << spriteActive << spriteDestroyed;
		return l;
	};
} defenderElectric;






// Questionnaire

static struct {
	QSize size = {192, 192};
	int height = 0;

	TiledObjectSprite spriteNormal = {
		"normal",
		27,
		0, 0, size.width(), size.height(),
		90,
		0,
		true
	};

	TiledObjectSprite spriteActive = {
		"active",
		27,
		0, 0, size.width(), size.height(),
		60,
		0,
		true
	};

	TiledObjectSprite spriteDestroyed = {
		"destroyed",
		1,
		576, 768, size.width(), size.height(),
		30,
		1,
		true
	};

	QString source = ":/rpg/defenderQuestionnaire/spritesheet.png";
	RpgControlCommon::SpriteAnimations animations = RpgControlCommon::AnimationNone;
	QPointF offset = { 95-((float) size.width()/2),
					   145-((float) size.height()/2) };

	TiledObjectSpriteList spriteList() const {
		TiledObjectSpriteList l;
		l.sprites << spriteNormal << spriteActive << spriteDestroyed;
		return l;
	};
} defenderQuestionnaire;















// HP healer

static struct {
	QSize size = {192, 192};
	int height = 0;

	TiledObjectSprite spriteNormal = {
		"normal",
		10,
		0, 0, size.width(), size.height(),
		75,
		0,
		true
	};

	TiledObjectSprite spriteActive = {
		"active",
		10,
		768, 192, size.width(), size.height(),
		75,
		0,
		true
	};

	TiledObjectSprite spriteDestroyed = {
		"destroyed",
		1,
		384, 576, size.width(), size.height(),
		30,
		1,
		true
	};

	QString source = ":/rpg/defenderHpHealer/spritesheet.png";
	RpgControlCommon::SpriteAnimations animations = RpgControlCommon::AnimationNone;
	QPointF offset = { 95-((float) size.width()/2),
					   145-((float) size.height()/2) };

	TiledObjectSpriteList spriteList() const {
		TiledObjectSpriteList l;
		l.sprites << spriteNormal << spriteActive << spriteDestroyed;
		return l;
	};
} defenderHpHealer;




/**
 * @brief RpgDefender::RpgDefender
 * @param gameItem
 * @param pos
 */

RpgDefender::RpgDefender(RpgGameItem *gameItem, const Rpg::DefenderObject &config)
	: RpgEntity(gameItem, config.pos, 20., CP_BODY_TYPE_STATIC)
	, m_config(config)
	, m_visual(StateHidden)
{
	m_defaultMotor = std::make_unique<RpgDefenderMotor>(this);

	filterSet(RpgGameItem::FixtureDefender, RpgGameItem::FixtureAll);
	setSensor(true);

	m_visual.setBasePosition(TiledObjectBody::toPointF(config.pos));
}


/**
 * @brief RpgDefender::~RpgDefender
 */

RpgDefender::~RpgDefender()
{
	m_visual.clear();

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
 * @brief RpgDefender::createDefender
 * @param npc
 * @param gameItem
 * @param scene
 * @param pos
 * @return
 */

RpgDefender *RpgDefender::createDefender(const Rpg::DefenderObject &defender, RpgGameItem *gameItem, TiledScene *scene)
{
	Q_ASSERT(gameItem);
	Q_ASSERT(scene);

	QString common;
	QHash<State, QString> baseImage;

	QString spriteSource;
	TiledObjectSpriteList spriteList;
	RpgControlCommon::SpriteAnimations animations = RpgControlCommon::AnimationNone;
	QPointF offset;
	int height = 0;

	switch (defender.type) {
		case RpgStream::BaseDefenderObject::Fog:
			return gameItem->createObject<RpgDefenderFog>(RpgLogicObjectMapper::toObjectId(defender.idTag),
														  scene, gameItem,
														  defender);

		case RpgStream::BaseDefenderObject::Questionnaire:
			spriteList = defenderQuestionnaire.spriteList();
			spriteSource = defenderQuestionnaire.source;
			animations = defenderQuestionnaire.animations;
			offset = defenderQuestionnaire.offset;
			height = defenderQuestionnaire.height;
			break;

		case RpgStream::BaseDefenderObject::Pulse:
			//common = QStringLiteral("def_pulse.tmx");
			/*baseImage = {
				{ StateNormal, QStringLiteral(":/rpg/time/pickable.png") },
				{ StateActive, QStringLiteral(":/rpg/key/pickable.png") },
			};*/
			spriteList = defenderPulse.spriteList();
			spriteSource = defenderPulse.source;
			animations = defenderPulse.animations;
			offset = defenderPulse.offset;
			height = defenderPulse.height;
			break;

		case RpgStream::BaseDefenderObject::Multiplier1:
			spriteList = defenderMultiplier1.spriteList();
			spriteSource = defenderMultiplier1.source;
			animations = defenderMultiplier1.animations;
			offset = defenderMultiplier1.offset;
			height = defenderMultiplier1.height;
			break;

		case RpgStream::BaseDefenderObject::Electric:
			spriteList = defenderElectric.spriteList();
			spriteSource = defenderElectric.source;
			animations = defenderElectric.animations;
			offset = defenderElectric.offset;
			height = defenderElectric.height;
			break;

		case RpgStream::BaseDefenderObject::HpHealer:
			spriteList = defenderHpHealer.spriteList();
			spriteSource = defenderHpHealer.source;
			animations = defenderHpHealer.animations;
			offset = defenderHpHealer.offset;
			height = defenderHpHealer.height;
			break;


		case RpgStream::BaseDefenderObject::None:
			LOG_CERROR("game") << "Invalid defender type" << defender.type;
			break;
	}



	if (!common.isEmpty())
		return gameItem->createObject<RpgDefenderCommon>(RpgLogicObjectMapper::toObjectId(defender.idTag),
														 scene,
														 common, gameItem, defender, baseImage);
	else if (!spriteSource.isEmpty()) {
		RpgDefenderCommon *c = gameItem->createObject<RpgDefenderCommon>(RpgLogicObjectMapper::toObjectId(defender.idTag),
																		 scene,
																		 gameItem, defender,
																		 spriteSource, spriteList, animations);
		if (!spriteList.sprites.isEmpty()) {
			c->visualItem()->setSize(QSizeF(spriteList.sprites.first().width, spriteList.sprites.first().height));
		}

		if (height > 0 && c->m_markerItem)
			c->m_markerItem->setProperty("entityHeight", height);

		c->setBodyOffset(offset);


		if (defender.type == RpgStream::BaseDefenderObject::Electric) {
			connect(c, &RpgDefenderCommon::hurt, c, [c]() {
				c->jumpToSprite("active");
				c->jumpToSpriteLater("normal");

				c->game()->playSfx(QStringLiteral(":/rpg/lightning/lightning2.mp3"),
								   c->scene(), c->bodyPositionF());
			});
		}

		return c;
	}

	return nullptr;
}





/**
 * @brief RpgDefender::updateVisibility
 */

void RpgDefender::updateVisibility()
{
	if (!m_rpgGame || !m_rpgGame->controlledPlayer()) {
		LOG_CERROR("game") << "Invalid game or player";
		return;
	}

	if (m_visibleToAll || m_team == m_rpgGame->controlledPlayer()->team())
		setState(isAlive() ? (m_hasTarget ? StateActive : StateNormal) : StateDestroyed);
	else
		setState(StateHidden);

	if (m_visualItem)
		m_visualItem->setVisible(m_visual.state() != StateHidden);

	if (m_markerItem)
		m_markerItem->setVisible(m_visual.state() != StateHidden && m_marked && isAlive());
}




/**
 * @brief RpgDefender::defenderPoint
 * @return
 */

RpgDefenderPoint *RpgDefender::defenderPoint() const
{
	return m_defenderPoint;
}

RpgTower *RpgDefender::tower() const
{
	return m_tower;
}

void RpgDefender::setDefenderPoint(RpgDefenderPoint *newDefenderPoint)
{
	m_defenderPoint = newDefenderPoint;
}

void RpgDefender::setTower(RpgTower *newTower)
{
	m_tower = newTower;
}


const Rpg::DefenderObject &RpgDefender::config() const
{
	return m_config;
}

bool RpgDefender::visibleToAll() const
{
	return m_visibleToAll;
}

void RpgDefender::setVisibleToAll(bool newVisibleToAll)
{
	if (m_visibleToAll == newVisibleToAll)
		return;
	m_visibleToAll = newVisibleToAll;
	emit visibleToAllChanged();
	updateVisibility();
}

bool RpgDefender::hasTarget() const
{
	return m_hasTarget;
}

void RpgDefender::setHasTarget(bool newHasTarget)
{
	if (m_hasTarget == newHasTarget)
		return;
	m_hasTarget = newHasTarget;
	emit hasTargetChanged();

	updateVisibility();
}


/**
 * @brief RpgDefender::setMarked
 * @param marked
 */

void RpgDefender::setMarked(const bool &marked)
{
	m_marked = marked;

	updateVisibility();
}


/**
 * @brief RpgDefender::onAlive
 */

void RpgDefender::onAlive()
{
	setSubZ(0.5);
	updateVisibility();
}


/**
 * @brief RpgDefender::onDead
 */

void RpgDefender::onDead()
{
	setSubZ(0.0);
	updateVisibility();
}



/**
 * @brief RpgDefender::updateColor
 */

void RpgDefender::updateColor()
{
	QColor color = m_rpgGame->getColor(m_team);

	if (m_markerItem) {
		m_markerItem->setProperty("progressBarColor", color);
		m_markerItem->setProperty("labelColor", color);
	}

	if (TiledVisualItem *item = m_visual.imageItem()) {
		item->setGlowColor(color);
	}

	updateVisibility();
}



/**
 * @brief RpgDefender::loadFromCommonMap
 * @param name
 * @return
 */

bool RpgDefender::loadFromCommonMap(const QString &name, const QHash<State, QString> &baseImageHash)
{
	m_scene = scene();

	Q_ASSERT(m_scene);

	const Tiled::Map *map = m_rpgGame->commonMap(name);
	Tiled::MapRenderer *renderer = m_rpgGame->commonRenderer(name);

	if (!map) {
		LOG_CERROR("game") << "Invalid map";
		return false;
	}

	if (!renderer) {
		LOG_CERROR("game") << "Invalid renderer";
		return false;
	}

	static const QHash<QString, State> stateHash = {
		{ "active", StateActive },
		{ "destroyed", StateDestroyed },
		{ "normal", StateNormal }
	};

	std::optional<QPointF> ref;

	for (Tiled::Layer *layer : map->layers()) {
		if (Tiled::ObjectGroup *gr = layer->asObjectGroup()) {
			for (Tiled::MapObject *object : std::as_const(gr->objects())) {
				if (object->className() == QStringLiteral("base")) {
					ref = renderer->pixelToScreenCoords(object->position())+gr->totalOffset();
				}
			}
		}
	}



	TiledVisualItem *item = m_scene->addVisualItem();
	m_visualItem = item;

	m_visual.setImageItem(item);


	for (const auto &[st, url] : baseImageHash.asKeyValueRange())
		m_visual.addSource(st, QUrl::fromLocalFile(url));


	TiledQuick::TileLayerItem *layerNormal = nullptr;
	TiledQuick::TileLayerItem *layerActive = nullptr;

	for (Tiled::Layer *layer : map->layers()) {
		if (Tiled::TileLayer *tl = layer->asTileLayer()) {
			TiledQuick::TileLayerItem *layerItem = m_scene->addTileLayer(tl, renderer);

			const State st = stateHash.value(tl->className(), StateNormal);

			if (st == StateNormal)
				layerNormal = layerItem;
			else if (st == StateActive)
				layerActive = layerItem;

			QPointF r;
			if (ref.has_value())
				r = ref.value();
			else {
				r.setX(layerItem->width()/2);
				r.setY(layerItem->height()/2);
			}

			QObject::connect(item, &TiledVisualItem::zChanged, layerItem, [item, layerItem]() {
				layerItem->setZ(item->z());
			});
			layerItem->setPosition(layerItem->position() + m_visual.basePosition() - r);

			m_layerItems.append(layerItem);
			m_visual.addLayer(st, layerItem);
		}
	}

	// Duplicate Active and Normal state if missing

	if (layerNormal && !layerActive)
		m_visual.addLayer(StateActive, layerNormal);
	else if (!layerNormal && layerActive)
		m_visual.addLayer(StateNormal, layerActive);


	addMarkerItem();

	updateColor();

	return true;
}





/**
 * @brief RpgDefender::addMarkerItem
 */

void RpgDefender::addMarkerItem()
{
	m_markerItem = createMarkerItem(QStringLiteral("qrc:/RpgDefenderMarker.qml"));
	m_markerItem->setVisible(false);
}





/**
 * @brief RpgDefenderMotor::beforeWorldStep
 * @param tick
 * @param entity
 * @return
 */

bool RpgDefenderMotor::beforeWorldStep(const qint64 &/*tick*/, entt::entity &entity)
{
	Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();

	const RpgStream::DefenderState *state = scope.getCurrentState<RpgStream::DefenderState>(entity);

	if (!state) {
		LOG_CERROR("game") << "Invalid defender state";
		return false;
	}

	m_defender->setHp(state->hp());
	m_defender->setHasTarget(state->targetId() > 0);
	m_defender->setVisibleToAll(state->visible());

	return true;
}


/**
 * @brief RpgDefenderCommon::RpgDefenderCommon
 * @param name
 * @param gameItem
 * @param config
 * @param baseImageHash
 */

RpgDefenderCommon::RpgDefenderCommon(const QString &name, RpgGameItem *gameItem, const Rpg::DefenderObject &config,
									 const QHash<State, QString> &baseImageHash)
	: RpgDefender(gameItem, config)
	, m_name(name)
	, m_baseImageHash(baseImageHash)
{

}



RpgDefenderCommon::RpgDefenderCommon(RpgGameItem *gameItem, const Rpg::DefenderObject &config,
									 const QString &spriteSource, const TiledObjectSpriteList &spriteList,
									 const RpgControlCommon::SpriteAnimations &animations)
	: RpgDefender(gameItem, config)
	, m_spriteSource(spriteSource)
	, m_spriteList(spriteList)
	, m_animations(animations)
{

}



/**
 * @brief RpgDefenderCommon::initialize
 */

void RpgDefenderCommon::initialize()
{
	if (!m_name.isEmpty()) {
		if (!loadFromCommonMap(m_name, m_baseImageHash)) {
			LOG_CERROR("game") << "Common defender load failed" << m_name;
		}
	} else {
		m_scene = scene();

		Q_ASSERT(m_scene);

		createVisual();
		appendSprite(m_spriteSource, m_spriteList);

		addMarkerItem();

		updateColor();

		if (!m_spriteList.sprites.isEmpty())
			jumpToSprite(m_spriteList.sprites.first().name.toLatin1());
	}

	onAlive();
}



/**
 * @brief RpgDefenderCommon::stateChange
 * @param from
 * @param to
 */

void RpgDefenderCommon::stateChange(const State &from, const State &to)
{
	if (m_spriteSource.isEmpty())
		return;

	//bool hasAnim = false;

	struct Anim {
		State from;
		State to;
		RpgControlCommon::SpriteAnimation flag;
		const char* sprite;
	};

	static const std::vector<Anim> animations = {
		{ StateNormal, StateActive, RpgControlCommon::AnimationNormalToActive, "activating" },
		{ StateActive, StateNormal, RpgControlCommon::AnimationActiveToNormal, "deactivating" },
		{ StateNormal, StateDestroyed, RpgControlCommon::AnimationNormalToDestroyed, "destroying" },
		{ StateDestroyed, StateNormal, RpgControlCommon::AnimationDestroyedToNormal, "reloading" },
		{ StateActive, StateDestroyed, RpgControlCommon::AnimationActiveToDestroyed, "unloading" },
		{ StateDestroyed, StateActive, RpgControlCommon::AnimationDestroyedToActive, "reactivating" },
		{ StateHidden, StateActive, RpgControlCommon::AnimationNormalToActive, "activating" },
	};

	for (const Anim &a : animations) {
		if (from == a.from && to == a.to && m_animations.testFlag(a.flag)) {
			//hasAnim = true;
			jumpToSprite(a.sprite);
			break;
		}
	}

	static const QHash<State, const char*> stateHash = {
		{ StateActive, "active" },
		{ StateDestroyed, "destroyed" },
		{ StateNormal, "normal" }
	};

	if (const char *sprite = stateHash.value(to))
		jumpToSpriteLater(sprite);
}
