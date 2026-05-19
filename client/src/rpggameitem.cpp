/*
 * ---- Call of Suli ----
 *
 * rpggameitem.cpp
 *
 * Created on: 2026. 05. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgGameItem
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

#include <libtiled/objectgroup.h>
#include "rpggameitem.h"
#include "grouplayer.h"
#include "rpggame.h"
#include "rpggame_p.h"
#include "rpgobject.h"
#include "rpgplayer.h"
#include "rpgtower.h"
#include "tileddebugdraw.h"
#include "utils_.h"



const QHash<RpgStream::Team, QColor> RpgGameItem::m_teamColor = {
	{ RpgStream::TeamNone, QColorConstants::Svg::white },
	{ RpgStream::TeamA, QColorConstants::Svg::cyan },
	{ RpgStream::TeamB, QColorConstants::Svg::yellow },
};



/**
 * @brief The RpgObjectExclude class
 */

class RpgObjectExclude : public TiledObjectBody
{
public:
	explicit RpgObjectExclude(const QPolygonF &polygon,
							  TiledGame *game,
							  Tiled::MapRenderer *renderer = nullptr,
							  const cpBodyType &type = CP_BODY_TYPE_DYNAMIC,
							  const QPointF &offset = {})
		: TiledObjectBody(polygon, game, renderer, type, offset)
	{
		m_drawBodyStyle = m_sytle;
	}

	explicit RpgObjectExclude(const QPointF &center, const qreal &radius,
							  TiledGame *game,
							  Tiled::MapRenderer *renderer = nullptr,
							  const cpBodyType &type = CP_BODY_TYPE_DYNAMIC,
							  const QPointF &offset = {})
		: TiledObjectBody(center, radius, game, renderer, type, offset)
	{
		m_drawBodyStyle = m_sytle;
	}

	explicit RpgObjectExclude(const Tiled::MapObject *object,
							  TiledGame *game,
							  Tiled::MapRenderer *renderer = nullptr,
							  const cpBodyType &type = CP_BODY_TYPE_DYNAMIC)
		: TiledObjectBody(object, game, renderer, type)
	{
		m_drawBodyStyle = m_sytle;
	}

private:
	inline const static DrawBodyStyle m_sytle = {
		.color = QColorConstants::Svg::blueviolet,
		.lineWidth = 1.,
		.filled = false,
		.outlined = true
	};
};




/**
 * @brief RpgGameItem::RpgGameItem
 * @param parent
 */

RpgGameItem::RpgGameItem(QQuickItem *parent)
	: TiledGame(parent)
{
	m_groundCategory = FixtureGround;
}



/**
 * @brief RpgGameItem::~RpgGameItem
 */

RpgGameItem::~RpgGameItem()
{

}


/**
 * @brief RpgGameItem::game
 * @return
 */

RpgGame *RpgGameItem::game() const
{
	return m_game;
}

void RpgGameItem::setGame(RpgGame *newGame)
{
	if (m_game == newGame)
		return;

	if (m_game)
		m_game->setGameItem(nullptr);

	m_game = newGame;

	if (m_game)
		m_game->setGameItem(this);

	emit gameChanged();

	d = m_game ? m_game->d : nullptr;
}





/**
 * @brief RpgGameItem::load
 * @param def
 * @return
 */

bool RpgGameItem::load(const RpgGameDefinition &def)
{
	LOG_CINFO("game") << "Create game";

	if (!def.minVersion.isEmpty()) {
		const QVersionNumber v = QVersionNumber::fromString(def.minVersion).normalized();

		if (!v.isNull() && v > Utils::versionNumber()) {
			LOG_CWARNING("game") << "Required version:" << v.majorVersion() << v.minorVersion();
			emit gameLoadFailed(tr("Szükséges verzió: %1.%2").arg(v.majorVersion()).arg(v.minorVersion()));
			return false;
		}
	}

	if (!TiledGame::load(def))
		return false;

	return true;
}


