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
#include "box2dbody.h"
#include "box2dfixture.h"
#include <QQuickItem>
#include <libtiled/maprenderer.h>
#include <libtiled/mapobject.h>
#include "tiledobjectspritedef.h"




class TiledGame;
class TiledScene;
class TiledSpriteHandler;
class TiledObjectBase;

#if QT_VERSION >= 0x060000

#ifndef OPAQUE_PTR_TiledGame
#define OPAQUE_PTR_TiledGame
Q_DECLARE_OPAQUE_POINTER(TiledGame*)
#endif

#ifndef OPAQUE_PTR_TiledScene
#define OPAQUE_PTR_TiledScene
Q_DECLARE_OPAQUE_POINTER(TiledScene*)
#endif

#endif





/**
 * @brief The TiledReportedFixture class
 */

struct TiledReportedFixture
{
	Box2DFixture *fixture = nullptr;
	b2Vec2 point = {0,0};
	b2Vec2 normal = {0,0};
};


typedef QMultiMap<float32, TiledReportedFixture> TiledReportedFixtureMap;




/**
 * @brief The TiledObjectBody class
 */


class TiledObjectBody : public Box2DBody
{
	Q_OBJECT

	Q_PROPERTY(QPointF bodyOffset READ bodyOffset WRITE setBodyOffset NOTIFY bodyOffsetChanged FINAL)
	Q_PROPERTY(bool opaque READ opaque WRITE setOpaque NOTIFY opaqueChanged FINAL)

public:
	explicit TiledObjectBody(TiledObjectBase *baseObject)
		: Box2DBody()
		, m_baseObject(baseObject)
	{
		Q_ASSERT(m_baseObject);
	}


	/**
	 * @brief The FixtureCategory enum
	 */

	// !!! set also in fixtureCategory()

	enum FixtureCategory {
		FixtureInvalid = 0,
		FixtureGround,
		FixturePlayerBody,
		FixtureEnemyBody,
		FixtureTarget,
		FixtureTransport,
		FixturePickable,
		FixtureTrigger,
		FixtureVirtualCircle,
		FixtureSensor,
		FixtureContainer
	};

	Q_ENUM(FixtureCategory);

	static Box2DFixture::CategoryFlag fixtureCategory(const FixtureCategory &category);
	static void setFixtureCollidesWithFlag(Box2DFixture *fixture, const FixtureCategory &category, const bool &on = true);

	void synchronize() override;
	void updateTransform() override;

	TiledObjectBase *baseObject() const { return m_baseObject; }
	QPointF bodyPosition() const;

	void emplace(const QPointF &center);
	void emplace(const qreal &centerX, const qreal &centerY) { emplace(QPointF(centerX, centerY)); }

	void setSpeed(const QPointF &point);
	void stop();

	QPointF bodyOffset() const;

	TiledReportedFixtureMap rayCast(const QPointF &dest, float32 *lengthPtr = nullptr);

	void setBodyOffset(QPointF newBodyOffset);

	bool opaque() const;
	void setOpaque(bool newOpaque);

signals:
	void bodyOffsetChanged();
	void opaqueChanged();

private:
	void emplace();
	QPointF m_bodyOffset;
	TiledObjectBase *const m_baseObject;
	bool m_opaque = true;

	friend class TiledObjectBase;
};





/**
 * @brief The TiledObjectSensorPolygon class
 */

class TiledObjectSensorPolygon : public Box2DPolygon
{
	Q_OBJECT

	Q_PROPERTY(qreal length READ length WRITE setLength NOTIFY lengthChanged FINAL)
	Q_PROPERTY(qreal range READ range WRITE setRange NOTIFY rangeChanged FINAL)

public:
	explicit TiledObjectSensorPolygon(Box2DBody *body, QQuickItem *parent = nullptr);

	qreal length() const;
	void setLength(qreal newLength);

	qreal range() const;
	void setRange(qreal newRange);

	Box2DCircle *virtualCircle() const;

signals:
	void lengthChanged();
	void rangeChanged();

private:
	void recreateFixture();

	qreal m_length = 50;
	qreal m_range = M_PI_4;

	std::unique_ptr<Box2DCircle> m_virtualCircle;
	Box2DBody *const m_body;
};








/**
 * @brief The TiledObjectPolygonIface class
 */

