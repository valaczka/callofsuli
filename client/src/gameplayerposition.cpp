/*
 * ---- Call of Suli ----
 *
 * gameplayerposition.cpp
 *
 * Created on: 2022. 12. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GamePlayerPosition
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

#include "gameplayerposition.h"
#include "box2dfixture.h"


GamePlayerPosition::GamePlayerPosition(const GameTerrain::PlayerPositionData &data, QQuickItem *parent)
	: GameObject(parent)
	, m_data(data)
{
	qreal w = 20;
	qreal h = 80;
	qreal x = data.point.x()-w/2;
	qreal y = data.point.y()-h/2;

	setX(x);
	setY(y);
	setZ(0);
	setWidth(w);
	setHeight(h);
	setVisible(true);

	Box2DBox *fixture = new Box2DBox(this);

	fixture->setX(0);
	fixture->setY(0);
	fixture->setWidth(w);
	fixture->setHeight(h);

	fixture->setDensity(1);
	fixture->setFriction(1);
	fixture->setRestitution(0);
	fixture->setCategories(CATEGORY_OTHER);
	fixture->setSensor(true);

	m_body->addFixture(fixture);

	bodyComplete();
}


/**
 * @brief GamePlayerPosition::~GamePlayerPosition
 */

GamePlayerPosition::~GamePlayerPosition()
{

}


/**
 * @brief GamePlayerPosition::data
 * @return
 */

const GameTerrain::PlayerPositionData &GamePlayerPosition::data() const
{
	return m_data;
}
