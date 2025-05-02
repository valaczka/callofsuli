/*
 * ---- Call of Suli ----
 *
 * tiledmapobject.h
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

#ifndef TILEDOBJECT_H
#define TILEDOBJECT_H

#include "Logger.h"
#include <QQuickItem>
#include <chipmunk/chipmunk.h>
#include <libtiled/maprenderer.h>
#include <libtiled/mapobject.h>
#include "tiledobjectspritedef.h"



class TiledDebugDraw;
class TiledGame;
class TiledScene;
class TiledSpriteHandler;
class TiledObjectBody;
class TiledObjectBodyPrivate;
class TiledObject;


#ifndef OPAQUE_PTR_TiledGame
#define OPAQUE_PTR_TiledGame
Q_DECLARE_OPAQUE_POINTER(TiledGame*)
#endif

#ifndef OPAQUE_PTR_TiledScene
#define OPAQUE_PTR_TiledScene
Q_DECLARE_OPAQUE_POINTER(TiledScene*)
#endif



#define POW2(x)	(x)*(x)

/**
 * @brief The RayCastInfoItem class
 */

struct RayCastInfoItem {
	cpShape *shape = nullptr;
	cpVect point;
	bool visible = false;
	bool walkable = false;
};


class RayCastInfo : public std::vector<RayCastInfoItem>
{
public:
	RayCastInfo() = default;

	bool contains(const cpShape *shape) const;
	bool contains(const cpBody *body) const;
	bool contains(TiledObjectBody *body) const;
	bool isVisible(const cpShape *shape) const;
	bool isVisible(const cpBody *body) const;
	bool isVisible(TiledObjectBody *body) const;
	bool isWalkable(const cpShape *shape) const;
	bool isWalkable(const cpBody *body) const;
	bool isWalkable(TiledObjectBody *body) const;
};


/**
 * @brief The TiledObjectBody class
 */

class TiledObjectBody
{
	Q_GADGET

private:
	TiledObjectBody(TiledGame *game);

public:
	explicit TiledObjectBody(const QPolygonF &polygon,
							 TiledGame *game,
							 Tiled::MapRenderer *renderer = nullptr,
							 const cpBodyType &type = CP_BODY_TYPE_DYNAMIC);

	explicit TiledObjectBody(const QPointF &center, const qreal &radius,
							 TiledGame *game,
							 Tiled::MapRenderer *renderer = nullptr,
							 const cpBodyType &type = CP_BODY_TYPE_DYNAMIC);

	explicit TiledObjectBody(const Tiled::MapObject *object,
							 TiledGame *game,
							 Tiled::MapRenderer *renderer = nullptr,
							 const cpBodyType &type = CP_BODY_TYPE_DYNAMIC);

	virtual ~TiledObjectBody();


	// ObjectId id

	struct ObjectId {
		int ownerId = -1;
		int sceneId = -1;
		int id = -1;

		friend bool operator== (const ObjectId &l, const ObjectId &r) {
			return l.ownerId == r.ownerId && l.id == r.id && l.sceneId == r.sceneId;
		}
	};

	enum FixtureCategory {
		FixtureInvalid		= 0,
		FixtureGround		= 0x1,
		FixturePlayerBody	= 0x1 << 1,
		FixtureEnemyBody	= 0x1 << 2,
		FixtureBulletBody	= 0x1 << 3,
		FixtureTarget		= 0x1 << 4,
		FixtureTransport	= 0x1 << 5,
		FixturePickable		= 0x1 << 6,
		FixtureTrigger		= 0x1 << 7,
		FixtureVirtualCircle = 0x1 << 8,
		FixtureSensor		= 0x1 << 9,
		FixtureContainer	= 0x1 << 10,

		FixtureAll =
		FixtureGround |
		FixturePlayerBody |
		FixtureEnemyBody |
		FixtureBulletBody |
		FixtureTarget |
		FixtureTransport |
		FixturePickable |
		FixtureTrigger |
		FixtureVirtualCircle |
		FixtureSensor |
		FixtureContainer
	};

	Q_ENUM(FixtureCategory);
	Q_DECLARE_FLAGS(FixtureCategories, FixtureCategory);
	Q_FLAG(FixtureCategories);