class TiledObjectPolygonIface
{
public:
	TiledObjectPolygonIface()
		: m_fixture(new Box2DPolygon)
	{}

	virtual Box2DPolygon *createFixture(const QPolygonF &polygon);

	Box2DPolygon* fixture() const { return m_fixture.get(); }
	const QPolygonF &screenPolygon() const{ return m_screenPolygon; }

protected:
	std::unique_ptr<Box2DPolygon> m_fixture;
	QPolygonF m_screenPolygon;
};







/**
 * @brief The TiledObjectCircleIface class
 */

class TiledObjectCircleIface
{
public:
	TiledObjectCircleIface()
		: m_fixture(new Box2DCircle)
	{}

	virtual Box2DCircle *createFixture(const QPointF &pos, const qreal &size);
	Box2DCircle* fixture() const { return m_fixture.get(); }

protected:
	std::unique_ptr<Box2DCircle> m_fixture;
};







/**
 * @brief The TiledObjectBase class
 */

class TiledObjectBase : public QQuickItem
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(TiledGame *game READ game WRITE setGame NOTIFY gameChanged FINAL)
	Q_PROPERTY(TiledScene *scene READ scene WRITE setScene NOTIFY sceneChanged FINAL)
	Q_PROPERTY(TiledObjectBody *body READ body CONSTANT FINAL)
	Q_PROPERTY(RemoteMode remoteMode READ remoteMode WRITE setRemoteMode NOTIFY remoteModeChanged FINAL)
	Q_PROPERTY(bool glowEnabled READ glowEnabled WRITE setGlowEnabled NOTIFY glowEnabledChanged FINAL)
	Q_PROPERTY(QColor glowColor READ glowColor WRITE setGlowColor NOTIFY glowColorChanged FINAL)
	Q_PROPERTY(bool overlayEnabled READ overlayEnabled WRITE setOverlayEnabled NOTIFY overlayEnabledChanged FINAL)
	Q_PROPERTY(QColor overlayColor READ overlayColor WRITE setOverlayColor NOTIFY overlayColorChanged FINAL)
	Q_PROPERTY(bool inVisibleArea READ inVisibleArea WRITE setInVisibleArea NOTIFY inVisibleAreaChanged FINAL)
	Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged FINAL)

public:
	explicit TiledObjectBase(QQuickItem *parent = 0);
	virtual ~TiledObjectBase();

	// For multiplayer

	enum RemoteMode {
		ObjectLocal = 0,
		ObjectRemote
	};

	Q_ENUM(RemoteMode);


	// ObjectId id

	struct ObjectId {
		int id = -1;
		int sceneId = -1;
	};

	Q_INVOKABLE void bodyComplete() { m_body->componentComplete(); }
	virtual void worldStep(const qreal &factor) { Q_UNUSED(factor); }

	TiledObjectBody *body() const { return m_body.get(); }
	TiledScene *scene() const;
	void setScene(TiledScene *newScene);

	TiledObjectSensorPolygon *sensorPolygon() const { return m_sensorPolygon.get(); }
	Box2DCircle *targetCircle() const { return m_targetCircle.get(); }

	static QPolygonF toPolygon(const Tiled::MapObject *object, Tiled::MapRenderer *renderer);
	static QPointF toPoint(const qreal &angle, const qreal &radius);

	template <typename T>
	static typename std::enable_if<std::is_base_of<TiledObjectPolygonIface, T>::value>::type
	createFromPolygon(T** dest, const QPolygonF &polygon, Tiled::MapRenderer *renderer,
					  QQuickItem *parent = nullptr);

	template <typename T>
	static typename std::enable_if<std::is_base_of<TiledObjectPolygonIface, T>::value>::type
	createFromMapObject(T** dest, const Tiled::MapObject *object, Tiled::MapRenderer *renderer,
						QQuickItem *parent = nullptr);


	template <typename T>
	static typename std::enable_if<std::is_base_of<TiledObjectCircleIface, T>::value>::type
	createFromCircle(T** dest, const QPointF &position, const qreal &size, Tiled::MapRenderer *renderer,
					 QQuickItem *parent = nullptr);

	template <typename T>
	static typename std::enable_if<std::is_base_of<TiledObjectCircleIface, T>::value>::type
	createFromMapObject(T** dest, const Tiled::MapObject *object, Tiled::MapRenderer *renderer,
						QQuickItem *parent = nullptr);

	static void setPolygonVertices(Box2DPolygon *fixture, const QPolygonF &polygon);

	static float32 normalizeFromRadian(const float32 &radian);
	static float32 normalizeToRadian(const float32 &normal);

	static TiledObjectBase *getFromFixture(const Box2DFixture *fixture);

	void setBodyOffset(QPointF newBodyOffset);
	void setBodyOffset(const qreal &x, const qreal &y) { setBodyOffset(QPointF(x, y)); }

	bool rotateBody(const float32 &desiredRadian);

	RemoteMode remoteMode() const;
	void setRemoteMode(const RemoteMode &newRemoteMode);

	bool glowEnabled() const;
	void setGlowEnabled(bool newGlowEnabled);

	bool overlayEnabled() const;
	void setOverlayEnabled(bool newOverlayEnabled);

	QColor glowColor() const;
	void setGlowColor(const QColor &newGlowColor);

	QColor overlayColor() const;
	void setOverlayColor(const QColor &newOverlayColor);

	bool inVisibleArea() const;
	void setInVisibleArea(bool newInVisibleArea);

	const ObjectId &objectId() const;
	void setObjectId(const ObjectId &newObjectId);

	TiledGame *game() const;
	void setGame(TiledGame *newGame);

	QString displayName() const;
	void setDisplayName(const QString &newDisplayName);

