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
#include "box2dbody.h"

#include "gameentity.h"

GameEntity::GameEntity(QQuickItem *parent)
	: QQuickItem(parent)
	, m_groundFixtures()
	, m_cosGame(nullptr)
	, m_qrcData()
	, m_bodyPolygon(nullptr)
	, m_boundBox(nullptr)
	, m_isAlive(true)
	, m_isOnGround(false)
{
	if (parent)
		onParentChanged();

	connect(this, &GameEntity::parentChanged, this, &GameEntity::onParentChanged);

	connect(this, &GameEntity::boundBeginContact, this, &GameEntity::onBoundBeginContact);
	connect(this, &GameEntity::boundEndContact, this, &GameEntity::onBoundEndContact);

}


/**
 * @brief GameEntityPrivate::~GameEntityPrivate
 */

GameEntity::~GameEntity()
{
	if (m_bodyPolygon)
		m_bodyPolygon->deleteLater();

	if (m_boundBox)
		m_boundBox->deleteLater();
}




/**
 * @brief GameEntity::parentEntity
 * @return
 */

Entity *GameEntity::parentEntity() const
{
	if (this->parent()) {
		Entity *e = qobject_cast<Entity *>(this->parent());
		if (e)
			return e;
	}

	return nullptr;
}


/**
 * @brief GameEntityPrivate::isOnBaseGround
 * @return
 */

bool GameEntity::isOnBaseGround() const
{
	foreach (Box2DFixture *f, m_groundFixtures) {
		if (f->property("baseGround").toBool())
			return true;
	}

	return false;
}



/**
 * @brief GameEntityPrivate::setFixtureVertices
 */

void GameEntity::updateFixtures(const QString &sprite, const bool &inverse)
{
	if (m_qrcData.isEmpty())
		return;

	if (!parentEntity())
		return;


	int boxWidth = m_qrcData.value("width", 10).toInt();
	int boxHeight = m_qrcData.value("height", 10).toInt();
	int boxX = m_qrcData.value("x", 0).toInt();
	int boxY = m_qrcData.value("y", 0).toInt();

	int parentWidth = parentEntity()->width();
	int parentHeight = parentEntity()->height();


	if (m_boundBox) {
		m_boundBox->setWidth(boxWidth);
		m_boundBox->setHeight(boxHeight);
		m_boundBox->setX(inverse ? (parentWidth-boxX-boxWidth) : boxX);
		m_boundBox->setY(boxY);
	}


	if (m_bodyPolygon) {

		QVariantList vertices;

		if (!sprite.isEmpty()) {
			QVariantList v = m_qrcData.value("sprites").toMap().value(sprite).toMap().value("bodyVertices").toList();

			if (!v.isEmpty())
				vertices = v;
		}

		QVariantList verticesList;


		if (vertices.isEmpty()) {
			verticesList.append(QVariant(QPoint(boxX, boxY)));
			verticesList.append(QVariant(QPoint(boxX+boxWidth, boxY)));
			verticesList.append(QVariant(QPoint(boxX+boxWidth, boxY+boxHeight)));
			verticesList.append(QVariant(QPoint(boxX, boxY+boxHeight)));
		} else {

			for (int i=0; i<vertices.count(); ++i) {
				QVariantMap m = vertices.at(i).toMap();
				int x = m.value("x", 0).toInt();
				int y = m.value("y", 0).toInt();
				verticesList.append(QVariant(QPoint(x,y)));
			}
		}

		if (inverse) {
			QList<QPoint> l = Client::rotatePolygon(verticesList, 180, QRect(QPoint(0,0), QPoint(parentWidth, parentHeight)), Qt::YAxis);
			verticesList.clear();
			foreach (QPoint p, l)
				verticesList.append(QVariant(p));
		}

		m_bodyPolygon->setVertices(verticesList);
	}
}





/**
 * @brief GameEntityPrivate::loadQrcData
 */

void GameEntity::loadQrcData()
{
	if (m_qrcDirName.isEmpty()) {
		qWarning() << "Qrc directory name empty";
		return;
	}

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

void GameEntity::setQrcDirName(QString qrcDirName)
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

void GameEntity::setQrcData(QVariantMap qrcData)
{
	if (m_qrcData == qrcData)
		return;

	m_qrcData = qrcData;
	emit qrcDataChanged(m_qrcData);
}


/**
 * @brief GameEntityPrivate::setBodyPolygon
 * @param bodyPolygon
 */

void GameEntity::setBodyPolygon(Box2DPolygon *bodyPolygon)
{
	if (m_bodyPolygon == bodyPolygon)
		return;

	m_bodyPolygon = bodyPolygon;
	emit bodyPolygonChanged(m_bodyPolygon);

	if (!m_bodyPolygon)
		return;

	connect(bodyPolygon, &Box2DPolygon::beginContact, this, &GameEntity::bodyBeginContact);
	connect(bodyPolygon, &Box2DPolygon::endContact, this, &GameEntity::bodyEndContact);
}


/**
 * @brief GameEntityPrivate::setBoundBox
 * @param boundBox
 */

void GameEntity::setBoundBox(Box2DBox *boundBox)
{
	if (m_boundBox == boundBox)
		return;

	m_boundBox = boundBox;
	emit boundBoxChanged(m_boundBox);

	if (!m_boundBox)
		return;

	connect(boundBox, &Box2DBox::beginContact, this, &GameEntity::boundBeginContact);
	connect(boundBox, &Box2DBox::endContact, this, &GameEntity::boundEndContact);
}


/**
 * @brief GameEntityPrivate::setIsAlive
 * @param isAlive
 */

void GameEntity::setIsAlive(bool isAlive)
{
	if (m_isAlive == isAlive)
		return;

	m_isAlive = isAlive;
	emit isAliveChanged(m_isAlive);

	if (!m_isAlive)
		emit die();
}


/**
 * @brief GameEntityPrivate::setIsOnGround
 * @param isOnGround
 */

void GameEntity::setIsOnGround(bool isOnGround)
{
	if (m_isOnGround == isOnGround)
		return;

	m_isOnGround = isOnGround;
	emit isOnGroundChanged(m_isOnGround);
}




/**
 * @brief GameEntityPrivate::onParentChanged
 * @param parent
 */

void GameEntity::onParentChanged()
{
	Entity *e = parentEntity();

	if (!parent() || !e)
		return;

	connect(e, &Entity::sceneChanged, this, &GameEntity::onSceneChanged);

	createFixtures();
}



/**
 * @brief GameEntityPrivate::onSceneChanged
 */

void GameEntity::onSceneChanged()
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


/**
 * @brief GameEntityPrivate::onBoundBeginContact
 * @param other
 */

void GameEntity::onBoundBeginContact(Box2DFixture *other)
{
	m_groundFixtures.append(other);

	setIsOnGround(m_groundFixtures.count());
}


/**
 * @brief GameEntityPrivate::onBoundEndContact
 * @param other
 */

void GameEntity::onBoundEndContact(Box2DFixture *other)
{
	m_groundFixtures.removeAll(other);

	setIsOnGround(m_groundFixtures.count());
}



