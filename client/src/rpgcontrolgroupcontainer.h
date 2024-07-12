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
#include <libtiledquick/tilelayeritem.h>



/**
 * @brief The RpgChestContainer class
 */

class RpgChestContainer : public TiledContainer
{
	Q_OBJECT

public:
	explicit RpgChestContainer(QObject *parent = nullptr);
	virtual ~RpgChestContainer() {}

	QVector<RpgPickableObject::PickableType> pickableList() const;
	void setPickableList(const QVector<RpgPickableObject::PickableType> &newPickableList);

	QPointF centerPoint() const;
	void setCenterPoint(QPointF newCenterPoint);

private:
	QVector<RpgPickableObject::PickableType> m_pickableList;			// TODO: add name handling (i.e. key)
	QPointF m_centerPoint;
};




/**
 * @brief The RpgControlGroupContainer class
 */

class RpgControlGroupContainer : public RpgControlGroup
{
public:
	RpgControlGroupContainer(RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer);

	RpgChestContainer *chestContainer() const { return m_container.get(); }

private:
	void onFixtureBeginContact(Box2DFixture *other);
	void onFixtureEndContact(Box2DFixture *other);
	void onControlledPlayerChanged();
	void connectFixture(Box2DFixture *fixture);
	void updateLayers();
	void onActiveChanged();

	QVector<QPointer<Box2DFixture>> m_containerFixtures;
	QVector<QPointer<QQuickItem>> m_tileLayers;
	QVector<QPointer<Box2DFixture>> m_contactedFixtures;
	std::unique_ptr<RpgChestContainer> m_container;
};


#endif // RPGCONTROLGROUPCONTAINER_H
