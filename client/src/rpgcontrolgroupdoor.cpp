/*
 * ---- Call of Suli ----
 *
 * rpgcontrolgroupdoor.cpp
 *
 * Created on: 2024. 10. 30.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlGroupDoor
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

#include "rpgcontrolgroupdoor.h"
#include <libtiled/objectgroup.h>
#include <libtiled/imagelayer.h>
#include "rpggame.h"


const QHash<QString, RpgControlGroupDoor::OpenState> RpgControlGroupDoor::m_stateHash = {
	{ QStringLiteral("open"), StateOpened },
	{ QStringLiteral("close"), StateClosed },
	{ QStringLiteral("damaged"), StateDamaged },
};


/**
 * @brief RpgControlGroupDoor::RpgControlGroupDoor
 * @param game
 * @param scene
 * @param group
 * @param renderer
 */

RpgControlGroupDoor::RpgControlGroupDoor(RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer)
	: RpgControlGroupState<DoorState>(ControlGroupDoor, game, scene, group)
{
	Q_ASSERT(game);
	Q_ASSERT(scene);
	Q_ASSERT(group);
	Q_ASSERT(renderer);

	createVisualItem(group);

	for (Tiled::Layer *layer : std::as_const(*group)) {
		if (Tiled::ImageLayer *tl = layer->asImageLayer()) {
			const OpenState s = m_stateHash.value(tl->name(), StateInvalid);

			if (s == StateInvalid) {
				LOG_CWARNING("scene") << "Invalid image layer" << tl->name() << "to door:" << this;
				continue;
			}

			if (cstate(s) != m_states.cend()) {
				LOG_CWARNING("scene") << "State already exists" << tl->name() << "to door:" << this;
				continue;
			}

			stateAdd(DoorState{
						 s,
						 tl->imageSource(),
						 tl->position() + tl->offset()
					 });

			LOG_CTRACE("scene") << "Add image layer" << tl->name() << "to door:" << this;

		} else if (Tiled::ObjectGroup *objgroup = layer->asObjectGroup()) {
			/*for (Tiled::MapObject *object : std::as_const(objgroup->objects())) {
				if (object->className() == QStringLiteral("ground")) {
					if (TiledObjectBasePolygon *p = game->loadGround(scene, object, renderer, m_basePosition)) {
						addTiledObject(p);
					}
				} else if (object->className() == QStringLiteral("trigger")) {
					TiledObject *base = nullptr;
					Box2DFixture *fixture = nullptr;

					if (object->shape() == Tiled::MapObject::Polygon ||
							object->shape() == Tiled::MapObject::Rectangle) {
						TiledObjectBasePolygon *ptr = nullptr;
						TiledObject::createFromMapObject<TiledObjectBasePolygon>(&ptr, object, renderer, scene);
						base = ptr;
						fixture = ptr->fixture();
					} else if (object->shape() == Tiled::MapObject::Point) {
						TiledObjectBaseCircle *ptr = nullptr;
						TiledObject::createFromCircle<TiledObjectBaseCircle>(&ptr, object->position(), 10., renderer, scene);
						base = ptr;
						fixture = ptr->fixture();
					}

					if (!base || !fixture) {
						LOG_CERROR("game") << "Invalid object" << object->id() << object->name() << "in" << group->id() << group->name();
						continue;
					}

					base->body()->emplace(base->bodyPosition() + m_basePosition);

					connectFixture(fixture);

					fixture->setCategories(fixture->categories().setFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTransport), true));
					fixture->setCollidesWith(fixture->collidesWith().setFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePlayerBody), true));

					m_transport = game->transportList().add(TiledTransport::TransportDoor, group->name(),
															group->property(QStringLiteral("lock")).toString(),
															-1, scene, base);

					base->setParent(m_game);
					base->setScene(scene);
				}
			}*/
		}
	}

	if (group->hasProperty(QStringLiteral("transparent")))
		setIsOpaque(!group->property(QStringLiteral("transparent")).toBool());
	else if (group->hasProperty(QStringLiteral("opaque")))
		setIsOpaque(group->property(QStringLiteral("opaque")).toBool());

	setOpenState(m_stateHash.value(group->property(QStringLiteral("state")).toString(), StateClosed));
}


/**
 * @brief RpgControlGroupDoor::~RpgControlGroupDoor
 */

RpgControlGroupDoor::~RpgControlGroupDoor()
{

}


/**
 * @brief RpgControlGroupDoor::transport
 * @return
 */

TiledTransport* RpgControlGroupDoor::transport() const
{
	return m_transport;
}


/**
 * @brief RpgControlGroupDoor::doorOpen
 * @return
 */

bool RpgControlGroupDoor::doorOpen()
{
	if (m_openState == StateClosed) {
		setOpenState(StateOpened);
		m_game->reloadTcodMap(m_scene);
		return true;
	}

	return false;
}


/**
 * @brief RpgControlGroupDoor::doorClose
 * @return
 */

bool RpgControlGroupDoor::doorClose()
{
	if (m_openState == StateOpened) {
		setOpenState(StateClosed);
		m_game->reloadTcodMap(m_scene);
		return true;
	}

	return false;
}


/**
 * @brief RpgControlGroupDoor::doorDamage
 * @return
 */

bool RpgControlGroupDoor::doorDamage()
{
	if (m_openState != StateDamaged) {
		setOpenState(StateDamaged);
		m_game->reloadTcodMap(m_scene);
		return true;
	}

	return false;
}


/**
 * @brief RpgControlGroupDoor::openState
 * @return
 */

const RpgControlGroupDoor::OpenState &RpgControlGroupDoor::openState() const
{
	return m_openState;
}

void RpgControlGroupDoor::setOpenState(OpenState newOpenState)
{
	m_openState = newOpenState;
	update();
}


/**
 * @brief RpgControlGroupDoor::isOpaque
 * @return
 */

const bool &RpgControlGroupDoor::isOpaque() const
{
	return m_isOpaque;
}

void RpgControlGroupDoor::setIsOpaque(bool newIsOpaque)
{
	m_isOpaque = newIsOpaque;
	update();
}


/**
 * @brief RpgControlGroupDoor::update
 */

void RpgControlGroupDoor::update()
{
	m_currentState = m_openState;
	refreshVisualItem();

	/*for (TiledObject *o : m_tiledObjects) {
		TiledObjectBasePolygon *obj = qobject_cast<TiledObjectBasePolygon*>(o);

		if (!obj)
			continue;

		obj->body()->setOpaque(m_isOpaque);

		switch (m_openState) {
			case StateClosed:
				obj->body()->setActive(true);
				obj->fixture()->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureGround));
				break;

			case StateOpened:
			case StateDamaged:
				obj->body()->setActive(false);
				obj->fixture()->setCategories(Box2DFixture::None);
				break;

			case StateInvalid:
				break;
		}
	}*/


	if (m_transport)
		m_transport->setIsActive(m_openState == StateOpened || m_openState == StateClosed);
}