/**
 * @brief RpgGameItem::onMouseClick
 * @param x
 * @param y
 * @param buttons
 * @param modifiers
 */

void RpgGameItem::onMouseClick(const qreal &x, const qreal &y, const int &buttons, const int &modifiers)
{
	if (m_paused || !m_game)
		return;

	RpgPlayer *player = m_game->controlledPlayer();

	if (!player)
		return;

	RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(player->currentMotor());

	/*if (Qt::MouseButtons::fromInt(buttons).testFlag(Qt::RightButton)) {
		m_controlledPlayer->clearDestinationPoint();
		return;
	}

#ifndef QT_NO_DEBUG
	if (modifiers & Qt::AltModifier) {
		m_controlledPlayer->clearDestinationPoint();
		m_controlledPlayer->TiledObject::emplace(x, y);
		return;
	}
#endif

	if (!m_controlledPlayer->isAlive())
		return;

	if (mouseAttack()) {
		m_controlledPlayer->attackToPoint(x, y);
		return;
	}*/

	if (!mouseNavigation())
		return;

	/*if (modifiers & Qt::ControlModifier) {
		m_controlledPlayer->attackToPoint(x, y);
	} else {
		if (modifiers & Qt::ShiftModifier)
			m_controlledPlayer->m_pickAtDestination = true;
		else
			m_controlledPlayer->m_pickAtDestination = false;

		if (const auto &ptr = findShortestPath(m_controlledPlayer, cpv(x,y))) {
			m_controlledPlayer->setDestinationPoint(ptr.value());

			if (!m_controlledPlayer->m_sfxAccept.soundList().isEmpty())
				m_controlledPlayer->m_sfxAccept.playOne();

		} else {
			if (!m_controlledPlayer->m_sfxDecline.soundList().isEmpty())
				m_controlledPlayer->m_sfxDecline.playOne();

			m_controlledPlayer->clearDestinationPoint();
		}
	}*/


	if (!motor)
		return;

	if (const auto &ptr = findShortestPath(player, cpv(x,y))) {
		motor->setDestination(ptr.value());
	}
}




/**
 * @brief RpgGameItem::sceneDebugDrawEvent
 * @param debugDraw
 * @param scene
 */
void RpgGameItem::sceneDebugDrawEvent(TiledDebugDraw *debugDraw, TiledScene *scene)
{
	TiledGame::sceneDebugDrawEvent(debugDraw, scene);

	if (!debugDraw || !scene)
		return;

	if (RpgPlayer *player = m_game->controlledPlayer()) {
		QPointF ch = player->currentChunkCenter();

		if (ch.x() >= 0 && ch.y() >= 0) {
			debugDraw->drawSolidCircle(ch, 6., QColor::fromRgb(230, 0, 0));
		}
	}

	/*
	for (const auto &e : m_enemyDataList) {
		if (e.scene != scene || e.motor.path.isEmpty())
			continue;

		if (!e.enemy || !e.enemy->isAlive())
			continue;


		if (TiledPathMotor *motor = e.enemy->destinationMotor()) {
			debugDraw->drawPolygon(motor->polygon(),
								   QColor::fromRgb(230, 150, 0),
								   3.);
		} else if (e.enemy->m_returnPathMotor) {
			debugDraw->drawPolygon(e.enemy->m_returnPathMotor->path(),
								   cpBodyGetType(e.enemy->body()) == CP_BODY_TYPE_KINEMATIC ?
									   QColor::fromRgb(0, 200, 0) :
									   QColor::fromRgb(230, 0, 200),
								   3.);
		} else if (e.motor.path.size() > 1) {
			debugDraw->drawPolygon(e.motor.path, QColor::fromRgb(230, 0, 0), 3.);
		} else
			debugDraw->drawSolidCircle(e.motor.path.first(), 3., QColor::fromRgb(230, 0, 0));
	}


	for (const RpgPlayer *p : m_players) {
		if (TiledPathMotor *motor = p->destinationMotor()) {
			debugDraw->drawPolygon(motor->polygon(),
								   p == m_controlledPlayer ? QColor::fromRgb(0, 230, 0) : QColor::fromRgb(230, 150, 0),
								   4.);
		}
	} */

	iterateOverBodies([debugDraw, this](TiledObjectBody *body) {
		RpgPlayer *p = dynamic_cast<RpgPlayer*>(body);

		if (p) {
			if (RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(p->currentMotor())) {

				if (const auto &ptr = motor->destination()) {
					debugDraw->drawPolygon(ptr.value(),
										   p == m_game->controlledPlayer() ? QColor::fromRgb(0, 230, 0) : QColor::fromRgb(230, 150, 0),
										   4.);
				}
			}
		}

	});
}