signals:
	void sceneChanged();
	void remoteModeChanged();
	void glowEnabledChanged();
	void overlayEnabledChanged();
	void glowColorChanged();
	void overlayColorChanged();
	void inVisibleAreaChanged();
	void gameChanged();
	void displayNameChanged();

protected:
	TiledObjectSensorPolygon *addSensorPolygon(const qreal &length = -1, const qreal &range = -1);
	Box2DCircle *addTargetCircle(const qreal &radius);
	virtual void onSceneVisibleAreaChanged();

protected:
	TiledGame *m_game = nullptr;
	TiledScene *m_scene = nullptr;
	std::unique_ptr<TiledObjectSensorPolygon> m_sensorPolygon;
	std::unique_ptr<TiledObjectBody> m_body;
	std::unique_ptr<Box2DCircle> m_targetCircle;
	RemoteMode m_remoteMode = ObjectLocal;
	bool m_glowEnabled = false;
	bool m_overlayEnabled = false;
	QColor m_glowColor = QColor(Qt::yellow);
	QColor m_overlayColor = QColor(Qt::white);
	bool m_inVisibleArea = false;
	ObjectId m_objectId;
	QString m_displayName;

	friend class TiledObjectBody;
	friend class TiledScene;


private:
	void recalculateTargetCircle();

	struct RotateAnimation {
		bool running = false;
		float32 destAngle = 0;
		bool clockwise = true;

		qreal speed = 0.2;				// 0.2 radian in 1/60 sec
	};

	RotateAnimation m_rotateAnimation;
};





/**
 * @brief The TiledObject class
 */

class TiledObject : public TiledObjectBase
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(Direction currentDirection READ currentDirection WRITE setCurrentDirection NOTIFY currentDirectionChanged FINAL)
	Q_PROPERTY(Directions availableDirections READ availableDirections WRITE setAvailableDirections NOTIFY availableDirectionsChanged FINAL)

public:
	explicit TiledObject(QQuickItem *parent = 0);
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
		jumpToSprite(sprite, m_currentDirection);
	}
	Q_INVOKABLE void jumpToSpriteLater(const char *sprite) const {
		jumpToSpriteLater(sprite, m_currentDirection);
	}

	QStringList availableSprites() const;


	static qreal toRadian(const qreal &angle);
	static qreal toDegree(const qreal &angle);
	static qreal directionToIsometricRaidan(const Direction &direction);
	static qreal directionToRadian(const Direction &direction);
	static qreal factorFromDegree(const qreal &angle, const qreal &xyRatio = 2.);
	static qreal factorFromRadian(const qreal &angle, const qreal &xyRatio = 2.);

	static Direction nearestDirectionFromRadian(const Directions &directions, const qreal &angle);
	Direction nearestDirectionFromRadian(const qreal &angle) const { return nearestDirectionFromRadian(m_availableDirections, angle); };

	TiledSpriteHandler *spriteHandler() const;
	TiledSpriteHandler *spriteHandlerAuxFront() const;
	TiledSpriteHandler *spriteHandlerAuxBack() const;

	virtual void createMarkerItem();

	Direction currentDirection() const;
	void setCurrentDirection(const Direction &newCurrentDirection);

	Directions availableDirections() const;
	void setAvailableDirections(const Directions &newAvailableDirections);


