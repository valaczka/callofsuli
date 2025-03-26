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
#include <box2d/base.h>


/**
 * @brief TiledObject::scene
 * @return
 */

TiledObject::TiledObject(TiledScene *scene)
	: TiledObject(scene ? scene->world() : nullptr, scene)
{

}



/**
 * @brief TiledObject::TiledObject
 * @param world
 * @param parent
 */

TiledObject::TiledObject(b2::World *world, QObject *parent)
	: QObject(parent)
	, TiledObjectBody(world)
{
	LOG_CTRACE("scene") << "TiledObject created" << this << scene();

}

/**
 * @brief TiledObject::~TiledObject
 */

TiledObject::~TiledObject()
{
	LOG_CTRACE("scene") << "TiledObject destroyed" << this << scene();

	if (m_spriteHandler)
		m_spriteHandler->setBaseObject(nullptr);

	if (m_spriteHandlerAuxFront)
		m_spriteHandlerAuxFront->setBaseObject(nullptr);

	if (m_spriteHandlerAuxBack)
		m_spriteHandlerAuxBack->setBaseObject(nullptr);
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
	if (radian < -B2_PI || radian > B2_PI) {
		LOG_CTRACE("scene") << "Invalid radian:" << radian;
		return B2_PI;
	}

	if (radian < 0)
		return B2_PI+B2_PI+radian;
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
	if (normal < 0 || normal > 2*B2_PI) {
		LOG_CTRACE("scene") << "Invalid normalized radian:" << normal;
		return 0.;
	}

	if (normal > B2_PI)
		return normal-2*B2_PI;
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

	const auto bPos = body().GetPosition();
	QPointF pos(bPos.x, bPos.y);

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

	const auto pos = body().GetPosition();

	setSpeed(point.x() - pos.x,
			 point.y() - pos.y);
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
 * @brief TiledObject::inVisibleArea
 * @return
 */

bool TiledObjectBody::inVisibleArea() const
{
	return m_inVisibleArea;
}




/**
 * @brief TiledObjectBody::setInVisibleArea
 * @param newInVisibleArea
 */

void TiledObjectBody::setInVisibleArea(bool newInVisibleArea)
{
	if (m_inVisibleArea == newInVisibleArea)
		return;
	m_inVisibleArea = newInVisibleArea;
}



/**
 * @brief TiledObjectBody::updateBodyInVisibleArea
 */

void TiledObjectBody::updateBodyInVisibleArea()
{
	TiledScene *s = scene();
	if (!s)
		return;

	setInVisibleArea(s->visibleArea().intersects(bodyAABB()));
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

	for (const b2::ShapeRef &sh : d->m_bodyShapes) {
		QColor c = color;
		const auto &category = sh.GetFilter().categoryBits;
		for (const auto &[fixture, fcolor] : fixtureColors.asKeyValueRange()) {
			if (category & fixture)
				c = fcolor;
		}

		if (sh.IsSensor()) {
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
	const b2Transform transform = d->m_bodyRef.GetTransform();

	static const float radius = 10.;

	b2Vec2 up = b2Vec2(transform.p.x + transform.q.c * radius,
					   transform.p.y + transform.q.s * radius);

	b2Rot r = b2MakeRot(b2Rot_GetAngle(transform.q)+B2_PI/2);

	b2Vec2 right = b2Vec2(transform.p.x + r.c * radius,
						  transform.p.y + r.s * radius);

	draw->drawSegment(up, transform.p, colorY, lineWidth);
	draw->drawSegment(transform.p, right, colorX, lineWidth);
}





/**
 * @brief TiledObjectBody::setGame
 * @param newGame
 */

void TiledObjectBody::setGame(TiledGame *newGame)
{
	m_game = newGame;
}




/**
 * @brief TiledObjectBody::updateVisibleArea
 */

void TiledObject::updateVisibleArea()
{
	updateBodyInVisibleArea();
}

void TiledObject::worldChanged()
{
	emit sceneChanged();
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

	m_visualItem->setParent(this);
	m_visualItem->setProperty("baseObject", QVariant::fromValue(this));

	connect(m_visualItem, &QQuickItem::xChanged, this, &TiledObject::updateVisibleArea, Qt::QueuedConnection);
	connect(m_visualItem, &QQuickItem::yChanged, this, &TiledObject::updateVisibleArea, Qt::QueuedConnection);
	connect(m_visualItem, &QQuickItem::widthChanged, this, &TiledObject::updateVisibleArea, Qt::QueuedConnection);
	connect(m_visualItem, &QQuickItem::heightChanged, this, &TiledObject::updateVisibleArea, Qt::QueuedConnection);

	emit visualItemChanged();
}




/**
 * @brief TiledObject::setInVisibleArea
 * @param newInVisibleArea
 */

void TiledObject::setInVisibleArea(bool newInVisibleArea)
{
	if (m_inVisibleArea == newInVisibleArea)
		return;
	m_inVisibleArea = newInVisibleArea;
	emit inVisibleAreaChanged();
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
	if (!d->m_bodyRef) {
		LOG_CERROR("scene") << "Missing body" << this;
		return;
	}

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
	if (!d->m_bodyRef) {
		LOG_CERROR("scene") << "Missing body" << this;
		return 0.;
	}

	const auto bp = d->m_bodyRef.GetPosition();

	const QVector2D p = point - QVector2D(bp.x, bp.y);
	return atan2(p.y(), p.x());
}


/**
 * @brief TiledObject::distanceToPoint
 * @param point
 * @return
 */

float TiledObjectBody::distanceToPoint(const QPointF &point) const
{
	if (!d->m_bodyRef) {
		LOG_CERROR("scene") << "Missing body" << this;
		return -1.;
	}

	return QVector2D(bodyPosition()).distanceToPoint(QVector2D(point));
}



/**
 * @brief TiledObjectBody::distanceToPoint
 * @param point
 * @return
 */

float TiledObjectBody::distanceToPoint(const QVector2D &point) const
{
	if (!d->m_bodyRef) {
		LOG_CERROR("scene") << "Missing body" << this;
		return -1.;
	}

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

void TiledObjectBody::setObjectId(const int &sceneId, const int &id)
{
	m_objectId = {.sceneId = sceneId, .id = id};
}



/**
 * @brief TiledObjectBody::world
 * @return
 */

b2::World *TiledObjectBody::world() const
{
	return d->m_world;
}



/**
 * @brief TiledObjectBody::setWorld
 * @param newWorld
 * @param position
 */

void TiledObjectBody::setWorld(b2::World *newWorld, const QPointF &position)
{
	setWorld(newWorld, position, d->m_bodyRef ? d->m_bodyRef.GetRotation() : b2MakeRot(0.));
}



/**
 * @brief TiledObjectBody::setWorld
 * @param newWorld
 * @param position
 * @param rotation
 */

void TiledObjectBody::setWorld(b2::World *newWorld, const QPointF &position, const b2Rot &rotation)
{
	Q_ASSERT(newWorld);

	d->replaceWorld(newWorld, position, rotation);
	d->updateScene();

	worldChanged();
}



/**
 * @brief TiledObjectBody::scene
 * @return
 */

TiledScene *TiledObjectBody::scene() const
{
	return d->m_world ? static_cast<TiledScene*>(d->m_world->GetUserData()) : nullptr;
}


/**
 * @brief TiledObjectBody::body
 * @return
 */

b2::BodyRef TiledObjectBody::body() const
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
 * @brief TiledObjectSensorPolygon::length
 * @return
 */

/*
TiledObjectSensorPolygon::TiledObjectSensorPolygon(Box2DBody *body, QQuickItem *parent)
	: QObject(parent)
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

*/





/**
 * @brief TiledObjectBody::synchronize
 */

TiledObjectBody::TiledObjectBody(b2::World *world)
	: d(new TiledObjectBodyPrivate(this, world))
{
	LOG_CTRACE("scene") << "TiledObjectBody created" << this << world;

	if (TiledScene *s = scene()) {
		setGame(s->game());
	}

	d->updateScene();
}


/**
 * @brief TiledObjectBody::TiledObjectBody
 * @param scene
 */

TiledObjectBody::TiledObjectBody(TiledScene *scene)
	: TiledObjectBody(scene ? scene->world() : nullptr)
{

}



/**
 * @brief TiledObjectBody::~TiledObjectBody
 */

TiledObjectBody::~TiledObjectBody()
{
	d->m_world = nullptr;
	d->updateScene();
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
	if (!d->m_bodyRef) {
		LOG_CERROR("scene") << "Missing body" << this;
		return {};
	}

	const auto pos = d->m_bodyRef.GetPosition();
	return QPointF{pos.x, pos.y};
}



/**
 * @brief TiledObjectBody::bodyAABB
 * @return
 */

QRectF TiledObjectBody::bodyAABB() const
{
	QRectF r;
	r.setLeft(d->m_bodyAABB.lowerBound.x);
	r.setTop(d->m_bodyAABB.lowerBound.y);
	r.setRight(d->m_bodyAABB.upperBound.x);
	r.setBottom(d->m_bodyAABB.upperBound.y);
	return r;
}


/**
 * @brief TiledObjectBody::currentSpeed
 * @return
 */

QVector2D TiledObjectBody::currentSpeed() const
{
	return d->m_currentSpeed;
}


/**
 * @brief TiledObjectBody::bodyShapes
 * @return
 */

const std::vector<b2::ShapeRef> &TiledObjectBody::bodyShapes() const
{
	return d->m_bodyShapes;
}


/**
 * @brief TiledObjectBody::sensorPolygon
 * @return
 */

b2::ShapeRef TiledObjectBody::sensorPolygon() const
{
	return d->m_sensorPolygon;
}

b2::ShapeRef TiledObjectBody::virtualCircle() const
{
	return d->m_virtualCircle;
}

b2::ShapeRef TiledObjectBody::targetCircle() const
{
	return d->m_targetCircle;
}


/**
 * @brief TiledObjectBody::isEqual
 * @param s1
 * @param s2
 * @return
 */

bool TiledObjectBody::isEqual(const b2::ShapeRef &s1, const b2::ShapeRef &s2)
{
	const auto &id1 = s1.Handle();
	const auto &id2 = s2.Handle();

	return (id1.index1 == id2.index1 && id1.world0 == id2.world0 && id1.revision == id2.revision);
}


/**
 * @brief TiledObjectBody::isEqual
 * @param s1
 * @param s2
 * @return
 */

bool TiledObjectBody::isEqual(const b2::BodyRef &s1, const b2::BodyRef &s2)
{
	const auto &id1 = s1.Handle();
	const auto &id2 = s2.Handle();

	return (id1.index1 == id2.index1 && id1.world0 == id2.world0 && id1.revision == id2.revision);
}


/**
 * @brief TiledObjectBody::isAny
 * @param s1
 * @param s2
 * @return
 */

bool TiledObjectBody::isAny(const std::vector<b2::ShapeRef> &s1, const b2::ShapeRef &s2)
{
	for (const auto &sh : s1) {
		if (isEqual(sh, s2))
			return true;
	}

	return false;
}


/**
 * @brief TiledObjectBody::emplace
 * @param centerX
 * @param centerY
 */

void TiledObjectBody::emplace(const QVector2D &center)
{
	if (!d->m_bodyRef) {
		LOG_CERROR("scene") << "Missing body" << this;
		return;
	}

	d->m_bodyRef.SetAngularVelocity(0.f);
	d->m_bodyRef.SetLinearVelocity({0.f, 0.f});
	d->m_bodyRef.SetTransform({(float)center.x(), (float)center.y()}, d->m_bodyRef.GetRotation());
	d->m_bodyRef.SetAwake(true);

	d->m_lastPosition = center;
	d->m_currentSpeed = {0., 0.};
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
	if (!d->m_bodyRef) {
		LOG_CERROR("scene") << "Missing body" << this;
		return;
	}

	d->m_bodyRef.SetLinearVelocity({x, y});
}


/**
 * @brief TiledObjectBody::setSpeedFromAngle
 * @param angle
 * @param radius
 */

void TiledObjectBody::setSpeedFromAngle(const float &angle, const float &radius)
{
	if (!d->m_bodyRef) {
		LOG_CERROR("scene") << "Missing body" << this;
		return;
	}

	d->m_bodyRef.SetLinearVelocity({
									   radius * cos(angle),
									   radius * sin(angle)
								   });
}


/**
 * @brief TiledObjectBody::stop
 */

void TiledObjectBody::stop()
{
	if (!d->m_bodyRef) {
		LOG_CERROR("scene") << "Missing body" << this;
		return;
	}

	d->m_bodyRef.SetAngularVelocity(0.f);
	d->m_bodyRef.SetLinearVelocity({0.f, 0.f});
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
	if (!d->m_bodyRef) {
		LOG_CERROR("scene") << "Missing body" << this;
		return 0.f;
	}

	return b2Rot_GetAngle(d->m_bodyRef.GetRotation());
}


/**
 * @brief TiledObjectBody::rotateBody
 * @param desiredRadian
 * @return
 */

bool TiledObjectBody::rotateBody(const float &desiredRadian, const bool &forced)
{
	if (!d->m_bodyRef) {
		LOG_CERROR("scene") << "Missing body" << this;
		return false;
	}

	if (forced) {
		d->m_rotateAnimation.running = false;
		d->m_bodyRef.SetAngularVelocity(0.f);
		d->m_bodyRef.SetTransform(d->m_bodyRef.GetPosition(), b2MakeRot(desiredRadian));
		d->m_bodyRef.SetAwake(true);
		return true;
	}

	const float currentRadian = bodyRotation();

	if (qFuzzyCompare(desiredRadian, currentRadian)) {
		d->m_rotateAnimation.running = false;
		return false;
	}


	const float currentNormal = TiledObject::normalizeFromRadian(currentRadian);
	const float desiredNormal = TiledObject::normalizeFromRadian(desiredRadian);

	const float diff = std::abs(currentNormal-desiredNormal);

	if (diff < 2* d->m_rotateAnimation.speed) {
		d->m_rotateAnimation.running = false;
		d->m_bodyRef.SetAngularVelocity(0.f);
		d->m_bodyRef.SetTransform(d->m_bodyRef.GetPosition(), b2MakeRot(desiredRadian));
		d->m_bodyRef.SetAwake(true);
		return true;
	}

	if (!qFuzzyCompare(d->m_rotateAnimation.destRadian, desiredRadian) || !d->m_rotateAnimation.running) {
		d->m_rotateAnimation.destRadian = desiredRadian;


		if (desiredNormal > currentNormal)
			d->m_rotateAnimation.clockwise = diff > B2_PI;
		else
			d->m_rotateAnimation.clockwise = diff < B2_PI;

		d->m_rotateAnimation.running = true;
	}

	if (!d->m_rotateAnimation.running)
		return false;


	const float delta = std::min(d->m_rotateAnimation.speed, M_PI_4);
	float newAngle = d->m_rotateAnimation.clockwise ? currentNormal - delta : currentNormal + delta;

	d->m_bodyRef.SetAngularVelocity(0.f);

	static const float pi2 = 2*B2_PI;

	// 0 átlépés miatt kell
	const float dd = (desiredNormal == 0 && newAngle > pi2) ? pi2 : desiredNormal;

	if ((newAngle >= dd && currentNormal < dd) ||
			(newAngle <= dd && currentNormal > dd)) {
		d->m_bodyRef.SetTransform(d->m_bodyRef.GetPosition(), b2MakeRot(desiredRadian));
		d->m_bodyRef.SetAwake(true);
		return true;
	}

	if (newAngle > pi2)
		newAngle -= pi2;
	else if (newAngle < 0)
		newAngle += pi2;

	d->m_bodyRef.SetTransform(d->m_bodyRef.GetPosition(), b2MakeRot(TiledObject::normalizeToRadian(newAngle)));
	d->m_bodyRef.SetAwake(true);

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
 * @brief TiledObjectBody::isBodyEnabled
 * @return
 */

bool TiledObjectBody::isBodyEnabled() const
{
	return d->m_bodyRef && d->m_bodyRef.IsEnabled();
}


/**
 * @brief TiledObjectBody::setBodyEnabled
 * @param enabled
 */

void TiledObjectBody::setBodyEnabled(const bool &enabled)
{
	if (!d->m_bodyRef) {
		LOG_CERROR("scene") << "Missing body" << this;
		return;
	}

	if (enabled)
		d->m_bodyRef.Enable();
	else
		d->m_bodyRef.Disable();
}


/**
 * @brief TiledObjectBody::overlap
 * @param pos
 * @return
 */

bool TiledObjectBody::overlap(const QPointF &pos) const
{
	if (!d->m_bodyRef)
		return false;

	for (const b2::ShapeRef &s : d->m_bodyShapes) {
		if (!s)
			continue;

		if (s.TestPoint({(float)pos.x(), (float)pos.y()}))
			return true;
	}

	return false;
}



/**
 * @brief TiledObjectBody::overlap
 * @param polygon
 * @return
 */

bool TiledObjectBody::overlap(const QPolygonF &polygon) const
{
	if (!d->m_bodyRef)
		return false;

	const b2Vec2 bodyPos = d->m_bodyRef.GetPosition();

	for (const b2::ShapeRef &s : d->m_bodyShapes) {
		if (!s)
			continue;

		if (s.GetType() == b2_polygonShape) {
			const b2Polygon p = s.GetPolygon();
			QPolygonF shapeP;
			shapeP.reserve(p.count);

			for (int i=0; i<p.count; ++i) {
				const b2Vec2 &v = p.vertices[i];
				shapeP.append(QPointF(v.x, v.y));
			}

			shapeP.translate(bodyPos.x, bodyPos.y);

			if (polygon.intersects(shapeP))
				return true;
		}
	}

	return false;
}


/**
 * @brief TiledObjectBody::worldStep
 * @param factor
 */

void TiledObjectBody::worldStep()
{
	const auto &p = d->m_bodyRef.GetPosition();
	QVector2D currPos(p.x, p.y);

	d->m_currentSpeed = currPos-d->m_lastPosition;
	d->m_lastPosition = currPos;

	if (d->m_rotateAnimation.running)
		rotateBody(d->m_rotateAnimation.destRadian);
}









/**
 * @brief TiledObjectBody::fromBodyRef
 * @param ref
 * @return
 */

TiledObjectBody *TiledObjectBody::fromBodyRef(b2::BodyRef ref)
{
	if (!ref)
		return nullptr;

	return static_cast<TiledObjectBody*>(ref.GetUserData());
}


/**
 * @brief TiledObjectBody::getFilter
 * @param categories
 * @return
 */

b2Filter TiledObjectBody::getFilter(const FixtureCategories &categories)
{
	b2Filter filter;
	filter.categoryBits = categories;
	filter.maskBits = B2_DEFAULT_MASK_BITS;
	return filter;
}


/**
 * @brief TiledObjectBody::getFilter
 * @param categories
 * @param collidesWith
 * @return
 */

b2Filter TiledObjectBody::getFilter(const FixtureCategories &categories, const FixtureCategories &collidesWith)
{
	b2Filter filter;
	filter.categoryBits = categories;
	filter.maskBits = collidesWith;
	return filter;
}




/**
 * @brief TiledObjectBody::createFromPolygon
 * @param polygon
 * @param renderer
 * @param params
 * @return
 */

bool TiledObjectBody::createFromPolygon(const QPolygonF &polygon, Tiled::MapRenderer *renderer, const b2::Shape::Params &params)
{
	b2::Body::Params bParams;

	bParams.type = b2BodyType::b2_staticBody;
	bParams.fixedRotation = true;

	return createFromPolygon(polygon, renderer, bParams, params);
}



/**
 * @brief TiledObjectBody::createFromPolygon
 * @param polygon
 * @param renderer
 * @param bParams
 * @param params
 * @return
 */

bool TiledObjectBody::createFromPolygon(const QPolygonF &polygon, Tiled::MapRenderer *renderer,
										b2::Body::Params bParams, const b2::Shape::Params &params)
{
	const QPolygonF &screenPolygon = renderer ? renderer->pixelToScreenCoords(polygon) : polygon;
	const QRectF &box = screenPolygon.boundingRect();
	const QPolygonF tPolygon = screenPolygon.translated(-box.center());

	bParams.position.x = box.center().x();
	bParams.position.y = box.center().y();

	d->createBody(bParams);

	b2::BodyRef b = d->m_bodyRef;

	Q_ASSERT(b);

	std::vector<b2Vec2> points;
	points.reserve(tPolygon.size());

	for (const QPointF &f : std::as_const(tPolygon))
		points.emplace_back(f.x(), f.y());


	const b2Hull hull = b2ComputeHull(points.data(), points.size());

	b2Polygon p = b2MakePolygon(&hull, 0.f);

	d->m_bodyShapes.push_back(b.CreateShape(b2::DestroyWithParent, params, p));

	d->m_bodyAABB = b.ComputeAABB();

	return true;
}



/**
 * @brief TiledObjectBody::createFromCircle
 * @param center
 * @param radius
 * @param renderer
 * @param params
 * @return
 */

bool TiledObjectBody::createFromCircle(const QPointF &center, const qreal &radius, Tiled::MapRenderer *renderer, const b2::Shape::Params &params)
{
	b2::Body::Params bParams;

	bParams.type = b2BodyType::b2_staticBody;
	bParams.fixedRotation = true;

	return createFromCircle(center, radius, renderer, bParams, params);
}


/**
 * @brief TiledObjectBody::createFromCircle
 * @param center
 * @param radius
 * @param renderer
 * @param bParams
 * @param params
 * @return
 */

bool TiledObjectBody::createFromCircle(const QPointF &center, const qreal &radius,
									   Tiled::MapRenderer *renderer, b2::Body::Params bParams,
									   const b2::Shape::Params &params)
{

	const QPointF &pos = renderer ? renderer->pixelToScreenCoords(center) : center;

	bParams.position.x = pos.x();
	bParams.position.y = pos.y();

	d->createBody(bParams);

	b2::BodyRef b = d->m_bodyRef;

	Q_ASSERT(b);

	d->m_bodyShapes.push_back(b.CreateShape(b2::DestroyWithParent, params, b2Circle{{0.f, 0.f}, (float) radius}));
	d->m_bodyAABB = b.ComputeAABB();

	return true;
}


/**
 * @brief TiledObjectBody::createFromMapObject
 * @param object
 * @param renderer
 * @param params
 * @return
 */

bool TiledObjectBody::createFromMapObject(const Tiled::MapObject *object, Tiled::MapRenderer *renderer, const b2::Shape::Params &params)
{
	b2::Body::Params bParams;

	bParams.type = b2BodyType::b2_staticBody;
	bParams.fixedRotation = true;

	return createFromMapObject(object, renderer, bParams, params);
}


/**
 * @brief TiledObjectBody::createFromMapObject
 * @param object
 * @param renderer
 * @param bParams
 * @param params
 * @return
 */

bool TiledObjectBody::createFromMapObject(const Tiled::MapObject *object, Tiled::MapRenderer *renderer,
										  b2::Body::Params bParams, const b2::Shape::Params &params)
{

	QPointF offset;

	if (Tiled::ObjectGroup *gLayer = object->objectGroup()) {
		offset = gLayer->totalOffset();
	}

	switch (object->shape()) {
		case Tiled::MapObject::Rectangle:
			return createFromPolygon(object->bounds().translated(offset), renderer, bParams, params);
		case Tiled::MapObject::Polygon:
			return createFromPolygon(object->polygon().translated(offset+object->position()), renderer, bParams, params);
		case Tiled::MapObject::Ellipse:
		case Tiled::MapObject::Point:
			return createFromCircle(offset+object->position(), std::max(object->size().width()/2, object->size().height()/2),
									renderer, bParams, params);
		default:
			LOG_CERROR("scene") << "Invalid Tiled::MapObject shape" << object->shape();
	}

	return false;
}




/**
 * @brief TiledObjectBody::setSensorPolygon
 * @param length
 * @param range
 */

void TiledObjectBody::setSensorPolygon(const float &length, const float &range)
{
	std::optional<b2Filter> collidesWith = std::nullopt;

	if (d->m_sensorPolygon)
		collidesWith = d->m_sensorPolygon.GetFilter();

	d->setSensorPolygon(length, range);

	if (collidesWith.has_value())
		d->m_sensorPolygon.SetFilter(collidesWith.value());
}



/**
 * @brief TiledObjectBody::setSensorPolygon
 * @param length
 * @param range
 * @param categories
 */

void TiledObjectBody::setSensorPolygon(const float &length, const float &range, const FixtureCategories &collidesWith)
{
	d->setSensorPolygon(length, range);

	auto filter = d->m_sensorPolygon.GetFilter();
	filter.maskBits = collidesWith;
	d->m_sensorPolygon.SetFilter(filter);
}



/**
 * @brief TiledObjectBody::addVirtualCircle
 */

void TiledObjectBody::addVirtualCircle(const float &length)
{
	std::optional<b2Filter> collidesWith = std::nullopt;

	if (d->m_virtualCircle)
		collidesWith = d->m_virtualCircle.GetFilter();

	d->addVirtualCircle(length > 0 ? length : d->m_sensorLength);

	if (collidesWith.has_value() && d->m_virtualCircle)
		d->m_virtualCircle.SetFilter(collidesWith.value());
}


/**
 * @brief TiledObjectBody::addVirtualCircle
 * @param collidesWith
 */

void TiledObjectBody::addVirtualCircle(const FixtureCategories &collidesWith, const float &length)
{
	d->addVirtualCircle(length > 0 ? length : d->m_sensorLength);

	if (d->m_virtualCircle) {
		auto filter = d->m_virtualCircle.GetFilter();
		filter.maskBits = collidesWith;
		d->m_virtualCircle.SetFilter(filter);
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
 */

TiledReportedFixtureMap TiledObjectBody::rayCast(const QPointF &dest, const TiledObjectBody::FixtureCategories &categories, const bool &forceLine) const
{
	Q_ASSERT(d->m_world);

	TiledReportedFixtureMap map;

	if (!d->m_bodyRef)
		return map;


	auto fcn = [&map]( b2ShapeId shapeId, b2Vec2 point, b2Vec2 /*normal*/, float fraction) -> float {
		TiledReportedFixture f {shapeId, {point.x, point.y}};

		f.body = fromBodyRef(f.shape.GetBody());

		if (f.shape) {
			map.insert(fraction, f);

			const b2Filter &filter = f.shape.GetFilter();

			if ((filter.categoryBits & FixtureGround) && f.body && f.body->opaque())
				return fraction;
			else
				return 1;
		} else {
			return -1;
		}
	};


	const b2Vec2 origin = d->m_bodyRef.GetPosition();
	const b2Vec2 end{(float)dest.x(), (float)dest.y()};

	b2Vec2 translation = b2Sub(end, origin);

	b2QueryFilter filter = b2DefaultQueryFilter();
	filter.categoryBits = FixtureAll;
	filter.maskBits = categories|FixtureGround;

	b2Transform transform = d->m_bodyRef.GetTransform();


	if (forceLine || d->m_bodyShapes.empty()) {
		d->m_world->CastRay(origin, translation, filter, fcn);
	} else {
		const b2::ShapeRef &sh = d->m_bodyShapes.front();

		if (sh.GetType() == b2_circleShape)
			d->m_world->Cast(sh.GetCircle(), transform, translation, filter, fcn);
		else if (sh.GetType() == b2_polygonShape)
			d->m_world->Cast(sh.GetPolygon(), transform, translation, filter, fcn);
		else if (sh.GetType() == b2_capsuleShape)
			d->m_world->Cast(sh.GetCapsule(), transform, translation, filter, fcn);
		else {
			LOG_CWARNING("scene") << "Invalid shape for ray cast" << sh.GetType();
			d->m_world->CastRay(origin, translation, filter, fcn);
		}
	}

	// Check contacted shapes

	int count = d->m_bodyShapes.front().GetContactCapacity();

	if (count > 0) {
		TiledReportedFixtureMap final = map;

		map.clear();

		std::vector<b2ContactData> contact;
		contact.resize(count);
		count = d->m_bodyRef.GetContactData(contact.data(), count);

		for (auto it = contact.cbegin(); it != contact.cend() && it != contact.cbegin()+count; ++it) {
			d->m_world->CastRay(origin, translation, filter, fcn);

			TiledObjectBody *other = nullptr;
			if (TiledObjectBody *b = fromBodyRef(b2::ShapeRef(it->shapeIdA).GetBody()); b && b != this
					&& b2::ShapeRef(it->shapeIdA).GetFilter().categoryBits & FixtureGround)
				other = b;
			else if (TiledObjectBody *b = fromBodyRef(b2::ShapeRef(it->shapeIdB).GetBody()); b && b != this
					 && b2::ShapeRef(it->shapeIdB).GetFilter().categoryBits & FixtureGround)
				other = b;

			if (other && other->opaque()) {
				const auto &it = map.find(other);
				if (it != map.cend()) {
					TiledReportedFixtureMap final;
					final.insert(0.0, *it);
					return final;
				}
			}
		}
		return final;
	}

	return map;
}



/**
 * @brief TiledObjectBody::debugDraw
 * @param draw
 */

void TiledObjectBody::debugDraw(TiledDebugDraw *draw) const
{
	if (!draw || !body())
		return;

	if (body().GetType() == b2_staticBody)
		drawBody(draw, QColorConstants::Svg::lightpink, 2.);
	else if (body().IsAwake())
		drawBody(draw, QColorConstants::Svg::limegreen, 2.);
	else if (body().IsEnabled())
		drawBody(draw, QColorConstants::Svg::steelblue, 2.);
	else
		drawBody(draw, QColorConstants::Svg::maroon, 2., true, false);


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

TiledObjectBodyPrivate::TiledObjectBodyPrivate(TiledObjectBody *body, b2::World *world)
	: q(body)
	, m_world(world)
{

}




/**
 * @brief TiledObjectBodyPrivate::createBody
 * @param params
 */

void TiledObjectBodyPrivate::createBody(const b2::Body::Params &params)
{
	Q_ASSERT(m_world);

	if (m_bodyRef)
		m_bodyRef.Destroy();

	m_bodyShapes.clear();

	m_bodyRef = m_world->CreateBody(b2::DestroyWithParent, params);
	m_bodyRef.SetUserData(q);
	m_lastPosition = QVector2D(params.position.x, params.position.y);
	m_currentSpeed = {0., 0.};
}



/**
 * @brief TiledObjectBodyPrivate::replaceWorld
 * @param world
 */

void TiledObjectBodyPrivate::replaceWorld(b2::World *world, const QPointF &position, const b2Rot &rotation)
{
	if (!m_bodyRef)
		return;

	struct ShapeData {
		b2ShapeType type;
		std::variant<b2Circle, b2Polygon> data;
		b2::Shape::Params params;
	};

	std::vector<ShapeData> shapes;

	for (const b2::ShapeRef &s : m_bodyShapes) {
		if (!s)
			continue;

		b2::Shape::Params params;

		params.friction = s.GetFriction();
		params.restitution = s.GetRestitution();
		params.density = s.GetDensity();
		params.filter = s.GetFilter();
		params.isSensor = s.IsSensor();

		b2ShapeType type = s.GetType();

		if (type == b2_circleShape) {
			shapes.emplace_back(type, s.GetCircle(), params);
		} else if (type == b2_polygonShape) {
			shapes.emplace_back(type, s.GetPolygon(), params);
		} else {
			LOG_CERROR("scene") << "Invalid shape" << type;
		}
	}

	b2::Body::Params bParams;

	bParams.type = m_bodyRef.GetType();
	bParams.fixedRotation = m_bodyRef.IsFixedRotation();
	bParams.position.x = position.x();
	bParams.position.y = position.y();
	bParams.rotation = rotation;

	m_bodyRef.Destroy();

	m_world = world;

	createBody(bParams);

	Q_ASSERT(m_bodyRef);

	for (const ShapeData &sh : shapes) {
		if (sh.type == b2_circleShape)
			m_bodyShapes.push_back(m_bodyRef.CreateShape(b2::DestroyWithParent, sh.params, std::get<b2Circle>(sh.data)));
	}

	///updateFilter();
}



/**
 * @brief TiledObjectBodyPrivate::updateScene
 */

void TiledObjectBodyPrivate::updateScene()
{
	TiledScene *currentScene = m_world ? static_cast<TiledScene*>(m_world->GetUserData()) : nullptr;

	if (m_scene == currentScene)
		return;

	if (m_scene)
		QObject::disconnect(m_sceneConnection);

	m_scene = currentScene;

	if (m_scene) {
		m_sceneConnection = QObject::connect(m_scene, &TiledScene::visibleAreaChanged, [this](){
			q->updateBodyInVisibleArea();
		});
	}
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

	if (m_sensorPolygon)
		m_sensorPolygon.Destroy(false);

	QPolygonF polygon;
	polygon.append(QPointF(0,0));

	for (int i=0; i < 7; ++i) {
		qreal angle = -(range/2.) + (i/6. * range);
		if (angle > M_PI)
			angle = -M_PI+(angle-M_PI);
		else if (angle < -M_PI)
			angle = M_PI-(-M_PI-angle);
		polygon.append(QPointF(length * cosf(angle), length * -sinf(angle)));
	}


	b2::Shape::Params params;
	params.isSensor = false;
	params.enableSensorEvents = true;
	params.enableContactEvents = true;
	params.filter = TiledObjectBody::getFilter(TiledObjectBody::FixtureSensor);

	std::vector<b2Vec2> points;
	points.reserve(polygon.size());

	for (const QPointF &f : polygon)
		points.emplace_back(f.x(), f.y());


	const b2Hull hull = b2ComputeHull(points.data(), points.size());

	b2Polygon p = b2MakePolygon(&hull, 0.f);

	m_sensorPolygon = m_bodyRef.CreateShape(b2::DestroyWithParent, params, p);

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

	if (m_virtualCircle)
		m_virtualCircle.Destroy(false);

	b2::Shape::Params params;
	params.isSensor = false;
	params.enableSensorEvents = true;
	params.enableContactEvents = true;
	params.filter = TiledObjectBody::getFilter(TiledObjectBody::FixtureVirtualCircle);

	m_virtualCircle = m_bodyRef.CreateShape(b2::DestroyWithParent, params, b2Circle{{0.f, 0.f}, length});
}


/**
 * @brief TiledObjectBodyPrivate::removeVirtualCircle
 */

void TiledObjectBodyPrivate::removeVirtualCircle()
{
	if (m_virtualCircle)
		m_virtualCircle.Destroy(false);

	m_virtualCircle = b2::ShapeRef();
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

	if (m_targetCircle)
		m_targetCircle.Destroy(false);

	b2::Shape::Params params;
	params.isSensor = true;
	params.enableSensorEvents = true;
	params.enableContactEvents = true;
	params.filter = TiledObjectBody::getFilter(TiledObjectBody::FixtureTarget,
											   TiledObjectBody::FixturePlayerBody |
											   TiledObjectBody::FixtureEnemyBody |
											   TiledObjectBody::FixtureBulletBody |
											   TiledObjectBody::FixtureSensor
											   );

	if (length > 0)
		m_targetLength = length;

	m_targetCircle = m_bodyRef.CreateShape(b2::DestroyWithParent, params, b2Circle{{0.f, 0.f}, m_targetLength});
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

void TiledObjectBodyPrivate::drawShape(TiledDebugDraw *draw, const b2::ShapeRef &shape,
									   const QColor &color, const qreal &lineWidth,
									   const bool filled, const bool outlined) const
{
	const b2Transform bTransform = m_bodyRef.GetTransform();
	if (shape.GetType() == b2_circleShape) {
		b2Circle c = shape.GetCircle();

		if (filled)
			draw->drawSolidCircle(bTransform, c.radius, color);

		if (outlined) {
			draw->drawCircle(b2TransformPoint(bTransform, c.center), c.radius,
							 filled && outlined ? color.lighter() : color,
							 lineWidth);
		}
	} else if (shape.GetType() == b2_polygonShape) {
		b2Polygon p = shape.GetPolygon();

		if (filled)
			draw->drawSolidPolygon(bTransform, p.vertices, p.count, p.radius, color);

		if (outlined) {
			draw->drawPolygon(bTransform, p.vertices, p.count,
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
	if (qFuzzyCompare(bodyRotation(), newCurrentAngle))
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


/**
 * @brief TiledReportedFixtureMap::containsTransparentGround
 * @return
 */

bool TiledReportedFixtureMap::containsTransparentGround() const
{
	for (const auto &ptr : *this) {
		if ((ptr.shape.GetFilter().categoryBits & TiledObjectBody::FixtureGround) && ptr.body && !ptr.body->opaque())
			return true;
	}
	return false;
}



/**
 * @brief TiledReportedFixtureMap::find
 * @param body
 * @return
 */

TiledReportedFixtureMap::QMultiMap::const_iterator TiledReportedFixtureMap::find(TiledObjectBody *body) const
{
	return std::find_if(this->cbegin(), this->cend(), [body](const TiledReportedFixture &f) { return f.body == body; });
}


/**
 * @brief TiledReportedFixtureMap::find
 * @param body
 * @return
 */

TiledReportedFixtureMap::iterator TiledReportedFixtureMap::find(TiledObjectBody *body)
{
	return std::find_if(this->begin(), this->end(), [body](const TiledReportedFixture &f) { return f.body == body; });
}


/**
 * @brief TiledObject::visualItem
 * @return
 */

QQuickItem *TiledObject::visualItem() const
{
	return m_visualItem;
}
