/*
 * ---- Call of Suli ----
 *
 * GamePlayerImpl.h
 *
 * Created on: 2020. 10. 21.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GamePlayerImpl
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
#include "qpropertyanimation.h"

#include <QJsonObject>

/**
 * @brief The GameEntity class
 */

class GameEntity : public GameObject
{
	Q_OBJECT

	Q_PROPERTY(QString dataDir READ dataDir WRITE setDataDir NOTIFY dataDirChanged)
	Q_PROPERTY(int hp READ hp WRITE setHp NOTIFY hpChanged)
	Q_PROPERTY(int maxHp READ maxHp WRITE setMaxHp NOTIFY maxHpChanged)
	Q_PROPERTY(bool isAlive READ isAlive NOTIFY isAliveChanged)
	Q_PROPERTY(bool facingLeft READ facingLeft WRITE setFacingLeft NOTIFY facingLeftChanged)
	Q_PROPERTY(qreal rayCastLength READ rayCastLength WRITE setRayCastLength NOTIFY rayCastLengthChanged)
	Q_PROPERTY(qreal rayCastElevation READ rayCastElevation WRITE setRayCastElevation NOTIFY rayCastElevationChanged)
	Q_PROPERTY(bool rayCastEnabled READ rayCastEnabled WRITE setRayCastEnabled NOTIFY rayCastEnabledChanged)
	Q_PROPERTY(QRectF bodyRect READ bodyRect WRITE setBodyRect NOTIFY bodyRectChanged)
	Q_PROPERTY(qreal walkSize READ walkSize WRITE setWalkSize NOTIFY walkSizeChanged)
	Q_PROPERTY(QUrl shotSound READ shotSound WRITE setShotSound NOTIFY shotSoundChanged)

	Q_PROPERTY(Box2DFixture::CategoryFlags categoryFixture READ categoryFixture WRITE setCategoryFixture NOTIFY categoryFixtureChanged)
	Q_PROPERTY(Box2DFixture::CategoryFlags categoryCollidesWith READ categoryCollidesWith WRITE setCategoryCollidesWith NOTIFY categoryCollidesWithChanged)
	Q_PROPERTY(Box2DFixture::CategoryFlags categoryRayCast READ categoryRayCast WRITE setCategoryRayCast NOTIFY categoryRayCastChanged)

	Q_PROPERTY(QQuickItem *spriteItem READ spriteItem WRITE setSpriteItem NOTIFY spriteItemChanged)
	Q_PROPERTY(QQuickItem *spriteSequence READ spriteSequence NOTIFY spriteSequenceChanged)

	Q_PROPERTY(bool isOnGround READ isOnGround NOTIFY isOnGroundChanged)

	Q_PROPERTY(bool glowEnabled READ glowEnabled WRITE setGlowEnabled NOTIFY glowEnabledChanged)
	Q_PROPERTY(QColor glowColor READ glowColor WRITE setGlowColor NOTIFY glowColorChanged)
	Q_PROPERTY(bool overlayEnabled READ overlayEnabled WRITE setOverlayEnabled NOTIFY overlayEnabledChanged)
	Q_PROPERTY(QColor overlayColor READ overlayColor WRITE setOverlayColor NOTIFY overlayColorChanged)
	Q_PROPERTY(bool hpProgressEnabled READ hpProgressEnabled WRITE setHpProgressEnabled NOTIFY hpProgressEnabledChanged)
	Q_PROPERTY(QColor hpProgressColor READ hpProgressColor WRITE setHpProgressColor NOTIFY hpProgressColorChanged)
	Q_PROPERTY(int hpProgressValue READ hpProgressValue WRITE setHpProgressValue NOTIFY hpProgressValueChanged)

public:
	explicit GameEntity(QQuickItem *parent = nullptr);
	virtual ~GameEntity();

	bool loadFromJsonFile();
	virtual bool loadFromJsonFile(const QString &filename);

	std::optional<QPair<QPointF, QPointF> > getRayPoints(const qreal &width);
	std::optional<QPair<QPointF, QPointF> > getRayPoints();

	bool isOnGround() const;

	const QJsonObject &dataObject() const;
	const QJsonObject &sprites() const;

	Q_INVOKABLE QJsonObject sprite(const QString &key) const;

	Box2DBox *fixture() const;

	Q_INVOKABLE bool createSpriteItem();


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

	QString shotSound() const;
	void setShotSound(const QUrl &newShotSound);

	bool isAlive() const;

	const Box2DFixture::CategoryFlags &categoryFixture() const;
	void setCategoryFixture(const Box2DFixture::CategoryFlags &newCategoryFixture);

	const Box2DFixture::CategoryFlags &categoryCollidesWith() const;
	void setCategoryCollidesWith(const Box2DFixture::CategoryFlags &newCategoryCollidesWith);

	const Box2DFixture::CategoryFlags &categoryRayCast() const;
	void setCategoryRayCast(const Box2DFixture::CategoryFlags &newCategoryRayCast);

