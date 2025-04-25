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
#include "tiledobject_p.h"
#include "application.h"
#include "Logger.h"
#include "tiledgame.h"
#include "tiledscene.h"
#include "tiledspritehandler.h"
#include "tileddebugdraw.h"
#include <libtiled/maprenderer.h>
#include <libtiled/objectgroup.h>
#include <chipmunk/chipmunk_structs.h>


#ifndef QT_NO_DEBUG
#include "rpgplayer.h"
#include "rpgenemy.h"

#define BODY_ERROR(body, msg) { \
	if (const RpgPlayer *p = dynamic_cast<const RpgPlayer*>(body)) \
		LOG_CERROR("scene") << msg << p; \
	else if (const RpgEnemy *p = dynamic_cast<const RpgEnemy*>(body)) \
		LOG_CERROR("scene") << msg << p; \
	}
#else
#define BODY_ERROR(body, msg) LOG_CERROR("scene") << msg << body;
#endif


#define CHECK_BODY()		{ \
	if (!d->m_bodyRef) { \
	BODY_ERROR(this, "Missing body"); \
	return; \
	} \
	}


#define CHECK_BODY_X(x)		{ \
	if (!d->m_bodyRef) { \
	BODY_ERROR(this, "Missing body"); \
	return x; \
	} \
	}


#define CHECK_LOCK()		{ \
	CHECK_BODY() \
	if (d->m_bodyRef->space && cpSpaceIsLocked(d->m_bodyRef->space)) { \
	BODY_ERROR(this, "Space locked"); \
	return; \
	} \
	}


#define CHECK_LOCK_X(x)		{ \
	CHECK_BODY_X(x) \
	if (d->m_bodyRef->space && cpSpaceIsLocked(d->m_bodyRef->space)) { \
	BODY_ERROR(this, "Space locked"); \
	return x; \
	} \
	}




/**
 * @brief TiledObject::TiledObject
 * @param polygon
 * @param renderer
 * @param game
 * @param type
 */

TiledObject::TiledObject(const QPolygonF &polygon, TiledGame *game, Tiled::MapRenderer *renderer, const cpBodyType &type)
	: QObject(nullptr)
	, TiledObjectBody(polygon, game, renderer, type)
{
	LOG_CTRACE("scene") << "TiledObject created" << this;
}



/**
 * @brief TiledObject::TiledObject
 * @param center
 * @param radius
 * @param renderer
 * @param game
 * @param type
 */

TiledObject::TiledObject(const QPointF &center, const qreal &radius, TiledGame *game, Tiled::MapRenderer *renderer, const cpBodyType &type)
	: QObject(nullptr)
	, TiledObjectBody(center, radius, game, renderer, type)
{
	LOG_CTRACE("scene") << "TiledObject created" << this;
}


/**
 * @brief TiledObject::TiledObject
 * @param object
 * @param renderer
 * @param game
 * @param type
 */

TiledObject::TiledObject(const Tiled::MapObject *object, TiledGame *game, Tiled::MapRenderer *renderer, const cpBodyType &type)
	: QObject(nullptr)
	, TiledObjectBody(object, game, renderer, type)
{
	LOG_CTRACE("scene") << "TiledObject created" << this;
}


/**
 * @brief TiledObject::~TiledObject
 */

TiledObject::~TiledObject()
{
	updateScene();

	if (m_spriteHandler)
		m_spriteHandler->setBaseObject(nullptr);

	if (m_spriteHandlerAuxFront)
		m_spriteHandlerAuxFront->setBaseObject(nullptr);

	if (m_spriteHandlerAuxBack)
		m_spriteHandlerAuxBack->setBaseObject(nullptr);

	LOG_CTRACE("scene") << "TiledObject destroyed" << this;
}





/**
 * @brief TiledObject::toPolygon
 * @param object
 * @param renderer
 * @return
 */

QPolygonF TiledObject::toPolygon(const Tiled::MapObject *object, Tiled::MapRenderer *renderer)
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
 * @brief TiledObject::shortestDistance
 * @param point
 * @param lineP1
 * @param lineP2
 * @param destPoint
 * @return
 */

float TiledObject::shortestDistance(const QVector2D &point, const QVector2D &lineP1, const QVector2D &lineP2, QVector2D *destPoint, float *factor)
{
	const float A = point.x() - lineP1.x();
	const float B = point.y() - lineP1.y();
	const float C = lineP2.x() - lineP1.x();
	const float D = lineP2.y() - lineP1.y();

	const float dot = A * C + B * D;
	const float len_sq = C * C + D * D;
	float param = -1;

	if (len_sq != 0)
		param = dot / len_sq;

	float xx, yy;

	if (param < 0) {
		xx = lineP1.x();
		yy = lineP1.y();
	}
	else if (param > 1) {
		xx = lineP2.x();
		yy = lineP2.y();
	}
	else {
		xx = lineP1.x() + param * C;
		yy = lineP1.y() + param * D;
	}

	const float dx = point.x() - xx;
	const float dy = point.y() - yy;

	if (destPoint) {
		destPoint->setX(xx);
		destPoint->setY(yy);
	}

	if (factor)
		*factor = param;

	return std::sqrt(dx * dx + dy * dy);
}






/**
 * @brief TiledObject::normalizeFromRadian
 * @param radian
 * @return
 */

float TiledObject::normalizeFromRadian(const float &radian)
{
	if (radian < -M_PI || radian > M_PI) {
		LOG_CTRACE("scene") << "Invalid radian:" << radian;
		return M_PI;
	}

	if (radian < 0)
		return M_PI+M_PI+radian;
	else
		return radian;
}


/**
 * @brief TiledObject::normalizeToRadian
 * @param normal
 * @return
 */

float TiledObject::normalizeToRadian(const float &normal)
{
	if (normal < 0 || normal > 2*M_PI) {
		LOG_CTRACE("scene") << "Invalid normalized radian:" << normal;
		return 0.;
	}

	if (normal > M_PI)
		return normal-2*M_PI;
	else
		return normal;
}



/**
 * @brief TiledObject::setBodyOffset
 * @param newBodyOffset
 */

void TiledObject::setBodyOffset(QPointF newBodyOffset)
{
	if (m_bodyOffset == newBodyOffset)
		return;

	m_bodyOffset = newBodyOffset;
	emit bodyOffsetChanged();
}



/**
 * @brief TiledObject::synchronize
 */

void TiledObject::synchronize()
{
	TiledObjectBody::synchronize();

	if (!body() || !m_visualItem)
		return;

	const QPointF &pos = bodyPosition();

	QPointF offset(m_visualItem->width()/2, m_visualItem->height()/2);
	offset += m_bodyOffset;

	m_visualItem->setPosition(pos-offset);

	if (!qFuzzyCompare(bodyRotation(), m_lastAngle))
		emit currentAngleChanged();

	if (m_facingDirectionLocked)
		setFacingDirection(nearestDirectionFromRadian(bodyRotation()));

	if (m_spriteHandler)
		m_spriteHandler->updateDirty();

	if (m_spriteHandlerAuxBack)
		m_spriteHandlerAuxBack->updateDirty();

	if (m_spriteHandlerAuxFront)
		m_spriteHandlerAuxFront->updateDirty();
}



/**
 * @brief TiledObject::onSpaceChanged
 */

void TiledObject::onSpaceChanged()
{
	TiledObjectBody::onSpaceChanged();
	updateScene();
}



/**
 * @brief TiledObject::moveTowards
 * @param point
 * @param speed
 * @return
 */

bool TiledObject::moveTowards(const QVector2D &point, const float &speed)
{
	const float dist = distanceToPoint(point);
	const float angle = angleToPoint(point);

	setCurrentAngle(angle);

	if (dist > 2 * speed/60.)
		setSpeedFromAngle(angle, speed);
	else
		return false;

	return true;
}



