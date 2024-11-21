/*
 * ---- Call of Suli ----
 *
 * X_RpgControlGroupContainer.cpp
 *
 * Created on: 2024. 04. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * X_RpgControlGroupContainer
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

#include "rpgcontrolgroupcontainer.h"
#include <libtiled/objectgroup.h>
#include "rpggame.h"



/**
 * @brief RpgControlGroupContainer::RpgControlGroupContainer
 * @param game
 * @param scene
 * @param group
 * @param renderer
 */

RpgControlGroupContainer::RpgControlGroupContainer(RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer)
	: RpgControlGroupState<ContainerState>(ControlGroupContainer, game, scene, group)
	, m_container(new TiledContainer)
{
	Q_ASSERT(game);
	Q_ASSERT(scene);
	Q_ASSERT(group);
	Q_ASSERT(renderer);

	createVisualItem(group);


	m_container->setType(TiledContainer::ContainerBase);
	m_container->setIsActive(true);
	m_container->setScene(scene);

	QObject::connect(m_container.get(), &TiledContainer::isActiveChanged, game, [this](){
		update();
	});


	if (group->hasProperty(QStringLiteral("pickable"))) {
		QVector<RpgPickableObject::PickableType> pickableList;

		const QStringList &pList = group->property(QStringLiteral("pickable")).toString().split(',', Qt::SkipEmptyParts);
		for (const QString &s : pList) {
			const RpgPickableObject::PickableType &type = RpgPickableObject::typeFromString(s.simplified());

			if (type == RpgPickableObject::PickableInvalid) {
				LOG_CWARNING("scene") << "Invalid pickable type:" << s << group->id() << group->className() << group->name();
				continue;
			}

			pickableList.append(type);
		}

		setPickableList(pickableList);
	}

	if (group->hasProperty(QStringLiteral("pickableName")))
		setNameList(group->property(QStringLiteral("pickableName")).toString().split(',', Qt::SkipEmptyParts));


	for (Tiled::Layer *layer : std::as_const(*group)) {
		if (Tiled::ImageLayer *tl = layer->asImageLayer()) {
			int s = 0;

			if (tl->name() == QStringLiteral("close"))
				s = 1;
			else if (tl->name() == QStringLiteral("open"))
				s = 2;

			if (s == 0) {
				LOG_CWARNING("scene") << "Invalid image layer" << tl->name() << "to container:" << this;
				continue;
			}

			if (cstate(s) != m_states.cend()) {
				LOG_CWARNING("scene") << "State already exists" << tl->name() << "to container:" << this;
				continue;
			}

			stateAdd(ContainerState{
						 s,
						 tl->imageSource(),
						 tl->position() + tl->offset()
					 });

			LOG_CTRACE("scene") << "Add image layer" << tl->name() << "to container:" << this;

		} else if (Tiled::ObjectGroup *objgroup = layer->asObjectGroup()) {
			for (Tiled::MapObject *object : std::as_const(objgroup->objects())) {
				if (object->className() == QStringLiteral("trigger")) {
					TiledObjectBase *base = nullptr;
					Box2DFixture *fixture = nullptr;

					if (object->shape() == Tiled::MapObject::Point) {
						TiledObjectBaseCircle *ptr = nullptr;
						TiledObject::createFromCircle<TiledObjectBaseCircle>(&ptr, object->position(), 35., renderer, scene);
						base = ptr;
						fixture = ptr->fixture();

						setCenterPoint(renderer ?
										   renderer->pixelToScreenCoords(object->position()) + m_basePosition :
										   object->position() + m_basePosition
										   );
					}

					if (!base || !fixture) {
						LOG_CERROR("game") << "Invalid object" << object->id() << object->name() << "in" << group->id() << group->name();
						continue;
					}

					base->body()->emplace(base->body()->bodyPosition() + m_basePosition);

					connectFixture(fixture);

					fixture->setCategories(fixture->categories().setFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureContainer), true));
					fixture->setCollidesWith(fixture->collidesWith().setFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePlayerBody), true));

					base->setParent(m_game);
					base->setScene(scene);
					base->setProperty("tiledContainer", QVariant::fromValue(m_container.get()));

					addTiledObject(base);
				}
			}
		}
	}

	update();
}



/**
 * @brief RpgControlGroupContainer::pickableList
 * @return
 */

QVector<RpgPickableObject::PickableType> RpgControlGroupContainer::pickableList() const
{
	return m_pickableList;
}

void RpgControlGroupContainer::setPickableList(const QVector<RpgPickableObject::PickableType> &newPickableList)
{
	m_pickableList = newPickableList;
}

QStringList RpgControlGroupContainer::nameList() const
{
	return m_nameList;
}

void RpgControlGroupContainer::setNameList(const QStringList &newNameList)
{
	m_nameList = newNameList;
}



/**
 * @brief RpgControlGroupContainer::update
 */

void RpgControlGroupContainer::update()
{
	m_currentState = m_container->isActive() ? 1 : 2;
	refreshVisualItem();

	for (TiledObjectBase *o : m_tiledObjects) {
		o->body()->setActive(m_container->isActive());
	}
}


/**
 * @brief RpgControlGroupContainer::centerPoint
 * @return
 */

QPointF RpgControlGroupContainer::centerPoint() const
{
	return m_centerPoint;
}

void RpgControlGroupContainer::setCenterPoint(QPointF newCenterPoint)
{
	m_centerPoint = newCenterPoint;
}