/**
 * @brief RpgGameItem::loadTileLayer
 * @param scene
 * @param layer
 * @param renderer
 */

void RpgGameItem::loadTileLayer(TiledScene *scene, Tiled::TileLayer *layer, Tiled::MapRenderer *renderer)
{
	/*if (layer->className() == QStringLiteral("chunkMarker")) {
		LOG_CERROR("game") << "LOAD CHUNK LAYER";

		d->m_chunkMarkerLayer = scene->addTileLayer(layer, renderer);
		d->m_chunkMarkerBaseOffset = -renderer->tileToScreenCoords(0, 0) + layer->totalOffset();

		LOG_CINFO("game") << "BASE OFFSET" << d->m_chunkMarkerBaseOffset;

		return;
	}*/
	TiledGame::loadTileLayer(scene, layer, renderer);
}



/**
 * @brief RpgGameItem::loadObjectLayer
 * @param scene
 * @param group
 * @param renderer
 * @return
 */


bool RpgGameItem::loadObjectLayer(TiledScene *scene, Tiled::ObjectGroup *group, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(d);

	for (Tiled::MapObject *object : std::as_const(group->objects())) {
		/*if (object->className().startsWith(QStringLiteral("player"))) {
			LOG_CINFO("game") << "REGISTER" << object->className();

			const QPointF pos = renderer->pixelToScreenCoords(object->position() + group->totalOffset());

			d->m_logic->playerPositionAdd(pos, Rpg::TeamTag::TeamNone);
		}*/

		if (group->className() == QStringLiteral("teamA") || group->name() == QStringLiteral("teamA")) {
			LOG_CINFO("game") << "REGISTER A" << object->className();

			const QPointF pos = renderer->pixelToScreenCoords(object->position() + group->totalOffset());

			d->playerPositionAdd(pos, RpgStream::TeamA);
		} else if (group->className() == QStringLiteral("teamB") || group->name() == QStringLiteral("teamB")) {
			LOG_CINFO("game") << "REGISTER B" << object->className();

			const QPointF pos = renderer->pixelToScreenCoords(object->position() + group->totalOffset());

			d->playerPositionAdd(pos, RpgStream::TeamB);
		}
	}

	return TiledGame::loadObjectLayer(scene, group, renderer);


}



/**
 * @brief RpgGameItem::loadObjectLayer
 * @param scene
 * @param object
 * @param groupClass
 * @param renderer
 */

void RpgGameItem::loadObjectLayer(TiledScene *scene, Tiled::MapObject *object, const QString &groupClass, Tiled::MapRenderer *renderer)
{
	Q_UNUSED(scene);
	Q_UNUSED(object);
	Q_UNUSED(groupClass);
	Q_UNUSED(renderer);
}




/**
 * @brief RpgGameItem::loadTower
 * @param scene
 * @param group
 * @param renderer
 */