signals:
	void currentDirectionChanged();
	void availableDirectionsChanged();

protected:

	bool appendSprite(const QString &source, const TiledObjectSprite &sprite);
	bool appendSprite(const QString &source, const TiledObjectSpriteList &spriteList);
	bool appendSprite(const TiledMapObjectLayeredSprite &sprite, const QString &path = QStringLiteral(""));
	bool appendSprite(const TiledObjectLayeredSpriteList &spriteList, const QString &path = QStringLiteral(""));

	bool appendSprite(const QString &source, const IsometricObjectSprite &sprite);
	bool appendSprite(const QString &source, const IsometricObjectSpriteList &spriteList);
	bool appendSprite(const IsometricObjectLayeredSprite &sprite, const QString &path = QStringLiteral(""));
	bool appendSprite(const IsometricObjectLayeredSpriteList &sprite, const QString &path = QStringLiteral(""));

	bool playAuxSprite(const AuxHandler &auxHandler, const bool &alignToBody,
					   const QString &source, const TiledObjectSprite &sprite, const bool &replaceCurrentSprite = false) const;
	bool playAuxSprite(const QString &source, const TiledObjectSprite &sprite, const bool &replaceCurrentSprite = false) const {
		return playAuxSprite(AuxFront, false, source, sprite, replaceCurrentSprite);
	}

	void createVisual();

	void rotateToPoint(const QPointF &point, float32 *anglePtr = nullptr, qreal *distancePtr = nullptr);
	float32 angleToPoint(const QPointF &point) const;
	qreal distanceToPoint(const QPointF &point) const;

protected:
	QQuickItem *m_visualItem = nullptr;
	Direction m_currentDirection = Invalid;
	Directions m_availableDirections = None;
	TiledSpriteHandler *m_spriteHandler = nullptr;
	TiledSpriteHandler *m_spriteHandlerAuxFront = nullptr;
	TiledSpriteHandler *m_spriteHandlerAuxBack = nullptr;

	friend class TiledEffect;
};





/**
 * @brief TiledObject::createFromCircle
 * @param dest
 * @param position
 * @param size
 * @param renderer
 * @param parent
 * @return
 */

template<typename T>
typename std::enable_if<std::is_base_of<TiledObjectCircleIface, T>::value>::type
TiledObjectBase::createFromCircle(T **dest, const QPointF &position, const qreal &size, Tiled::MapRenderer *renderer, QQuickItem *parent)
{
	Q_ASSERT(dest);

	const QPointF &pos = renderer ?
							 renderer->pixelToScreenCoords(position) :
							 position;


	*dest = new T(parent);
	(*dest)->setX(pos.x());
	(*dest)->setY(pos.y());
	(*dest)->setWidth(size);
	(*dest)->setHeight(size);

	(*dest)->createFixture(QPointF{}, size);

	(*dest)->body()->addFixture((*dest)->fixture());
	(*dest)->bodyComplete();
	(*dest)->body()->emplace(pos);
}




/**
 * @brief TiledObject::createFromMapObject
 * @param dest
 * @param object
 * @param renderer
 * @param parent
 * @return
 */

template<typename T>
typename std::enable_if<std::is_base_of<TiledObjectPolygonIface, T>::value>::type
TiledObjectBase::createFromMapObject(T **dest, const Tiled::MapObject *object, Tiled::MapRenderer *renderer, QQuickItem *parent)
{
	Q_ASSERT(dest);

	if (!object) {
		LOG_CERROR("scene") << "Empty Tiled::MapObject";
		return;
	}

	switch (object->shape()) {
		case Tiled::MapObject::Rectangle:
			return createFromPolygon<T>(dest, object->bounds(), renderer, parent);
		case Tiled::MapObject::Polygon:
			return createFromPolygon<T>(dest, object->polygon().translated(object->position()), renderer, parent);
		default:
			LOG_CERROR("scene") << "Invalid Tiled::MapObject shape" << object->shape();
	}
}