/**
 * @brief TiledObject::moveTowards
 * @param point
 * @param speedBelow
 * @param destinationLimit
 * @param speedAbove
 * @return
 */

bool TiledObject::moveTowards(const QVector2D &point, const float &speedBelow, const float &destinationLimit, const float &speedAbove)
{
	const float dist = distanceToPoint(point);
	const float angle = angleToPoint(point);

	setCurrentAngle(angle);

	if (dist > destinationLimit)
		setSpeedFromAngle(angle, speedAbove);
	else if (dist > 2 * speedBelow/60.)
		setSpeedFromAngle(angle, speedBelow);
	else
		return false;

	return true;
}


/**
 * @brief TiledObject::moveTowards
 * @param point
 * @return
 */

void TiledObject::moveTowards(const QVector2D &point)
{
	if (!body()) {
		LOG_CERROR("scene") << "Missing body" << this;
		return;
	}

	setCurrentAngle(angleToPoint(point));

	const auto &pos = bodyPosition();

	setSpeed(point.x() - pos.x(),
			 point.y() - pos.y());
}




QString TiledObject::displayName() const
{
	return m_displayName;
}

void TiledObject::setDisplayName(const QString &newDisplayName)
{
	if (m_displayName == newDisplayName)
		return;
	m_displayName = newDisplayName;
	emit displayNameChanged();
}




/**
 * @brief TiledObject::game
 * @return
 */

TiledGame *TiledObjectBody::game() const
{
	return m_game;
}







/**
 * @brief TiledObjectBody::overrideCurrentSpeed
 * @param speed
 */

void TiledObjectBody::overrideCurrentSpeed(const QVector2D &speed)
{
	d->m_currentSpeed = speed.length()/60.;
}




/**
 * @brief TiledObjectBody::setSpace
 * @param space
 */

void TiledObjectBody::setSpace(cpSpace *space)
{
	if (!d->m_bodyRef)
		return;

	cpSpace *old = d->m_bodyRef->space;
	if (old)
		cpSpaceRemoveBody(old, d->m_bodyRef);

	cpSpaceAddBody(space, d->m_bodyRef);

	static const auto fn = [](cpBody *body, cpShape *shape) {
		if (shape->space)
			cpSpaceRemoveShape(shape->space, shape);				// also cpBodyRemoveShape
		cpSpaceAddShape(body->space, shape);					// also cpBodyAddShape
	};

	for (cpShape *s : d->m_bodyShapes)
		fn(d->m_bodyRef, s);

	if (d->m_sensorPolygon)
		fn(d->m_bodyRef, d->m_sensorPolygon);

	if (d->m_virtualCircle)
		fn(d->m_bodyRef, d->m_virtualCircle);

	if (d->m_targetCircle)
		fn(d->m_bodyRef, d->m_targetCircle);

	onSpaceChanged();
}





/**
 * @brief TiledObjectBody::drawBody
 * @param draw
 * @param color
 * @param lineWidth
 * @param filled
 * @param outlined
 */

void TiledObjectBody::drawBody(TiledDebugDraw *draw, const QColor &color, const qreal &lineWidth, const bool filled, const bool outlined) const
{
	static const QHash<FixtureCategory, QColor> fixtureColors = {
		{ FixtureGround, QColorConstants::Svg::saddlebrown },
		{ FixtureSensor, QColorConstants::Svg::lime },
		{ FixtureTrigger, QColorConstants::Svg::magenta },
		{ FixtureContainer, QColorConstants::Svg::orange },
	};

	for (cpShape *sh : d->m_bodyShapes) {
		QColor c = color;
		const cpBitmask &category = cpShapeGetFilter(sh).categories;
		for (const auto &[fixture, fcolor] : fixtureColors.asKeyValueRange()) {
			if (category & fixture)
				c = fcolor;
		}

		if (cpShapeGetSensor(sh)) {
			c.setAlphaF(0.3);
			d->drawShape(draw, sh, c, lineWidth, filled, false);
		} else
			d->drawShape(draw, sh, c, lineWidth, filled, outlined);
	}
}


/**
 * @brief TiledObjectBody::drawSensor
 * @param draw
 * @param color
 * @param lineWidth
 * @param filled
 * @param outlined
 */

void TiledObjectBody::drawSensor(TiledDebugDraw *draw, const QColor &color, const qreal &lineWidth, const bool filled, const bool outlined) const
{
	if (d->m_sensorPolygon)
		d->drawShape(draw, d->m_sensorPolygon, color, lineWidth, filled, outlined);
}


/**
 * @brief TiledObjectBody::drawVirtualCircle
 * @param draw
 * @param color
 * @param lineWidth
 * @param filled
 * @param outlined
 */

void TiledObjectBody::drawVirtualCircle(TiledDebugDraw *draw, const QColor &color, const qreal &lineWidth, const bool filled, const bool outlined) const
{
	if (d->m_virtualCircle)
		d->drawShape(draw, d->m_virtualCircle, color, lineWidth, filled, outlined);
}



/**
 * @brief TiledObjectBody::drawTargetCircle
 * @param draw
 * @param color
 * @param lineWidth
 * @param filled
 * @param outlined
 */

void TiledObjectBody::drawTargetCircle(TiledDebugDraw *draw, const QColor &color, const qreal &lineWidth, const bool filled, const bool outlined) const
{
	if (d->m_targetCircle)
		d->drawShape(draw, d->m_targetCircle, color, lineWidth, filled, outlined);
}


/**
 * @brief TiledObjectBody::drawCenter
 * @param draw
 * @param colorX
 * @param colorY
 * @param lineWidth
 */

void TiledObjectBody::drawCenter(TiledDebugDraw *draw, const QColor &colorX, const QColor &colorY, const qreal &lineWidth) const
{
	const cpVect center{ .x = bodyPosition().x(), .y = bodyPosition().y() };
	float angle = bodyRotation();

	static const float radius = 10.;

	cpVect up{ .x = center.x + radius * cos(angle),
				.y = center.y + radius * sin(angle)
			 };

	angle += M_PI/2;

	cpVect right{ .x = center.x + radius * cos(angle),
				.y = center.y + radius * sin(angle)
				};

	draw->drawSegment(up, center, colorY, lineWidth);
	draw->drawSegment(center, right, colorX, lineWidth);
}



/**
 * @brief TiledObjectBody::visualItem
 * @return
 */

QQuickItem *TiledObjectBody::visualItem() const
{
	return m_visualItem;
}


/**
 * @brief TiledObjectBody::setVisualItem
 * @param newVisualItem
 */

void TiledObjectBody::setVisualItem(QQuickItem *newVisualItem)
{
	m_visualItem = newVisualItem;
}



/**
 * @brief TiledObjectBody::onSpaceChanged
 */

void TiledObjectBody::onSpaceChanged()
{

}








/**
 * @brief TiledObjectBody::updateVisibleArea
 */

void TiledObject::updateVisibleArea()
{
	if (!m_currentScene)
		return;

	const bool i = m_currentScene->visibleArea().intersects(bodyAABB());

	if (m_inVisibleArea == i)
		return;

	m_inVisibleArea = i;
	emit inVisibleAreaChanged();
}




/**
 * @brief TiledObject::updateScene
 */

void TiledObject::updateScene()
{
	cpBody *b = body();

	if (!b)
		LOG_CTRACE("scene") << "Update scene: missing body" << this;

	TiledScene *currentScene = nullptr;

	if (b) {
		if (cpSpace *s = cpBodyGetSpace(b))
			currentScene = static_cast<TiledScene*>(cpSpaceGetUserData(s));
	}

	if (m_currentScene == currentScene)
		return;

	LOG_CTRACE("scene") << "Scene changed for" << this << " - " << m_currentScene << "->" << currentScene;

	if (m_currentScene)
		m_currentScene->disconnect(this);

	m_currentScene = currentScene;

	if (m_currentScene)
		connect(m_currentScene, &TiledScene::visibleAreaChanged, this, &TiledObject::updateVisibleArea);
}