void RpgGameItem::loadTower(TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer)
{
	LOG_CDEBUG("game") << "LOAD TOWER" << group->name();

	QMultiMap<RpgStream::Team, TiledQuick::TileLayerItem *> layers;
	RpgTower *tower = nullptr;

	TiledVisualItem *visualItem = nullptr;

	for (Tiled::Layer *layer : std::as_const(*group)) {
		if (Tiled::TileLayer *tl = layer->asTileLayer()) {
			RpgStream::Team team = RpgStream::TeamNone;

			if (tl->className() == QStringLiteral("teamA"))
				team = RpgStream::TeamA;
			else if (tl->className() == QStringLiteral("teamB"))
				team = RpgStream::TeamB;

			if (team == RpgStream::TeamNone) {
				visualItem = scene->addVisualItem(tl, renderer);
				visualItem->setGlowColor(QColorConstants::Svg::gold);

				LOG_CDEBUG("game") << "Load tower TileLayer" << tl->name() << "in" << group->name() << visualItem->position();
				continue;
			}

			tl->setName(group->name());							// Name override for dynamicZ

			LOG_CDEBUG("game") << "LOAD TOWER TILE" << group->name() << tl->name() << team;
			TiledQuick::TileLayerItem *item = scene->addTileLayer(tl, renderer);
			item->setVisible(false);

			layers.insert(team, item);

		} else if (Tiled::ObjectGroup *gr = layer->asObjectGroup()) {
			for (Tiled::MapObject *object : std::as_const(gr->objects())) {

				if (object->className() == QStringLiteral("tower")) {
					if (object->shape() != Tiled::MapObject::Rectangle &&
							object->shape() != Tiled::MapObject::Polygon) {
						LOG_CERROR("game") << "Invalid shape" << layer->name() << "in" << group->name();
						continue;

					}
					LOG_CDEBUG("game") << "LOAD TOWER OBJECT" << group->name() << gr->name() << gr->className();

					tower = createObject<RpgTower>(TiledObjectBody::ObjectId{.ownerId = 0,
																			 .sceneId = scene->sceneId(),
																			 .id = static_cast<quint32>(object->id())
												   }, scene,
												   this, object, renderer);

					object->setName(group->name());				// Name override for dynamicZ

					loadDynamicZ(scene, object, renderer);

				} else if (object->className() == QStringLiteral("exclude")) {
					LOG_CDEBUG("game") << "LOAD TOWER EXCLUED" << group->name() << layer->name();
					RpgObjectExclude *mapObject = createObject<RpgObjectExclude>(TiledObjectBody::ObjectId{.ownerId = 0,
																										   .sceneId = scene->sceneId(),
																										   .id = static_cast<quint32>(object->id())
																				 }, scene,
																				 object, this, renderer, CP_BODY_TYPE_STATIC);

					if (mapObject)
						mapObject->filterSet(FixtureExcluded, FixtureAll);
				}

			}
		}
	}

	if (!tower) {
		LOG_CERROR("game") << "Load tower error" << group->name();
		return;
	}

	LOG_CINFO("game") << "**************" << tower->scene() << "visual" << visualItem;

	if (visualItem)
		tower->setVisualItem(visualItem);

	tower->addLayers(layers);

	d->towerAdd(tower);
}



/**
 * @brief RpgGameItem::loadMp
 * @param group
 * @param scene
 * @param renderer
 */

void RpgGameItem::loadMp(Tiled::GroupLayer *group, TiledScene *scene, Tiled::MapRenderer *renderer)
{
	LOG_CDEBUG("game") << "LOAD MP" << group->name();

	for (Tiled::Layer *layer : std::as_const(*group)) {
		if (Tiled::TileLayer *tl = layer->asTileLayer()) {
			LOG_CDEBUG("game") << "LOAD MP TILE" << group->name() << layer->name();
			scene->addTileLayer(tl, renderer);
		} else if (Tiled::ObjectGroup *gr = layer->asObjectGroup()) {
			LOG_CDEBUG("game") << "LOAD MP OBJECT" << group->name() << gr->name() << gr->className();
			for (Tiled::MapObject *object : std::as_const(gr->objects())) {
				if (object->className() == QStringLiteral("exclude")) {
					LOG_CDEBUG("game") << "LOAD MP EXCLUED" << group->name() << layer->name();
					RpgObjectExclude *mapObject = createObject<RpgObjectExclude>(TiledObjectBody::ObjectId{.ownerId = 0,
																										   .sceneId = scene->sceneId(),
																										   .id = static_cast<quint32>(object->id())
																				 }, scene,
																				 object, this, renderer, CP_BODY_TYPE_STATIC);

					if (mapObject)
						mapObject->filterSet(FixtureExcluded, FixtureAll);
				} else {
					const QPointF pos = renderer->pixelToScreenCoords(object->position() + gr->totalOffset());
					LOG_CWARNING("game") << "LOAD MP POINT" << pos;
					d->mpEmitterAdd(pos, Rpg::RpgLogic::packId(scene->sceneId(), 0, object->id()));
				}

			}
		}

	}
}

