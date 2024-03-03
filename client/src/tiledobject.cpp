/*
 * ---- Call of Suli ----
 *
 * tiledmapobject.cpp
 *
 * Created on: 2024. 02. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledMapObject
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

#include "tiledobject.h"
#include "application.h"
#include "Logger.h"
#include "box2dfixture.h"
#include "tiledscene.h"
#include <libtiled/maprenderer.h>





/**
 * @brief TiledObject::scene
 * @return
 */

TiledObjectBase::TiledObjectBase(QQuickItem *parent)
	: QQuickItem(parent)
	, m_body(new TiledObjectBody)
{
	LOG_CTRACE("scene") << "TiledObjectBase created" << this;

	m_body->setBodyType(Box2DBody::Static);
	m_body->setTarget(this);
	m_body->setActive(true);
	m_body->setSleepingAllowed(true);
}


/**
 * @brief TiledObjectBase::~TiledObjectBase
 */

TiledObjectBase::~TiledObjectBase()
{
	LOG_CTRACE("scene") << "TiledObjectBase destroyed" << this;
}



/**
 * @brief TiledObjectBase::scene
 * @return
 */

TiledScene *TiledObjectBase::scene() const
{
	return m_scene;
}

void TiledObjectBase::setScene(TiledScene *newScene)
{
	if (m_scene == newScene)
		return;
	m_scene = newScene;
	emit sceneChanged();

	if (m_scene) {
		setParentItem(m_scene);
		if (m_scene->world())
			m_body->setWorld(m_scene->world());
		onSceneConnected();
	}
}




/**
 * @brief TiledObjectBase::toPolygon
 * @param object
 * @param renderer
 * @return
 */

QPolygonF TiledObjectBase::toPolygon(const Tiled::MapObject *object, Tiled::MapRenderer *renderer)
{
	if (!object) {
		LOG_CERROR("scene") << "Empty Tiled::MapObject";
		return {};
	}

	if (!renderer) {
		LOG_CERROR("scene") << "Missing Tiled::MapRenderer";
		return {};
	}

	QPolygonF polygon = object->polygon().translated(object->position());

	if (object->shape() == Tiled::MapObject::Polygon && !polygon.isClosed() && !polygon.isEmpty())
		polygon << polygon.first();

	return renderer->pixelToScreenCoords(polygon);
}




/**
 * @brief TiledObjectBase::setPolygonVertices
 * @param fixture
 * @param polygon
 */

void TiledObjectBase::setPolygonVertices(Box2DPolygon *fixture, const QPolygonF &polygon) {
	if (!fixture)
		return;

	QVariantList vert;
	vert.reserve(polygon.size());

	for (const QPointF &f : polygon)
		vert.append(f);

	fixture->setVertices(vert);
}



/**
 * @brief TiledObjectBase::createSensorPolygon
 * @param length
 * @param range
 * @return
 */

TiledObjectSensorPolygon *TiledObjectBase::addSensorPolygon(const qreal &length, const qreal &range) {
	if (m_sensorPolygon || !m_body)
		return nullptr;

	m_sensorPolygon.reset(new TiledObjectSensorPolygon(m_body.get()));

	if (length > 0)
		m_sensorPolygon->setLength(length);
	if (range > 0)
		m_sensorPolygon->setRange(range);

	m_body->addFixture(m_sensorPolygon.get());

	return m_sensorPolygon.get();
}





/**
 * @brief TiledObject::TiledObject
 * @param parent
 */

TiledObject::TiledObject(QQuickItem *parent)
	: TiledObjectBase(parent)
{

}






/**
 * @brief TiledObject::jumpToSprite
 * @param sprite
 */

void TiledObject::jumpToSprite(const QString &sprite) const
{
	if (!m_spriteSequence) {
		LOG_CERROR("scene") << "Missing spriteSequence";
		return;
	}

	if (m_spriteSequence->property("currentSprite").toString() == sprite)
		return;

	QMetaObject::invokeMethod(m_spriteSequence, "jumpTo", Qt::DirectConnection,
							  Q_ARG(QString, sprite)
							  );
}



