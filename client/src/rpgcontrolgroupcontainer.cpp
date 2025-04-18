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
{
	Q_ASSERT(game);
	Q_ASSERT(scene);
	Q_ASSERT(group);
	Q_ASSERT(renderer);

	createVisualItem(group);

	if (group->hasProperty(QStringLiteral("pickable"))) {
		QVector<RpgGameData::PickableBaseData::PickableType> pickableList;

		const QStringList &pList = group->property(QStringLiteral("pickable")).toString().split(',', Qt::SkipEmptyParts);
		for (const QString &s : pList) {
			const RpgGameData::PickableBaseData::PickableType &type = RpgPickableObject::typeFromString(s.simplified());

			if (type == RpgGameData::PickableBaseData::PickableInvalid) {
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
				if (object->className() == QStringLiteral("trigger") || object->name() == QStringLiteral("trigger") ) {
					if (m_container) {
						LOG_CERROR("scene") << "RpgContainer object already exists" << object->id() << object->name() << "in" << group->id() << group->name();
						continue;
					}

					if (object->shape() == Tiled::MapObject::Point) {
						/*m_container = m_game->createFromCircle<RpgContainer>(-1, object->id(), scene,
																			 object->position(),
																			 40., renderer,
																			 getShapeParams(TiledObjectBody::FixtureContainer));

						setCenterPoint(renderer ?
										   renderer->pixelToScreenCoords(object->position()) + m_basePosition :
										   object->position() + m_basePosition
										   );*/
					}

					if (!m_container) {
						LOG_CERROR("scene") << "Invalid object" << object->id() << object->name() << "in" << group->id() << group->name();
						continue;
					}

					m_container->setIsActive(true);
					m_container->emplace(m_container->bodyPosition() + m_basePosition);
					m_container->m_control = this;

					QObject::connect(m_container, &RpgContainer::isActiveChanged, game, [this](){
						update();
					});

					addTiledObjectBody(m_container);
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

QVector<RpgGameData::PickableBaseData::PickableType> RpgControlGroupContainer::pickableList() const
{
	return m_pickableList;
}

void RpgControlGroupContainer::setPickableList(const QVector<RpgGameData::PickableBaseData::PickableType> &newPickableList)
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
	m_currentState = m_container && m_container->isActive() ? 1 : 2;
	refreshVisualItem();

	/*for (TiledObjectBody *o : m_tiledObjects) {
		o->setBodyEnabled(m_container && m_container->isActive());
	}*/
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
