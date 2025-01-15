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
#include <box2cpp/box2cpp.h>
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


struct TiledReportedFixture {
	b2::ShapeRef shape;
	QVector2D point;
	TiledObjectBody *body = nullptr;
};


class TiledReportedFixtureMap : public QMultiMap<float, TiledReportedFixture>
{
public:
	TiledReportedFixtureMap() : QMultiMap<float, TiledReportedFixture>() {}

	bool containsTransparentGround() const;
	TiledReportedFixtureMap::iterator find(TiledObjectBody *body);
	TiledReportedFixtureMap::const_iterator find(TiledObjectBody *body) const;
	bool contains(TiledObjectBody *body) const { return find(body) != this->cend(); }
};



/**
 * @brief The TiledObjectBody class
 */

class TiledObjectBody
{
	Q_GADGET

public:
	explicit TiledObjectBody(b2::World *world);
	explicit TiledObjectBody(TiledScene *scene);
	virtual ~TiledObjectBody();


	// ObjectId id

	struct ObjectId {
		int sceneId = -1;
		int id = -1;

		friend bool operator== (const ObjectId &l, const ObjectId &r) {
			return l.id == r.id && l.sceneId == r.sceneId;
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
	void setObjectId(const int &sceneId, const int &id);

	TiledGame *game() const;
	void setGame(TiledGame *newGame);

	b2::World *world() const;
	void setWorld(b2::World *newWorld, const QPointF &position = {});
	void setWorld(b2::World *newWorld, const QPointF &position, const b2Rot &rotation);
	TiledScene *scene() const;

	b2::BodyRef body() const;
	QPointF bodyPosition() const;
	QRectF bodyAABB() const;
	QVector2D currentSpeed() const;


	const std::vector<b2::ShapeRef> &bodyShapes() const;
	b2::ShapeRef sensorPolygon() const;
	b2::ShapeRef virtualCircle() const;
	b2::ShapeRef targetCircle() const;

	static bool isEqual(const b2::ShapeRef &s1, const b2::ShapeRef &s2);
	static bool isEqual(const b2::BodyRef &s1, const b2::BodyRef &s2);
	static bool isAny(const std::vector<b2::ShapeRef> &s1, const b2::ShapeRef &s2);

	void emplace(const QVector2D &center);
	void emplace(const QPointF &center) { emplace(QVector2D(center)); }
	void emplace(const qreal &centerX, const qreal &centerY) { emplace(QVector2D(centerX, centerY)); }

	void setSpeed(const QVector2D &point);
	void setSpeed(const QPointF &point);
	void setSpeed(const float &x, const float &y);
	void setSpeedFromAngle(const float &angle, const float &radius);
	void stop();

	static QVector2D vectorFromAngle(const float &angle, const float &radius);

	float bodyRotation() const;
	bool rotateBody(const float &desiredRadian, const bool &forced = false);

	bool opaque() const;
	void setOpaque(bool newOpaque);

	bool isBodyEnabled() const;
	void setBodyEnabled(const bool &enabled);

	bool inVisibleArea() const;

	bool overlap(const QPointF &pos) const;
	bool overlap(const QPolygonF &polygon) const;

	virtual void worldStep();
	virtual void synchronize();

	static TiledObjectBody *fromBodyRef(b2::BodyRef ref);

	static b2Filter getFilter(const FixtureCategories &categories);
	static b2Filter getFilter(const FixtureCategories &categories, const FixtureCategories &collidesWith);


	void setSensorPolygon(const float &length, const float &range);
	void setSensorPolygon(const float &length, const float &range, const FixtureCategories &collidesWith);
	void addVirtualCircle(const float &length = 0.);
	void addVirtualCircle(const FixtureCategories &collidesWith, const float &length = 0.);
	void addTargetCircle(const float &length);

	TiledReportedFixtureMap rayCast(const QPointF &dest, const FixtureCategories &categories,
									const bool &forceLine = false) const;

	virtual void debugDraw(TiledDebugDraw *draw) const;

	virtual void onShapeContactBegin(b2::ShapeRef self, b2::ShapeRef other) { Q_UNUSED(self); Q_UNUSED(other) }
	virtual void onShapeContactEnd(b2::ShapeRef self, b2::ShapeRef other) { Q_UNUSED(self); Q_UNUSED(other) }

	void rotateToPoint(const QPointF &point, const bool &forced = false);
	float angleToPoint(const QVector2D &point) const;
	float angleToPoint(const QPointF &point) const { return angleToPoint(QVector2D(point)); }
	float distanceToPoint(const QPointF &point) const;
	float distanceToPoint(const QVector2D &point) const;


protected:
	virtual void worldChanged() {}

	virtual void setInVisibleArea(bool newInVisibleArea);
	void updateBodyInVisibleArea();

	void drawBody(TiledDebugDraw *draw, const QColor &color, const qreal &lineWidth = 1., const bool filled = true, const bool outlined = true) const;
	void drawSensor(TiledDebugDraw *draw, const QColor &color, const qreal &lineWidth = 1., const bool filled = true, const bool outlined = true) const;
	void drawVirtualCircle(TiledDebugDraw *draw, const QColor &color, const qreal &lineWidth = 1., const bool filled = false, const bool outlined = true) const;
	void drawTargetCircle(TiledDebugDraw *draw, const QColor &color, const qreal &lineWidth = 1., const bool filled = false, const bool outlined = true) const;
	void drawCenter(TiledDebugDraw *draw, const QColor &colorX, const QColor &colorY, const qreal &lineWidth = 2.) const;

	TiledGame *m_game = nullptr;
	bool m_inVisibleArea = false;

private:
	bool createFromPolygon(const QPolygonF &polygon,
						   Tiled::MapRenderer *renderer,
						   const b2::Shape::Params &params = {});

	bool createFromPolygon(const QPolygonF &polygon,
						   Tiled::MapRenderer *renderer,
						   b2::Body::Params bParams,
						   const b2::Shape::Params &params = {});

	bool createFromCircle(const QPointF &center, const qreal &radius,
						  Tiled::MapRenderer *renderer,
						  const b2::Shape::Params &params = {});

	bool createFromCircle(const QPointF &center, const qreal &radius,
						  Tiled::MapRenderer *renderer,
						  b2::Body::Params bParams,
						  const b2::Shape::Params &params = {});

	bool createFromMapObject(const Tiled::MapObject *object,
							 Tiled::MapRenderer *renderer,
							 const b2::Shape::Params &params = {});

	bool createFromMapObject(const Tiled::MapObject *object,
							 Tiled::MapRenderer *renderer,
							 b2::Body::Params bParams,
							 const b2::Shape::Params &params = {});

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

class TiledObject : public QQuickItem, public TiledObjectBody
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


public:
	explicit TiledObject(TiledScene *scene);
	explicit TiledObject(b2::World *world, QQuickItem *parent = nullptr);

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

	static float shortestDistance(const QVector2D &point, const QVector2D &lineP1, const QVector2D &lineP2,
								  QVector2D *destPoint = nullptr, float *factor = nullptr);
	static float shortestDistance(const QVector2D &point, const QPointF &lineP1, const QPointF &lineP2,
								  QVector2D *destPoint = nullptr, float *factor = nullptr) {
		return shortestDistance(point, QVector2D(lineP1), QVector2D(lineP2), destPoint, factor);
	}
	static float shortestDistance(const QVector2D &point, const QLineF &line,
								  QVector2D *destPoint = nullptr, float *factor = nullptr) {
		return shortestDistance(point, line.p1(), line.p2(), destPoint, factor);
	}
	static float shortestDistance(const QPointF &point, const QPointF &lineP1, const QPointF &lineP2,
								  QVector2D *destPoint = nullptr, float *factor = nullptr) {
		return shortestDistance(QVector2D(point), QVector2D(lineP1), QVector2D(lineP2), destPoint, factor);
	}
	static float shortestDistance(const QPointF &point, const QLineF &line,
								  QVector2D *destPoint = nullptr, float *factor = nullptr) {
		return shortestDistance(QVector2D(point), line, destPoint, factor);
	}

	static float normalizeFromRadian(const float &radian);
	static float normalizeToRadian(const float &normal);

	void setBodyOffset(QPointF newBodyOffset);
	void setBodyOffset(const qreal &x, const qreal &y) { setBodyOffset(QPointF(x, y)); }
	QPointF bodyOffset() const;

	virtual void synchronize() override;

	bool moveTowards(const QVector2D &point, const float &speed);
	bool moveTowards(const QVector2D &point, const float &speedBelow, const float &destinationLimit, const float &speedAbove);

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

	bool facingDirectionLocked() const;
	void setFacingDirectionLocked(bool newFacingDirectionLocked);


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

	virtual void setInVisibleArea(bool newInVisibleArea) override;
	void updateVisibleArea();

	virtual void worldChanged() override;

protected:
	bool m_glowEnabled = false;
	bool m_overlayEnabled = false;
	QColor m_glowColor = QColor(Qt::yellow);
	QColor m_overlayColor = QColor(Qt::white);
	QString m_displayName;
	QPointF m_bodyOffset;

	QQuickItem *m_visualItem = nullptr;
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
};






#endif // TILEDOBJECT_H
