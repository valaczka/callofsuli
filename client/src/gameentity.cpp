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

#include <QTimer>

#include "application.h"
#include "gameentity.h"
#include "actiongame.h"
#include "utils.h"
#include "gamescene.h"

GameEntity::GameEntity(QQuickItem *parent)
	: GameObject(parent)
	, m_rayCast(new Box2DRayCast(this))
	, m_dieAnimation(new QPropertyAnimation(this, "opacity", this))
{
	m_body->setBodyType(Box2DBody::Dynamic);
	m_body->setGravityScale(5.0);
	m_body->setFixedRotation(true);

	connect(m_rayCast, &Box2DRayCast::fixtureReported, this, &GameEntity::onRayCastFixtureReported);

	connect(this, &GameObject::sceneConnected, this, &GameEntity::onSceneConnected);

	connect(this, &GameEntity::beginContact, this, &GameEntity::onBeginContact);
	connect(this, &GameEntity::endContact, this, &GameEntity::onEndContact);

	connect(this, &GameEntity::facingLeftChanged, this, &GameEntity::onFacingLeftChanged);

	connect(this, &GameEntity::isAliveChanged, this, [this](){
		QTimer::singleShot(2000, this, [this]() {
			onIsAliveDisabled();
		});
	});

	connect(m_dieAnimation, &QAbstractAnimation::finished, this, &GameEntity::deleteLater);

}


/**
 * @brief GameEntityPrivate::~GameEntityPrivate
 */

GameEntity::~GameEntity()
{
	delete m_dieAnimation;
	delete m_rayCast;

	if (m_fixture)
		delete m_fixture;
}


/**
 * @brief GameEntity::loadFromJsonFile
 * @return
 */

bool GameEntity::loadFromJsonFile()
{
	if (m_dataDir.isEmpty()) {
		qCWarning(lcScene).noquote() << tr("Missing entity data directory");
		return false;
	}

	return loadFromJsonFile(m_dataDir+"/data.json");
}



/**
 * @brief GameEntity::loadFromJsonFile
 * @param file
 * @return
 */

bool GameEntity::loadFromJsonFile(const QString &filename)
{
	bool error = false;

	m_dataObject = Utils::fileToJsonObject(filename, &error);


	if (error)
		return false;

	qCDebug(lcScene).noquote() << tr("Load entity data from:") << filename;

	QRectF r;

	r.setX(m_dataObject.value("x").toDouble(0));
	r.setY(m_dataObject.value("y").toDouble(0));
	r.setWidth(m_dataObject.value("width").toDouble(5));
	r.setHeight(m_dataObject.value("height").toDouble(5));

	setBodyRect(r.normalized());

	m_spriteFacingLeft = m_dataObject.value("facingLeft").toBool(false);

	onFacingLeftChanged();

	setWalkSize(m_dataObject.value("walk").toDouble(0));

	setShotSound(m_dataObject.value("shotSound").toString());

	m_sprites = m_dataObject.value("sprites").toObject();

	m_frameSize.setWidth(m_dataObject.value("frameWidth").toDouble(r.x()+r.width()));
	m_frameSize.setHeight(m_dataObject.value("frameHeight").toDouble(r.y()+r.height()));


	setWidth(m_frameSize.width());
	setHeight(m_frameSize.height());

	updateFixturesJson(sprite("idle"));

	return true;
}




/**
 * @brief GameEntity::doRayCast
 * @param point1
 * @param point2
 */


void GameEntity::doRayCast(const QPointF &point1, const QPointF &point2)
{
	rayCastFixtureCheck();

	if (!m_scene || !m_scene->world()) {
		qCWarning(lcScene).noquote() << tr("Missing scene or world, unable to ray cast");
		return;
	}

	QRectF r(point1, point2);

	QRectF r2 = mapRectFromItem(m_scene, r.normalized());

	emit rayCastPerformed(r2);

	m_scene->world()->rayCast(m_rayCast, point1, point2);
}


