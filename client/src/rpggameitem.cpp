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
#include "client.h"
#include "grouplayer.h"
#include "rpggame.h"
#include "rpggame_p.h"
#include "rpgnpc.h"
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

	if (!mouseNavigation())
		return;

	RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(player->currentMotor());

	if (!motor)
		return;

	if (Qt::MouseButtons::fromInt(buttons).testFlag(Qt::RightButton)) {
		motor->clearDestination();
		return;
	}


#ifndef QT_NO_DEBUG
	if (modifiers & Qt::AltModifier) {
		motor->clearDestination();
		player->emplace(x, y);
		return;
	}
#endif


	/*
	if (!m_controlledPlayer->isAlive())
		return;

	if (mouseAttack()) {
		m_controlledPlayer->attackToPoint(x, y);
		return;
	}*/


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



	if (const auto &ptr = findShortestPath(player, cpv(x,y))) {
		motor->setDestination(ptr.value());
	}
}




/**
 * @brief RpgGameItem::setScatterSeries
 * @param list
 */

void RpgGameItem::setScatterSeries(const QList<QScatterSeries *> &list)
{
	if (!d->m_scatters.empty()) {
		LOG_CERROR("game") << "Scatter list already loaded";
		return;
	}

	d->m_scatters = list;

	if (d->m_scatters.size() < 2) {
		LOG_CERROR("game") << "Scatter list size error";
		return;
	}

	// Players

	d->m_scatters.at(0)->setBorderColor(QColorConstants::Svg::black);


	// Towers

	d->m_scatters.at(1)->setBorderColor(QColorConstants::Svg::black);
	d->m_scatters.at(1)->setColor(QColorConstants::Svg::gray);
	d->m_scatters.at(1)->setMarkerShape(QScatterSeries::MarkerShapeStar);
	d->m_scatters.at(1)->setMarkerSize(18);
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
		if (RpgPlayer *p = dynamic_cast<RpgPlayer*>(body)) {
			if (RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(p->currentMotor())) {

				if (const auto &ptr = motor->destination()) {
					debugDraw->drawPolygon(ptr.value(),
										   p == m_game->controlledPlayer() ? QColorConstants::Svg::lightgreen : QColorConstants::Svg::orange,
										   4.);
				}
			}
		} else if (RpgNpc *p = dynamic_cast<RpgNpc*>(body)) {
			if (RpgMotorNpcControlled *motor = dynamic_cast<RpgMotorNpcControlled*>(p->currentMotor())) {

				if (const auto &ptr = motor->destination()) {
					debugDraw->drawPolygon(ptr.value(),
										   p->targetEntity() ? QColorConstants::Svg::red : QColorConstants::Svg::lightblue,
										   3.);
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
		} else if (group->className() == QStringLiteral("chest") || group->name() == QStringLiteral("chest")) {
			LOG_CINFO("game") << "REGISTER CHEST" << object->className();

			const QPointF pos = renderer->pixelToScreenCoords(object->position() + group->totalOffset());

			d->chestPositionAdd(pos);
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
	QList<RpgDefenderPoint*> defenders;

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
		} else if (Tiled::GroupLayer *gr = layer->asGroupLayer()) {
			if (gr->className() == QStringLiteral("defender")) {
				if (RpgDefenderPoint *d = loadDefender(scene, gr, renderer))
					defenders.append(d);
			} else {
				LOG_CWARNING("game") << "Invalid group layer" << gr->className() << gr->name();
			}
		}
	}

	if (!tower) {
		LOG_CERROR("game") << "Load tower error" << group->name();
		return;
	}

	LOG_CINFO("game") << "**************" << tower->scene() << "visual" << visualItem;

	if (visualItem) {
		tower->setVisualItem(visualItem);
		visualItem->setVisible(false);
	}

	tower->addLayers(layers);
	tower->addDefenderPoints(defenders);

	d->towerAdd(tower);

	if (auto ptr = d->addToScatter(1))
		tower->setScatterPoint(ptr.value());
}





/**
 * @brief RpgGameItem::loadDefender
 * @param scene
 * @param group
 * @param renderer
 */

RpgDefenderPoint *RpgGameItem::loadDefender(TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer)
{
	LOG_CDEBUG("game") << "LOAD DEFENDER" << group->name();

	TiledQuick::TileLayerItem *item = nullptr;
	RpgDefenderPoint *defender = nullptr;

	for (Tiled::Layer *layer : std::as_const(*group)) {
		if (Tiled::TileLayer *tl = layer->asTileLayer()) {
			LOG_CDEBUG("game") << "LOAD DEFENDER TILE" << group->name() << layer->name();
			item = scene->addTileLayer(tl, renderer);
			item->setVisible(false);
		} else if (Tiled::ObjectGroup *gr = layer->asObjectGroup()) {
			LOG_CDEBUG("game") << "LOAD DEFENDER OBJECT" << group->name() << gr->name() << gr->className();
			for (Tiled::MapObject *object : std::as_const(gr->objects())) {
				if (object->className() == QStringLiteral("defender")) {
					defender = createObject<RpgDefenderPoint>(TiledObjectBody::ObjectId{.ownerId = 0,
																						.sceneId = scene->sceneId(),
																						.id = static_cast<quint32>(object->id())
															  }, scene,
															  object->position(), this, renderer, gr->totalOffset());

				} else {
					LOG_CWARNING("game") << "Invalid object" << gr->className() << gr->name();
				}
			}
		}
	}

	if (defender)
		defender->setVisualItem(item);

	return defender;
}



/**
 * @brief RpgGameItem::onStageChanged
 * @param stage
 */

void RpgGameItem::onStageChanged(const RpgStream::GameConfig::Stage &stage)
{
	message(QObject::tr("Next stage: %1").arg(stage));

	if (stage == RpgStream::GameConfig::StageSelect) {
		m_game->m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/prepare_yourself.mp3"), Sound::VoiceoverChannel);
	} else if (stage == RpgStream::GameConfig::StageWarmingUp) {
		m_game->m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/begin.mp3"), Sound::VoiceoverChannel);
		emit stageChanged();					// mark time label
	} else if (stage == RpgStream::GameConfig::StageLast) {
		m_game->m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/final_round.mp3"), Sound::VoiceoverChannel);
		emit stageChanged();					// mark time label
	} else if (stage == RpgStream::GameConfig::StageFinished) {
		m_game->m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"), Sound::VoiceoverChannel);
		emit stageChanged();					// mark time label
	}

}



/**
 * @brief RpgGameItem::loadMp
 * @param group
 * @param scene
 * @param renderer
 */

void RpgGameItem::loadMp(Tiled::GroupLayer *group, TiledScene *scene, Tiled::MapRenderer *renderer)
{
	QQuickItem *visual = nullptr;

	for (Tiled::Layer *layer : std::as_const(*group)) {
		if (Tiled::TileLayer *tl = layer->asTileLayer()) {
			visual = scene->addTileLayer(tl, renderer);
			visual->setVisible(false);
		} else if (Tiled::ObjectGroup *gr = layer->asObjectGroup()) {
			for (Tiled::MapObject *object : std::as_const(gr->objects())) {
				if (object->className() == QStringLiteral("exclude")) {
					RpgObjectExclude *mapObject = createObject<RpgObjectExclude>(TiledObjectBody::ObjectId{.ownerId = 0,
																										   .sceneId = scene->sceneId(),
																										   .id = static_cast<quint32>(object->id())
																				 }, scene,
																				 object, this, renderer, CP_BODY_TYPE_STATIC);

					if (mapObject)
						mapObject->filterSet(FixtureExcluded, FixtureAll);
				} else {
					const QPointF pos = renderer->pixelToScreenCoords(object->position()) + gr->totalOffset();
					d->mpEmitterAdd(pos, Rpg::RpgLogic::packId(scene->sceneId(), 0, object->id()), visual);
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
 * @brief RpgGameItem::prepareEvent
 */

void RpgGameItem::prepareEvent()
{

	//d->onBeforeWorldStep(-1);		// Csak objektum szinkronizáció

	timeBeforeWorldStepEvent(-1);

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

	// A serverTick-be tesszük ideiglenesen azt a ticket, aminél kisebbet nem küldünk (mert nem is fogadja el a szerver)
	// = számított aktuális server tick (=lastAuthTick on the server) - lastauthdiff
	//
	// Ez valójában csak belső használatra kell, a szerver nem fogja nézni

	full.setServerTick(m_game->rpgLogicClient()->estimatedServerTick()
					   - m_game->rpgLogicClient()->lastAuthDiff());

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

void RpgGameItem::timeSteppedEvent(const std::vector<TiledObjectBody *> &aboutDestruction)
{
	TiledGame::timeSteppedEvent(aboutDestruction);


	d->onTimeStepped(aboutDestruction);



	/*
	const quint32 serverTick = d->m_logic->estimatedServerTick(d->m_logic->lastAuthTick());			// ???



	if (serverTick == 0)
		return;



	const qint64 tick = d->m_logic->lastAuthTick();
	const qint64 curr = m_tickTimer->currentTick();
	const qint64 diff = tick-curr;


	if (diff < 0) {
		LOG_CERROR("game") << "Time reset to server time" << curr << "->" << tick;
		m_tickTimer->start(m_game, tick);
		overrideCurrentFrame(tick);
	}
*/
	/*
	static const qint64 delta = 3;

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

	/*
	WASD + Shift: Sprint or run faster.
	WASD + Spacebar: Jump.
	WASD + Ctrl: Crouch or go prone.
	WASD + E: Interact with objects or perform actions.
	WASD + Q: Switch weapons or equipment.
	WASD + R: Reload your weapon.
	WASD + F: Activate a flashlight or use a special ability.
	WASD + Numbers (1-9): Switch between different inventory items or abilities.
	*/

	switch (key) {
		/*		case Qt::Key_X:
		case Qt::Key_Clear:
		case Qt::Key_5:
			if (m_controlledPlayer)
				m_controlledPlayer->exitHiding();
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


		case Qt::Key_F10:
			emit questsRequest();
			break;*/

		case Qt::Key_Space:
		case Qt::Key_Insert:
		case Qt::Key_0:
			if (motor)
				motor->attackCurrentTarget();
			break;

		case Qt::Key_Tab:
			emit minimapToggleRequest();
			break;


		case Qt::Key_F10:
			//d->changeControlledPlayer();
			break;

		case Qt::Key_F11:
			if (motor)
				motor->changeMpToDefender();
			break;

		case Qt::Key_F9:
			if (motor)
				motor->changeMpToBullet();
			break;


		case Qt::Key_Return:
		case Qt::Key_Enter:
		case Qt::Key_E:
			if (motor)
				motor->useCurrentControl();
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
		d->setJoystickState(p, joystick, state);
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
 * @brief RpgGameItem::loadTextureSprites
 * @param handler
 * @param mapper
 * @param path
 * @return
 */

bool RpgGameItem::loadTextureSprites(TiledSpriteHandler *handler, const QVector<TextureSpriteMapper> &mapper,
									 const QString &path)
{
	Q_ASSERT(handler);

	LOG_CTRACE("game") << "Load base texture sprites" << path;

	const auto &ptr = Utils::fileToJsonObject(
						  path.endsWith('/') ?
							  path+QStringLiteral("/texture.json") :
							  path+QStringLiteral(".json"));

	if (!ptr)
		return false;

	TextureSpriteDef def;
	def.fromJson(*ptr);

	auto sprites = spritesFromMapper(mapper, def);


	// Add hurt virtual sprites

	if (const QStringList list = spriteNamesFromMapper(mapper);
			list.contains(QStringLiteral("death")) && !list.contains(QStringLiteral("hurt"))) {

		const QVector<TiledObject::Direction> directions = directionsFromMapper(mapper, QStringLiteral("death"));

		for (const auto &d : directions) {
			TextureSpriteDirection data;
			data.sprite = spriteFromMapper(mapper, def, QStringLiteral("death"), d, 4);
			data.sprite.name = QStringLiteral("hurt");
			data.direction = d;
			sprites.append(data);
		}
	}

	static const QString layer = "default";

	return appendToSpriteHandler(handler, sprites,
								 path.endsWith('/') ?
									 path+QStringLiteral("/texture.png") :
									 path+QStringLiteral(".png"),
								 layer);
}



/**
 * @brief RpgGameItem::baseSpriteMapper
 * @return
 */

const QVector<TiledGame::TextureSpriteMapper> &RpgGameItem::baseSpriteMapper()
{
	static std::unique_ptr<QVector<TiledGame::TextureSpriteMapper>> mapper;

	if (mapper)
		return *(mapper.get());

	mapper.reset(new QVector<TiledGame::TextureSpriteMapper>);

	struct BaseMapper {
		QString name;
		int count = 0;
		int duration = 0;
		int loops = 0;
	};

	static const QVector<TiledObject::Direction> directions = {
		TiledObject::SouthWest, TiledObject::West, TiledObject::NorthWest, TiledObject::North, TiledObject::NorthEast,
		TiledObject::East, TiledObject::SouthEast, TiledObject::South
	};


	static const QVector<BaseMapper> baseMapper = {
		{ QStringLiteral("idle"), 4, 250, 0 },
		{ QStringLiteral("attack"), 10, 40, 1 },
		{ QStringLiteral("bow"), 9, 40, 1 },
		{ QStringLiteral("cast"), 9, 60, 1 },
		{ QStringLiteral("walk"), 11, 60, 0 },
		{ QStringLiteral("run"), 10, 60, 0 },
		{ QStringLiteral("death"), 8, 60, 1 },
	};


	for (const auto &d : directions) {
		for (const auto &m : baseMapper) {
			TiledGame::TextureSpriteMapper dst;
			dst.name = m.name;
			dst.direction = d;
			dst.width = 148;
			dst.height = 130;
			dst.duration = m.duration;
			dst.loops = m.loops;

			for (int i=0; i<m.count; ++i)
				mapper->append(dst);
		}
	}

	return *(mapper.get());
}




/**
 * @brief RpgGameItem::loadGround
 * @param scene
 * @param object
 * @param renderer
 * @return
 */

TiledObjectBody *RpgGameItem::loadGround(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer)
{
	TiledObjectBody *p = TiledGame::loadGround(scene, object, renderer);

	if (object->hasProperty(QStringLiteral("sound")) && p) {
		d->addLocationSound(p, object->propertyAsString(QStringLiteral("sound")));
	}

	return p;
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