	const ObjectId &objectId() const;
	void setObjectId(const ObjectId &newObjectId);
	void setObjectId(const int &ownerId, const int &sceneId, const int &id);

	TiledGame *game() const;

	static QPointF toPointF(const cpVect &vect);
	static QVector2D toVector2D(const cpVect &vect);
	static cpVect toVect(const QPointF &point);
	static cpVect toVect(const QVector2D &point);

	cpSpace *space() const;
	TiledScene *scene() const;
	cpBody *body() const;
	cpVect bodyPosition() const;
	QPointF bodyPositionF() const { return toPointF(bodyPosition()); }
	QRectF bodyAABB() const;
	float currentSpeedSq() const;

	cpShapeFilter filterGet() const;
	void filterSet(const FixtureCategories &categories);
	void filterSet(const FixtureCategories &categories, const FixtureCategories &collidesWith);

	bool isSensor() const;
	void setSensor(const bool &sensor);


	const std::vector<cpShape*> &bodyShapes() const;
	bool isBodyShape(cpShape *shape) const;
	cpShape *sensorPolygon() const;
	cpShape *virtualCircle() const;
	cpShape *targetCircle() const;

	void emplace(const cpVect &center);
	void emplace(const QPointF &center) { emplace(toVect(center)); }
	void emplace(const qreal &centerX, const qreal &centerY) { emplace(cpv(centerX, centerY)); }

	void setSpeed(const cpVect &point);
	void setSpeed(const float &x, const float &y) { setSpeed(cpv(x, y)); }
	void setSpeedFromAngle(const float &angle, const float &radius);
	void stop();

	static cpVect vectorFromAngle(const float &angle, const float &radius) { return cpvmult(cpvforangle(angle), radius); }

	virtual bool moveTowards(const cpVect &point, const float &speed);
	bool moveTowardsLimited(const cpVect &point, const float &speedBelow, const float &destinationLimit, const float &speedAbove);
	virtual bool moveToPoint(const cpVect &point, const int &inFrame = 1, const float &maxSpeed = 0.);

	float bodyRotation() const;
	float desiredBodyRotation() const;
	bool rotateBody(const float &desiredRadian, const bool &forced = false);

	bool opaque() const;
	void setOpaque(bool newOpaque);

	virtual void worldStep();

	static TiledObjectBody *fromBodyRef(cpBody *ref);
	static TiledObjectBody *fromShapeRef(cpShape *ref);

	static cpShapeFilter getFilter(const FixtureCategories &categories);
	static cpShapeFilter getFilter(const FixtureCategories &categories, const FixtureCategories &collidesWith);


	void setSensorPolygon(const float &length, const float &range);
	void setSensorPolygon(const float &length, const float &range, const FixtureCategories &collidesWith);
	void addVirtualCircle(const float &length = 0.);
	void addVirtualCircle(const FixtureCategories &collidesWith, const float &length = 0.);
	void removeVirtualCircle();
	void addTargetCircle(const float &length);

	RayCastInfo rayCast(const cpVect &dest,
						const TiledObjectBody::FixtureCategories &categories = FixtureAll,
						const float &radius = 7.) const;

	virtual void debugDraw(TiledDebugDraw *draw) const;

	virtual void onShapeContactBegin(cpShape *self, cpShape *other) { Q_UNUSED(self); Q_UNUSED(other) }
	virtual void onShapeContactEnd(cpShape *self, cpShape *other) { Q_UNUSED(self); Q_UNUSED(other) }

	void rotateToPoint(const cpVect &point, const bool &forced = false);

	float angleToPoint(const cpVect &point) const;

	float distanceToPointSq(const cpVect &point) const;
	float distanceToPointSq(const QPointF &point) const { return distanceToPointSq(toVect(point)); }
	float distanceToPointSq(const QVector2D &point) const { return distanceToPointSq(toVect(point)); }

	QQuickItem *visualItem() const;
	void setVisualItem(QQuickItem *newVisualItem);

protected:
	virtual void synchronize() {}

	virtual void onSpaceChanged();
	void overrideCurrentSpeed(const cpVect &speed);

	void setSpace(cpSpace *space);