/**
 * @brief TiledObject::createFromPolygon
 * @param polygon
 * @param renderer
 * @param parent
 * @return
 */

template<typename T>
typename std::enable_if<std::is_base_of<TiledObjectPolygonIface, T>::value>::type
TiledObjectBase::createFromPolygon(T **dest, const QPolygonF &polygon, Tiled::MapRenderer *renderer, QQuickItem *parent)
{
	Q_ASSERT(dest);

	const QPolygonF &screenPolygon = renderer ? renderer->pixelToScreenCoords(polygon) : polygon;

	const QRectF &boundingRect = screenPolygon.boundingRect();

	*dest = new T(parent);

	(*dest)->createFixture(screenPolygon);

	(*dest)->setX(boundingRect.center().x());
	(*dest)->setY(boundingRect.center().y());
	(*dest)->setWidth(boundingRect.width());
	(*dest)->setHeight(boundingRect.height());

	(*dest)->body()->addFixture((*dest)->fixture());
	(*dest)->bodyComplete();

}






/**
 * @brief TiledObject::createFromMapObject
 * @param dest
 * @param object
 * @param renderer
 * @param parent
 * @return
 */

template<typename T>
typename std::enable_if<std::is_base_of<TiledObjectCircleIface, T>::value>::type
TiledObjectBase::createFromMapObject(T **dest, const Tiled::MapObject *object, Tiled::MapRenderer */*renderer*/, QQuickItem */*parent*/)
{
	Q_ASSERT(dest);

	if (!object) {
		LOG_CERROR("scene") << "Empty Tiled::MapObject";
		return;
	}

	/*if (object->shape())
		createFromCircle<T>(dest, object->position(), object->radius(), renderer, parent);
	else*/
		LOG_CERROR("scene") << "Invalid Tiled::MapObject shape" << object->shape();

}




/**
 * @brief The TiledObjectRayCast class
 */

class TiledObjectRayCast : public b2RayCastCallback
{
public:
	TiledObjectRayCast(Box2DWorld *world)
		: m_world(world)
	{
		Q_ASSERT(m_world);
	}


	float32 ReportFixture(b2Fixture *fixture,
						  const b2Vec2 &point,
						  const b2Vec2 &normal,
						  float32 fraction) override;

	TiledReportedFixtureMap reportedFixtures() const;

private:
	const Box2DWorld *m_world;
	TiledReportedFixtureMap m_reportedFixtures;
};




/**
 * @brief The TiledObjectBasePolygon class
 */

class TiledObjectBasePolygon : public TiledObjectBase, public TiledObjectPolygonIface
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(Box2DPolygon* fixture READ fixture CONSTANT FINAL)

public:
	explicit TiledObjectBasePolygon(QQuickItem *parent = 0)
		: TiledObjectBase(parent)
		, TiledObjectPolygonIface()
	{}
};




/**
 * @brief The TiledObjectBasePolygon class
 */

class TiledObjectPolygon : public TiledObject, public TiledObjectPolygonIface
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(Box2DPolygon* fixture READ fixture CONSTANT FINAL)

public:
	explicit TiledObjectPolygon(QQuickItem *parent = 0)
		: TiledObject(parent)
		, TiledObjectPolygonIface()
	{ }
};








/**
 * @brief The TiledObjectBasePolygon class
 */

class TiledObjectBaseCircle : public TiledObjectBase, public TiledObjectCircleIface
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(Box2DCircle* fixture READ fixture CONSTANT FINAL)

public:
	explicit TiledObjectBaseCircle(QQuickItem *parent = 0)
		: TiledObjectBase(parent)
		, TiledObjectCircleIface()
	{}
};




/**
 * @brief The TiledObjectBasePolygon class
 */

class TiledObjectCircle : public TiledObject, public TiledObjectCircleIface
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(Box2DCircle* fixture READ fixture CONSTANT FINAL)

public:
	explicit TiledObjectCircle(QQuickItem *parent = 0)
		: TiledObject(parent)
		, TiledObjectCircleIface()
	{}
};


#endif // TILEDOBJECT_H
