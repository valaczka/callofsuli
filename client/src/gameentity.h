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
#include "gameobject.h"

#include <QJsonObject>



class GameEntity : public GameObject
{
	Q_OBJECT

	Q_PROPERTY(QString dataDir READ dataDir WRITE setDataDir NOTIFY dataDirChanged)
	Q_PROPERTY(int hp READ hp WRITE setHp NOTIFY hpChanged)
	Q_PROPERTY(int maxHp READ maxHp WRITE setMaxHp NOTIFY maxHpChanged)
	Q_PROPERTY(bool isAlive READ isAlive WRITE setIsAlive NOTIFY isAliveChanged)
	Q_PROPERTY(bool facingLeft READ facingLeft WRITE setFacingLeft NOTIFY facingLeftChanged)
	Q_PROPERTY(qreal rayCastLength READ rayCastLength WRITE setRayCastLength NOTIFY rayCastLengthChanged)
	Q_PROPERTY(qreal rayCastElevation READ rayCastElevation WRITE setRayCastElevation NOTIFY rayCastElevationChanged)
	Q_PROPERTY(bool rayCastEnabled READ rayCastEnabled WRITE setRayCastEnabled NOTIFY rayCastEnabledChanged)
	Q_PROPERTY(QRectF bodyRect READ bodyRect WRITE setBodyRect NOTIFY bodyRectChanged)
	Q_PROPERTY(qreal walkSize READ walkSize WRITE setWalkSize NOTIFY walkSizeChanged)
	Q_PROPERTY(QString shotSound READ shotSound WRITE setShotSound NOTIFY shotSoundChanged)

	Q_PROPERTY(Box2DFixture::CategoryFlag categoryFixture READ categoryFixture WRITE setCategoryFixture NOTIFY categoryFixtureChanged)
	Q_PROPERTY(Box2DFixture::CategoryFlag categoryCollidesWith READ categoryCollidesWith WRITE setCategoryCollidesWith NOTIFY categoryCollidesWithChanged)
	Q_PROPERTY(Box2DFixture::CategoryFlag categoryRayCast READ categoryRayCast WRITE setCategoryRayCast NOTIFY categoryRayCastChanged)

	Q_PROPERTY(bool isOnGround READ isOnGround NOTIFY isOnGroundChanged)

	Q_PROPERTY(QQuickItem * spriteSequence READ spriteSequence WRITE setSpriteSequence NOTIFY spriteSequenceChanged)

public:
	explicit GameEntity(QQuickItem *parent = nullptr);
	virtual ~GameEntity();

	bool loadFromJsonFile();
	virtual bool loadFromJsonFile(const QString &filename);

	QPair<QPointF, QPointF> getRayPoints(const qreal &width);
	QPair<QPointF, QPointF> getRayPoints();

	bool facingLeft() const;
	void setFacingLeft(bool newFacingLeft);

	qreal rayCastLength() const;
	void setRayCastLength(qreal newRayCastLength);

	qreal rayCastElevation() const;
	void setRayCastElevation(qreal newRayCastElevation);

	bool rayCastEnabled() const;
	void setRayCastEnabled(bool newRayCastEnabled);

	qreal walkSize() const;
	void setWalkSize(qreal newWalkSize);

	int hp() const;
	void setHp(int newHp);

	int maxHp() const;
	void setMaxHp(int newMaxHp);

	const QRectF &bodyRect() const;
	void setBodyRect(const QRectF &newBodyRect);

	const QString &dataDir() const;
	void setDataDir(const QString &newDataDir);

	const QString &shotSound() const;
	void setShotSound(const QString &newShotSound);

	bool isAlive() const;
	void setIsAlive(bool newIsAlive);


	QQuickItem *spriteSequence() const;
	void setSpriteSequence(QQuickItem *newSpriteSequence);

	const Box2DFixture::CategoryFlag &categoryFixture() const;
	void setCategoryFixture(const Box2DFixture::CategoryFlag &newCategoryFixture);

	const Box2DFixture::CategoryFlag &categoryCollidesWith() const;
	void setCategoryCollidesWith(const Box2DFixture::CategoryFlag &newCategoryCollidesWith);

	const Box2DFixture::CategoryFlag &categoryRayCast() const;
	void setCategoryRayCast(const Box2DFixture::CategoryFlag &newCategoryRayCast);


	bool isOnGround() const;


	const QJsonObject &dataObject() const;
	const QJsonObject &sprites() const;


	Q_INVOKABLE QJsonObject sprite(const QString &key) const;

	Box2DBox *fixture() const;



public slots:
	inline void decreaseHp() { setHp(m_hp-1); }
	inline void kill() { setHp(0); }

	void updateFixtures(QString spriteName = "");


private slots:
	void onBeginContact(Box2DFixture *other);
	void onEndContact(Box2DFixture *other);
	void onRayCastTimerTimeout();
	void onRayCastFixtureReported(Box2DFixture *fixture, const QPointF &, const QPointF &, qreal fraction);
	void onCurrentSpriteChanged();

signals:
	void baseGroundContact();
	void died();

	void beginContact(Box2DFixture *other);
	void endContact(Box2DFixture *other);

	void rayCastPerformed(QRectF rect);

	void facingLeftChanged();
	void rayCastLengthChanged();
	void rayCastElevationChanged();
	void rayCastEnabledChanged();
	void walkSizeChanged();
	void hpChanged();
	void maxHpChanged();
	void isAliveChanged();
	void bodyRectChanged();
	void dataDirChanged();
	void spriteSequenceChanged();
	void isOnGroundChanged();
	void shotSoundChanged();

	void categoryFixtureChanged(Box2DFixture::CategoryFlag flag);
	void categoryCollidesWithChanged(Box2DFixture::CategoryFlag flag);
	void categoryRayCastChanged(Box2DFixture::CategoryFlag flag);


protected:
	virtual void rayCastReport(const QMultiMap<qreal, GameEntity *> &items);
	void updateFixtures(const QJsonObject &spriteData);

	QRectF m_bodyRect;
	QSizeF m_frameSize;
	bool m_spriteFacingLeft = false;
	qreal m_walkSize = 0;
	QJsonObject m_dataObject;
	QJsonObject m_sprites;
	QString m_dataDir;


private:
	void doRayCast();
	void doRayCast(const QPointF &point1, const QPointF &point2);
	void rayCastFixtureCheck();

	void loadSprites();

	Box2DRayCast *m_rayCast = nullptr;
	QTimer *m_rayCastTimer = nullptr;
	int m_hp = 0;
	int m_maxHp = 0;
	bool m_facingLeft = false;
	qreal m_rayCastLength = 0;
	qreal m_rayCastElevation = 0;
	bool m_rayCastEnabled = true;
	QString m_shotSound;
	bool m_isAlive = true;

	QQuickItem *m_spriteSequence = nullptr;
	bool m_spritesLoaded = false;

	Box2DBox *m_fixture = nullptr;
	Box2DFixture::CategoryFlag m_categoryFixture = Box2DFixture::None;
	Box2DFixture::CategoryFlag m_categoryCollidesWith = CATEGORY_GROUND;
	Box2DFixture::CategoryFlag m_categoryRayCast = Box2DFixture::None;
	QList<QPointer<Box2DFixture>> m_groundFixtures;
	QMap<float32, QList<QPointer<Box2DFixture>>> m_rayCastFixtures;
};

#endif // GAMEPLAYERPRIVATE_H
