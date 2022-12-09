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
#include <QTimer>

#include "gameentity.h"

GameEntity::GameEntity(QQuickItem *parent)
	: QQuickItem(parent)
	, m_groundFixtures()
	, m_qrcDirName()
	, m_cosGame(nullptr)
	, m_qrcData()
	, m_bodyPolygon(nullptr)
	, m_boundBox(nullptr)
	, m_isAlive(true)
	, m_isOnGround(false)
	, m_rayCastFlag()
	, m_rayCastElevation(30)
	, m_rayCastLength(100)
	, m_rayCastEnabled(false)
	, m_rayCast(nullptr)
	, m_rayCastTimer(nullptr)
	, m_rayCastFixtures()
	, m_hp(1)
	, m_maxHp(-1)
	, m_fixturesUpdated(false)
	, m_inverse(false)
{
	connect(this, &GameEntity::parentChanged, this, &GameEntity::onParentChanged);

	connect(this, &GameEntity::boundBeginContact, this, &GameEntity::onBoundBeginContact);
	connect(this, &GameEntity::boundEndContact, this, &GameEntity::onBoundEndContact);

	m_rayCastTimer = new QTimer(this);
	m_rayCastTimer->start(50);
	connect(m_rayCastTimer, &QTimer::timeout, this, [=](){
		if (m_cosGame && m_cosGame->running()) {
			rayCastFixtureCheck();

			if (this->m_rayCastEnabled && this->isAlive())
				doRayCast();
		}
	});


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

	if (m_rayCast)
		m_rayCast->deleteLater();

	if (m_rayCastTimer)
		m_rayCastTimer->deleteLater();

}




/**
 * @brief GameEntity::doRayCast
 * @param point1
 * @param point2
 */


void GameEntity::doRayCast(const QPointF &point1, const QPointF &point2)
{
	if (!m_rayCast)
		return;

	QRectF r(point1, point2);

	emit rayCastPerformed(r.normalized());

	parentEntity()->scene()->rayCast(m_rayCast, point1, point2);
}


/**
 * @brief GameEntity::doRayCast
 */

void GameEntity::doRayCast()
{
	QPair<QPointF, QPointF> p = getRayPoints();
	doRayCast(p.first, p.second);

}


/**
 * @brief GameEntity::getRayPoints
 * @param width
 * @return
 */

QPair<QPointF, QPointF> GameEntity::getRayPoints(const qreal &width)
{
	Entity *e = parentEntity();

	if (!e) {
		return qMakePair(QPointF(0,0), QPointF(0,0));
	}

	bool facingLeft = e->property("facingLeft").toBool();

	QRectF realRect;

	if (m_bodyPolygon) {
		QVariantList vertices = m_bodyPolygon->vertices();
		QPolygonF polygon;

		foreach (QVariant v, vertices) {
			polygon.append(v.toPointF());
		}

		realRect = polygon.boundingRect();
	} else if (m_boundBox) {
		realRect.setX(m_boundBox->x());
		realRect.setY(m_boundBox->y());
		realRect.setWidth(m_boundBox->width());
		realRect.setHeight(m_boundBox->height());
	} else {
		realRect.setX(e->x());
		realRect.setY(e->y());
		realRect.setWidth(e->width());
		realRect.setHeight(e->height());
	}

	QPointF point1;
	QPointF point2;

	point1.setY(realRect.bottom()-m_rayCastElevation);
	point2.setY(point1.y());

	if (facingLeft) {
		point1.setX(realRect.right());
		point2.setX(realRect.left()-width);
	} else {
		point1.setX(realRect.left());
		point2.setX(realRect.right()+width);
	}

	point1 += QPointF(e->x(), e->y());
	point2 += QPointF(e->x(), e->y());

	return qMakePair(point1, point2);
}


/**
 * @brief GameEntity::getRayPoints
 * @return
 */

