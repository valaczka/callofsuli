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
#include "rpggame.h"
#include "rpggame_p.h"
#include "rpgobject.h"
#include "utils_.h"



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
 * @brief RpgGameItem::sceneDebugDrawEvent
 * @param debugDraw
 * @param scene
 */
void RpgGameItem::sceneDebugDrawEvent(TiledDebugDraw *debugDraw, TiledScene *scene)
{
	TiledGame::sceneDebugDrawEvent(debugDraw, scene);

	/*if (!debugDraw || !scene)
		return;

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
	Q_ASSERT(scene);
	Q_ASSERT(group);
	Q_ASSERT(d);

	for (Tiled::MapObject *object : std::as_const(group->objects())) {
		if (object->className().startsWith(QStringLiteral("player"))) {
			LOG_CINFO("game") << "REGISTER" << object->className();

			const QPointF pos = renderer->pixelToScreenCoords(object->position() + group->totalOffset());

			d->m_logic.playerPositionAdd(pos, Rpg::TeamTag::TeamNone);
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
 * @brief RpgGameItem::loadGroupLayer
 * @param scene
 * @param group
 * @param renderer
 */

void RpgGameItem::loadGroupLayer(TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer)
{
	Q_UNUSED(scene);
	Q_UNUSED(group);
	Q_UNUSED(renderer);
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
