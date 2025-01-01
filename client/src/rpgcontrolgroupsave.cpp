/*
 * ---- Call of Suli ----
 *
 * rpgcontrolgroupsave.cpp
 *
 * Created on: 2024. 05. 08.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlGroupSave
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

#include "rpgcontrolgroupsave.h"
#include "rpggame.h"
#include <libtiled/grouplayer.h>
#include <libtiled/objectgroup.h>




/**
 * @brief RpgControlGroupSave::RpgControlGroupSave
 * @param game
 * @param scene
 * @param group
 * @param renderer
 */

RpgControlGroupSave::RpgControlGroupSave(RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer)
	: RpgControlGroup(ControlGroupSave, game, scene)
{
	Q_ASSERT(game);
	Q_ASSERT(scene);
	Q_ASSERT(group);
	Q_ASSERT(renderer);

	m_timer.setInterval(5000);
	m_timer.setSingleShot(true);

	QObject::connect(&m_timer, &QTimer::timeout, m_game, [this](){
		show();
	});


	if (group->hasProperty(QStringLiteral("count"))) {
		setCount(group->property(QStringLiteral("count")).toInt());
	}


	for (Tiled::Layer *layer : std::as_const(*group)) {
		if (Tiled::TileLayer *tl = layer->asTileLayer()) {
			TiledQuick::TileLayerItem *item = scene->addTileLayer(tl, renderer);
			item->setVisible(true);
			m_tileLayers.append(QPointer(item));

			LOG_CTRACE("game") << "Add tile layer" << tl->name() << "to control group:" << this;

		} else if (Tiled::ObjectGroup *group = layer->asObjectGroup()) {
			for (Tiled::MapObject *object : std::as_const(group->objects())) {
				if (object->className() != QStringLiteral("trigger")) {
					LOG_CWARNING("game") << "RpgControlGroupSave object skipped:" << object->id() << object->name();
					continue;
				}

				if (object->shape() == Tiled::MapObject::Point) {
					b2::Body::Params bParams;
					bParams.type = b2BodyType::b2_staticBody;
					bParams.fixedRotation = true;
					b2::Shape::Params params;
					params.isSensor = true;
					params.enableSensorEvents = true;
					params.filter = TiledObjectBody::getFilter(TiledObjectBody::FixtureTrigger, TiledObjectBody::FixturePlayerBody);

					m_body = m_game->createFromCircle<RpgControlGroupSaveBody>(-1, object->id(), scene, object->position(), 30., renderer, bParams, params);
					m_body->m_control = this;
				}

				if (!m_body) {
					LOG_CERROR("game") << "Invalid object" << object->id() << object->name() << "in" << group->id() << group->name();
					return;
				}
			}

		}
	}
}



/**
 * @brief RpgControlGroupSave::updateLayers
 */

void RpgControlGroupSave::updateLayers()
{
	const bool visible = m_active && !m_timer.isActive();

	for (TiledQuick::TileLayerItem *item : std::as_const(m_tileLayers))
		item->setVisible(visible);
}


/**
 * @brief RpgControlGroupSave::hide
 */

void RpgControlGroupSave::hide(RpgPlayer *player)
{
	if (!m_active || m_count == 0)
		return deactivate();

	if (RpgGame *g = qobject_cast<RpgGame*>(m_game))
		g->saveSceneState(player);

	m_timer.start();

	if (m_count > 0)
		setCount(m_count-1);

	if (m_count == 0)
		deactivate();

	updateLayers();
}


/**
 * @brief RpgControlGroupSave::show
 */

void RpgControlGroupSave::show()
{
	if (m_count == 0)
		deactivate();
	else
		updateLayers();
}


/**
 * @brief RpgControlGroupSave::deactivate
 */

void RpgControlGroupSave::deactivate()
{
	m_active = false;

	if (m_body)
		m_body->setBodyEnabled(false);

	updateLayers();
}


/**
 * @brief RpgControlGroupSave::sensorBegin
 * @param shape
 */

void RpgControlGroupSave::sensorBegin(b2::ShapeRef shape)
{
	if (m_timer.isActive() || !m_active)
		return;

	RpgPlayer *player = dynamic_cast<RpgPlayer*>(TiledObjectBody::fromBodyRef(shape.GetBody()));

	if (!player || !m_game)
		return;

	if (player == m_game->controlledPlayer()) {
		hide(player);
	}
}







/**
 * @brief RpgControlGroupSave::count
 * @return
 */

int RpgControlGroupSave::count() const
{
	return m_count;
}

void RpgControlGroupSave::setCount(int newCount)
{
	m_count = newCount;
}