/**
 * @brief RpgGameItem::loadGroupLayer
 * @param scene
 * @param group
 * @param renderer
 */

void RpgGameItem::loadGroupLayer(TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer)
{
	const QString &cname = group->className();

	if (cname == QStringLiteral("mp")) {
		loadMp(group, scene, renderer);
	} else if (cname == QStringLiteral("tower")) {
		loadTower(scene, group, renderer);
	}

	/*if (cname == QStringLiteral("container")) {
			controlAdd<RpgControlContainer>(this, scene, group, renderer);
		} else if (cname == QStringLiteral("container2") && q->m_loadForPlayerCount > 1) {
			controlAdd<RpgControlContainer>(this, scene, group, renderer);
		} else if (cname == QStringLiteral("container3") && q->m_loadForPlayerCount > 2) {
			controlAdd<RpgControlContainer>(this, scene, group, renderer);
		} else if (cname == QStringLiteral("container4") && q->m_loadForPlayerCount > 3) {
			controlAdd<RpgControlContainer>(this, scene, group, renderer);
		} else if (cname == QStringLiteral("container5") && q->m_loadForPlayerCount > 4) {
			controlAdd<RpgControlContainer>(this, scene, group, renderer);
		} else if (cname == QStringLiteral("gate")) {
			controlAdd<RpgControlGate>(this, scene, group, renderer);
		} else if (cname == QStringLiteral("teleport")) {
			controlAdd<RpgControlTeleport>(this, scene, group, false, renderer);
		} else if (cname == QStringLiteral("hideout")) {
			controlAdd<RpgControlTeleport>(this, scene, group, true, renderer);
		} else if (cname == QStringLiteral("randomizer")) {
			if (RpgControlRandomizer *r = RpgControlRandomizer::find(m_controls, group, scene->sceneId()))
				r->addGroupLayer(scene, group, renderer);
			else
				controlAdd<RpgControlRandomizer>(this, scene, group, renderer);
		} else if (cname == QStringLiteral("collection")) {
			addCollection(scene, group, renderer);
		}*/
}


/**
 * @brief RpgGameItem::loadImageLayer
 * @param scene
 * @param image
 * @param renderer
 */

void RpgGameItem::loadImageLayer(TiledScene *scene, Tiled::ImageLayer *image, Tiled::MapRenderer *renderer)
{
	Q_UNUSED(scene);
	Q_UNUSED(image);
	Q_UNUSED(renderer);
}



/**
 * @brief RpgGameItem::timeStepPrepareEvent
 */

void RpgGameItem::timeStepPrepareEvent()
{

}






/**
 * @brief RpgGameItem::timeBeforeWorldStepEvent
 * @param tick
 */

void RpgGameItem::timeBeforeWorldStepEvent(const qint64 &tick)
{
	d->onBeforeWorldStep(tick);


	Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();
	RpgLogicObjectMapper *mapper = scope.getCtx<RpgLogicObjectMapper>();

	if (!mapper) {
		LOG_CERROR("game") << "Missing RpgLogicObjectMapper";
		return;
	}

	for (auto it=mapper->map.cbegin(); it != mapper->map.cend(); ++it) {
		if (!it.value())
			continue;

		AbstractRpgMotor *motor = it.value()->currentMotor();

		if (!motor) {
			LOG_CERROR("game") << "Missing RpgMotor";
			continue;
		}

		entt::entity ent = scope.entityFromIdTag(it.key());

		motor->beforeWorldStep(tick, ent);
	}
}





