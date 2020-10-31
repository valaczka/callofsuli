/*
 * ---- Call of Suli ----
 *
 * gameplayerprivate.h
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

#ifndef GAMEENTITYPRIVATE_H
#define GAMEENTITYPRIVATE_H

#include "box2dfixture.h"
#include "box2draycast.h"
#include <QQuickItem>

#include "entity.h"
#include "cosgame.h"


/**
 * COLLISION CATEGORIES
 *
 * Category1: ground
 * Category2: player bound
 * Category3: player body (sensor)
 * Category4: item bound (sensor)
 * Category5: enemy bound
 */

class GameEntity : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(CosGame* cosGame READ cosGame NOTIFY cosGameChanged)
	Q_PROPERTY(Entity* parentEntity READ parentEntity)
	Q_PROPERTY(Box2DPolygon* bodyPolygon READ bodyPolygon WRITE setBodyPolygon NOTIFY bodyPolygonChanged)
	Q_PROPERTY(Box2DBox* boundBox READ boundBox WRITE setBoundBox NOTIFY boundBoxChanged)

	Q_PROPERTY(QString qrcDirName READ qrcDirName WRITE setQrcDirName NOTIFY qrcDirNameChanged)
	Q_PROPERTY(QVariantMap qrcData READ qrcData WRITE setQrcData NOTIFY qrcDataChanged)

	Q_PROPERTY(bool isAlive READ isAlive WRITE setIsAlive NOTIFY isAliveChanged)
	Q_PROPERTY(bool isOnGround READ isOnGround WRITE setIsOnGround NOTIFY isOnGroundChanged)
	Q_PROPERTY(bool isOnBaseGround READ isOnBaseGround)

	Q_PROPERTY(Box2DRayCast * rayCast READ rayCast WRITE setRayCast NOTIFY rayCastChanged)
	Q_PROPERTY(qreal rayCastElevation READ rayCastElevation WRITE setRayCastElevation NOTIFY rayCastElevationChanged)
	Q_PROPERTY(qreal rayCastLength READ rayCastLength WRITE setRayCastLength NOTIFY rayCastLengthChanged)
	Q_PROPERTY(bool rayCastEnabled READ rayCastEnabled WRITE setRayCastEnabled NOTIFY rayCastEnabledChanged)
	Q_PROPERTY(QMultiMap<qreal, QQuickItem *> rayCastItems READ rayCastItems NOTIFY rayCastItemsChanged)


public:
	GameEntity(QQuickItem *parent = 0);
	~GameEntity();

	virtual void setQrcDir() {}
	virtual void createFixtures() {}

	Q_INVOKABLE void doRayCast(const QPointF &point1, const QPointF &point2);
	Q_INVOKABLE void doRayCast();
	Q_INVOKABLE virtual QPair<QPointF, QPointF> getRayPoints(const qreal &width);
	Q_INVOKABLE QPair<QPointF, QPointF> getRayPoints();

	CosGame* cosGame() const { return m_cosGame; }
	Entity *parentEntity() const;
	QString qrcDirName() const { return m_qrcDirName; }
	QVariantMap qrcData() const { return m_qrcData; }
	Box2DPolygon* bodyPolygon() const { return m_bodyPolygon; }
	Box2DBox* boundBox() const { return m_boundBox; }
	bool isAlive() const { return m_isAlive; }
	bool isOnGround() const { return m_isOnGround; }
	bool isOnBaseGround() const;

	Box2DRayCast * rayCast() const { return m_rayCast; }
	qreal rayCastElevation() const { return m_rayCastElevation; }
	qreal rayCastLength() const { return m_rayCastLength; }
	QMultiMap<qreal, QQuickItem *> rayCastItems() const { return m_rayCastItems; }
	bool rayCastEnabled() const { return m_rayCastEnabled; }

public slots:
	void updateFixtures(const QString &sprite, const bool &inverse = false);
	void loadQrcData();
	void setQrcDirName(QString qrcDirName);
	void setQrcData(QVariantMap qrcData);
	void setBodyPolygon(Box2DPolygon* bodyPolygon);
	void setBoundBox(Box2DBox* boundBox);
	void setIsAlive(bool isAlive);
	void setIsOnGround(bool isOnGround);
	void setRayCast(Box2DRayCast * rayCast);
	void setRayCastElevation(qreal rayCastElevation);
	void setRayCastLength(qreal rayCastLength);
	void setRayCastEnabled(bool rayCastEnabled);

protected slots:

private slots:
	void onParentChanged();
	void onSceneChanged();
	void onBoundBeginContact(Box2DFixture *other);
	void onBoundEndContact(Box2DFixture *other);
	void rayCastFixtureReported(Box2DFixture *fixture, const QPointF &, const QPointF &, float32 fraction);
	void rayCastFixtureCheck();
	void setRayCastItems(QMultiMap<qreal, QQuickItem *> rayCastItems);
	void onGameStateChanged();

signals:
	void die();

	void bodyBeginContact(Box2DFixture *other);
	void bodyEndContact(Box2DFixture *other);
	void boundBeginContact(Box2DFixture *other);
	void boundEndContact(Box2DFixture *other);

	void sceneReady(Scene *scene);

	void spritesChanged(QVariantMap sprites);
	void qrcDirNameChanged(QString qrcDirName);
	void cosGameChanged(CosGame* cosGame);
	void qrcDataChanged(QVariantMap qrcData);
	void bodyPolygonChanged(Box2DPolygon* bodyPolygon);
	void boundBoxChanged(Box2DBox* boundBox);
	void isAliveChanged(bool isAlive);
	void isOnGroundChanged(bool isOnGround);

	void rayCastChanged(Box2DRayCast * rayCast);
	void rayCastElevationChanged(qreal rayCastElevation);
	void rayCastLengthChanged(qreal rayCastLength);
	void rayCastPerformed(QRectF rect);
	void rayCastItemsChanged(QMultiMap<qreal, QQuickItem *> rayCastItems);
	void rayCastEnabledChanged(bool rayCastEnabled);

protected:
	QList<Box2DFixture *> m_groundFixtures;
	QString m_qrcDirName;
	CosGame* m_cosGame;
	QVariantMap m_qrcData;
	Box2DPolygon* m_bodyPolygon;
	Box2DBox* m_boundBox;
	bool m_isAlive;
	bool m_isOnGround;
	Box2DFixture::CategoryFlag m_rayCastFlag;
	qreal m_rayCastElevation;
	qreal m_rayCastLength;
	bool m_rayCastEnabled;

private:
	Box2DRayCast * m_rayCast;
	QTimer *m_rayCastTimer;
	QMap<float32, QList<Box2DFixture*>> m_rayCastFixtures;
	QMultiMap<qreal, QQuickItem *> m_rayCastItems;
};

#endif // GAMEPLAYERPRIVATE_H