QColor TiledObject::overlayColor() const
{
	return m_overlayColor;
}

void TiledObject::setOverlayColor(const QColor &newOverlayColor)
{
	if (m_overlayColor == newOverlayColor)
		return;
	m_overlayColor = newOverlayColor;
	emit overlayColorChanged();
}

QColor TiledObject::glowColor() const
{
	return m_glowColor;
}

void TiledObject::setGlowColor(const QColor &newGlowColor)
{
	if (m_glowColor == newGlowColor)
		return;
	m_glowColor = newGlowColor;
	emit glowColorChanged();
}

bool TiledObject::overlayEnabled() const
{
	return m_overlayEnabled;
}

void TiledObject::setOverlayEnabled(bool newOverlayEnabled)
{
	if (m_overlayEnabled == newOverlayEnabled)
		return;
	m_overlayEnabled = newOverlayEnabled;
	emit overlayEnabledChanged();
}

bool TiledObject::glowEnabled() const
{
	return m_glowEnabled;
}

void TiledObject::setGlowEnabled(bool newGlowEnabled)
{
	if (m_glowEnabled == newGlowEnabled)
		return;
	m_glowEnabled = newGlowEnabled;
	emit glowEnabledChanged();
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

	const auto &ptr = toTextureSprite(sprite, source);

	if (!ptr)
		return false;

	return TiledGame::appendToSpriteHandler(m_spriteHandler, { ptr.value() }, source);
}



/**
 * @brief TiledObject::appendSprite
 * @param source
 * @param spriteList
 * @return
 */