/**
 * @brief RpgGameItem::timeAfterWorldStepEvent
 * @param tick
 */

void RpgGameItem::timeAfterWorldStepEvent(const qint64 &tick)
{
	RpgStream::FullState full;

	full.setIsDeltaMode(false);

	{
		Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();
		RpgLogicObjectMapper *mapper = scope.getCtx<RpgLogicObjectMapper>();

		if (!mapper) {
			LOG_CERROR("game") << "Missing RpgLogicObjectMapper";
			return;
		}

		for (auto it=mapper->map.cbegin(); it != mapper->map.cend(); ++it) {
			if (!it.value())
				continue;

			AbstractRpgMotor *motor = it.value()->currentMotor();

			if (!motor) {
				LOG_CERROR("game") << "Missing RpgMotor";
				continue;
			}

			motor->afterWorldStep(tick, &full);
		}
	}


	d->onAfterWorldStep(full);
}




/**
 * @brief RpgGameItem::timeSteppedEvent
 */

void RpgGameItem::timeSteppedEvent()
{
	d->onTimeStepped();
	/*	static const qint64 delta = 3;

			const qint64 tick = q->m_timeSync.get();
			const qint64 curr = m_rpgGame->tickTimer()->currentTick();
			const qint64 diff = tick-curr;

			if (diff > 2*delta || diff < -3*delta) {
					LOG_CERROR("game") << "Time reset" << curr << "->" << tick;
					m_rpgGame->tickTimer()->start(this, tick);
			} else if (diff > delta) {
					LOG_CDEBUG("game") << "Time skew +1 frame" << curr << "->" << tick;
					m_rpgGame->tickTimer()->start(this, curr+1);
			} else if (diff < -2*delta) {
					LOG_CWARNING("game") << "Time skew -1 frame" << curr << "->" << tick;
					m_rpgGame->tickTimer()->start(this, curr-1);
			}


			m_rpgGame->iterateOverBodies([this](TiledObjectBody *b){
					if (RpgGameData::LifeCycle *iface = dynamic_cast<RpgGameData::LifeCycle*> (b)) {
							if (iface->stage() == RpgGameData::LifeCycle::StageDestroy) {
									onLifeCycleDelete(b);
							}
					}
			});

			emit msecLeftChanged();
	TiledGame::timeSteppedEvent();

	if (ActionRpgGame *a = actionRpgGame())
		a->onTimeStepped();

	updateScatterEnemies();
	updateScatterPlayers();
	updateScatterPoints();

	for (const auto &ptr : m_sfxLocations) {
		if (ptr->baseObject()->scene() != ptr->connectedScene())
			ptr->setConnectedScene(ptr->baseObject()->scene());
		ptr->checkPosition();
	}
*/
}


/**
 * @brief RpgGameItem::keyPressEvent
 * @param event
 */

