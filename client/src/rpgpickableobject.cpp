/*
 * ---- Call of Suli ----
 *
 * rpgpickableobject.cpp
 *
 * Created on: 2024. 03. 19.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgPickableObject
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

#include "rpgpickableobject.h"



/// Static hash

const QHash<QString, RpgPickableObject::PickableType> RpgPickableObject::m_typeHash = {
	{ QStringLiteral("shield"), PickableShield }
};



RpgPickableObject::RpgPickableObject(const PickableType &type, QQuickItem *parent)
	: IsometricObjectCircle(parent)
	, TiledPickableIface()
	, m_pickableType(type)
{

}



void RpgPickableObject::initialize()
{
	m_body->setBodyType(Box2DBody::Static);

	setZ(1);
	setDefaultZ(1);

	m_fixture->setDensity(0);
	m_fixture->setFriction(0);
	m_fixture->setRestitution(0);
	m_fixture->setSensor(true);

	createVisual();

	load();
	onDeactivated();
}



/**
 * @brief RpgPickableObject::pickableType
 * @return
 */

RpgPickableObject::PickableType RpgPickableObject::pickableType() const
{
	return m_pickableType;
}



/**
 * @brief RpgPickableObject::onActivated
 */

void RpgPickableObject::onActivated()
{
	setVisible(true);
	m_body->setActive(true);
	m_fixture->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePickable));
	m_fixture->setCollidesWith(Box2DFixture::All);
	setSubZ(0.3);
}


/**
 * @brief RpgPickableObject::onDeactivated
 */

void RpgPickableObject::onDeactivated()
{
	setVisible(false);
	m_body->setActive(false);
	m_fixture->setCategories(Box2DFixture::None);
	m_fixture->setCollidesWith(Box2DFixture::None);
	setSubZ(0.);
}
