/*
 * ---- Call of Suli ----
 *
 * rpgcontrolgroupoverlay.cpp
 *
 * Created on: 2024. 03. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlGroupOverlay
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

#include "rpgcontrolgroupoverlay.h"
#include <libtiled/grouplayer.h>
#include <libtiled/objectgroup.h>
#include "rpggame.h"


RpgControlGroupOverlay::RpgControlGroupOverlay(RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer)
	: RpgControlGroup(ControlGroupOverlay, game, scene)
{
	Q_ASSERT(game);
	Q_ASSERT(scene);
	Q_ASSERT(group);
	Q_ASSERT(renderer);

	QObject::connect(game, &RpgGame::controlledPlayerChanged, game, [this](){
		this->onControlledPlayerChanged();
	});


	for (Tiled::Layer *layer : std::as_const(*group)) {
		if (Tiled::TileLayer *tl = layer->asTileLayer()) {
			TiledQuick::TileLayerItem *item = scene->addTileLayer(tl, renderer);
			item->setVisible(false);
			m_tileLayers.append(QPointer(item));

			LOG_CTRACE("game") << "Add tile layer" << tl->name() << "to control group:" << this;

		} else if (Tiled::ObjectGroup *group = layer->asObjectGroup()) {
			for (Tiled::MapObject *object : std::as_const(group->objects())) {
				TiledObject *base = nullptr;

				if (object->className() != QStringLiteral("trigger")) {
					LOG_CWARNING("game") << "RpgControlGroupOverlay object skipped:" << object->id() << object->name();
					continue;
				}

				if (object->shape() == Tiled::MapObject::Polygon ||
					object->shape() == Tiled::MapObject::Rectangle) {
					TiledObject *ptr = m_game->createFromMapObject<TiledObject>(scene, object, renderer);
					//connectFixture(ptr->fixture());
					base = ptr;
				} else if (object->shape() == Tiled::MapObject::Point) {
					TiledObject *ptr = m_game->createFromCircle<TiledObject>(scene, object->position(), 10., renderer);
					//ptr->body()->emplace(renderer->pixelToScreenCoords(object->position()));
					//connectFixture(ptr->fixture());
					base = ptr;
				}

				if (!base) {
					LOG_CERROR("game") << "Invalid object" << object->id() << object->name() << "in" << group->id() << group->name();
					return;
				}

				base->setParent(m_game);
			}

		}
	}
}


/**
 * @brief RpgControlGroupOverlay::removePlayerFixture
 * @param player
 */

void RpgControlGroupOverlay::removePlayerFixture(RpgPlayer *player)
{
	if (!player)
		return;

	/*if (auto *p = player->sensorPolygon()) {
		if (auto *c = p->virtualCircle()) {
			onFixtureEndContact(c);
		}
	}*/
}



/**
 * @brief RpgControlGroupOverlay::onFixtureBeginContact
 * @param other
 */

/*
void RpgControlGroupOverlay::onFixtureBeginContact(Box2DFixture *other)
{
	if (!m_contactedFixtures.contains(other)) {
		m_contactedFixtures.append(QPointer(other));
		updateLayers();
	}
}


void RpgControlGroupOverlay::onFixtureEndContact(Box2DFixture *other)
{
	m_contactedFixtures.removeAll(QPointer(other));
	updateLayers();
}
*/

/**
 * @brief RpgControlGroupOverlay::onControlledPlayerChanged
 */

void RpgControlGroupOverlay::onControlledPlayerChanged()
{
	//m_contactedFixtures.clear();
	updateLayers();
}


/**
 * @brief RpgControlGroupOverlay::connectFixture
 * @param fixture
 */

/*
void RpgControlGroupOverlay::connectFixture(Box2DFixture *fixture)
{
	fixture->setSensor(true);
	fixture->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTrigger));
	fixture->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureVirtualCircle));
	QObject::connect(fixture, &Box2DFixture::beginContact, m_game, [this](Box2DFixture *other){
		this->onFixtureBeginContact(other);
	});
	QObject::connect(fixture, &Box2DFixture::endContact, m_game, [this](Box2DFixture *other){
		this->onFixtureEndContact(other);
	});
}

*/

/**
 * @brief RpgControlGroupOverlay::updateLayers
 */

void RpgControlGroupOverlay::updateLayers()
{
	/*const bool visible = !m_contactedFixtures.isEmpty();

	for (TiledQuick::TileLayerItem *item : std::as_const(m_tileLayers))
		item->setVisible(visible);*/
}
