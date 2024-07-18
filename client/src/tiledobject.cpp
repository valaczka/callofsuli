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
#include "tiledspritehandler.h"
#include <libtiled/maprenderer.h>



/**
 * @brief TiledObject::scene
 * @return
 */

TiledObjectBase::TiledObjectBase(QQuickItem *parent)
	: QQuickItem(parent)
	, m_body(new TiledObjectBody(this))
{
	LOG_CTRACE("scene") << "TiledObjectBase created" << this << parentItem() << QQuickItem::parent();

	m_body->setBodyType(Box2DBody::Static);
	m_body->setTarget(this);
	m_body->setActive(true);
	m_body->setSleepingAllowed(true);

	connect(this, &TiledObjectBase::xChanged, this, &TiledObjectBase::onSceneVisibleAreaChanged);
	connect(this, &TiledObjectBase::yChanged, this, &TiledObjectBase::onSceneVisibleAreaChanged);
	connect(this, &TiledObjectBase::widthChanged, this, &TiledObjectBase::onSceneVisibleAreaChanged);
	connect(this, &TiledObjectBase::heightChanged, this, &TiledObjectBase::onSceneVisibleAreaChanged);
}


/**
 * @brief TiledObjectBase::~TiledObjectBase
 */

TiledObjectBase::~TiledObjectBase()
{
	LOG_CTRACE("scene") << "TiledObjectBase destroyed" << this << parentItem() << parent();
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

	if (m_scene)
		disconnect(m_scene, &TiledScene::visibleAreaChanged, this, &TiledObjectBase::onSceneVisibleAreaChanged);

	m_scene = newScene;
	emit sceneChanged();

	if (m_scene) {
		setParentItem(m_scene);
		if (m_scene->world())
			m_body->setWorld(m_scene->world());

		connect(m_scene, &TiledScene::visibleAreaChanged, this, &TiledObjectBase::onSceneVisibleAreaChanged);
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
 * @brief TiledObjectBase::toPoint
 * @param angle
 * @param radius
 * @return
 */

QPointF TiledObjectBase::toPoint(const qreal &angle, const qreal &radius)
{
	QPointF p;
	p.setX(radius * cos(angle));
	p.setY(radius * -sin(angle));
	return p;
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

	for (const QPointF &f : std::as_const(polygon))
		vert.append(f);

	fixture->setVertices(vert);
}



/**
 * @brief TiledObjectBase::normalizeFromRadian
 * @param radian
 * @return
 */

float32 TiledObjectBase::normalizeFromRadian(const float32 &radian)
{
	if (radian < -b2_pi || radian > b2_pi) {
		LOG_CTRACE("scene") << "Invalid radian:" << radian;
		return M_PI;
	}

	if (radian < 0)
		return M_PI+M_PI+radian;
	else
		return radian;
}


/**
 * @brief TiledObjectBase::normalizeToRadian
 * @param normal
 * @return
 */

float32 TiledObjectBase::normalizeToRadian(const float32 &normal)
{
	if (normal < 0 || normal > 2*b2_pi) {
		LOG_CTRACE("scene") << "Invalid normalized radian:" << normal;
		return 0.;
	}

	if (normal > b2_pi)
		return normal-2*b2_pi;
	else
		return normal;
}





/**
 * @brief TiledObjectBase::getFromFixture
 * @param fixture
 * @return
 */

TiledObjectBase *TiledObjectBase::getFromFixture(const Box2DFixture *fixture)
{
	if (!fixture)
		return nullptr;

	Box2DBody *body = fixture->getBody();

	TiledObjectBody *b = dynamic_cast<TiledObjectBody*>(body);

	if (!b)
		return nullptr;

	return b->baseObject();
}



/**
 * @brief TiledObjectBase::setBodyOffset
 * @param newBodyOffset
 */

void TiledObjectBase::setBodyOffset(QPointF newBodyOffset)
{
	m_body->setBodyOffset(newBodyOffset);
	recalculateTargetCircle();
}



/**
 * @brief TiledObjectBase::rotateBody
 */

bool TiledObjectBase::rotateBody(const float32 &desiredRadian)
{
	if (!m_sensorPolygon)
		return false;

	const float32 currentAngle = m_body->body()->GetAngle();


	if (qFuzzyCompare(desiredRadian, currentAngle)) {
		if (m_rotateAnimation.running) {
			m_rotateAnimation.running = false;
		}
		return false;
	}

	const float32 currentNormal = normalizeFromRadian(currentAngle);
	const float32 desiredNormal = normalizeFromRadian(desiredRadian);


	if (!qFuzzyCompare(m_rotateAnimation.destAngle, desiredRadian) || !m_rotateAnimation.running) {
		m_rotateAnimation.destAngle = desiredRadian;

		const float32 diff = std::abs(currentNormal-desiredNormal);

		if (desiredNormal > currentNormal)
			m_rotateAnimation.clockwise = diff > b2_pi;
		else
			m_rotateAnimation.clockwise = diff < b2_pi;

		m_rotateAnimation.running = true;
	}


	if (!m_rotateAnimation.running)
		return false;


	const float32 delta = std::min(m_rotateAnimation.speed * m_body->world()->timeStep()*60., M_PI_4);
	float32 newAngle = m_rotateAnimation.clockwise ? currentNormal - delta : currentNormal + delta;

	m_body->setAngularVelocity(0);


	static const float32 pi2 = 2*b2_pi;

	// 0 átlépés miatt kell
	const float32 d = (desiredNormal == 0 && newAngle > pi2) ? pi2 : desiredNormal;


	if ((newAngle >= d && currentNormal < d) ||
			(newAngle <= d && currentNormal > d)) {
		m_body->body()->SetTransform(m_body->body()->GetPosition(), desiredRadian );
		m_body->setAwake(true);
		return true;
	}

	if (newAngle > pi2)
		newAngle -= pi2;
	else if (newAngle < 0)
		newAngle += pi2;

	m_body->body()->SetTransform(m_body->body()->GetPosition(), normalizeToRadian(newAngle) );
	m_body->setAwake(true);

	return true;
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
 * @brief TiledObjectBase::addTargetCircle
 * @return
 */

Box2DCircle *TiledObjectBase::addTargetCircle(const qreal &radius)
{
	if (m_targetCircle || !m_body)
		return nullptr;

	m_targetCircle.reset(new Box2DCircle);


	m_targetCircle->setCollidesWith(Box2DFixture::All);
	m_targetCircle->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTarget));
	m_targetCircle->setSensor(true);

	m_targetCircle->setRadius(radius);
	/*m_targetCircle->setX(-radius);
	m_targetCircle->setY(-radius);*/

	m_body->addFixture(m_targetCircle.get());

	recalculateTargetCircle();

	return m_targetCircle.get();
}



/**
 * @brief TiledObjectBase::onSceneVisibleAreaChanged
 */

void TiledObjectBase::onSceneVisibleAreaChanged()
{
	if (!m_scene)
		return;

	QRectF rect(x(), y(), width(), height());

	setInVisibleArea(m_scene->visibleArea().intersects(rect));
}

QString TiledObjectBase::displayName() const
{
	return m_displayName;
}

void TiledObjectBase::setDisplayName(const QString &newDisplayName)
{
	if (m_displayName == newDisplayName)
		return;
	m_displayName = newDisplayName;
	emit displayNameChanged();
}


/**
 * @brief TiledObjectBase::recalculateTargetCircle
 */

void TiledObjectBase::recalculateTargetCircle()
{
	if (!m_targetCircle)
		return;

	const qreal &r = m_targetCircle->radius();

	/*if (m_body) {
		const QPointF &p = m_body->m_bodyOffset;
		m_targetCircle->setX(-r -p.x());
		m_targetCircle->setY(-r -p.y());
	} else {*/
	m_targetCircle->setX(-r);
	m_targetCircle->setY(-r);
	//}
}




/**
 * @brief TiledObjectBase::game
 * @return
 */

TiledGame *TiledObjectBase::game() const
{
	return m_game;
}

void TiledObjectBase::setGame(TiledGame *newGame)
{
	if (m_game == newGame)
		return;
	m_game = newGame;
	emit gameChanged();
}

const TiledObjectBase::ObjectId &TiledObjectBase::objectId() const
{
	return m_objectId;
}

void TiledObjectBase::setObjectId(const ObjectId &newObjectId)
{
	m_objectId = newObjectId;
}



/**
 * @brief TiledObjectBase::inVisibleArea
 * @return
 */

bool TiledObjectBase::inVisibleArea() const
{
	return m_inVisibleArea;
}

void TiledObjectBase::setInVisibleArea(bool newInVisibleArea)
{
	if (m_inVisibleArea == newInVisibleArea)
		return;
	m_inVisibleArea = newInVisibleArea;
	emit inVisibleAreaChanged();
}



QColor TiledObjectBase::overlayColor() const
{
	return m_overlayColor;
}

void TiledObjectBase::setOverlayColor(const QColor &newOverlayColor)
{
	if (m_overlayColor == newOverlayColor)
		return;
	m_overlayColor = newOverlayColor;
	emit overlayColorChanged();
}

QColor TiledObjectBase::glowColor() const
{
	return m_glowColor;
}

void TiledObjectBase::setGlowColor(const QColor &newGlowColor)
{
	if (m_glowColor == newGlowColor)
		return;
	m_glowColor = newGlowColor;
	emit glowColorChanged();
}

bool TiledObjectBase::overlayEnabled() const
{
	return m_overlayEnabled;
}

void TiledObjectBase::setOverlayEnabled(bool newOverlayEnabled)
{
	if (m_overlayEnabled == newOverlayEnabled)
		return;
	m_overlayEnabled = newOverlayEnabled;
	emit overlayEnabledChanged();
}

bool TiledObjectBase::glowEnabled() const
{
	return m_glowEnabled;
}

void TiledObjectBase::setGlowEnabled(bool newGlowEnabled)
{
	if (m_glowEnabled == newGlowEnabled)
		return;
	m_glowEnabled = newGlowEnabled;
	emit glowEnabledChanged();
}



/**
 * @brief TiledObjectBase::remoteMode
 * @return
 */

TiledObject::RemoteMode TiledObjectBase::remoteMode() const
{
	return m_remoteMode;
}

void TiledObjectBase::setRemoteMode(const RemoteMode &newRemoteMode)
{
	if (m_remoteMode == newRemoteMode)
		return;
	m_remoteMode = newRemoteMode;
	emit remoteModeChanged();
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
 * @brief TiledObject::~TiledObject
 */

TiledObject::~TiledObject()
{
	if (m_spriteHandler) {
		m_spriteHandler->setBaseObject(nullptr);
	}
}








/**
 * @brief TiledObject::jumpToSprite
 * @param sprite
 */

void TiledObject::jumpToSprite(const char *sprite, const Direction &direction) const
{
	if (!m_spriteHandler) {
		LOG_CERROR("scene") << "Missing spriteHandler";
		return;
	}

	/*if (m_spriteHandler->currentSprite() != sprite || m_spriteHandler->currentDirection() != direction)
		LOG_CTRACE("scene") << "[SPRITE]" << this << sprite << direction;*/

	m_spriteHandler->jumpToSprite(sprite, direction, TiledSpriteHandler::JumpImmediate);
}


/**
 * @brief TiledObject::jumpToSpriteLater
 * @param sprite
 * @param alteration
 */

void TiledObject::jumpToSpriteLater(const char *sprite, const Direction &direction) const
{
	if (!m_spriteHandler) {
		LOG_CERROR("scene") << "Missing spriteHandler";
		return;
	}

	m_spriteHandler->jumpToSprite(sprite, direction, TiledSpriteHandler::JumpAtFinished);
}



/**
 * @brief TiledObject::loadSprite
 * @param sprite
 * @return
 */

bool TiledObject::appendSprite(const QString &source, const TiledObjectSprite &sprite)
{
	Q_ASSERT(m_spriteHandler);

	if (m_spriteHandler->spriteNames().contains(sprite.name)) {
		LOG_CERROR("scene") << "Sprite already loaded:" << source << sprite.name;
		return false;
	}

	return m_spriteHandler->addSprite(sprite, QStringLiteral("default"), source);
}



/**
 * @brief TiledObject::appendSprite
 * @param source
 * @param spriteList
 * @return
 */

bool TiledObject::appendSprite(const QString &source, const TiledObjectSpriteList &spriteList)
{
	bool r = true;

	for (const TiledObjectSprite &s : std::as_const(spriteList.sprites))
		r &= m_spriteHandler->addSprite(s, QStringLiteral("default"), source);

	return r;
}



/**
 * @brief TiledObject::appendSprite
 * @param path
 * @param sprite
 * @return
 */

bool TiledObject::appendSprite(const TiledMapObjectLayeredSprite &sprite, const QString &path)
{
	Q_ASSERT(m_visualItem);

	bool r = true;

	for (const TiledObjectSprite &s : std::as_const(sprite.sprites)) {
		for (auto it = sprite.layers.constBegin(); it != sprite.layers.constEnd(); ++it) {
			const QString &alteration = it.key();
			r &= m_spriteHandler->addSprite(s, alteration, path+it.value());
		}
	}

	//m_availableAlterations.append(sprite.layers.keys());

	return r;
}


/**
 * @brief TiledObject::appendSprite
 * @param path
 * @param spriteList
 * @return
 */

bool TiledObject::appendSprite(const TiledObjectLayeredSpriteList &spriteList, const QString &path)
{
	Q_ASSERT(m_visualItem);

	for (const auto &s : std::as_const(spriteList.list)) {
		if (!appendSprite(s, path)) {
			LOG_CERROR("scene") << "Load sprite error:" << path;
			return false;
		}
	}

	return true;
}







/**
 * @brief TiledObject::createVisual
 */

void TiledObject::createVisual()
{
	Q_ASSERT(!m_visualItem);

	QQmlComponent component(Application::instance()->engine(), QStringLiteral("qrc:/TiledObjectVisual.qml"), this);

	m_visualItem = qobject_cast<QQuickItem*>(component.create());

	if (!m_visualItem) {
		LOG_CERROR("scene") << "TiledObject createVisual error" << component.errorString();
		return;
	}

	m_spriteHandler = qvariant_cast<TiledSpriteHandler*>(m_visualItem->property("spriteHandler"));
	Q_ASSERT(m_spriteHandler);

	m_spriteHandlerAuxFront = qvariant_cast<TiledSpriteHandler*>(m_visualItem->property("spriteHandlerAuxFront"));
	m_spriteHandlerAuxBack = qvariant_cast<TiledSpriteHandler*>(m_visualItem->property("spriteHandlerAuxBack"));

	m_visualItem->setParentItem(this);
	m_visualItem->setParent(this);
	m_visualItem->setProperty("baseObject", QVariant::fromValue(this));
}



/**
 * @brief TiledObject::createMarkerItem
 */

void TiledObject::createMarkerItem()
{
	QQmlComponent component(Application::instance()->engine(), QStringLiteral("qrc:/TiledPlayerMarker.qml"), this);

	QQuickItem *item = qobject_cast<QQuickItem*>(component.createWithInitialProperties(
													 QVariantMap{
														 { QStringLiteral("target"), QVariant::fromValue(this) }
													 }));

	if (!item) {
		LOG_CERROR("scene") << "TiledPlayerMarker error" << component.errorString();
		return;
	}

	item->setParent(this);
}



/**
 * @brief TiledObject::rotateToPoint
 * @param point
 * @param anglePtr
 * @param distancePtr
 */

void TiledObject::rotateToPoint(const QPointF &point, float32 *anglePtr, qreal *distancePtr)
{
	const QPointF p = point - m_body->bodyPosition();

	float32 angle = atan2(-p.y(), p.x());

	setCurrentDirection(nearestDirectionFromRadian(angle));
	m_body->body()->SetTransform(m_body->body()->GetPosition(), angle);
	m_body->setAwake(true);

	if (anglePtr)
		*anglePtr = angle;

	if (distancePtr)
		*distancePtr = QVector2D(p).length();
}



/**
 * @brief TiledObject::angleToPoint
 * @param point
 * @return
 */

float32 TiledObject::angleToPoint(const QPointF &point) const
{
	const QPointF p = point - m_body->bodyPosition();
	return atan2(-p.y(), p.x());
}


/**
 * @brief TiledObject::distanceToPoint
 * @param point
 * @return
 */

qreal TiledObject::distanceToPoint(const QPointF &point) const
{
	return QVector2D(point - m_body->bodyPosition()).length();
}



TiledSpriteHandler *TiledObject::spriteHandlerAuxBack() const
{
	return m_spriteHandlerAuxBack;
}

TiledSpriteHandler *TiledObject::spriteHandlerAuxFront() const
{
	return m_spriteHandlerAuxFront;
}

TiledSpriteHandler *TiledObject::spriteHandler() const
{
	return m_spriteHandler;
}

TiledObject::Directions TiledObject::availableDirections() const
{
	return m_availableDirections;
}

void TiledObject::setAvailableDirections(const Directions &newAvailableDirections)
{
	if (m_availableDirections == newAvailableDirections)
		return;
	m_availableDirections = newAvailableDirections;
	emit availableDirectionsChanged();
}

TiledObject::Direction TiledObject::currentDirection() const
{
	return m_currentDirection;
}

void TiledObject::setCurrentDirection(const Direction &newCurrentDirection)
{
	if (m_currentDirection == newCurrentDirection)
		return;
	m_currentDirection = newCurrentDirection;
	emit currentDirectionChanged();
}


/**
 * @brief TiledObject::toRadian
 * @param angle
 * @return
 */

qreal TiledObject::toRadian(const qreal &angle)
{
	if (angle <= 180.)
		return angle * M_PI / 180.;
	else
		return -(360-angle) * M_PI / 180.;
}



/**
 * @brief TiledObject::toDegree
 * @param angle
 * @return
 */

qreal TiledObject::toDegree(const qreal &angle)
{
	if (angle >= 0)
		return angle * 180 / M_PI;
	else
		return 360+(angle * 180 / M_PI);
}



/**
 * @brief TiledObject::directionToIsometricRaidan
 * @param direction
 * @return
 */

qreal TiledObject::directionToIsometricRaidan(const Direction &direction)
{
	switch (direction) {
		case West: return M_PI; break;
		case NorthWest: return M_PI - atan(0.5); break;
		case North: return M_PI_2; break;
		case NorthEast: return atan(0.5); break;
		case East: return 0; break;
		case SouthEast: return -atan(0.5); break;
		case South: return -M_PI_2; break;
		case SouthWest: return -M_PI + atan(0.5); break;
		case Invalid: return 0; break;
	}

	return 0;
}


/**
 * @brief TiledObject::factorFromDegree
 * @param angle
 * @param xyRatio
 * @return
 */

qreal TiledObject::factorFromDegree(const qreal &angle, const qreal &xyRatio)
{
	return factorFromRadian(toRadian(angle), xyRatio);
}



/**
 * @brief TiledObject::factorFromRadian
 * @param angle
 * @param xyRatio
 * @return
 */

qreal TiledObject::factorFromRadian(const qreal &angle, const qreal &xyRatio)
{
	Q_ASSERT(xyRatio != 0);

	if (angle == 0 || angle == M_PI)
		return 1.0;

	qreal r = 0.;

	if (angle > M_PI_2)
		r = (M_PI-angle)/M_PI_2;
	else if (angle > 0)
		r = angle/M_PI_2;
	else if (angle > -M_PI_2)
		r = -angle/M_PI_2;
	else
		r = (M_PI+angle)/M_PI_2;

	return std::lerp(1., 0.7, r);
}




/**
 * @brief TiledObject::nearestDirectionFromRadian
 * @param directions
 * @param angle
 * @return
 */

TiledObject::Direction TiledObject::nearestDirectionFromRadian(const Directions &directions, const qreal &angle)
{
	static const std::map<qreal, Direction, std::greater<qreal>> map8 = {
		{ M_PI			, West },
		{ M_PI_4 * 3	, NorthWest },
		{ M_PI_2		, North },
		{ M_PI_4		, NorthEast },
		{ 0				, East },
		{ -M_PI_4		, SouthEast },
		{ -M_PI_2		, South },
		{ -M_PI_4 * 3	, SouthWest },
		{ -M_PI			, West },
	};

	static const std::map<qreal, Direction, std::greater<qreal>> map4 = {
		{ M_PI		, West },
		{ M_PI_2	, North },
		{ 0			, East },
		{ -M_PI_2	, South },
		{ -M_PI		, West },
	};

	qreal diff = -1;
	Direction ret = Invalid;

	const std::map<qreal, Direction, std::greater<qreal>> *ptr = nullptr;

	switch (directions) {
		case Direction_4:
			ptr = &map4;
			break;
		case Direction_8:
		case Direction_Infinite:
			ptr = &map8;
			break;
		default:
			break;
	}

	if (!ptr)
		return Invalid;

	for (const auto &[a, dir] : std::as_const(*ptr)) {
		const qreal d = std::abs(a-angle);
		if (diff == -1 || d < diff) {
			diff = d;
			ret = dir;
		}
	}

	return ret;
}




/**
 * @brief TiledObject::directionToRadian
 * @param direction
 * @return
 */

qreal TiledObject::directionToRadian(const Direction &direction)
{
	static const QMap<Direction, qreal> map8 = {
		{ West ,		M_PI		},
		{ NorthWest ,	M_PI_4 * 3	},
		{ North ,		M_PI_2		},
		{ NorthEast ,	M_PI_4		},
		{ East ,		0			},
		{ SouthEast ,	-M_PI_4		},
		{ South ,		-M_PI_2		},
		{ SouthWest ,	-M_PI_4 * 3	},
		{ West ,		-M_PI		},
	};

	return map8.value(direction, 0);
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
	setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureSensor));

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

Box2DFixture::CategoryFlag TiledObjectBody::fixtureCategory(const FixtureCategory &category)
{
	switch (category) {
		case FixtureGround:		return Box2DFixture::Category1;
		case FixturePlayerBody:		return Box2DFixture::Category2;
		case FixtureEnemyBody:		return Box2DFixture::Category3;
		case FixtureTransport:		return Box2DFixture::Category4;
		case FixtureTarget:		return Box2DFixture::Category5;
		case FixturePickable:		return Box2DFixture::Category6;
		case FixtureContainer:	return Box2DFixture::Category7;

		case FixtureTrigger:		return Box2DFixture::Category14;
		case FixtureVirtualCircle:		return Box2DFixture::Category15;
		case FixtureSensor:		return Box2DFixture::Category16;
		case FixtureInvalid:		return Box2DFixture::None;
	}

	return Box2DFixture::None;
}


/**
 * @brief TiledObjectBody::setFixtureCollidesWithFlag
 * @param fixture
 * @param category
 * @param on
 */

void TiledObjectBody::setFixtureCollidesWithFlag(Box2DFixture *fixture, const FixtureCategory &category, const bool &on)
{
	Q_ASSERT(fixture);
	auto flag = fixture->collidesWith();
	fixture->setCollidesWith(flag.setFlag(TiledObjectBody::fixtureCategory(category), on));
}




/**
 * @brief TiledObjectBody::synchronize
 */

void TiledObjectBody::synchronize()
{
	Q_ASSERT(mBody);

	if (mTarget) {
		//if (TiledObjectBase *object = qobject_cast<TiledObjectBase*>(mTarget)) {
		b2Vec2 &value = mBodyDef.position;
		const b2Vec2 &newValue = mBody->GetPosition();

		if (qFuzzyCompare(value.x, newValue.x) && qFuzzyCompare(value.y, newValue.y))
			return;

		emplace();

		return;
		//}
	}

	Box2DBody::synchronize();
}


/**
 * @brief TiledObjectBody::updateTransform
 */

void TiledObjectBody::updateTransform()
{
	Q_ASSERT(mTarget);
	Q_ASSERT(mBody);
	Q_ASSERT(mTransformDirty);

	mBodyDef.angle = toRadians(mTarget->rotation());
	/*mBodyDef.position = mWorld->toMeters(
				mTarget->transformOrigin() == QQuickItem::TopLeft ?
					mTarget->position() :
					mTarget->position() + originOffset());*/

	QPointF offset(mTarget->width()/2, mTarget->height()/2);

	mBodyDef.position = mWorld->toMeters(
							mTarget->position() + offset + m_bodyOffset
							);

	mBody->SetTransform(mBodyDef.position, mBodyDef.angle);
	mTransformDirty = false;
}



/**
 * @brief TiledObjectBody::bodyPosition
 * @return
 */

QPointF TiledObjectBody::bodyPosition() const
{
	Q_ASSERT(mWorld);
	return mWorld->toPixels(mBodyDef.position);
}


/**
 * @brief TiledObjectBody::emplace
 * @param centerX
 * @param centerY
 */

void TiledObjectBody::emplace(const QPointF &center)
{
	Q_ASSERT(mBody);
	Q_ASSERT(mWorld);

	setAngularVelocity(0.);
	setLinearVelocity(QPointF{0.,0.});
	mBody->SetTransform(mWorld->toMeters(center), mBody->GetAngle());
	synchronize();
}


/**
 * @brief TiledObjectBody::setSpeed
 * @param point
 */

void TiledObjectBody::setSpeed(const QPointF &point)
{
	setLinearVelocity(point * mWorld->metersPerPixel());
}


/**
 * @brief TiledObjectBody::stop
 */

void TiledObjectBody::stop()
{
	setLinearVelocity(QPointF{0,0});
	setAngularVelocity(0.);
}



/**
 * @brief TiledObjectBody::emplace
 */

void TiledObjectBody::emplace()
{
	Q_ASSERT(mTarget);
	Q_ASSERT(mBody);
	Q_ASSERT(mWorld);

	const b2Vec2 &newValue = mBody->GetPosition();
	mBodyDef.position = newValue;

	QPointF offset(mTarget->width()/2, mTarget->height()/2);

	mTarget->setPosition(mWorld->toPixels(mBodyDef.position) - offset - m_bodyOffset);

	emit positionChanged();

}



/**
 * @brief TiledObjectBody::opaque
 * @return
 */

bool TiledObjectBody::opaque() const
{
	return m_opaque;
}

void TiledObjectBody::setOpaque(bool newOpaque)
{
	if (m_opaque == newOpaque)
		return;
	m_opaque = newOpaque;
	emit opaqueChanged();
}



/**
 * @brief TiledObjectBody::setBodyOffset
 * @param newBodyOffset
 */

void TiledObjectBody::setBodyOffset(QPointF newBodyOffset)
{
	if (m_bodyOffset == newBodyOffset)
		return;
	m_bodyOffset = newBodyOffset;
	emit bodyOffsetChanged();
}




QPointF TiledObjectBody::bodyOffset() const
{
	return m_bodyOffset;
}



/**
 * @brief TiledObjectBody::rayCast
 * @param dest
 */

TiledReportedFixtureMap TiledObjectBody::rayCast(const QPointF &dest, float32 *lengthPtr)
{
	Q_ASSERT(mWorld);

	const b2Vec2 bPos = mBody->GetPosition();
	const b2Vec2 tPos = mWorld->toMeters(dest);

	if (lengthPtr) {
		b2Vec2 l = tPos-bPos;
		*lengthPtr = mWorld->toPixels(l.Length());
	}

	TiledObjectRayCast ray(mWorld);

	mWorld->world().RayCast(&ray, bPos, tPos);

	return ray.reportedFixtures();
}




/**
 * @brief TiledObjectPolygonIface::createFixture
 * @param polygon
 * @return
 */

Box2DPolygon *TiledObjectPolygonIface::createFixture(const QPolygonF &polygon)
{
	Q_ASSERT(m_fixture);

	const QRectF &box = polygon.boundingRect();

	m_screenPolygon = polygon.translated(-box.center());

	TiledObjectBase::setPolygonVertices(m_fixture.get(), m_screenPolygon);

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

	const qreal r = size/2.;

	m_fixture->setX(pos.x()-r);
	m_fixture->setY(pos.y()-r);
	m_fixture->setRadius(r);

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
	Q_ASSERT(m_spriteHandler);

	bool r = true;

	for (int i=0; i<sprite.directions.size(); ++i) {
		const int n = sprite.directions.at(i);
		Direction direction = QVariant::fromValue(n).value<Direction>();

		if (direction == Invalid) {
			LOG_CERROR("scene") << "Sprite invalid direction:" << n << source;
			return false;
		}

		TiledObjectSprite s2 = sprite;

		if (sprite.startColumn >= 0) {
			s2.x = sprite.startColumn + i*sprite.width;
		} else if (sprite.startRow >= 0) {
			s2.y = sprite.startRow + i*sprite.height;
		}

		r &= m_spriteHandler->addSprite(s2, QStringLiteral("default"), direction, source);
	}

	return r;
}




/**
 * @brief IsometricObjectIface::appendSprite
 * @param source
 * @param spriteList
 * @return
 */

bool TiledObject::appendSprite(const QString &source, const IsometricObjectSpriteList &spriteList)
{
	Q_ASSERT(m_spriteHandler);

	for (const auto &s : std::as_const(spriteList.sprites)) {
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

bool TiledObject::appendSprite(const IsometricObjectLayeredSprite &sprite, const QString &path)
{
	Q_ASSERT(m_spriteHandler);

	bool r = true;

	for (const IsometricObjectSprite &s : std::as_const(sprite.sprites)) {
		for (auto it = sprite.layers.constBegin(); it != sprite.layers.constEnd(); ++it) {
			const QString &alteration = it.key();

			for (int i=0; i<s.directions.size(); ++i) {
				const int n = s.directions.at(i);
				Direction direction = QVariant::fromValue(n).value<Direction>();

				if (direction == Invalid) {
					LOG_CERROR("scene") << "Sprite invalid direction:" << n << path;
					return false;
				}

				TiledObjectSprite s2 = s;

				if (s.startColumn >= 0) {
					s2.x = s.startColumn + i*s.width;
				} else if (s.startRow >= 0) {
					s2.y = s.startRow + i*s.height;
				}

				if (it.value().startsWith(QStringLiteral(":/")) || it.value().startsWith(QStringLiteral("qrc:/")))
					r &= m_spriteHandler->addSprite(s2, alteration, direction, it.value());
				else
					r &= m_spriteHandler->addSprite(s2, alteration, direction, path+it.value());
			}
		}
	}

	return r;
}



/**
 * @brief IsometricObjectIface::appendSprite
 * @param sprite
 * @param path
 * @return
 */

bool TiledObject::appendSprite(const IsometricObjectLayeredSpriteList &sprite, const QString &path)
{
	Q_ASSERT(m_spriteHandler);

	for (const auto &s : std::as_const(sprite.list)) {
		if (!appendSprite(s, path)) {
			LOG_CERROR("scene") << "Load sprite error:" << path;
			return false;
		}
	}

	return true;
}



/**
 * @brief TiledObject::playAuxSprite
 * @param source
 * @param sprite
 * @return
 */

bool TiledObject::playAuxSprite(const AuxHandler &auxHandler, const bool &alignToBody, const QString &source, const TiledObjectSprite &sprite, const bool &replaceCurrentSprite) const
{
	TiledSpriteHandler *handler = nullptr;

	switch (auxHandler) {
		case AuxBack:
			handler = m_spriteHandlerAuxBack;
			break;

		case AuxFront:
			handler = m_spriteHandlerAuxFront;
			break;
	}

	Q_ASSERT(handler);

	// ->property("alignToBody")

	if (!handler->currentSprite().isEmpty() && !replaceCurrentSprite)
		return false;

	handler->clear();

	handler->setClearAtEnd(true);
	handler->setWidth(sprite.width);
	handler->setHeight(sprite.height);
	handler->addSprite(sprite, QStringLiteral("default"), Direction::Invalid, source);
	handler->setProperty("alignToBody", alignToBody);
	handler->jumpToSprite(sprite.name, Direction::Invalid, TiledSpriteHandler::JumpImmediate);

	return true;
}



/**
 * @brief TiledObjectRayCast::ReportFixture
 * @param fixture
 * @param point
 * @param normal
 * @param fraction
 * @return
 */

float32 TiledObjectRayCast::ReportFixture(b2Fixture *fixture, const b2Vec2 &point, const b2Vec2 &normal, float32 fraction)
{
	Box2DFixture *box2dFixture = toBox2DFixture(fixture);

	TiledReportedFixture f;
	f.fixture = box2dFixture;
	f.point = point;
	f.normal = normal;

	m_reportedFixtures.insert(fraction, f);

	return -1.0;
}


/**
 * @brief TiledObjectRayCast::reportedFixtures
 * @return
 */

TiledReportedFixtureMap TiledObjectRayCast::reportedFixtures() const
{
	return m_reportedFixtures;
}