/**
 * @brief TiledObject::loadSprite
 * @param sprite
 * @return
 */

bool TiledObject::appendSprite(const QString &source, const TiledObjectSprite &sprite)
{
	Q_ASSERT(m_visualItem);

	if (m_availableSprites.contains(getSpriteName(sprite.name))) {
		LOG_CERROR("scene") << "Sprite already loaded:" << source << sprite.name;
		return false;
	}

	QVariantMap data = sprite.toJson().toVariantMap();

	if (source.startsWith(QStringLiteral(":/")))
		data[QStringLiteral("source")] = QUrl(QStringLiteral("qrc")+source);
	else
		data[QStringLiteral("source")] = QUrl(source);

	QMetaObject::invokeMethod(m_visualItem, "appendSprite", Qt::DirectConnection,
							  Q_ARG(QVariant, data));

	m_availableSprites.append(sprite.name);

	return true;
}



/**
 * @brief TiledObject::appendSpriteList
 * @param source
 * @param spriteList
 * @return
 */

bool TiledObject::appendSpriteList(const QString &source, const TiledObjectSpriteList &spriteList)
{
	bool r = true;

	const auto it = std::find_if(spriteList.sprites.constBegin(),
								 spriteList.sprites.constEnd(),
								 [](const TiledObjectSprite &s) {
		return s.name == QStringLiteral("default");
	});

	if (it != spriteList.sprites.constEnd())
		r &= appendSprite(source, *it);


	for (const TiledObjectSprite &s : spriteList.sprites) {
		if (s.name == QStringLiteral("default"))
			continue;

		r &= appendSprite(source, s);
	}

	return r;
}



/**
 * @brief TiledObject::appendSprite
 * @param path
 * @param sprite
 * @return
 */

bool TiledObject::appendSprite(const TiledMapObjectAlterableSprite &sprite, const QString &path)
{
	Q_ASSERT(m_visualItem);

	for (const TiledObjectSprite &s : sprite.sprites) {
		for (auto it = sprite.alterations.constBegin(); it != sprite.alterations.constEnd(); ++it) {
			const QString &alteration = it.key();
			const QString &spriteName = getSpriteName(s.name, alteration);

			if (m_availableSprites.contains(spriteName)) {
				LOG_CERROR("scene") << "Sprite already loaded:" << s.name << alteration << path << it.value();
				return false;
			}

			TiledObjectSprite s2 = s;
			s2.name = spriteName;
			QJsonObject to2;
			for (auto it = s.to.constBegin(); it != s.to.constEnd(); ++it) {
				to2.insert(getSpriteName(it.key(), alteration), it.value());
			}
			s2.to = to2;

			QVariantMap data = s2.toJson().toVariantMap();

			if (path.startsWith(QStringLiteral(":/")))
				data[QStringLiteral("source")] = QUrl(QStringLiteral("qrc")+path+it.value());
			else
				data[QStringLiteral("source")] = QUrl(path+it.value());

			QMetaObject::invokeMethod(m_visualItem, "appendSprite", Qt::DirectConnection,
									  Q_ARG(QVariant, data));

			LOG_CTRACE("scene") << "Append sprite" << data;

			m_availableSprites.append(spriteName);
		}
	}

	m_availableAlterations.append(sprite.alterations.keys());

	return true;
}


/**
 * @brief TiledObject::appendSpriteList
 * @param path
 * @param spriteList
 * @return
 */

bool TiledObject::appendSpriteList(const TiledObjectAlterableSpriteList &spriteList, const QString &path)
{
	Q_ASSERT(m_visualItem);

	for (const auto &s : spriteList.list) {
		if (!appendSprite(s, path)) {
			LOG_CERROR("scene") << "Load sprite error:" << path;
			return false;
		}
	}

	return true;
}


/**
 * @brief TiledObject::getSpriteName
 * @param sprite
 * @param alteration
 * @return
 */

