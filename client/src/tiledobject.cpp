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
	LOG_CTRACE("scene") << "TiledObjectBase created" << this;

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

	for (const QPointF &f : polygon)
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
 * @brief TiledObjectBase::rotateBody
 */

void TiledObjectBase::rotateBody(const float32 &desiredRadian)
{
	if (!m_sensorPolygon)
		return;

	const float32 currentAngle = m_body->body()->GetAngle();


	if (qFuzzyCompare(desiredRadian, currentAngle)) {
		if (m_rotateAnimation.running) {
			m_rotateAnimation.running = false;
		}
		return;
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
		return;


	const float32 delta = std::min(m_rotateAnimation.speed * m_body->world()->timeStep()*60., M_PI_4);
	float32 newAngle = m_rotateAnimation.clockwise ? currentNormal - delta : currentNormal + delta;

	m_body->setAngularVelocity(0);


	static const float32 pi2 = 2*b2_pi;

	// 0 átlépés miatt kell
	const float32 d = (desiredNormal == 0 && newAngle > pi2) ? pi2 : desiredNormal;


	if ((newAngle >= d && currentNormal < d) ||
			(newAngle <= d && currentNormal > d)) {
		m_body->body()->SetTransform(m_body->body()->GetPosition(), desiredRadian );
		return;
	}

	if (newAngle > pi2)
		newAngle -= pi2;
	else if (newAngle < 0)
		newAngle += pi2;

	m_body->body()->SetTransform(m_body->body()->GetPosition(), normalizeToRadian(newAngle) );

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
 * @brief TiledObjectBase::onSceneVisibleAreaChanged
 */

void TiledObjectBase::onSceneVisibleAreaChanged()
{
	if (!m_scene)
		return;

	QRectF rect(x(), y(), width(), height());

	setInVisibleArea(m_scene->visibleArea().intersects(rect));
}

int TiledObjectBase::objectId() const
{
	return m_objectId;
}

void TiledObjectBase::setObjectId(int newObjectId)
{
	if (m_objectId == newObjectId)
		return;
	m_objectId = newObjectId;
	emit objectIdChanged();
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
 * @brief TiledObject::jumpToSprite
 * @param sprite
 */

void TiledObject::jumpToSprite(const QString &sprite, const Direction &direction, const QString &alteration) const
{
	if (!m_spriteHandler) {
		LOG_CERROR("scene") << "Missing spriteHandler";
		return;
	}

	m_spriteHandler->jumpToSprite(sprite, alteration, direction, TiledSpriteHandler::JumpImmediate);
}


/**
 * @brief TiledObject::jumpToSpriteLater
 * @param sprite
 * @param alteration
 */

void TiledObject::jumpToSpriteLater(const QString &sprite, const Direction &direction, const QString &alteration) const
{
	if (!m_spriteHandler) {
		LOG_CERROR("scene") << "Missing spriteHandler";
		return;
	}

	m_spriteHandler->jumpToSprite(sprite, alteration, direction, TiledSpriteHandler::JumpAtFinished);
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

	return m_spriteHandler->addSprite(sprite, source);
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

	for (const TiledObjectSprite &s : spriteList.sprites)
		r &= m_spriteHandler->addSprite(s, source);

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

	bool r = true;

	for (const TiledObjectSprite &s : sprite.sprites) {
		for (auto it = sprite.alterations.constBegin(); it != sprite.alterations.constEnd(); ++it) {
			const QString &alteration = it.key();
			r &= m_spriteHandler->addSprite(s, alteration, path+it.value());
		}
	}

	m_availableAlterations.append(sprite.alterations.keys());

	return r;
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

	m_visualItem->setParentItem(this);
	m_visualItem->setParent(this);
	m_visualItem->setProperty("baseObject", QVariant::fromValue(this));
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

QStringList TiledObject::availableAlterations() const
{
	return m_availableAlterations;
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
 * @brief TiledObject::radianFromDirection
 * @param direction
 * @return
 */

qreal TiledObject::radianFromDirection(const Direction &direction)
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

	for (const auto &[a, dir] : *ptr) {
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
	//setCategories(Box2DFixture::None);
	setCollidesWith(Category1|Category2);

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

				emplace();

				return;
			}
		}
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




QPointF TiledObjectBody::bodyOffset() const
{
	return m_bodyOffset;
}

void TiledObjectBody::setBodyOffset(QPointF newBodyOffset)
{
	m_bodyOffset = newBodyOffset;
}


/**
 * @brief TiledObjectBody::rayCast
 * @param dest
 */

TiledReportedFixtureMap TiledObjectBody::rayCast(const QPointF &dest)
{
	Q_ASSERT(mWorld);

	TiledObjectRayCast ray(mWorld);

	mWorld->world().RayCast(&ray, mBody->GetPosition(), mWorld->toMeters(dest));

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

		r &= m_spriteHandler->addSprite(s2, direction, source);
	}

	return r;
}




/**
 * @brief IsometricObjectIface::appendSpriteList
 * @param source
 * @param spriteList
 * @return
 */

bool TiledObject::appendSpriteList(const QString &source, const IsometricObjectSpriteList &spriteList)
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
 * @brief IsometricObjectIface::appendSprite
 * @param sprite
 * @param path
 * @return
 */

bool TiledObject::appendSprite(const IsometricObjectAlterableSprite &sprite, const QString &path)
{
	Q_ASSERT(m_spriteHandler);

	bool r = true;

	for (const IsometricObjectSprite &s : sprite.sprites) {
		for (auto it = sprite.alterations.constBegin(); it != sprite.alterations.constEnd(); ++it) {
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

				r &= m_spriteHandler->addSprite(s2, alteration, direction, path+it.value());
			}
		}
	}

	m_availableAlterations.append(sprite.alterations.keys());

	return r;
}



/**
 * @brief IsometricObjectIface::appendSpriteList
 * @param sprite
 * @param path
 * @return
 */

bool TiledObject::appendSpriteList(const IsometricObjectAlterableSpriteList &sprite, const QString &path)
{
	Q_ASSERT(m_spriteHandler);

	for (const auto &s : sprite.list) {
		if (!appendSprite(s, path)) {
			LOG_CERROR("scene") << "Load sprite error:" << path;
			return false;
		}
	}

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
