/*
 * ---- Call of Suli ----
 *
 * gameobject.cpp
 *
 * Created on: 2020. 10. 25.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameObject
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "gameobject.h"



GameObject::GameObject(QQuickItem *parent) :
	QQuickItem(parent),
	m_id(0),
	m_body(nullptr),
	m_density(.0),
	m_friction(.0),
	m_restitution(.0),
	m_categories(),
	m_collidesWith(),
	m_sensor(false),
	m_groupIndex(0),
	m_extra()
{

}


/**
 * @brief GameObject::~GameObject
 */

GameObject::~GameObject()
{
	if (m_body) {
		m_body->deleteFixtures();
		m_body->deleteLater();
	}

	m_body = nullptr;
}

/**
 * @brief GameObject::createFixture
 * @param object
 */


void GameObject::createFixture(Tiled::MapObject *object)
{
	if (!object)
		return;

	switch(object->shape())
	{
		case Tiled::MapObject::Rectangle:
			createRectangularFixture(object);
			break;
		case Tiled::MapObject::Ellipse:
			qWarning() << "Missing createEllipseFixture";
			//createEllipseFixture(object, item);
			break;
		case Tiled::MapObject::Polygon:
			qWarning() << "Missing createPolygonFixture";
			//createPolygonFixture(object, item);
			break;
		case Tiled::MapObject::Polyline:
			qWarning() << "Missing createPolylineFixture";
			//createPolylineFixture(object, item);
			break;
		default:
			qWarning() << "Unhandled object group: " << object->name();
			break;
	}
}


/**
 * @brief GameObject::createRectangularFixture
 */

void GameObject::createRectangularFixture(Tiled::MapObject *object)
{
	if (m_body)
		return;

	if(!object)
		return;

	m_body = new Box2DBody(this);
	m_body->setBodyType(Box2DBody::Static);
	m_body->setTarget(this);
	m_body->setActive(true);
	m_body->setSleepingAllowed(false);

	Box2DBox *fixture = new Box2DBox(this);

	fixture->setX(0);
	fixture->setY(0);
	fixture->setWidth(object->width());
	fixture->setHeight(object->height());

	fixture->setDensity(m_density);
	fixture->setFriction(m_friction);
	fixture->setRestitution(m_restitution);
	fixture->setSensor(m_sensor);
	fixture->setCategories(m_categories);
	fixture->setCollidesWith(m_collidesWith);
	fixture->setGroupIndex(m_groupIndex);

	m_body->addFixture(fixture);

	m_body->componentComplete();
}



/**
 * @brief GameObject::createRectangularFixture
 */

Box2DBox * GameObject::createRectangularFixture()
{
	if (m_body)
		return nullptr;

	m_body = new Box2DBody(this);
	m_body->setBodyType(Box2DBody::Static);
	m_body->setTarget(this);
	m_body->setActive(true);
	m_body->setSleepingAllowed(false);

	Box2DBox *fixture = new Box2DBox(this);

	fixture->setX(0);
	fixture->setY(0);
	fixture->setWidth(width());
	fixture->setHeight(height());

	fixture->setDensity(m_density);
	fixture->setFriction(m_friction);
	fixture->setRestitution(m_restitution);
	fixture->setSensor(m_sensor);
	fixture->setCategories(m_categories);
	fixture->setCollidesWith(m_collidesWith);
	fixture->setGroupIndex(m_groupIndex);

	m_body->addFixture(fixture);

	m_body->componentComplete();

	return fixture;
}



void GameObject::setDensity(float density)
{
	if (qFuzzyCompare(m_density, density))
		return;

	m_density = density;
	emit densityChanged(m_density);
}

void GameObject::setFriction(float friction)
{
	if (qFuzzyCompare(m_friction, friction))
		return;

	m_friction = friction;
	emit frictionChanged(m_friction);
}

void GameObject::setRestitution(float restitution)
{
	if (qFuzzyCompare(m_restitution, restitution))
		return;

	m_restitution = restitution;
	emit restitutionChanged(m_restitution);
}

void GameObject::setCategories(Box2DFixture::CategoryFlags categories)
{
	if (m_categories == categories)
		return;

	m_categories = categories;
	emit categoriesChanged(m_categories);
}

void GameObject::setCollidesWith(Box2DFixture::CategoryFlags collidesWith)
{
	if (m_collidesWith == collidesWith)
		return;

	m_collidesWith = collidesWith;
	emit collidesWithChanged(m_collidesWith);
}

void GameObject::setSensor(bool sensor)
{
	if (m_sensor == sensor)
		return;

	m_sensor = sensor;
	emit sensorChanged(m_sensor);
}

void GameObject::setGroupIndex(int groupIndex)
{
	if (m_groupIndex == groupIndex)
		return;

	m_groupIndex = groupIndex;
	emit groupIndexChanged(m_groupIndex);
}

const QVariantMap &GameObject::extra() const
{
	return m_extra;
}

void GameObject::setExtra(const QVariantMap &newExtra)
{
	if (m_extra == newExtra)
		return;
	m_extra = newExtra;
	emit extraChanged();
}