QString TiledObject::getSpriteName(const QString &sprite, const QString &alteration)
{
	if (alteration.isEmpty())
		return sprite;
	else
		return alteration+QStringLiteral("-").append(sprite);
}


/**
 * @brief TiledObject::createVisual
 */

void TiledObject::createVisual()
{
	Q_ASSERT(!m_visualItem);

	setTransformOrigin(QQuickItem::Center);
	setTransformOriginPoint(QPointF(0,0));

	QQmlComponent component(Application::instance()->engine(), QStringLiteral("qrc:/TiledObjectVisual.qml"), this);

	LOG_CDEBUG("scene") << "Create sprite item:" << component.isReady();

	m_visualItem = qobject_cast<QQuickItem*>(component.create());

	if (!m_visualItem) {
		LOG_CERROR("scene") << "TiledObject createVisual error" << component.errorString();
		return;
	}

	m_spriteSequence = qvariant_cast<QQuickItem*>(m_visualItem->property("spriteSequence"));

	m_visualItem->setParentItem(this);
	m_visualItem->setProperty("baseObject", QVariant::fromValue(this));

	if (!m_spriteSequence) {
		LOG_CERROR("scene") << "TiledObject SpriteSequence error";
		return;
	}

	const QMetaObject *mo = m_spriteSequence->metaObject();

	auto p = mo->property(mo->indexOfProperty("currentSprite"));
	auto t = this->metaObject()->indexOfMethod("onCurrentSpriteChanged(QString)");

	if (p.hasNotifySignal()) {
		connect(m_spriteSequence, p.notifySignal(), this, this->metaObject()->method(t));
	}
}

QStringList TiledObject::availableAlterations() const
{
	return m_availableAlterations;
}

QStringList TiledObject::availableSprites() const
{
	return m_availableSprites;
}



/**
 * @brief TiledObject::onCurrentSpriteChanged
 * @param sprite
 */

void TiledObject::onCurrentSpriteChanged(QString sprite)
{
	//LOG_CTRACE("scene") << "Current sprite" << sprite;
}



/**
 * @brief TiledObjectSensorPolygon::length
 * @return
 */

TiledObjectSensorPolygon::TiledObjectSensorPolygon(Box2DBody *body, QQuickItem *parent)
	: Box2DPolygon(parent)
	, m_virtualCircle(new Box2DCircle)
	, m_body(body)
{
	Q_ASSERT(m_body);

	setSensor(true);

	m_virtualCircle->setSensor(true);
	m_virtualCircle->setCollidesWith(Box2DFixture::None);
	m_virtualCircle->setCategories(Box2DFixture::None);

	m_body->addFixture(m_virtualCircle.get());

	recreateFixture();
}



/**
 * @brief TiledObjectSensorPolygon::length
 * @return
 */

qreal TiledObjectSensorPolygon::length() const
{
	return m_length;
}

void TiledObjectSensorPolygon::setLength(qreal newLength)
{
	if (qFuzzyCompare(m_length, newLength))
		return;
	m_length = newLength;
	emit lengthChanged();
	recreateFixture();
}


/**
 * @brief TiledObjectSensorPolygon::range
 * @return
 */

qreal TiledObjectSensorPolygon::range() const
{
	return m_range;
}

void TiledObjectSensorPolygon::setRange(qreal newRange)
{
	if (qFuzzyCompare(m_range, newRange))
		return;
	m_range = newRange;
	emit rangeChanged();
	recreateFixture();
}






/**
 * @brief TiledObjectSensorPolygon::recreateFixture
 */

void TiledObjectSensorPolygon::recreateFixture()
{
	QPolygonF polygon;
	polygon.append(QPointF(0,0));

	for (int i=0; i < 7; ++i) {
		qreal angle = -(m_range/2.) + (i/6. * m_range);
		if (angle > M_PI)
			angle = -M_PI+(angle-M_PI);
		else if (angle < -M_PI)
			angle = M_PI-(-M_PI-angle);
		polygon.append(QPointF(m_length * cosf(angle), m_length * -sinf(angle)));
	}

	TiledObjectBase::setPolygonVertices(this, polygon);

	m_virtualCircle->setX(-m_length);
	m_virtualCircle->setY(-m_length);
	m_virtualCircle->setRadius(m_length);

}