	void drawBody(TiledDebugDraw *draw, const QColor &color, const qreal &lineWidth = 1., const bool filled = true, const bool outlined = true) const;
	void drawSensor(TiledDebugDraw *draw, const QColor &color, const qreal &lineWidth = 1., const bool filled = true, const bool outlined = true) const;
	void drawVirtualCircle(TiledDebugDraw *draw, const QColor &color, const qreal &lineWidth = 1., const bool filled = false, const bool outlined = true) const;
	void drawTargetCircle(TiledDebugDraw *draw, const QColor &color, const qreal &lineWidth = 1., const bool filled = false, const bool outlined = true) const;
	/**
	 * @brief TiledObjectBody::drawCenter
	 * @param draw
	 * @param colorX
	 * @param colorY
	 * @param lineWidth
	 */
	void drawCenter(TiledDebugDraw *draw, const QColor &colorX, const QColor &colorY, const qreal &lineWidth = 2.) const;

	TiledGame *const m_game;
	QQuickItem *m_visualItem = nullptr;

private:
	cpShape *createFromPolygon(const QPolygonF &polygon,
							   Tiled::MapRenderer *renderer,
							   const cpBodyType &type = CP_BODY_TYPE_DYNAMIC);

	cpShape *createFromCircle(const QPointF &center, const qreal &radius,
							  Tiled::MapRenderer *renderer,
							  const cpBodyType &type = CP_BODY_TYPE_DYNAMIC);

	cpShape *createFromMapObject(const Tiled::MapObject *object,
								 Tiled::MapRenderer *renderer,
								 const cpBodyType &type = CP_BODY_TYPE_DYNAMIC);

	void deleteBody();

	TiledObjectBodyPrivate *d;
	bool m_opaque = true;
	ObjectId m_objectId;

	friend class TiledObjectBodyPrivate;
	friend class TiledGame;
};


Q_DECLARE_OPERATORS_FOR_FLAGS(TiledObjectBody::FixtureCategories);






/**
 * @brief The TiledObject class
 */

class TiledObject : public QObject, public TiledObjectBody
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(TiledGame *game READ game CONSTANT FINAL)
	Q_PROPERTY(TiledScene *scene READ scene NOTIFY sceneChanged FINAL)
	Q_PROPERTY(bool glowEnabled READ glowEnabled WRITE setGlowEnabled NOTIFY glowEnabledChanged FINAL)
	Q_PROPERTY(QColor glowColor READ glowColor WRITE setGlowColor NOTIFY glowColorChanged FINAL)
	Q_PROPERTY(bool overlayEnabled READ overlayEnabled WRITE setOverlayEnabled NOTIFY overlayEnabledChanged FINAL)
	Q_PROPERTY(QColor overlayColor READ overlayColor WRITE setOverlayColor NOTIFY overlayColorChanged FINAL)
	Q_PROPERTY(bool inVisibleArea READ inVisibleArea NOTIFY inVisibleAreaChanged FINAL)
	Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged FINAL)
	Q_PROPERTY(float currentAngle READ currentAngle WRITE setCurrentAngle NOTIFY currentAngleChanged FINAL)
	Q_PROPERTY(QPointF bodyOffset READ bodyOffset WRITE setBodyOffset NOTIFY bodyOffsetChanged FINAL)
	Q_PROPERTY(Direction facingDirection READ facingDirection WRITE setFacingDirection NOTIFY facingDirectionChanged FINAL)
	Q_PROPERTY(Directions availableDirections READ availableDirections WRITE setAvailableDirections NOTIFY availableDirectionsChanged FINAL)
	Q_PROPERTY(bool facingDirectionLocked READ facingDirectionLocked WRITE setFacingDirectionLocked NOTIFY facingDirectionLockedChanged FINAL)
	Q_PROPERTY(QQuickItem* visualItem READ visualItem NOTIFY visualItemChanged FINAL)