/**
 * @brief GameEntity::fixture
 * @return
 */

Box2DBox *GameEntity::fixture() const
{
	return m_fixture;
}


/**
 * @brief GameEntity::createSpriteItem
 * @return
 */

bool GameEntity::createSpriteItem()
{
	QQmlComponent component(Application::instance()->engine(), "qrc:/GameEntitySpriteSequence.qml", this);

	qCDebug(lcScene).noquote() << tr("Create sprite item:") << component.isReady();

	QQuickItem *item = qobject_cast<QQuickItem*>(component.create());

	if (item) {
		setSpriteItem(item);

		item->setParentItem(this);
		item->setProperty("entity", QVariant::fromValue(this));

		if (m_spriteSequence) {
			const QMetaObject *mo = m_spriteSequence->metaObject();

			auto p = mo->property(mo->indexOfProperty("currentSprite"));
			auto t = this->metaObject()->indexOfMethod("onSequenceCurrentSpriteChanged(QString)");

			if (p.hasNotifySignal()) {
				connect(m_spriteSequence, p.notifySignal(), this, this->metaObject()->method(t));
			}
		}

		addChildItem(item);

		loadSprites();
		return true;
	}

	qCWarning(lcScene).noquote() << tr("Can't create sprite item") << this;

	return false;
}


/**
 * @brief GameEntity::maxHp
 * @return
 */

int GameEntity::maxHp() const
{
	return m_maxHp;
}

void GameEntity::setMaxHp(int newMaxHp)
{
	if (m_maxHp == newMaxHp)
		return;
	m_maxHp = newMaxHp;
	emit maxHpChanged();
}


/**
 * @brief GameEntity::hp
 * @return
 */

int GameEntity::hp() const
{
	return m_hp;
}