void RpgGameItem::keyPressEvent(QKeyEvent *event)
{
	if (m_paused)
		return;

	const int &key = event->key();

	RpgPlayer *player = m_game ? m_game->controlledPlayer() : nullptr;
	RpgMotorPlayerControlled *motor = player ? dynamic_cast<RpgMotorPlayerControlled*>(player->currentMotor()) : nullptr;

	if (!player || !motor)
		LOG_CERROR("game") << "Missing player or motor";

	switch (key) {
		/*		case Qt::Key_X:
		case Qt::Key_Clear:
		case Qt::Key_5:
			if (m_controlledPlayer)
				m_controlledPlayer->exitHiding();
			break;

		case Qt::Key_Space:
		case Qt::Key_Insert:
		case Qt::Key_0:
			if (m_controlledPlayer)
				m_controlledPlayer->attackCurrentWeapon();
			break;

		case Qt::Key_Q:
		case Qt::Key_Delete:
		case Qt::Key_Comma:
			if (m_controlledPlayer)
				m_controlledPlayer->armory()->changeToNextWeapon();
			break;



		case Qt::Key_C:
			if (m_controlledPlayer)
				m_controlledPlayer->cast();
			break;

		case Qt::Key_Tab:
			emit minimapToggleRequest();
			break;

		case Qt::Key_F10:
			emit questsRequest();
			break;*/

		case Qt::Key_Tab:
			d->changeControlledPlayer();
			break;

		case Qt::Key_Return:
		case Qt::Key_Enter:
		case Qt::Key_E:
			if (motor)
				motor->useCurrentControl();
			break;

		case Qt::Key_Space:
		case Qt::Key_Insert:
		case Qt::Key_0:
			if (motor)
				motor->eventTest();
			break;

		default:
			TiledGame::keyPressEvent(event);
	}
}


/**
 * @brief RpgGameItem::keyReleaseEvent
 * @param event
 */

void RpgGameItem::keyReleaseEvent(QKeyEvent *event)
{
	TiledGame::keyReleaseEvent(event);
}


/**
 * @brief RpgGameItem::joystickStateEvent
 * @param joystick
 * @param state
 */

void RpgGameItem::joystickStateEvent(const Joystick &joystick, const JoystickState &state)
{
	if (RpgPlayer *p = m_game->controlledPlayer()) {
		if (RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(p->currentMotor())) {
			if (joystick == JoystickA)
				motor->setCurrentJoystickState(state);
		}
	}
	TiledGame::joystickStateEvent(joystick, state);
}









/**
 * @brief RpgGameItem::loadTextureSprites
 * @param handler
 * @param path
 * @return
 */