public:
	explicit TiledObject(const QPolygonF &polygon,
						 TiledGame *game,
						 Tiled::MapRenderer *renderer = nullptr,
						 const cpBodyType &type = CP_BODY_TYPE_DYNAMIC);

	explicit TiledObject(const QPointF &center, const qreal &radius,
						 TiledGame *game,
						 Tiled::MapRenderer *renderer = nullptr,
						 const cpBodyType &type = CP_BODY_TYPE_DYNAMIC);

	explicit TiledObject(const Tiled::MapObject *object,
						 TiledGame *game,
						 Tiled::MapRenderer *renderer = nullptr,
						 const cpBodyType &type = CP_BODY_TYPE_DYNAMIC);

	virtual ~TiledObject();


	// Object moving (facing) directions

	enum Direction {
		Invalid		= 0,
		NorthEast	= 45,
		East		= 90,
		SouthEast	= 135,
		South		= 180,
		SouthWest	= 225,
		West		= 270,
		NorthWest	= 315,
		North		= 360,
	};

	Q_ENUM(Direction);


	// Available sprite directions count

	enum Directions {
		None = 0,
		Direction_4,			// N, E, S, W
		Direction_8,			// N, NE, E, SE, S, SW, W, NW
		Direction_Infinite
	};

	Q_ENUM(Directions);



	enum AuxHandler {
		AuxFront = 0,
		AuxBack
	};


	Q_INVOKABLE void jumpToSprite(const char *sprite, const Direction &direction) const;
	Q_INVOKABLE void jumpToSpriteLater(const char *sprite, const Direction &direction) const;
	Q_INVOKABLE void jumpToSprite(const char *sprite) const {
		jumpToSprite(sprite, m_facingDirection);
	}
	Q_INVOKABLE void jumpToSpriteLater(const char *sprite) const {
		jumpToSpriteLater(sprite, m_facingDirection);
	}

	QStringList availableSprites() const;


	static qreal toRadian(const qreal &angle);
	static qreal directionToIsometricRadian(const Direction &direction);
	static qreal directionToRadian(const Direction &direction);

	static Direction nearestDirectionFromRadian(const Directions &directions, const qreal &angle);
	Direction nearestDirectionFromRadian(const qreal &angle) const { return nearestDirectionFromRadian(m_availableDirections, angle); };

	TiledSpriteHandler *spriteHandler() const;
	TiledSpriteHandler *spriteHandlerAuxFront() const;
	TiledSpriteHandler *spriteHandlerAuxBack() const;

	static std::optional<TextureSprite> toTextureSprite(const TiledObjectSprite &sprite, const QString &source);

	virtual QQuickItem *createMarkerItem(const QString &qrc = QStringLiteral("qrc:/TiledPlayerMarker.qml"));


	static QPolygonF toPolygon(const Tiled::MapObject *object, Tiled::MapRenderer *renderer);

	static float shortestDistance(const cpVect &point, const cpVect &lineP1, const cpVect &lineP2,
								  cpVect *destPoint = nullptr, float *factor = nullptr);
	static float shortestDistance(const cpVect &point, const QPointF &lineP1, const QPointF &lineP2,
								  cpVect *destPoint = nullptr, float *factor = nullptr)
	{
		return shortestDistance(point, toVect(lineP1), toVect(lineP2), destPoint, factor);
	}
	static float shortestDistance(const cpVect &point, const QLineF &line,
								  cpVect *destPoint = nullptr, float *factor = nullptr)
	{
		return shortestDistance(point, line.p1(), line.p2(), destPoint, factor);
	}


	virtual bool moveTowards(const cpVect &point, const float &speed) override;
	virtual bool moveToPoint(const cpVect &point, const int &inFrame = 1, const float &maxSpeed = 0.) override;

	void setBodyOffset(QPointF newBodyOffset);
	void setBodyOffset(const qreal &x, const qreal &y) { setBodyOffset(QPointF(x, y)); }
	QPointF bodyOffset() const;

	bool glowEnabled() const;
	void setGlowEnabled(bool newGlowEnabled);

	bool overlayEnabled() const;
	void setOverlayEnabled(bool newOverlayEnabled);

	QColor glowColor() const;
	void setGlowColor(const QColor &newGlowColor);

	QColor overlayColor() const;
	void setOverlayColor(const QColor &newOverlayColor);

	QString displayName() const;
	void setDisplayName(const QString &newDisplayName);

	Direction facingDirection() const;
	void setFacingDirection(const Direction &newFacingDirection);

	Directions availableDirections() const;
	void setAvailableDirections(const Directions &newAvailableDirections);

	float currentAngle() const;
	void setCurrentAngle(float newCurrentAngle);
	void setCurrentAngleForced(float newCurrentAngle);

	bool facingDirectionLocked() const;
	void setFacingDirectionLocked(bool newFacingDirectionLocked);

	bool inVisibleArea() const;