void GameEntity::setHp(int newHp)
{
	if (m_hp == newHp)
		return;

	if (newHp < 0)
		newHp = 0;

	if (newHp > 0 && newHp < m_hp)
		emit hurt();

	m_hp = newHp;
	emit hpChanged();

	if (m_hp == 0)
		emit isAliveChanged();
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
	if (!m_fixture) {
		qCDebug(lcScene).noquote() << tr("Missing fixture");
		return qMakePair<QPointF, QPointF>(QPointF(.0,.0), QPointF(.0,.0));
	}

	QPointF point1;
	QPointF point2;

	qreal _y = m_fixture->y() + m_fixture->height() - m_rayCastElevation;

	point1.setY(_y);
	point2.setY(_y);

	if (m_facingLeft) {
		point1.setX(m_fixture->x() + m_fixture->width());
		point2.setX(m_fixture->x() - width);
	} else {
		point1.setX(m_fixture->x());
		point2.setX(m_fixture->x() + m_fixture->width() + width);
	}

	point1 += QPointF(x(), y());
	point2 += QPointF(x(), y());

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
 * @brief GameEntityPrivate::setFixtureVertices
 */

void GameEntity::updateFixtures(QString spriteName)
{
	if (!game() || !game()->running())
		return;

	if (spriteName.isEmpty() && m_spriteItem) {
		spriteName = m_spriteItem->property("currentSprite").toString();
	}

	updateFixturesJson(sprite(spriteName.isEmpty() ? "idle" : spriteName));
}





/**
 * @brief GameEntity::jumpToSprite
 * @param sprite
 */

void GameEntity::jumpToSprite(const QString &sprite)
{
	if (!m_spriteSequence) {
		qCWarning(lcScene).noquote() << tr("Missing sprite sequence");
		return;
	}

	QMetaObject::invokeMethod(m_spriteSequence, "jumpTo",
							  Q_ARG(QString, sprite)
							  );
}









/**
 * @brief GameEntityPrivate::onBoundBeginContact
 * @param other
 */

void GameEntity::onBeginContact(Box2DFixture *other)
{
	if (other->categories().testFlag(CATEGORY_GROUND)) {
		m_groundFixtures.append(other);

		emit isOnGroundChanged();

		if (other->property("baseGround").toBool()) {
			emit baseGroundContact();
		}
	}
}


/**
 * @brief GameEntityPrivate::onBoundEndContact
 * @param other
 */

void GameEntity::onEndContact(Box2DFixture *other)
{
	if (other->categories().testFlag(CATEGORY_GROUND)) {
		m_groundFixtures.removeAll(other);

		emit isOnGroundChanged();
	}
}




/**
 * @brief GameEntity::onRayCastTimerTimeout
 */

void GameEntity::onRayCastTimerTimeout()
{
	if (game() && game()->running() && m_rayCastEnabled && isAlive())
		doRayCast();
}





/**
 * @brief GameEntity::onRayCastFixtureReported
 * @param fixture
 * @param point
 * @param normal
 * @param fraction
 */

void GameEntity::onRayCastFixtureReported(Box2DFixture *fixture, const QPointF &, const QPointF &, qreal fraction)
{
	QQuickItem *item = fixture->getBody()->target();

	if (!item)
		return;

	GameEntity *e = qobject_cast<GameEntity *>(item);

	if (!e || !e->isAlive())
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
 * @brief GameEntity::onSequenceCurrentSpriteChanged
 * @param sprite
 */

void GameEntity::onSequenceCurrentSpriteChanged(QString sprite)
{
	m_lastCurrentSprite = sprite;
	emit currentSpriteChanged(sprite);
	updateFixtures(sprite);
}






/**
 * @brief GameEntity::onIsAliveChanged
 */

void GameEntity::onIsAliveDisabled()
{
	qCDebug(lcScene).noquote() << tr("Entity died") << this;

	emit died();

	m_dieAnimation->setDuration(150);
	m_dieAnimation->setStartValue(1.0);
	m_dieAnimation->setEndValue(.0);

	m_dieAnimation->start();
}



/**
 * @brief GameEntity::rayCastReport
 * @param items
 */

void GameEntity::rayCastReport(const QMultiMap<qreal, GameEntity *> &items)
{
	qCDebug(lcScene).noquote() << tr("Missing ray cast report implementation") << items;
}






/**
 * @brief GameEntity::updateFixtures
 * @param spriteData
 */

void GameEntity::updateFixturesJson(const QJsonObject &spriteData)
{
	if (!m_fixture) {
		qCDebug(lcScene).noquote() << tr("Create fixture for:") << this << m_categoryFixture << m_categoryCollidesWith;

		m_fixture = new Box2DBox(this);
		m_fixture->setDensity(1);
		m_fixture->setFriction(1);
		m_fixture->setRestitution(0);
		m_fixture->setCategories(m_categoryFixture);
		m_fixture->setCollidesWith(m_categoryCollidesWith);

		connect(this, &GameEntity::categoryFixtureChanged, m_fixture, &Box2DBox::setCategories);
		connect(this, &GameEntity::categoryCollidesWithChanged, m_fixture, &Box2DBox::setCollidesWith);

		connect(m_fixture, &Box2DBox::beginContact, this, &GameEntity::beginContact);
		connect(m_fixture, &Box2DBox::endContact, this, &GameEntity::endContact);

		m_body->addFixture(m_fixture);

		bodyComplete();
	}

	if (m_facingLeft == m_spriteFacingLeft)
		m_fixture->setX(m_bodyRect.x());
	else
		m_fixture->setX(width()-m_bodyRect.x()-m_bodyRect.width());

	m_fixture->setY(m_bodyRect.y());
	m_fixture->setWidth(m_bodyRect.width());
	m_fixture->setHeight(m_bodyRect.height());

	qreal w = qMax(spriteData.value("frameWidth").toDouble(), m_frameSize.width());
	qreal h = qMax(spriteData.value("frameHeight").toDouble(),  m_frameSize.height());

	if (m_spriteItem) {
		setWidth(w);
		setHeight(h);

		if (m_spriteSequence) {
			m_spriteSequence->setWidth(w);
			m_spriteSequence->setHeight(h);
		}

		if (!m_spritesLoaded)
			loadSprites();
	} else {
		qCDebug(lcScene).noquote() << tr("Missing sprite item") << this;
	}

}

const Box2DFixture::CategoryFlags &GameEntity::categoryRayCast() const
{
	return m_categoryRayCast;
}

void GameEntity::setCategoryRayCast(const Box2DFixture::CategoryFlags &newCategoryRayCast)
{
	if (m_categoryRayCast == newCategoryRayCast)
		return;
	m_categoryRayCast = newCategoryRayCast;
	emit categoryRayCastChanged(m_categoryRayCast);
}


/**
 * @brief GameEntity::categoryCollidesWith
 * @return
 */

const Box2DFixture::CategoryFlags &GameEntity::categoryCollidesWith() const
{
	return m_categoryCollidesWith;
}

void GameEntity::setCategoryCollidesWith(const Box2DFixture::CategoryFlags &newCategoryCollidesWith)
{
	if (m_categoryCollidesWith == newCategoryCollidesWith)
		return;

	m_categoryCollidesWith = newCategoryCollidesWith;
	emit categoryCollidesWithChanged(m_categoryCollidesWith);
}


/**
 * @brief GameEntity::categoryFixture
 * @return
 */

const Box2DFixture::CategoryFlags &GameEntity::categoryFixture() const
{
	return m_categoryFixture;
}

void GameEntity::setCategoryFixture(const Box2DFixture::CategoryFlags &newCategoryFixture)
{
	if (m_categoryFixture == newCategoryFixture)
		return;
	m_categoryFixture = newCategoryFixture;
	emit categoryFixtureChanged(m_categoryFixture);
}




/**
 * @brief GameEntity::sprites
 * @return
 */

const QJsonObject &GameEntity::sprites() const
{
	return m_sprites;
}


/**
 * @brief GameEntity::dataObject
 * @return
 */

const QJsonObject &GameEntity::dataObject() const
{
	return m_dataObject;
}




/**
 * @brief GameEntity::rayCastFixtureCheck
 */

void GameEntity::rayCastFixtureCheck()
{
	bool active = true;

	QMultiMap<qreal, GameEntity *> items;

	for (auto i = m_rayCastFixtures.constBegin(); i != m_rayCastFixtures.constEnd(); ++i) {
		foreach (Box2DFixture *fixture, i.value()) {
			if (!fixture)
				continue;

			Box2DFixture::CategoryFlags categories = fixture->categories();
			if (categories.testFlag(CATEGORY_GROUND) && !fixture->property("invisible").toBool()) {
				active = false;
			}

			if (!active)
				break;

			if (categories & m_categoryRayCast) {
				qreal fraction = i.key();
				GameEntity *e = qobject_cast<GameEntity*>(fixture->getBody()->target());

				items.insert(fraction, e);
			}
		}

		if (!active)
			break;
	}

	rayCastReport(items);

	m_rayCastFixtures.clear();
}


/**
 * @brief GameEntity::onFacingLeftChanged
 */

void GameEntity::onFacingLeftChanged()
{
	if (!m_spriteSequence)
		return;

	if (m_facingLeft != m_spriteFacingLeft)
		m_spriteSequence->setState("inverse");
	else
		m_spriteSequence->setState("");

	updateFixtures();
}


/**
 * @brief GameEntity::onSceneConnected
 */

void GameEntity::onSceneConnected()
{
	connect(this, &GameObject::timingTimerTimeout, this, &GameEntity::onRayCastTimerTimeout);
}


/**
 * @brief GameEntity::loadSprites
 */

void GameEntity::loadSprites()
{
	if (m_spritesLoaded)
		return;

	if (!m_spriteItem) {
		qCWarning(lcScene).noquote() << tr("Missing sprite item");
		return;
	}

	QQuickItem *sequence = qvariant_cast<QQuickItem*>(m_spriteItem->property("spriteSequence"));

	if (!sequence) {
		qCWarning(lcScene).noquote() << tr("Missing sprite sequence");
		return;
	}

	QStringList keys = m_sprites.keys();

	if (keys.isEmpty())
		return;

	keys.removeAll("idle");
	keys.prepend("idle");

	qCDebug(lcScene).noquote() << tr("Load sprites:") << keys;

	foreach (const QString &k, keys) {
		const QJsonObject &data = sprite(k);

		if (data.isEmpty())
			continue;

		QString dir = m_dataDir;

		if (dir.startsWith(":"))
			dir.replace(":", "qrc:");
		else if (!dir.startsWith("qrc:"))
			dir.prepend("qrc:");


		QVariantMap map({
							{ "name", k },
							{ "source", dir+"/"+data.value("source").toString() },
							{ "frameCount", data.value("frameCount").toInt(1) },
							{ "frameWidth", data.value("frameWidth").toInt(m_frameSize.width()) },
							{ "frameHeight", data.value("frameHeight").toInt(m_frameSize.height()) },
							{ "frameX", data.value("frameX").toInt(0) },
							{ "frameY", data.value("frameY").toInt(0) },
							{ "frameDuration", data.value("frameDuration").toInt(100) },
							{ "frameDurationVariation", data.value("frameDurationVariation").toInt(0) },
							{ "to", data.value("to").toObject().toVariantMap() }
						});

		QMetaObject::invokeMethod(m_spriteItem, "addToSprites",
								  Q_ARG(QVariant, map));

	}

	m_spritesLoaded = true;
}


/**
 * @brief GameEntity::spriteItem
 * @return
 */

QQuickItem *GameEntity::spriteItem() const
{
	return m_spriteItem;
}

void GameEntity::setSpriteItem(QQuickItem *newSpriteItem)
{
	if (m_spriteItem == newSpriteItem)
		return;
	m_spriteItem = newSpriteItem;
	emit spriteItemChanged();

	m_spriteSequence = qvariant_cast<QQuickItem*>(m_spriteItem->property("spriteSequence"));

	emit spriteSequenceChanged();
}





bool GameEntity::facingLeft() const
{
	return m_facingLeft;
}

void GameEntity::setFacingLeft(bool newFacingLeft)
{
	if (m_facingLeft == newFacingLeft)
		return;
	m_facingLeft = newFacingLeft;
	emit facingLeftChanged();
}

qreal GameEntity::rayCastLength() const
{
	return m_rayCastLength;
}

void GameEntity::setRayCastLength(qreal newRayCastLength)
{
	if (qFuzzyCompare(m_rayCastLength, newRayCastLength))
		return;
	m_rayCastLength = newRayCastLength;
	emit rayCastLengthChanged();
}

qreal GameEntity::rayCastElevation() const
{
	return m_rayCastElevation;
}

void GameEntity::setRayCastElevation(qreal newRayCastElevation)
{
	if (qFuzzyCompare(m_rayCastElevation, newRayCastElevation))
		return;
	m_rayCastElevation = newRayCastElevation;
	emit rayCastElevationChanged();
}

bool GameEntity::rayCastEnabled() const
{
	return m_rayCastEnabled;
}

void GameEntity::setRayCastEnabled(bool newRayCastEnabled)
{
	if (m_rayCastEnabled == newRayCastEnabled)
		return;
	m_rayCastEnabled = newRayCastEnabled;
	emit rayCastEnabledChanged();
}

qreal GameEntity::walkSize() const
{
	return m_walkSize;
}

void GameEntity::setWalkSize(qreal newWalkSize)
{
	if (qFuzzyCompare(m_walkSize, newWalkSize))
		return;
	m_walkSize = newWalkSize;
	emit walkSizeChanged();
}


/**
 * @brief GameEntity::bodyRect
 * @return
 */

const QRectF &GameEntity::bodyRect() const
{
	return m_bodyRect;
}

void GameEntity::setBodyRect(const QRectF &newBodyRect)
{
	if (m_bodyRect == newBodyRect)
		return;
	m_bodyRect = newBodyRect;
	emit bodyRectChanged();
}


/**
 * @brief GameEntity::sprite
 * @param key
 * @return
 */

QJsonObject GameEntity::sprite(const QString &key) const
{
	return m_sprites.value(key).toObject();
}





/**
 * @brief GameEntity::dataDir
 * @return
 */


const QString &GameEntity::dataDir() const
{
	return m_dataDir;
}

void GameEntity::setDataDir(const QString &newDataDir)
{
	if (m_dataDir == newDataDir)
		return;
	m_dataDir = newDataDir;
	emit dataDirChanged();
}




bool GameEntity::isOnGround() const
{
	return !m_groundFixtures.isEmpty();
}


/**
 * @brief GameEntity::shotSound
 * @return
 */

QUrl GameEntity::shotSound() const
{
	QString sound = m_shotSound.toString();

	if (m_shotSound.isEmpty())
		return m_defaultShotSound;

	if (sound.startsWith("qrc:"))
		return sound;

	if (sound.startsWith("/"))
		return "qrc:"+sound;

	QString dir = m_dataDir;

	if (dir.startsWith("qrc:"))
		return dir+"/"+sound;

	if (dir.startsWith(":"))
		return "qrc"+dir+"/"+sound;

	return "qrc:"+dir+"/"+sound;

}


/**
 * @brief GameEntity::setShotSound
 * @param newShotSound
 */

void GameEntity::setShotSound(const QUrl &newShotSound)
{
	if (m_shotSound == newShotSound)
		return;
	m_shotSound = newShotSound;
	emit shotSoundChanged(shotSound());
}

bool GameEntity::isAlive() const
{
	return (m_hp > 0);
}


bool GameEntity::glowEnabled() const
{
	return m_glowEnabled;
}

void GameEntity::setGlowEnabled(bool newGlowEnabled)
{
	if (m_glowEnabled == newGlowEnabled)
		return;
	m_glowEnabled = newGlowEnabled;
	emit glowEnabledChanged();
}

const QColor &GameEntity::glowColor() const
{
	return m_glowColor;
}

void GameEntity::setGlowColor(const QColor &newGlowColor)
{
	if (m_glowColor == newGlowColor)
		return;
	m_glowColor = newGlowColor;
	emit glowColorChanged();
}

bool GameEntity::overlayEnabled() const
{
	return m_overlayEnabled;
}

void GameEntity::setOverlayEnabled(bool newOverlayEnabled)
{
	if (m_overlayEnabled == newOverlayEnabled)
		return;
	m_overlayEnabled = newOverlayEnabled;
	emit overlayEnabledChanged();
}

const QColor &GameEntity::overlayColor() const
{
	return m_overlayColor;
}

void GameEntity::setOverlayColor(const QColor &newOverlayColor)
{
	if (m_overlayColor == newOverlayColor)
		return;
	m_overlayColor = newOverlayColor;
	emit overlayColorChanged();
}

bool GameEntity::hpProgressEnabled() const
{
	return m_hpProgressEnabled;
}

void GameEntity::setHpProgressEnabled(bool newHpProgressEnabled)
{
	if (m_hpProgressEnabled == newHpProgressEnabled)
		return;
	m_hpProgressEnabled = newHpProgressEnabled;
	emit hpProgressEnabledChanged();
}

const QColor &GameEntity::hpProgressColor() const
{
	return m_hpProgressColor;
}

void GameEntity::setHpProgressColor(const QColor &newHpProgressColor)
{
	if (m_hpProgressColor == newHpProgressColor)
		return;
	m_hpProgressColor = newHpProgressColor;
	emit hpProgressColorChanged();
}




/**
 * @brief GameEntity::kill
 */

void GameEntity::kill()
{
	emit killed();
	setHp(0);
}



/**
 * @brief GameEntity::spriteSequence
 * @return
 */


QQuickItem *GameEntity::spriteSequence() const
{
	return m_spriteSequence;
}