bool TiledObject::appendSprite(const QString &source, const TiledObjectSpriteList &spriteList)
{
	for (const TiledObjectSprite &s : spriteList.sprites) {
		const auto &ptr = toTextureSprite(s, source);
		if (!ptr)
			return false;

		if (!TiledGame::appendToSpriteHandler(m_spriteHandler, { ptr.value() }, source))
			return false;
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

	m_visualItem->setParent(m_game);
	m_visualItem->setProperty("baseObject", QVariant::fromValue(this));

	connect(m_visualItem, &QQuickItem::xChanged, this, &TiledObject::updateVisibleArea, Qt::QueuedConnection);
	connect(m_visualItem, &QQuickItem::yChanged, this, &TiledObject::updateVisibleArea, Qt::QueuedConnection);
	connect(m_visualItem, &QQuickItem::widthChanged, this, &TiledObject::updateVisibleArea, Qt::QueuedConnection);
	connect(m_visualItem, &QQuickItem::heightChanged, this, &TiledObject::updateVisibleArea, Qt::QueuedConnection);

	emit visualItemChanged();
}






/**
 * @brief TiledObject::createMarkerItem
 */

QQuickItem *TiledObject::createMarkerItem(const QString &qrc)
{
	Q_ASSERT(!qrc.isEmpty());

	QQmlComponent component(Application::instance()->engine(), qrc, this);

	QQuickItem *item = qobject_cast<QQuickItem*>(component.createWithInitialProperties(
													 QVariantMap{
														 { QStringLiteral("target"), QVariant::fromValue(this) }
													 }));

	if (!item) {
		LOG_CERROR("scene") << "TiledPlayerMarker error" << component.errorString();
		return nullptr;
	}

	item->setParent(this);

	return item;
}



/**
 * @brief TiledObject::rotateToPoint
 * @param point
 * @param anglePtr
 * @param distancePtr
 */

void TiledObjectBody::rotateToPoint(const QPointF &point, const bool &forced)
{
	CHECK_BODY();

	rotateBody(angleToPoint(point), forced);

	/*d->m_bodyRef.SetTransform(d->m_bodyRef.GetPosition(), b2MakeRot(radian));
	d->m_bodyRef.SetAwake(true);
	d->m_rotateAnimation.running = false;*/
}



/**
 * @brief TiledObject::angleToPoint
 * @param point
 * @return
 */

float TiledObjectBody::angleToPoint(const QVector2D &point) const
{
	CHECK_BODY_X(0.);

	const QVector2D p = point - QVector2D(bodyPosition());
	return atan2(p.y(), p.x());
}


/**
 * @brief TiledObject::distanceToPoint
 * @param point
 * @return
 */

float TiledObjectBody::distanceToPoint(const QPointF &point) const
{
	CHECK_BODY_X(-1.);

	return QVector2D(bodyPosition()).distanceToPoint(QVector2D(point));
}



/**
 * @brief TiledObjectBody::distanceToPoint
 * @param point
 * @return
 */

float TiledObjectBody::distanceToPoint(const QVector2D &point) const
{
	CHECK_BODY_X(-1.);

	return QVector2D(bodyPosition()).distanceToPoint(point);
}


const TiledObjectBody::ObjectId &TiledObjectBody::objectId() const
{
	return m_objectId;
}

void TiledObjectBody::setObjectId(const ObjectId &newObjectId)
{
	m_objectId = newObjectId;
}

void TiledObjectBody::setObjectId(const int &ownerId, const int &sceneId, const int &id)
{
	m_objectId = {.ownerId = ownerId, .sceneId = sceneId, .id = id};
}



/**
 * @brief TiledObjectBody::world
 * @return
 */

cpSpace *TiledObjectBody::space() const
{
	return d->m_bodyRef ? d->m_bodyRef->space : nullptr;
}




/**
 * @brief TiledObjectBody::scene
 * @return
 */

TiledScene *TiledObjectBody::scene() const
{
	return d->m_bodyRef && d->m_bodyRef->space ? static_cast<TiledScene*>(cpSpaceGetUserData(d->m_bodyRef->space)) : nullptr;
}


/**
 * @brief TiledObjectBody::body
 * @return
 */

cpBody *TiledObjectBody::body() const
{
	return d->m_bodyRef;
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

TiledObject::Direction TiledObject::facingDirection() const
{
	return m_facingDirection;
}

void TiledObject::setFacingDirection(const Direction &newFacingDirection)
{
	if (m_facingDirection == newFacingDirection)
		return;
	m_facingDirection = newFacingDirection;
	emit facingDirectionChanged();
}


/**
 * @brief TiledObject::toRadian
 * @param angle
 * @return
 */

qreal TiledObject::toRadian(const qreal &angle)
{
	if (angle <= 180.)
		return -angle * M_PI / 180.;
	else
		return (360-angle) * M_PI / 180.;
}





/**
 * @brief TiledObject::directionToIsometricRadian
 * @param direction
 * @return
 */

qreal TiledObject::directionToIsometricRadian(const Direction &direction)
{
	switch (direction) {
		case West: return M_PI; break;
		case SouthWest: return M_PI - atan(0.5); break;
		case South: return M_PI_2; break;
		case SouthEast: return atan(0.5); break;
		case East: return 0; break;
		case NorthEast: return -atan(0.5); break;
		case North: return -M_PI_2; break;
		case NorthWest: return -M_PI + atan(0.5); break;
		case Invalid: return 0; break;
	}

	return 0;
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
		{ M_PI_4 * 3	, SouthWest },
		{ M_PI_2		, South },
		{ M_PI_4		, SouthEast },
		{ 0				, East },
		{ -M_PI_4		, NorthEast },
		{ -M_PI_2		, North },
		{ -M_PI_4 * 3	, NorthWest },
		{ -M_PI			, West },
	};

	static const std::map<qreal, Direction, std::greater<qreal>> map4 = {
		{ M_PI		, West },
		{ M_PI_2	, South },
		{ 0			, East },
		{ -M_PI_2	, North },
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
		{ West ,		-M_PI		},
		{ NorthWest ,	-M_PI_4 * 3	},
		{ North ,		-M_PI_2		},
		{ NorthEast ,	-M_PI_4		},
		{ East ,		0			},
		{ SouthEast ,	M_PI_4		},
		{ South ,		M_PI_2		},
		{ SouthWest ,	M_PI_4 * 3	},
		{ West ,		M_PI		},
	};

	return map8.value(direction, 0);
}







/**
 * @brief TiledObjectBody::TiledObjectBody
 */

TiledObjectBody::TiledObjectBody(TiledGame *game)
	: m_game(game)
	, d(new TiledObjectBodyPrivate(this))
{
	LOG_CTRACE("scene") << "TiledObjectBody created" << this;
}



/**
 * @brief TiledObjectBody::TiledObjectBody
 * @param polygon
 * @param renderer
 * @param game
 * @param type
 */

TiledObjectBody::TiledObjectBody(const QPolygonF &polygon,
								 TiledGame *game, Tiled::MapRenderer *renderer, const cpBodyType &type)
	: TiledObjectBody(game)
{
	createFromPolygon(polygon, renderer, type);
}



/**
 * @brief TiledObjectBody::TiledObjectBody
 * @param center
 * @param radius
 * @param renderer
 * @param game
 * @param type
 */

TiledObjectBody::TiledObjectBody(const QPointF &center, const qreal &radius,
								 TiledGame *game, Tiled::MapRenderer *renderer, const cpBodyType &type)
	: TiledObjectBody(game)
{
	createFromCircle(center, radius, renderer, type);
}




/**
 * @brief TiledObjectBody::TiledObjectBody
 * @param object
 * @param renderer
 * @param game
 * @param type
 */

TiledObjectBody::TiledObjectBody(const Tiled::MapObject *object,
								 TiledGame *game, Tiled::MapRenderer *renderer, const cpBodyType &type)
	: TiledObjectBody(game)
{
	createFromMapObject(object, renderer, type);
}




/**
 * @brief TiledObjectBody::~TiledObjectBody
 */

TiledObjectBody::~TiledObjectBody()
{
	delete d;
	d = nullptr;

	LOG_CTRACE("scene") << "TiledObjectBody destroyed" << this;
}






/**
 * @brief TiledObjectBody::bodyPosition
 * @return
 */

QPointF TiledObjectBody::bodyPosition() const
{
	CHECK_BODY_X({});


	const cpVect pos = cpBodyGetPosition(d->m_bodyRef);
	return QPointF{pos.x, pos.y};
}



/**
 * @brief TiledObjectBody::bodyAABB
 * @return
 */

QRectF TiledObjectBody::bodyAABB() const
{
	CHECK_BODY_X({});

	QRectF r;

	for (cpShape *sh : d->m_bodyShapes) {
		const cpBB &bb = cpShapeGetBB(sh);

		if (r.isNull()) {
			r.setLeft(bb.l);
			r.setTop(bb.t);
			r.setRight(bb.r);
			r.setBottom(bb.b);
		} else {
			if (bb.l < r.left()) r.setLeft(bb.l);
			if (bb.t < r.top()) r.setTop(bb.t);
			if (bb.r > r.right()) r.setRight(bb.r);
			if (bb.b > r.bottom()) r.setBottom(bb.b);
		}
	}

	return r;
}


/**
 * @brief TiledObjectBody::currentSpeed
 * @return
 */

float TiledObjectBody::currentSpeed() const
{
	return d->m_currentSpeed;
}



/**
 * @brief TiledObjectBody::filterGet
 * @return
 */

cpShapeFilter TiledObjectBody::filterGet() const
{
	CHECK_LOCK_X(CP_SHAPE_FILTER_NONE);

	if (d->m_bodyShapes.empty())
		return CP_SHAPE_FILTER_NONE;
	else
		return cpShapeGetFilter(d->m_bodyShapes.front());
}


/**
 * @brief TiledObjectBody::filterSet
 * @param categories
 */

void TiledObjectBody::filterSet(const FixtureCategories &categories)
{
	CHECK_LOCK();

	const cpShapeFilter &filter = getFilter(categories);

	for (cpShape *sh : d->m_bodyShapes) {
		cpShapeSetFilter(sh, filter);
	}
}


/**
 * @brief TiledObjectBody::filterSet
 * @param categories
 * @param collidesWith
 */

void TiledObjectBody::filterSet(const FixtureCategories &categories, const FixtureCategories &collidesWith)
{
	CHECK_LOCK();

	const cpShapeFilter &filter = getFilter(categories, collidesWith);

	for (cpShape *sh : d->m_bodyShapes) {
		cpShapeSetFilter(sh, filter);
	}
}


/**
 * @brief TiledObjectBody::isSensor
 * @return
 */

bool TiledObjectBody::isSensor() const
{
	CHECK_LOCK_X(false);

	if (d->m_bodyShapes.empty())
		return false;
	else
		return cpShapeGetSensor(d->m_bodyShapes.front());
}



/**
 * @brief TiledObjectBody::setSensor
 * @param sensor
 */

void TiledObjectBody::setSensor(const bool &sensor)
{
	CHECK_LOCK();

	for (cpShape *sh : d->m_bodyShapes) {
		cpShapeSetSensor(sh, sensor);
	}
}


/**
 * @brief TiledObjectBody::bodyShapes
 * @return
 */

const std::vector<cpShape *> &TiledObjectBody::bodyShapes() const
{
	return d->m_bodyShapes;
}


/**
 * @brief TiledObjectBody::isBodyShape
 * @param shape
 * @return
 */

bool TiledObjectBody::isBodyShape(cpShape *shape) const
{
	return d->m_bodyShapes.cend() != std::find(d->m_bodyShapes.cbegin(), d->m_bodyShapes.cend(), shape);
}


/**
 * @brief TiledObjectBody::sensorPolygon
 * @return
 */

cpShape *TiledObjectBody::sensorPolygon() const
{
	return d->m_sensorPolygon;
}

cpShape *TiledObjectBody::virtualCircle() const
{
	return d->m_virtualCircle;
}

cpShape *TiledObjectBody::targetCircle() const
{
	return d->m_targetCircle;
}


/**
 * @brief TiledObjectBody::emplace
 * @param centerX
 * @param centerY
 */

void TiledObjectBody::emplace(const QVector2D &center)
{
	CHECK_LOCK();

	d->setVelocity(cpvzero);
	cpBodySetPosition(d->m_bodyRef, { center.x(), center.y() });

	d->m_lastPosition.clear();
	d->m_lastPosition.push_back(center);

	d->m_currentSpeed = 0.;
}


/**
 * @brief TiledObjectBody::setSpeed
 * @param point
 */

void TiledObjectBody::setSpeed(const QVector2D &point)
{
	setSpeed(point.x(), point.y());
}


/**
 * @brief TiledObjectBody::setSpeed
 * @param point
 */

void TiledObjectBody::setSpeed(const QPointF &point)
{
	setSpeed(point.x(), point.y());
}



/**
 * @brief TiledObjectBody::setSpeed
 * @param x
 * @param y
 */

void TiledObjectBody::setSpeed(const float &x, const float &y)
{
	CHECK_LOCK();

	if (isnan(x) || isnan(y) ||
			isinf(x) || isinf(y)) {
		LOG_CERROR("scene") << "Invalid speed" << this << x << y;
		d->setVelocity(cpvzero);
		return;
	}

	d->setVelocity({x, y});
}


/**
 * @brief TiledObjectBody::setSpeedFromAngle
 * @param angle
 * @param radius
 */

void TiledObjectBody::setSpeedFromAngle(const float &angle, const float &radius)
{
	CHECK_LOCK();

	float x = radius * cos(angle);
	float y = radius * sin(angle);

	if (isnan(x) || isnan(y) ||
			isinf(x) || isinf(y)) {
		LOG_CERROR("scene") << "Invalid speed" << this << x << y << angle << radius;
		d->setVelocity(cpvzero);
		return;
	}

	d->setVelocity({
					   radius * cos(angle),
					   radius * sin(angle)
				   });
}


/**
 * @brief TiledObjectBody::stop
 */

void TiledObjectBody::stop()
{
	CHECK_LOCK();

	d->setVelocity(cpvzero);
}


/**
 * @brief TiledObjectBody::vectorFromAngle
 * @param angle
 * @param radius
 * @return
 */

QVector2D TiledObjectBody::vectorFromAngle(const float &angle, const float &radius)
{
	return QVector2D (
				radius * cos(angle),
				radius * sin(angle)
				);
}



/**
 * @brief TiledObjectBody::bodyRotation
 * @return
 */

float TiledObjectBody::bodyRotation() const
{
	CHECK_BODY_X(0.);

	return cpBodyGetAngle(d->m_bodyRef);
}


/**
 * @brief TiledObjectBody::desiredBodyRotation
 * @return
 */

float TiledObjectBody::desiredBodyRotation() const
{
	CHECK_BODY_X(0.);

	if (d->m_rotateAnimation.running)
		return d->m_rotateAnimation.destRadian;
	else
		return bodyRotation();
}


/**
 * @brief TiledObjectBody::rotateBody
 * @param desiredRadian
 * @return
 */

bool TiledObjectBody::rotateBody(const float &desiredRadian, const bool &forced)
{
	CHECK_LOCK_X(false);

	cpBody *body = d->m_bodyRef;


	if (forced) {
		d->m_rotateAnimation.running = false;
		cpBodySetAngle(body, desiredRadian);
		return true;
	}

	const float currentRadian = bodyRotation();

	if (qFuzzyCompare(desiredRadian, currentRadian)) {
		cpBodySetAngle(body, desiredRadian);
		d->m_rotateAnimation.running = false;
		return false;
	}


	const float currentNormal = TiledObject::normalizeFromRadian(currentRadian);
	const float desiredNormal = TiledObject::normalizeFromRadian(desiredRadian);

	const float diff = std::abs(currentNormal-desiredNormal);

	if (diff < 2* d->m_rotateAnimation.speed) {
		d->m_rotateAnimation.running = false;
		cpBodySetAngle(body, desiredRadian);
		return true;
	}

	if (!qFuzzyCompare(d->m_rotateAnimation.destRadian, desiredRadian) || !d->m_rotateAnimation.running) {
		d->m_rotateAnimation.destRadian = desiredRadian;

		if (desiredNormal > currentNormal)
			d->m_rotateAnimation.clockwise = diff > M_PI;
		else
			d->m_rotateAnimation.clockwise = diff < M_PI;

		d->m_rotateAnimation.running = true;
	}

	if (!d->m_rotateAnimation.running)
		return false;


	const float delta = std::min(d->m_rotateAnimation.speed, M_PI_4);
	float newAngle = d->m_rotateAnimation.clockwise ? currentNormal - delta : currentNormal + delta;

	//d->m_bodyRef.SetAngularVelocity(0.f);

	static const float pi2 = 2*M_PI;

	// 0 átlépés miatt kell
	const float dd = (desiredNormal == 0 && newAngle > pi2) ? pi2 : desiredNormal;

	if ((newAngle >= dd && currentNormal < dd) ||
			(newAngle <= dd && currentNormal > dd)) {
		cpBodySetAngle(body, desiredRadian);
		return true;
	}

	if (newAngle > pi2)
		newAngle -= pi2;
	else if (newAngle < 0)
		newAngle += pi2;


	cpBodySetAngle(body, TiledObject::normalizeToRadian(newAngle));

	return true;
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
}





/**
 * @brief TiledObjectBody::worldStep
 * @param factor
 */

void TiledObjectBody::worldStep()
{
	CHECK_LOCK();

	QVector2D currPos(bodyPosition());

	d->m_lastPosition.push_back(currPos);
	while (d->m_lastPosition.size() > 4)
		d->m_lastPosition.pop_front();


	if (const int s = d->m_lastPosition.size(); s > 1) {
		auto first = d->m_lastPosition.cbegin();
		float diff = 0.;

		for (auto it = std::next(d->m_lastPosition.cbegin()); it != d->m_lastPosition.cend(); ++it) {
			diff += first->distanceToPoint(*it);
			first = it;
		}

		d->m_currentSpeed = diff / (float) (s-1);
	} else {
		d->m_currentSpeed = 0.;
	}


	if (d->m_rotateAnimation.running)
		rotateBody(d->m_rotateAnimation.destRadian);
}









/**
 * @brief TiledObjectBody::fromBodyRef
 * @param ref
 * @return
 */

TiledObjectBody *TiledObjectBody::fromBodyRef(cpBody *ref)
{
	if (!ref)
		return nullptr;

	return static_cast<TiledObjectBody*>(cpBodyGetUserData(ref));
}


/**
 * @brief TiledObjectBody::fromShapeRef
 * @param ref
 * @return
 */

TiledObjectBody *TiledObjectBody::fromShapeRef(cpShape *ref)
{
	if (!ref)
		return nullptr;

	return fromBodyRef(cpShapeGetBody(ref));
}


/**
 * @brief TiledObjectBody::getFilter
 * @param categories
 * @return
 */

cpShapeFilter TiledObjectBody::getFilter(const FixtureCategories &categories)
{
	return cpShapeFilterNew(CP_NO_GROUP, categories.toInt(), CP_ALL_CATEGORIES);
}


/**
 * @brief TiledObjectBody::getFilter
 * @param categories
 * @param collidesWith
 * @return
 */

cpShapeFilter TiledObjectBody::getFilter(const FixtureCategories &categories, const FixtureCategories &collidesWith)
{
	return cpShapeFilterNew(CP_NO_GROUP, categories.toInt(), collidesWith.toInt());
}




/**
 * @brief TiledObjectBody::createFromPolygon
 * @param polygon
 * @param renderer
 * @param type
 * @return
 */

cpShape* TiledObjectBody::createFromPolygon(const QPolygonF &polygon, Tiled::MapRenderer *renderer, const cpBodyType &type)
{
	const QPolygonF &screenPolygon = renderer ? renderer->pixelToScreenCoords(polygon) : polygon;
	const QRectF &box = screenPolygon.boundingRect();
	const QPolygonF tPolygon = screenPolygon.translated(-box.center());

	std::vector<cpVect> points;
	points.reserve(tPolygon.size());

	for (const QPointF &f : std::as_const(tPolygon))
		points.emplace_back(f.x(), f.y());

	if (!d->m_bodyRef)
		d->createBody(type, 1., cpMomentForPoly(1., points.size(), points.data(), cpvzero, 2.));
	else
		cpBodySetType(d->m_bodyRef, type);

	cpBodySetPosition(d->m_bodyRef, {box.center().x(), box.center().y()});


	d->m_bodyShapes.push_back(cpPolyShapeNew(d->m_bodyRef, points.size(), points.data(), cpTransformIdentity, 2.));

	return d->m_bodyShapes.back();
}





/**
 * @brief TiledObjectBody::createFromCircle
 * @param center
 * @param radius
 * @param renderer
 * @param type
 * @return
 */

cpShape* TiledObjectBody::createFromCircle(const QPointF &center, const qreal &radius, Tiled::MapRenderer *renderer, const cpBodyType &type)
{
	const QPointF &pos = renderer ? renderer->pixelToScreenCoords(center) : center;

	if (!d->m_bodyRef)
		d->createBody(type, 1., cpMomentForCircle(1., 0., radius, cpvzero));
	else
		cpBodySetType(d->m_bodyRef, type);

	cpBodySetPosition(d->m_bodyRef, {pos.x(), pos.y()});

	d->m_bodyShapes.push_back(cpCircleShapeNew(d->m_bodyRef, radius, cpvzero));

	return d->m_bodyShapes.back();
}


/**
 * @brief TiledObjectBody::createFromMapObject
 * @param object
 * @param renderer
 * @param type
 * @return
 */

cpShape* TiledObjectBody::createFromMapObject(const Tiled::MapObject *object, Tiled::MapRenderer *renderer, const cpBodyType &type)
{
	QPointF offset;

	if (Tiled::ObjectGroup *gLayer = object->objectGroup()) {
		offset = gLayer->totalOffset();
	}

	switch (object->shape()) {
		case Tiled::MapObject::Rectangle:
			return createFromPolygon(object->bounds().translated(offset), renderer, type);
		case Tiled::MapObject::Polygon:
			return createFromPolygon(object->polygon().translated(offset+object->position()), renderer, type);
		case Tiled::MapObject::Ellipse:
		case Tiled::MapObject::Point:
			return createFromCircle(offset+object->position(), std::max(object->size().width()/2, object->size().height()/2),
									renderer, type);
		default:
			LOG_CERROR("scene") << "Invalid Tiled::MapObject shape" << object->shape();
	}

	return nullptr;
}


/**
 * @brief TiledObjectBody::deleteBody
 */

void TiledObjectBody::deleteBody()
{
	d->deleteBody();
}




/**
 * @brief TiledObjectBody::setSensorPolygon
 * @param length
 * @param range
 */

void TiledObjectBody::setSensorPolygon(const float &length, const float &range)
{
	CHECK_LOCK();

	std::optional<cpShapeFilter> collidesWith = std::nullopt;

	if (d->m_sensorPolygon)
		collidesWith = cpShapeGetFilter(d->m_sensorPolygon);

	d->setSensorPolygon(length, range);

	if (collidesWith.has_value())
		cpShapeSetFilter(d->m_sensorPolygon, collidesWith.value());
}



/**
 * @brief TiledObjectBody::setSensorPolygon
 * @param length
 * @param range
 * @param categories
 */

void TiledObjectBody::setSensorPolygon(const float &length, const float &range, const FixtureCategories &collidesWith)
{
	CHECK_LOCK();

	d->setSensorPolygon(length, range);

	auto filter = cpShapeGetFilter(d->m_sensorPolygon);
	filter.mask = collidesWith.toInt();
	cpShapeSetFilter(d->m_sensorPolygon, filter);
}



/**
 * @brief TiledObjectBody::addVirtualCircle
 */

void TiledObjectBody::addVirtualCircle(const float &length)
{
	CHECK_LOCK();

	std::optional<cpShapeFilter> collidesWith = std::nullopt;

	if (d->m_virtualCircle)
		collidesWith = cpShapeGetFilter(d->m_virtualCircle);

	d->addVirtualCircle(length > 0 ? length : d->m_sensorLength);

	if (collidesWith.has_value() && d->m_virtualCircle)
		cpShapeSetFilter(d->m_virtualCircle, collidesWith.value());
}


/**
 * @brief TiledObjectBody::addVirtualCircle
 * @param collidesWith
 */

void TiledObjectBody::addVirtualCircle(const FixtureCategories &collidesWith, const float &length)
{
	CHECK_LOCK();

	d->addVirtualCircle(length > 0 ? length : d->m_sensorLength);

	if (d->m_virtualCircle) {
		auto filter = cpShapeGetFilter(d->m_virtualCircle);
		filter.mask = collidesWith;
		cpShapeSetFilter(d->m_virtualCircle, filter);
	}
}


/**
 * @brief TiledObjectBody::removeVirtualCircle
 */

void TiledObjectBody::removeVirtualCircle()
{
	d->removeVirtualCircle();
}



/**
 * @brief TiledObjectBody::addTargetCircle
 * @param length
 */

void TiledObjectBody::addTargetCircle(const float &length)
{
	d->addTargetCircle(length);
}




/**
 * @brief TiledObjectBody::rayCast
 * @param dest
 * @param categories
 * @param radius
 * @return
 */

RayCastInfo TiledObjectBody::rayCast(const QPointF &dest, const FixtureCategories &categories, const float &radius) const
{
	RayCastInfo list;

	if (!d->m_bodyRef)
		return list;

	const cpVect origin = cpBodyGetPosition(d->m_bodyRef);
	const cpVect end{(cpFloat) dest.x(), (cpFloat) dest.y()};


	QMap<cpFloat, RayCastInfoItem> map;

	static const auto fn = [](cpShape *shape, cpVect point, cpVect, cpFloat alpha, void *data) {
		QMap<cpFloat, RayCastInfoItem> *map = (QMap<cpFloat, RayCastInfoItem>*) data;

		RayCastInfoItem info {
			.shape = shape,
					.point = QVector2D(point.x, point.y),
					.visible = false,
					.walkable = false
		};

		map->insert(alpha, std::move(info));
	};

	cpSpaceSegmentQuery(d->m_bodyRef->space,
						origin, end,
						radius,
						cpShapeFilter{CP_NO_GROUP, CP_ALL_CATEGORIES, categories.toInt() | FixtureGround},
						fn,
						&map);

	bool visible = true;
	bool walkable = true;

	for (const RayCastInfoItem &i : map) {
		const cpShapeFilter &filter = cpShapeGetFilter(i.shape);
		TiledObjectBody *body = TiledObjectBody::fromShapeRef(i.shape);

		if (filter.categories & FixtureGround) {
			walkable = false;

			if (body->opaque())
				visible = false;
		}

		if (i.shape->body == d->m_bodyRef)
			continue;

		RayCastInfoItem item = i;
		item.visible = visible;
		item.walkable = walkable;
		list.push_back(item);
	}


	return list;
}



/**
 * @brief TiledObjectBody::debugDraw
 * @param draw
 */

void TiledObjectBody::debugDraw(TiledDebugDraw *draw) const
{
	if (!draw)
		return;

	CHECK_LOCK();

	if (cpBodyGetType(d->m_bodyRef) == CP_BODY_TYPE_STATIC)
		drawBody(draw, QColorConstants::Svg::lightpink, 2.);
	else if (!cpBodyIsSleeping(d->m_bodyRef))
		drawBody(draw, QColorConstants::Svg::limegreen, 2.);
	else
		drawBody(draw, QColorConstants::Svg::steelblue, 2.);
	/*else
		drawBody(draw, QColorConstants::Svg::maroon, 2., true, false);*/


	QColor scolor = QColorConstants::Svg::peru;
	scolor.setAlphaF(0.5);

	drawSensor(draw, scolor, 1., true, false);
	drawTargetCircle(draw, QColorConstants::Svg::orangered, 2.);
	drawVirtualCircle(draw, QColorConstants::Svg::peru, 1.);
	drawCenter(draw, QColorConstants::Svg::midnightblue, QColorConstants::Svg::red);
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

		const auto &ptr = toTextureSprite(s2, source);

		if (!ptr)
			return false;

		if (!m_spriteHandler->addSprite(ptr.value(), QStringLiteral("default"), direction, source))
			return false;
	}

	return true;
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

	for (const auto &s : spriteList.sprites) {
		if (!appendSprite(source, s)) {
			LOG_CERROR("scene") << "Load sprite error:" << source;
			return false;
		}
	}

	return true;
}