signals:
	void remoteModeChanged();
	void glowEnabledChanged();
	void overlayEnabledChanged();
	void glowColorChanged();
	void overlayColorChanged();
	void inVisibleAreaChanged();
	void displayNameChanged();
	void facingDirectionChanged();
	void availableDirectionsChanged();
	void sceneChanged();
	void currentAngleChanged();
	void facingDirectionLockedChanged();
	void bodyOffsetChanged();
	void visualItemChanged();

protected:
	bool appendSprite(const QString &source, const TiledObjectSprite &sprite);
	bool appendSprite(const QString &source, const TiledObjectSpriteList &spriteList);

	bool appendSprite(const QString &source, const IsometricObjectSprite &sprite);
	bool appendSprite(const QString &source, const IsometricObjectSpriteList &spriteList);

	bool appendSprite(const QString &source, const TextureSprite &sprite);
	bool appendSprite(const QString &source, const QVector<TextureSprite> &spriteList);

	bool playAuxSprite(const AuxHandler &auxHandler, const bool &alignToBody,
					   const QString &source, const TiledObjectSprite &sprite, const bool &replaceCurrentSprite = false) const;
	bool playAuxSprite(const QString &source, const TiledObjectSprite &sprite, const bool &replaceCurrentSprite = false) const {
		return playAuxSprite(AuxFront, false, source, sprite, replaceCurrentSprite);
	}

	bool playAuxSprite(const AuxHandler &auxHandler, const bool &alignToBody,
					   const QString &source, const TextureSprite &sprite, const bool &replaceCurrentSprite = false) const;
	bool playAuxSprite(const QString &source, const TextureSprite &sprite, const bool &replaceCurrentSprite = false) const {
		return playAuxSprite(AuxFront, false, source, sprite, replaceCurrentSprite);
	}

	void createVisual();
	virtual void synchronize() override;
	virtual void onSpaceChanged() override;

	void updateVisibleArea();
	void updateScene();

protected:
	bool m_inVisibleArea = false;
	bool m_glowEnabled = false;
	bool m_overlayEnabled = false;
	QColor m_glowColor = QColor(Qt::yellow);
	QColor m_overlayColor = QColor(Qt::white);
	QString m_displayName;
	QPointF m_bodyOffset;

	Direction m_facingDirection = Invalid;
	Directions m_availableDirections = None;
	bool m_facingDirectionLocked = true;
	float m_lastAngle = 0.f;
	TiledSpriteHandler *m_spriteHandler = nullptr;
	TiledSpriteHandler *m_spriteHandlerAuxFront = nullptr;
	TiledSpriteHandler *m_spriteHandlerAuxBack = nullptr;

	friend class TiledEffect;
	friend class TiledScene;
	friend class TiledObjectBody;

private:
	QPointer<TiledScene> m_currentScene;
};





/**
 * @brief TiledObjectBody::toPointF
 * @param vect
 * @return
 */

inline QPointF TiledObjectBody::toPointF(const cpVect &vect)
{
	return QPointF{vect.x, vect.y};
}


/**
 * @brief TiledObjectBody::toVector2D
 * @param vect
 * @return
 */

inline QVector2D TiledObjectBody::toVector2D(const cpVect &vect)
{
	return QVector2D(vect.x, vect.y);
}


/**
 * @brief TiledObjectBody::toVect
 * @param point
 * @return
 */

inline cpVect TiledObjectBody::toVect(const QPointF &point)
{
	return cpv(point.x(), point.y());
}


/**
 * @brief TiledObjectBody::toVect
 * @param point
 * @return
 */

inline cpVect TiledObjectBody::toVect(const QVector2D &point)
{
	return cpv(point.x(), point.y());
}



#endif // TILEDOBJECT_H