QPair<QPointF, QPointF> GameEntity::getRayPoints()
{
	return getRayPoints(m_rayCastLength);
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
 * @brief GameEntity::checkGrounds
 */

void GameEntity::removeGround(Box2DFixture *fixture)
{
	if (!m_groundFixtures.contains(fixture))
		return;

	m_groundFixtures.removeAll(fixture);

	setIsOnGround(!m_groundFixtures.isEmpty());
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

	if (!m_cosGame || !m_cosGame->running())
		return;

	if (inverse != m_inverse)
		m_fixturesUpdated = true;

	m_inverse = inverse;

	QVariantMap spriteData = m_qrcData.value("sprites").toMap().value(sprite).toMap();

	qreal boxWidth = m_qrcData.value("width", 10).toReal();
	qreal boxHeight = m_qrcData.value("height", 10).toReal();
	qreal boxX = m_qrcData.value("x", 0).toReal();
	qreal boxY = m_qrcData.value("y", 0).toReal();

	qreal parentWidth = parentEntity()->width();
	qreal parentHeight = parentEntity()->height();


	if (spriteData.contains("transformX")) {
		qreal tx = spriteData.value("transformX", 0).toReal();
		parentEntity()->setX(parentEntity()->x()+tx);
	}

	if (spriteData.contains("transformY")) {
		qreal ty = spriteData.value("transformY", 0).toReal();
		parentEntity()->setY(parentEntity()->y()+ty);
		qDebug() << "TRANSFORM" << parentEntity() << ty;
	}

	if (spriteData.contains("boxWidth"))
		boxWidth = spriteData.value("boxWidth", 0).toReal();

	if (spriteData.contains("boxHeight"))
		boxHeight = spriteData.value("boxHeight", 0).toReal();

	if (m_boundBox) {
		m_boundBox->setWidth(boxWidth);
		m_boundBox->setHeight(boxHeight);
		m_boundBox->setX(inverse ? (parentWidth-boxX-boxWidth) : boxX);
		m_boundBox->setY(boxY);
	}


	if (m_bodyPolygon) {

		QVariantList vertices;

		if (!sprite.isEmpty()) {
			QVariantList v = spriteData.value("bodyVertices").toList();

			if (!v.isEmpty())
				vertices = v;
		}

		QVariantList verticesList;


		if (vertices.isEmpty()) {
			verticesList.append(QVariant(QPointF(boxX, boxY)));
			verticesList.append(QVariant(QPointF(boxX+boxWidth, boxY)));
			verticesList.append(QVariant(QPointF(boxX+boxWidth, boxY+boxHeight)));
			verticesList.append(QVariant(QPointF(boxX, boxY+boxHeight)));
		} else {

			for (int i=0; i<vertices.count(); ++i) {
				QVariantMap m = vertices.at(i).toMap();
				qreal x = m.value("x", 0).toReal();
				qreal y = m.value("y", 0).toReal();
				verticesList.append(QVariant(QPointF(x,y)));
			}
		}

		if (inverse) {
			QList<QPointF> l = Client::rotatePolygon(verticesList, 180, QRectF(QPointF(0,0), QPointF(parentWidth, parentHeight)), Qt::YAxis);
			verticesList.clear();
			foreach (QPointF p, l)
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

	if (!m_isAlive) {
		QTimer::singleShot(2000, this, [=](){
			emit die();
		});
	}
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

void GameEntity::setRayCast(Box2DRayCast *rayCast)
{
	if (m_rayCast == rayCast)
		return;

	m_rayCast = rayCast;
	emit rayCastChanged(m_rayCast);
}

void GameEntity::setRayCastElevation(qreal rayCastElevation)
{
	if (qFuzzyCompare(m_rayCastElevation, rayCastElevation))
		return;

	m_rayCastElevation = rayCastElevation;
	emit rayCastElevationChanged(m_rayCastElevation);
}

void GameEntity::setRayCastLength(qreal rayCastLength)
{
	if (qFuzzyCompare(m_rayCastLength, rayCastLength))
		return;

	m_rayCastLength = rayCastLength;
	emit rayCastLengthChanged(m_rayCastLength);
}

void GameEntity::setRayCastEnabled(bool rayCastEnabled)
{
	if (m_rayCastEnabled == rayCastEnabled)
		return;

	m_rayCastEnabled = rayCastEnabled;
	emit rayCastEnabledChanged(m_rayCastEnabled);
}


/**
 * @brief GameEntity::setHp
 * @param hp
 */

void GameEntity::setHp(int hp)
{
	if (m_hp == hp)
		return;

	if (hp<=0)
		hp = 0;

	m_hp = hp;
	emit hpChanged(m_hp);

	if (m_hp == 0)
		setIsAlive(false);
}

void GameEntity::setMaxHp(int maxHp)
{
	if (m_maxHp == maxHp)
		return;

	m_maxHp = maxHp;
	emit maxHpChanged(m_maxHp);
}




/**
 * @brief GameEntityPrivate::onParentChanged
 * @param parent
 */

void GameEntity::onParentChanged()
{
	Entity *e = parentEntity();

	if (!this->parent() || !e)
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


	if (!m_rayCast) {
		m_rayCast = new Box2DRayCast(entity);
		connect(m_rayCast, &Box2DRayCast::fixtureReported, this, &GameEntity::rayCastFixtureReported);
	}

	CosGame *game = qobject_cast<CosGame *>(s->game());

	if (game) {
		m_cosGame = game;
		emit cosGameChanged(game);
		connect(m_cosGame, &CosGame::fixturesReadyToDestroy, this, &GameEntity::onFixturesReadyToDestroy);
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

	setIsOnGround(!m_groundFixtures.isEmpty());

	if (m_fixturesUpdated)
		m_fixturesUpdated = false;
}


/**
 * @brief GameEntityPrivate::onBoundEndContact
 * @param other
 */

void GameEntity::onBoundEndContact(Box2DFixture *other)
{
	m_groundFixtures.removeAll(other);

	if (m_fixturesUpdated)
		m_fixturesUpdated = false;
	else
		setIsOnGround(!m_groundFixtures.isEmpty());
}


/**
 * @brief GameEntity::rayCastFixtureReported
 * @param fixture
 * @param point
 * @param normal
 * @param fraction
 */

void GameEntity::rayCastFixtureReported(Box2DFixture *fixture, const QPointF &, const QPointF &, float32 fraction)
{
	QQuickItem *item = fixture->getBody()->target();

	if (!item)
		return;

	GameEntity *e = qvariant_cast<GameEntity *>(item->property("entityPrivate"));

	if (e && !e->isAlive())
		return;

	QPointer<Box2DFixture> p = fixture;

	if (m_rayCastFixtures.contains(fraction)) {
		if (!m_rayCastFixtures.value(fraction).contains(p))
			m_rayCastFixtures[fraction].append(p);
	} else {
		QList<QPointer<Box2DFixture>> list;
		list.append(p);
		m_rayCastFixtures.insert(fraction, list);
	}
}


/**
 * @brief GameEntity::rayCastFixtureCheck
 */

void GameEntity::rayCastFixtureCheck()
{
	QMapIterator<float32, QList<QPointer<Box2DFixture>>> i(m_rayCastFixtures);

	bool active = true;

	QMultiMap<qreal, QQuickItem *> items;

	while (i.hasNext()) {
		i.next();

		foreach (Box2DFixture *fixture, i.value()) {
			if (!fixture)
				continue;


			Box2DFixture::CategoryFlags categories = fixture->categories();
			if (categories.testFlag(Box2DFixture::Category1) && !fixture->property("invisible").toBool()) {
				active = false;
			}

			if (!active)
				break;

			if (categories.testFlag(m_rayCastFlag)) {
				qreal fraction = i.key();
				QQuickItem *item = fixture->getBody()->target();

				items.insert(fraction, item);
			}
		}

		if (!active)
			break;
	}

	rayCastItemsReported(items);
	m_rayCastFixtures.clear();
}


/**
 * @brief GameEntity::onFixturesReadyToDestroy
 * @param list
 */

void GameEntity::onFixturesReadyToDestroy(QList<Box2DFixture *> list)
{
	foreach (Box2DFixture *fixture, list)
		removeGround(fixture);
}