/**
 * @brief TiledObject::appendSprite
 * @param source
 * @param sprite
 * @return
 */

bool TiledObject::appendSprite(const QString &source, const TextureSprite &sprite)
{
	Q_ASSERT(m_spriteHandler);
	return m_spriteHandler->addSprite(sprite, QStringLiteral("default"), source);
}




/**
 * @brief TiledObject::appendSprite
 * @param source
 * @param spriteList
 * @return
 */

bool TiledObject::appendSprite(const QString &source, const QVector<TextureSprite> &spriteList)
{
	for (const auto &s : spriteList) {
		if (!appendSprite(source, s)) {
			LOG_CERROR("scene") << "Load sprite error:" << source;
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

bool TiledObject::playAuxSprite(const AuxHandler &auxHandler, const bool &alignToBody, const QString &source,
								const TiledObjectSprite &sprite, const bool &replaceCurrentSprite) const
{
	const auto &ptr = toTextureSprite(sprite, source);

	if (!ptr)
		return false;

	return playAuxSprite(auxHandler, alignToBody, source, ptr.value(), replaceCurrentSprite);
}



/**
 * @brief TiledObject::playAuxSprite
 * @param auxHandler
 * @param alignToBody
 * @param source
 * @param sprite
 * @param replaceCurrentSprite
 * @return
 */

bool TiledObject::playAuxSprite(const AuxHandler &auxHandler, const bool &alignToBody, const QString &source,
								const TextureSprite &sprite, const bool &replaceCurrentSprite) const
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

	if (!handler->currentSprite().isEmpty() && !replaceCurrentSprite)
		return false;

	handler->clear();
	handler->setClearAtEnd(true);
	handler->setWidth(sprite.size.w);
	handler->setHeight(sprite.size.h);
	handler->addSprite(sprite, QStringLiteral("default"), Direction::Invalid, source);
	handler->setProperty("alignToBody", alignToBody);
	handler->jumpToSprite(sprite.name, Direction::Invalid, TiledSpriteHandler::JumpImmediate);

	return true;
}



/**
 * @brief TiledObject::toTextureSprite
 * @param sprite
 * @param source
 * @return
 */

std::optional<TextureSprite> TiledObject::toTextureSprite(const TiledObjectSprite &sprite, const QString &source)
{
	int sourceWidth = 0;

	if (sprite.flow) {
		QImage img(source);
		if (img.isNull()) {
			LOG_CERROR("scene") << "Invalid image:" << source;
			return std::nullopt;
		}
		sourceWidth = img.width();
	}

	TextureSprite s;

	s.name = sprite.name;
	s.size.w = sprite.width;
	s.size.h = sprite.height;
	s.duration = sprite.duration;
	s.loops = sprite.loops;


	int x = sprite.x;
	int y = sprite.y;

	for (int i=0; i<sprite.count; ++i) {
		TextureSpriteFrame frame;
		frame.frame.x = x;
		frame.frame.y = y;
		frame.frame.w = sprite.width;
		frame.frame.h = sprite.height;

		frame.spriteSourceSize.x = 0;
		frame.spriteSourceSize.y = 0;
		frame.spriteSourceSize.w = sprite.width;
		frame.spriteSourceSize.h = sprite.height;

		frame.sourceSize.w = sprite.width;
		frame.sourceSize.h = sprite.height;

		s.frames.append(frame);

		x += sprite.width;

		if (sprite.flow && sourceWidth > 0 && x >= sourceWidth) {
			x = 0;
			y += sprite.height;
		}
	}

	return s;
}





/**
 * @brief TiledObjectBodyPrivate::TiledObjectBodyPrivate
 * @param body
 */

TiledObjectBodyPrivate::TiledObjectBodyPrivate(TiledObjectBody *body)
	: q(body)
{

}


/**
 * @brief TiledObjectBodyPrivate::~TiledObjectBodyPrivate
 */

TiledObjectBodyPrivate::~TiledObjectBodyPrivate()
{
	deleteBody();
}



/**
 * @brief TiledObjectBodyPrivate::deleteBody
 */

void TiledObjectBodyPrivate::deleteBody()
{
	LOG_CTRACE("scene") << "DESTROY body" << this;

	if (!m_bodyRef)
		return;

	if (m_bodyRef->space) {
		Q_ASSERT(!cpSpaceIsLocked(m_bodyRef->space));
	};


	static const auto fn = [](cpBody *, cpShape *shape, void *) {
		if (shape->space)
			cpSpaceRemoveShape(shape->space, shape);
		cpShapeFree(shape);
	};

	cpBodyEachShape(m_bodyRef, fn, nullptr);

	if (m_bodyRef->space)
		cpSpaceRemoveBody(m_bodyRef->space, m_bodyRef);

	cpBodyFree(m_bodyRef);
	m_bodyRef = nullptr;


	m_bodyShapes.clear();
	m_sensorPolygon = nullptr;
	m_virtualCircle = nullptr;
	m_targetCircle = nullptr;
}




/**
 * @brief TiledObjectBodyPrivate::createBody
 * @param type
 */

void TiledObjectBodyPrivate::createBody(const cpBodyType &type, const cpFloat &mass, const cpFloat &moment)
{
	if (m_bodyRef) {
		LOG_CERROR("scene") << "Body already created" << q;
		return;
	}

	switch (type) {
		case CP_BODY_TYPE_KINEMATIC:
			m_bodyRef = cpBodyNewKinematic();
			break;
		case CP_BODY_TYPE_STATIC:
			m_bodyRef = cpBodyNewStatic();
			break;
		case CP_BODY_TYPE_DYNAMIC:
			m_bodyRef = cpBodyNew(mass, moment);
			break;
	}


	cpBodySetUserData(m_bodyRef, q);
}








/**
 * @brief TiledObjectBodyPrivate::setSensorPolygon
 * @param length
 * @param range
 */

void TiledObjectBodyPrivate::setSensorPolygon(const float &length, const float &range)
{
	if (!m_bodyRef) {
		LOG_CERROR("scene") << "Missing body";
		return;
	}

	if (m_sensorPolygon) {
		if (m_sensorPolygon->space)
			cpSpaceRemoveShape(m_sensorPolygon->space, m_sensorPolygon);
		cpShapeFree(m_sensorPolygon);
		m_sensorPolygon = nullptr;
	}

	std::vector<cpVect> points;
	points.reserve(7);
	points.emplace_back(0., 0.);

	for (int i=0; i < 7; ++i) {
		qreal angle = -(range/2.) + (i/6. * range);
		if (angle > M_PI)
			angle = -M_PI+(angle-M_PI);
		else if (angle < -M_PI)
			angle = M_PI-(-M_PI-angle);
		points.emplace_back(length * cosf(angle), length * -sinf(angle));
	}


	m_sensorPolygon = cpPolyShapeNew(m_bodyRef, points.size(), points.data(), cpTransformIdentity, 0.);
	cpShapeSetSensor(m_sensorPolygon, true);
	cpShapeSetFilter(m_sensorPolygon, TiledObjectBody::getFilter(TiledObjectBody::FixtureSensor));

	if (m_bodyRef->space != NULL) {
		cpSpaceAddShape(m_bodyRef->space, m_sensorPolygon);
	}

	m_sensorLength = length;

	if (m_virtualCircle)
		addVirtualCircle(m_sensorLength);

}


/**
 * @brief TiledObjectBodyPrivate::addVirtualCircle
 */

void TiledObjectBodyPrivate::addVirtualCircle(const float &length)
{
	if (!m_bodyRef) {
		LOG_CERROR("scene") << "Missing body";
		return;
	}

	if (length <= 0) {
		LOG_CERROR("scene") << "Invalid sensor length";
		return;
	}

	removeVirtualCircle();

	m_virtualCircle = cpCircleShapeNew(m_bodyRef, length, {0.f , 0.f});
	cpShapeSetSensor(m_virtualCircle, true);
	cpShapeSetFilter(m_virtualCircle, TiledObjectBody::getFilter(TiledObjectBody::FixtureVirtualCircle));

	if (m_bodyRef->space != NULL) {
		cpSpaceAddShape(m_bodyRef->space, m_virtualCircle);
	}
}


/**
 * @brief TiledObjectBodyPrivate::removeVirtualCircle
 */

void TiledObjectBodyPrivate::removeVirtualCircle()
{
	if (m_virtualCircle) {
		if (m_virtualCircle->space)
			cpSpaceRemoveShape(m_virtualCircle->space, m_virtualCircle);
		cpShapeFree(m_virtualCircle);
		m_virtualCircle = nullptr;
	}
}



/**
 * @brief TiledObjectBodyPrivate::addTargetCircle
 * @param length
 */

void TiledObjectBodyPrivate::addTargetCircle(const float &length)
{
	if (!m_bodyRef) {
		LOG_CERROR("scene") << "Missing body";
		return;
	}

	if (length <= 0 && m_targetLength <= 0) {
		LOG_CERROR("scene") << "Invalid target length";
		return;
	}

	if (m_targetCircle) {
		if (m_targetCircle->space)
			cpSpaceRemoveShape(m_targetCircle->space, m_targetCircle);
		cpShapeFree(m_targetCircle);
		m_targetCircle= nullptr;
	}

	if (length > 0)
		m_targetLength = length;

	m_targetCircle = cpCircleShapeNew(m_bodyRef, m_targetLength, {0.f , 0.f});

	cpShapeSetSensor(m_targetCircle, true);
	cpShapeSetFilter(m_targetCircle, TiledObjectBody::getFilter(TiledObjectBody::FixtureTarget,

																TiledObjectBody::FixturePlayerBody |
																TiledObjectBody::FixtureEnemyBody |
																TiledObjectBody::FixtureBulletBody |
																TiledObjectBody::FixtureSensor
																));

	if (m_bodyRef->space != NULL) {
		cpSpaceAddShape(m_bodyRef->space, m_targetCircle);
	}
}




/**
 * @brief TiledObjectBodyPrivate::setVelocity
 * @param speed
 */

void TiledObjectBodyPrivate::setVelocity(const cpVect &speed)
{
	const cpVect &curr = cpBodyGetVelocity(m_bodyRef);

	cpVect norm;
	norm.x = qAbs(speed.x) < 0.00001f ? 0.0f : speed.x;
	norm.y = qAbs(speed.y) < 0.00001f ? 0.0f : speed.y;

	if (!(curr == cpvzero) && norm == cpvzero) {
		cpBodySetVelocity(m_bodyRef, cpvzero);
		cpBodySetAngularVelocity(m_bodyRef, 0.f);
		return;
	}

	if ((qAbs(curr.x - norm.x) * 10000.f > QtPrivate::min(qAbs(curr.x), qAbs(norm.x))) ||
			(qAbs(curr.y - norm.y) * 10000.f > QtPrivate::min(qAbs(curr.y), qAbs(norm.y)))) {
		cpBodySetVelocity(m_bodyRef, norm);
	}

}





/**
 * @brief TiledObjectBodyPrivate::drawShape
 * @param draw
 * @param shape
 * @param color
 * @param lineWidth
 * @param filled
 * @param outlined
 */

void TiledObjectBodyPrivate::drawShape(TiledDebugDraw *draw, cpShape *shape,
									   const QColor &color, const qreal &lineWidth,
									   const bool filled, const bool outlined) const
{
	Q_ASSERT(shape);

	const QPointF &pos = q->bodyPosition();

	if (shape->klass->type == CP_CIRCLE_SHAPE) {
		cpVect offset = cpCircleShapeGetOffset(shape);

		if (cpBody *body = shape->body)
			offset = cpTransformVect(body->transform, offset);

		offset.x += pos.x();
		offset.y += pos.y();


		const cpFloat &radius = cpCircleShapeGetRadius(shape);

		if (filled)
			draw->drawSolidCircle(offset, radius, color);

		if (outlined) {
			draw->drawCircle(offset, radius,
							 filled && outlined ? color.lighter() : color,
							 lineWidth);
		}
	} else if (shape->klass->type == CP_POLY_SHAPE) {
		if (filled)
			draw->drawSolidPolygon(shape, pos, color);

		if (outlined) {
			draw->drawPolygon(shape, pos,
							  filled && outlined ? color.lighter() : color,
							  lineWidth);
		}
	}
}





/**
 * @brief TiledObject::currentAngle
 * @return
 */

float TiledObject::currentAngle() const
{
	return bodyRotation();
}



void TiledObject::setCurrentAngle(float newCurrentAngle)
{
	if (qFuzzyCompare(desiredBodyRotation(), newCurrentAngle))
		return;

	rotateBody(newCurrentAngle);
	emit currentAngleChanged();
}



/**
 * @brief TiledObject::facingDirectionLocked
 * @return
 */

bool TiledObject::facingDirectionLocked() const
{
	return m_facingDirectionLocked;
}

void TiledObject::setFacingDirectionLocked(bool newFacingDirectionLocked)
{
	if (m_facingDirectionLocked == newFacingDirectionLocked)
		return;
	m_facingDirectionLocked = newFacingDirectionLocked;
	emit facingDirectionLockedChanged();
}

QPointF TiledObject::bodyOffset() const
{
	return m_bodyOffset;
}


bool TiledObject::inVisibleArea() const
{
	return m_inVisibleArea;
}




/**
 * @brief RayCastInfo::contains
 * @param shape
 * @return
 */

bool RayCastInfo::contains(const cpShape *shape) const {
	return cend() != std::find_if(cbegin(), cend(), [shape](const RayCastInfoItem &item){
		return item.shape == shape;
	});
}

bool RayCastInfo::contains(const cpBody *body) const {
	return cend() != std::find_if(cbegin(), cend(), [body](const RayCastInfoItem &item){
		return cpShapeGetBody(item.shape) == body;
	});
}

bool RayCastInfo::contains(TiledObjectBody *body) const
{
	if (body && body->body())
		return contains(body->body());
	else
		return false;
}

bool RayCastInfo::isVisible(const cpShape *shape) const {
	const auto &it = std::find_if(cbegin(), cend(), [shape](const RayCastInfoItem &item){
		return item.shape == shape;
	});

	return it != cend() && it->visible;
}

bool RayCastInfo::isVisible(const cpBody *body) const {
	const auto &it = std::find_if(cbegin(), cend(), [body](const RayCastInfoItem &item){
		return cpShapeGetBody(item.shape) == body;
	});

	return it != cend() && it->visible;
}

bool RayCastInfo::isVisible(TiledObjectBody *body) const
{
	if (body && body->body())
		return isVisible(body->body());
	else
		return false;
}

bool RayCastInfo::isWalkable(const cpShape *shape) const {
	const auto &it = std::find_if(cbegin(), cend(), [shape](const RayCastInfoItem &item){
		return item.shape == shape;
	});

	return it != cend() && it->walkable;
}

bool RayCastInfo::isWalkable(const cpBody *body) const {
	const auto &it = std::find_if(cbegin(), cend(), [body](const RayCastInfoItem &item){
		return cpShapeGetBody(item.shape) == body;
	});

	return it != cend() && it->walkable;
}

bool RayCastInfo::isWalkable(TiledObjectBody *body) const
{
	if (body && body->body())
		return isWalkable(body->body());
	else
		return false;
}