/**
 * @brief TiledObjectSensorPolygon::virtualCircle
 * @return
 */

Box2DCircle *TiledObjectSensorPolygon::virtualCircle() const
{
	return m_virtualCircle.get();
}




/**
 * @brief TiledObjectBody::synchronize
 */

void TiledObjectBody::synchronize()
{
	Q_ASSERT(mBody);

	if (mTarget) {
		if (TiledObjectBase *object = qobject_cast<TiledObjectBase*>(mTarget)) {
			if (object->m_sensorPolygon) {
				b2Vec2 &value = mBodyDef.position;
				const b2Vec2 &newValue = mBody->GetPosition();

				if (qFuzzyCompare(value.x, newValue.x) && qFuzzyCompare(value.y, newValue.y))
					return;

				value = newValue;

				QPointF bodyOffset(0, 90*0.3);			/// ???

				QPointF offset(mTarget->width()/2, mTarget->height()/2);

				mTarget->setPosition(mWorld->toPixels(mBodyDef.position) - offset - bodyOffset);

				emit positionChanged();

				return;
			}
		}
	}

	Box2DBody::synchronize();
}




/**
 * @brief TiledObjectPolygonIface::createFixture
 * @param polygon
 * @return
 */

Box2DPolygon *TiledObjectPolygonIface::createFixture(const QPointF &pos, const QPolygonF &polygon)
{
	Q_ASSERT(m_fixture);

	//const QRectF &box = polygon.boundingRect();			// ???

	TiledObjectBase::setPolygonVertices(m_fixture.get(), polygon.translated(pos/*-box.center()*/));

	return m_fixture.get();
}



/**
 * @brief TiledObjectCircleIface::createFixture
 * @param pos
 * @param size
 * @return
 */

Box2DCircle *TiledObjectCircleIface::createFixture(const QPointF &pos, const qreal &size)
{
	Q_ASSERT(m_fixture);

	m_fixture->setX(pos.x()-size/2.);
	m_fixture->setY(pos.y()-size/2.);
	m_fixture->setRadius(size);

	return m_fixture.get();
}















/**
 * @brief IsometricObjectIface::appendSprite
 * @param sprite
 * @param source
 * @return
 */

bool TiledObject::appendSprite(const QString &source, const IsometricObjectSprite &sprite)
{
	Q_ASSERT(m_visualItem);

	for (int i=0; i<sprite.directions.size(); ++i) {
		const int n = sprite.directions.at(i);
		IsometricObjectIface::Direction direction = QVariant::fromValue(n).value<IsometricObjectIface::Direction>();

		if (direction == IsometricObjectIface::Invalid) {
			LOG_CERROR("scene") << "Sprite invalid direction:" << n << source;
			return false;
		}

		TiledObjectSprite s2 = sprite;

		if (sprite.startColumn >= 0) {
			s2.frameX = sprite.startColumn + i*sprite.frameWidth;
		} else if (sprite.startRow >= 0) {
			s2.frameY = sprite.startRow + i*sprite.frameHeight;
		}

		const QString &spriteName = getSpriteName(sprite.name, direction);

		if (m_availableSprites.contains(spriteName)) {
			LOG_CERROR("scene") << "Sprite already loaded:" << sprite.name << direction << source;
			return false;
		}

		s2.name = spriteName;
		QJsonObject to2;
		for (auto it = sprite.to.constBegin(); it != sprite.to.constEnd(); ++it) {
			to2.insert(getSpriteName(it.key(), direction), it.value());
		}
		s2.to = to2;

		QVariantMap data = s2.toJson().toVariantMap();

		if (source.startsWith(QStringLiteral(":/")))
			data[QStringLiteral("source")] = QUrl(QStringLiteral("qrc")+source);
		else
			data[QStringLiteral("source")] = QUrl(source);

		QMetaObject::invokeMethod(m_visualItem, "appendSprite", Qt::DirectConnection,
								  Q_ARG(QVariant, data));

		LOG_CTRACE("scene") << "Append sprite" << data;

		m_availableSprites.append(spriteName);
	}

	return true;
}




