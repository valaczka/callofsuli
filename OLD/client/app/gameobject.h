/*
 * ---- Call of Suli ----
 *
 * gameobject.h
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

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <QObject>
#include <QQuickItem>
#include "box2dbody.h"
#include "box2dfixture.h"
#include "mapobject.h"

class GameObject : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(float density READ density WRITE setDensity NOTIFY densityChanged)
	Q_PROPERTY(float friction READ friction WRITE setFriction NOTIFY frictionChanged)
	Q_PROPERTY(float restitution READ restitution WRITE setRestitution NOTIFY restitutionChanged)
	Q_PROPERTY(bool sensor READ sensor WRITE setSensor NOTIFY sensorChanged)
	Q_PROPERTY(Box2DFixture::CategoryFlags categories READ categories WRITE setCategories NOTIFY categoriesChanged)
	Q_PROPERTY(Box2DFixture::CategoryFlags collidesWith READ collidesWith WRITE setCollidesWith NOTIFY collidesWithChanged)
	Q_PROPERTY(int groupIndex READ groupIndex WRITE setGroupIndex NOTIFY groupIndexChanged)
	Q_PROPERTY(QVariantMap extra READ extra WRITE setExtra NOTIFY extraChanged)

public:
	GameObject(QQuickItem *parent = 0);
	virtual ~GameObject();

	QMap<QString, QVariant> properties() const { return m_properties; }
	void setProperties(const QMap<QString, QVariant> &properties) { m_properties = properties; }

	Box2DBody *body() const { return m_body; }
	void setBody(Box2DBody *body) { m_body = body; }

	int id() const { return m_id; }
	void setId(int id) { m_id = id; }

	void createFixture(Tiled::MapObject *object);
	void createRectangularFixture(Tiled::MapObject *object);
	Box2DBox *createRectangularFixture();

	float density() const { return m_density; }
	float friction() const { return m_friction; }
	float restitution() const { return m_restitution; }
	Box2DFixture::CategoryFlags categories() const { return m_categories; }
	Box2DFixture::CategoryFlags collidesWith() const { return m_collidesWith; }
	bool sensor() const { return m_sensor; }
	int groupIndex() const { return m_groupIndex; }

	const QVariantMap &extra() const;
	void setExtra(const QVariantMap &newExtra);

public slots:
	void setDensity(float density);
	void setFriction(float friction);
	void setRestitution(float restitution);
	void setCategories(Box2DFixture::CategoryFlags categories);
	void setCollidesWith(Box2DFixture::CategoryFlags collidesWith);
	void setSensor(bool sensor);
	void setGroupIndex(int groupIndex);

signals:
	void densityChanged(float density);
	void frictionChanged(float friction);
	void restitutionChanged(float restitution);
	void categoriesChanged(Box2DFixture::CategoryFlags categories);
	void collidesWithChanged(Box2DFixture::CategoryFlags collidesWith);
	void sensorChanged(bool sensor);
	void groupIndexChanged(int groupIndex);
	void extraChanged();

private:
	int m_id;
	Box2DBody *m_body;
	QMap<QString, QVariant> m_properties;
	float m_density;
	float m_friction;
	float m_restitution;
	Box2DFixture::CategoryFlags m_categories;
	Box2DFixture::CategoryFlags m_collidesWith;
	bool m_sensor;
	int m_groupIndex;
	QVariantMap m_extra;
};

#endif // GAMEOBJECT_H