	bool glowEnabled() const;
	void setGlowEnabled(bool newGlowEnabled);

	const QColor &glowColor() const;
	void setGlowColor(const QColor &newGlowColor);

	QQuickItem *spriteItem() const;
	void setSpriteItem(QQuickItem *newSpriteItem);

	QQuickItem *spriteSequence() const;

	bool overlayEnabled() const;
	void setOverlayEnabled(bool newOverlayEnabled);

	const QColor &overlayColor() const;
	void setOverlayColor(const QColor &newOverlayColor);

	bool hpProgressEnabled() const;
	void setHpProgressEnabled(bool newHpProgressEnabled);

	const QColor &hpProgressColor() const;
	void setHpProgressColor(const QColor &newHpProgressColor);

	int hpProgressValue() const;
	void setHpProgressValue(int newHpProgressValue);

	void performRayCast();

	virtual void setStateFromSnapshot(ObjectStateBase *ptr) override;

public slots:
	void decreaseHp();
	void kill();

	void updateFixtures(QString spriteName = "");
	void jumpToSprite(const QString &sprite);


private slots:
	void onBeginContact(Box2DFixture *other);
	void onEndContact(Box2DFixture *other);
	void onRayCastFixtureReported(Box2DFixture *fixture, const QPointF &, const QPointF &, qreal fraction);
	void onSequenceCurrentSpriteChanged(QString sprite);

signals:
	void baseGroundContact();
	void hurt();
	void killed(GameEntity *entity);
	void died(GameEntity *entity);
	void allHpLost();

	void beginContact(Box2DFixture *other);
	void endContact(Box2DFixture *other);

	void currentSpriteChanged(QString sprite);
	void rayCastPerformed(QRectF rect);

	void facingLeftChanged();
	void rayCastLengthChanged();
	void rayCastElevationChanged();
	void rayCastEnabledChanged();
	void walkSizeChanged();
	void hpChanged(int hp);
	void maxHpChanged();
	void isAliveChanged();
	void bodyRectChanged();
	void dataDirChanged();
	void isOnGroundChanged();
	void shotSoundChanged(QUrl sound);

	void categoryFixtureChanged(Box2DFixture::CategoryFlags flag);
	void categoryCollidesWithChanged(Box2DFixture::CategoryFlags flag);
	void categoryRayCastChanged(Box2DFixture::CategoryFlags flag);

	void glowEnabledChanged();
	void glowColorChanged();
	void overlayEnabledChanged();
	void overlayColorChanged();
	void hpProgressEnabledChanged();
	void hpProgressColorChanged();
	void hpProgressValueChanged();

	void spriteItemChanged();
	void spriteSequenceChanged();

protected:
	virtual void rayCastReport(const QMultiMap<qreal, GameEntity *> &items);
	void updateFixturesJson(const QJsonObject &spriteData);
	void onIsAliveDisabled();
	virtual void hpProgressValueSetup();

	QRectF m_bodyRect;
	QSizeF m_frameSize;
	bool m_spriteFacingLeft = false;
	qreal m_walkSize = 0;
	QJsonObject m_dataObject;
	QJsonObject m_sprites;
	QString m_dataDir;
	QString m_defaultShotSound;
	int m_hp = 1;
	int m_maxHp = 1;
	bool m_facingLeft = false;
	QList<QPointer<Box2DFixture>> m_groundFixtures;
	QString m_lastCurrentSprite = "";



	bool getCurrentState(ObjectStateEntity *ptr) const;
	void setCurrentState(const ObjectStateEntity &state);



private:
	void doRayCast();
	void doRayCast(const QPointF &point1, const QPointF &point2);
	void rayCastFixtureCheck();
	void onFacingLeftChanged();

	void loadSprites();

	std::unique_ptr<Box2DRayCast> m_rayCast;
	qreal m_rayCastLength = 0;
	qreal m_rayCastElevation = 0;
	bool m_rayCastEnabled = false;
	QUrl m_shotSound;

	bool m_glowEnabled = false;
	QColor m_glowColor = QColor("white");
	bool m_overlayEnabled = false;
	QColor m_overlayColor = QColor("white");
	bool m_hpProgressEnabled = false;
	QColor m_hpProgressColor = QColor("red");
	int m_hpProgressValue = 0;

	QQuickItem *m_spriteItem = nullptr;
	QQuickItem *m_spriteSequence = nullptr;
	bool m_spritesLoaded = false;


	std::unique_ptr<Box2DBox> m_fixture;
	Box2DFixture::CategoryFlags m_categoryFixture = Box2DFixture::None;
	Box2DFixture::CategoryFlags m_categoryCollidesWith = CATEGORY_GROUND;
	Box2DFixture::CategoryFlags m_categoryRayCast = Box2DFixture::None;
	QMap<float32, QList<QPointer<Box2DFixture>>> m_rayCastFixtures;

	std::unique_ptr<QPropertyAnimation> m_dieAnimation;

};

#endif // GAMEPLAYERPRIVATE_H
