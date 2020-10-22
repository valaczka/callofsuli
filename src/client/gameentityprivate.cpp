/*
 * ---- Call of Suli ----
 *
 * gameplayerprivate.cpp
 *
 * Created on: 2020. 10. 21.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GamePlayerPrivate
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

#include "scene.h"
#include "cosclient.h"

#include "gameentityprivate.h"

GameEntityPrivate::GameEntityPrivate(QQuickItem *parent)
	: QQuickItem(parent)
	, m_cosGame(nullptr)
	, m_qrcData()
	, m_bodyPolygon(nullptr)
{
	if (parent)
		onParentChanged();

	connect(this, &GameEntityPrivate::parentChanged, this, &GameEntityPrivate::onParentChanged);
}



Entity *GameEntityPrivate::parentEntity() const
{
	if (this->parent()) {
		Entity *e = qobject_cast<Entity *>(this->parent());
		if (e)
			return e;
	}

	return nullptr;
}


/**
 * @brief GameEntityPrivate::setFixtureVertices
 */

void GameEntityPrivate::setFixtureVertices(const QString &sprite, const bool &inverse)
{
	if (sprite.isEmpty())
		return;

	if (!m_bodyPolygon)
		return;

	QVariantList vertices = m_qrcData.value("sprites").toMap().value(sprite).toMap().value("bodyVertices").toList();

	if (vertices.isEmpty())
		return;

	QList<QPoint> list;

	for (int i=0; i<vertices.count(); ++i) {
		QVariantMap m = vertices.at(i).toMap();
		int x = m.value("x", 0).toInt();
		int y = m.value("y", 0).toInt();
		list.append(QPoint(x,y));
	}

	if (inverse)
		list = Client::rotatePolygon(list, 180, Qt::YAxis);

	QVariantList l;

	foreach (QPoint p, list) {
		l.append(QVariant(p));
	}

	m_bodyPolygon->setVertices(l);
}





/**
 * @brief GameEntityPrivate::loadQrcData
 */

void GameEntityPrivate::loadQrcData()
{
	if (m_qrcDirName.isEmpty()) {
		qWarning() << "Qrc directory name empty";
		return;
	}

	qDebug() << "LOAD" << m_qrcDirName;

	QVariant v = Client::readJsonFile(m_qrcDirName+"/data.json");

	if (!v.isValid()) {
		qWarning() << "Invalid json data";
		return;
	}

	setQrcData(v.toMap());
}







/**
 * @brief GameEntityPrivate::setQrcDirName
 * @param qrcDirName
 */

void GameEntityPrivate::setQrcDirName(QString qrcDirName)
{
	if (m_qrcDirName == qrcDirName)
		return;

	m_qrcDirName = qrcDirName;
	emit qrcDirNameChanged(m_qrcDirName);
}


/**
 * @brief GameEntityPrivate::setQrcData
 * @param qrcData
 */

void GameEntityPrivate::setQrcData(QVariantMap qrcData)
{
	if (m_qrcData == qrcData)
		return;

	m_qrcData = qrcData;
	emit qrcDataChanged(m_qrcData);
}

void GameEntityPrivate::setBodyPolygon(Box2DPolygon *bodyPolygon)
{
	if (m_bodyPolygon == bodyPolygon)
		return;

	m_bodyPolygon = bodyPolygon;
	emit bodyPolygonChanged(m_bodyPolygon);

	connect(bodyPolygon, &Box2DPolygon::beginContact, this, &GameEntityPrivate::onBodyBeginContact);
	connect(bodyPolygon, &Box2DPolygon::endContact, this, &GameEntityPrivate::onBodyEndContact);
}



/**
 * @brief GameEntityPrivate::onParentChanged
 * @param parent
 */

void GameEntityPrivate::onParentChanged()
{
	Entity *e = parentEntity();

	if (!e)
		return;

	connect(e, &Entity::sceneChanged, this, &GameEntityPrivate::onSceneChanged);
}



/**
 * @brief GameEntityPrivate::onSceneChanged
 */

void GameEntityPrivate::onSceneChanged()
{
	Entity *entity = parentEntity();
	if (!entity)
		return;

	Scene *s = entity->scene();

	if (!s)
		return;


	CosGame *game = qobject_cast<CosGame *>(s->game());

	if (game) {
		m_cosGame = game;
		emit cosGameChanged(game);
		return;
	}

	qWarning() << "Invalid type cast" << s->game();
}



