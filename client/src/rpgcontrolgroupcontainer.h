/*
 * ---- Call of Suli ----
 *
 * rpgcontrolgroupcontainer.h
 *
 * Created on: 2024. 04. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlGroupContainer
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

#ifndef RPGCONTROLGROUPCONTAINER_H
#define RPGCONTROLGROUPCONTAINER_H

#include "rpgcontrolgroup.h"
#include "tiledcontainer.h"
#include "rpgpickableobject.h"
#include "rpgcontrolgroupstate.h"


/**
 * @brief The DoorState class
 */

struct ContainerState {
	int id = -1;
	QUrl image;
	QPointF relativePosition;
};



/**
 * @brief The RpgControlGroupContainer class
 */

class RpgControlGroupContainer : public RpgControlGroupState<ContainerState>
{
public:
	RpgControlGroupContainer(RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer);

	TiledContainer *tiledContainer() const { return m_container.get(); }

	QVector<RpgPickableObject::PickableType> pickableList() const;
	void setPickableList(const QVector<RpgPickableObject::PickableType> &newPickableList);

	QStringList nameList() const;
	void setNameList(const QStringList &newNameList);

	QPointF centerPoint() const;
	void setCenterPoint(QPointF newCenterPoint);

private:
	void update();

	std::unique_ptr<TiledContainer> m_container;
	QVector<RpgPickableObject::PickableType> m_pickableList;
	QStringList m_nameList;
	QPointF m_centerPoint;
};



#endif // RPGCONTROLGROUPCONTAINER_H