QRect RpgGameItem::loadTextureSprites(TiledSpriteHandler *handler, const QString &path)
{
	// 								  QHash<QString, RpgArmory::LayerData> *layerPtr

	static const QVector<TiledObject::Direction> directions = {
		TiledObject::SouthWest,
		TiledObject::South,
		TiledObject::SouthEast,
		TiledObject::East,
		TiledObject::NorthEast,
		TiledObject::North,
		TiledObject::NorthWest,
		TiledObject::West,
	};


	QByteArray input = Utils::fileContentRead(path+QStringLiteral("input.txt"));

	if (input.isEmpty())
		return QRect();

	QTextStream buffer(&input, QIODevice::ReadOnly);
	int n = 0;

	QRect measure;

	/// input.txt format
	///
	/// line 1: <width> \t <height> \t [<bodyOffsetX>] \t [<bodyOffsetY>]
	/// line ...: <sprite> \t <frames> \t <duration> \t [<loops>] \ [<baked>]

	QVector<RpgGameItem::TextureSpriteMapper> mapper;

	QString line;

	while (buffer.readLineInto(&line)) {
		const QStringList field = line.split('\t');

		if (n == 0) {
			if (field.size() > 3)
				measure.setY(field.at(3).toInt());

			if (field.size() > 2)
				measure.setX(field.at(2).toInt());

			if (field.size() > 1)
				measure.setHeight(field.at(1).toInt());

			if (field.size() > 0)
				measure.setWidth(field.at(0).toInt());
		} else {
			if (field.size() < 3) {
				LOG_CERROR("game") << "Invalid line" << line;
				continue;
			}

			const QString sprite = field.at(0);
			const int frames = field.at(1).toInt();
			const int duration = field.at(2).toInt();

			const int loops = field.size() > 3 ? field.at(3).toInt() : 0;
			const bool baked = field.size() > 4 ? field.at(4).toInt() : false;

			for (const auto &d : directions) {
				TextureSpriteMapper dst;
				dst.name = sprite;
				dst.direction = d;
				dst.width = measure.width();
				dst.height = measure.height();
				dst.duration = duration;
				dst.loops = loops;
				dst.baked = baked;

				for (int i=0; i<frames; ++i)
					mapper.append(dst);
			}
		}

		++n;
	}

	/*

	QHash<QString, RpgArmory::LayerData> RpgGamePrivate::readLayerData(const QString &file)
	{
		QHash<QString, RpgArmory::LayerData> hash;

		hash.insert(QStringLiteral("default"), RpgArmory::LayerData(RpgGameData::Weapon::WeaponInvalid, 0, RpgArmory::ShieldNeutral));

		/// layer.txt format
		///
		/// <sprite-prefix> \t [<weapon-str>] \t [<shield-layer-str>] \t [<baked>]

		QByteArray layerData = Utils::fileContentRead(file);
		QTextStream layerBuffer(&layerData, QIODevice::ReadOnly);

		QString line;

		while (layerBuffer.readLineInto(&line)) {
			const QStringList field = line.split('\t');

			if (field.isEmpty())
				continue;


			RpgArmory::LayerData data;

			if (field.size() > 4)
				data.baked = field.at(4).toInt();

			if (field.size() > 3)
				data.shield = QVariant::fromValue(field.at(3)).value<RpgArmory::ShieldLayer>();

			if (field.size() > 2)
				data.subType = field.at(2).toInt();

			if (field.size() > 1)
				data.weapon = RpgArmory::weaponHash().key(field.at(1), RpgGameData::Weapon::WeaponInvalid);

			hash.insert(field.at(0), data);
		}

		return hash;
	}



	QHash<QString, RpgArmory::LayerData> layerData = RpgGamePrivate::readLayerData(path+QStringLiteral("layers.txt"));
*/
	//for (const auto &[layer, data] : layerData.asKeyValueRange()) {


	static const QString layer = "default";

	QString basePath = path;
	if (layer == QStringLiteral("default"))
		basePath += QStringLiteral("texture");
	else
		basePath += layer + QStringLiteral("-texture");

	/*if (data.baked)
			LOG_CDEBUG("scene") << "Load texture from" << qPrintable(basePath) << "to baked layer" << qPrintable(layer);
		else*/
	LOG_CDEBUG("scene") << "Load texture from" << qPrintable(basePath) << "to layer" << qPrintable(layer);

	const auto &ptr = Utils::fileToJsonObject(basePath+QStringLiteral(".json"));

	if (!ptr) {
		LOG_CERROR("scene") << "Missing" << qPrintable(basePath) << "JSON";
		//continue;
		return QRect();
	}

	TextureSpriteDef def;
	def.fromJson(*ptr);

	QVector<RpgGameItem::TextureSpriteMapper> filteredMapper;

	filteredMapper.reserve(mapper.size());

	const QString bakedName = layer+QStringLiteral("-");

	for (const RpgGameItem::TextureSpriteMapper &m : mapper) {
		/*if (data.baked) {
				if (m.baked && m.name.startsWith(bakedName)) {
					m.name.remove(0, bakedName.size());
					filteredMapper.append(m);
				}
			} else {
				if (!m.baked)*/
		filteredMapper.append(m);
		//}
	}

	const QVector<TiledGame::TextureSpriteDirection> &sprites = spritesFromMapper(filteredMapper, def);

	if (!appendToSpriteHandler(handler, sprites, basePath+QStringLiteral(".png"), layer))
		return QRect();
	/*}

	if (layerPtr)
		layerPtr->swap(layerData);
		 */

	return measure;
}







/**
 * @brief RpgGameItem::isContentReady
 * @return
 */

bool RpgGameItem::isContentReady() const
{
	return m_isContentReady;
}

void RpgGameItem::setIsContentReady(bool newIsContentReady)
{
	if (m_isContentReady == newIsContentReady)
		return;
	m_isContentReady = newIsContentReady;
	emit isContentReadyChanged();
}