/**
 * @brief IsometricObjectIface::appendSpriteList
 * @param source
 * @param spriteList
 * @return
 */

bool TiledObject::appendSpriteList(const QString &source, const IsometricObjectSpriteList &spriteList)
{
	Q_ASSERT(m_visualItem);

	for (const auto &s : spriteList.sprites) {
		if (!appendSprite(source, s)) {
			LOG_CERROR("scene") << "Load sprite error:" << source;
			return false;
		}
	}

	return true;
}



/**
 * @brief IsometricObjectIface::appendSprite
 * @param sprite
 * @param path
 * @return
 */

bool TiledObject::appendSprite(const IsometricObjectAlterableSprite &sprite, const QString &path)
{
	Q_ASSERT(m_visualItem);

	for (const IsometricObjectSprite &s : sprite.sprites) {
		for (auto it = sprite.alterations.constBegin(); it != sprite.alterations.constEnd(); ++it) {
			const QString &alteration = it.key();

			for (int i=0; i<s.directions.size(); ++i) {
				const int n = s.directions.at(i);
				IsometricObjectIface::Direction direction = QVariant::fromValue(n).value<IsometricObjectIface::Direction>();

				if (direction == IsometricObjectIface::Invalid) {
					LOG_CERROR("scene") << "Sprite invalid direction:" << n << path;
					return false;
				}

				const QString &spriteName = getSpriteName(s.name, direction, alteration);

				if (m_availableSprites.contains(spriteName)) {
					LOG_CERROR("scene") << "Sprite already loaded:" << s.name << alteration << path << it.value();
					return false;
				}

				TiledObjectSprite s2 = s;

				if (s.startColumn >= 0) {
					s2.frameX = s.startColumn + i*s.frameWidth;
				} else if (s.startRow >= 0) {
					s2.frameY = s.startRow + i*s.frameHeight;
				}

				s2.name = spriteName;
				QJsonObject to2;
				for (auto it = s.to.constBegin(); it != s.to.constEnd(); ++it) {
					to2.insert(getSpriteName(it.key(), direction, alteration), it.value());
				}
				s2.to = to2;

				QVariantMap data = s2.toJson().toVariantMap();

				if (path.startsWith(QStringLiteral(":/")))
					data[QStringLiteral("source")] = QUrl(QStringLiteral("qrc")+path+it.value());
				else
					data[QStringLiteral("source")] = QUrl(path+it.value());

				QMetaObject::invokeMethod(m_visualItem, "appendSprite", Qt::DirectConnection,
										  Q_ARG(QVariant, data));

				///LOG_CTRACE("scene") << "Append sprite" << data;

				m_availableSprites.append(spriteName);
			}
		}
	}

	m_availableAlterations.append(sprite.alterations.keys());

	return true;
}



/**
 * @brief IsometricObjectIface::appendSpriteList
 * @param sprite
 * @param path
 * @return
 */

bool TiledObject::appendSpriteList(const IsometricObjectAlterableSpriteList &sprite, const QString &path)
{
	Q_ASSERT(m_visualItem);

	for (const auto &s : sprite.list) {
		if (!appendSprite(s, path)) {
			LOG_CERROR("scene") << "Load sprite error:" << path;
			return false;
		}
	}

	return true;
}


/**
 * @brief IsometricObjectIface::getSpriteName
 * @param sprite
 * @param direction
 * @param alteration
 * @return
 */

QString TiledObject::getSpriteName(const QString &sprite, const IsometricObjectIface::Direction &direction, const QString &alteration)
{
	QString s = TiledObject::getSpriteName(sprite, alteration);

	if (direction != IsometricObjectIface::Invalid)
		s.append(QStringLiteral("-")).append(QString::number(direction));

	return s;
}

