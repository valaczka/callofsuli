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



class TiledGame;
class TiledScene;
class TiledSpriteHandler;
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



typedef QMultiMap<float, b2::ShapeRef> TiledReportedFixtureMap;



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


	// For multiplayer

	enum RemoteMode {
		ObjectLocal = 0,
		ObjectRemote
	};

	Q_ENUM(RemoteMode);


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
		FixtureTarget		= 0x1 << 3,
		FixtureTransport	= 0x1 << 4,
		FixturePickable		= 0x1 << 5,
		FixtureTrigger		= 0x1 << 6,
		FixtureVirtualCircle = 0x1 << 7,
		FixtureSensor		= 0x1 << 8,
		FixtureContainer	= 0x1 << 9,
	};

	Q_ENUM(FixtureCategory);
	Q_DECLARE_FLAGS(FixtureCategories, FixtureCategory);
	Q_FLAG(FixtureCategories);

	FixtureCategories collidesWith() const;
	void setCollidesWith(const FixtureCategories &categories);

	FixtureCategories categories() const;
	void setCategories(const FixtureCategories &categories);


	RemoteMode remoteMode() const;
	void setRemoteMode(const RemoteMode &newRemoteMode);

	ObjectId objectId() const;
	void setObjectId(const ObjectId &newObjectId);
	void setObjectId(const int &sceneId, const int &id);


	TiledGame *game() const;
	void setGame(TiledGame *newGame);

	b2::World *world() const;
	void setWorld(b2::World *newWorld, const QPointF &position = {});
	void setWorld(b2::World *newWorld, const QPointF &position, const float &rotation);
	TiledScene *scene() const;

	b2::BodyRef body() const;
	QPointF bodyPosition() const;
	QRectF bodyAABB() const;

	void emplace(const QPointF &center);
	void emplace(const qreal &centerX, const qreal &centerY) { emplace(QPointF(centerX, centerY)); }

	void setSpeed(const QPointF &point);
	void stop();

	bool rotateBody(const float &desiredRadian);

	bool opaque() const;
	void setOpaque(bool newOpaque);

	bool isBodyEnabled() const;
	void setBodyEnabled(const bool &enabled);

	bool inVisibleArea() const;

	bool overlap(const QPointF &pos) const;

	virtual void worldStep(const qreal &factor) { Q_UNUSED(factor); }
	virtual void synchronize();

	static TiledObjectBody *fromBodyRef(b2::BodyConstRef ref);

	static b2Filter getFilter(const FixtureCategories &categories);
	static b2Filter getFilter(const FixtureCategories &categories, const FixtureCategories &collidesWith);

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



	TiledReportedFixtureMap rayCast(const QPointF &dest, const FixtureCategories &categories);

	void rotateToPoint(const QPointF &point, float *anglePtr = nullptr, qreal *distancePtr = nullptr);
	float angleToPoint(const QPointF &point) const;
	float distanceToPoint(const QPointF &point) const;


protected:
	virtual void worldChanged() {}

	virtual void setInVisibleArea(bool newInVisibleArea);
	void updateBodyInVisibleArea();

	TiledGame *m_game = nullptr;
	bool m_inVisibleArea = false;

private:
	TiledObjectBodyPrivate *d;
	bool m_opaque = true;
	RemoteMode m_remoteMode = ObjectLocal;
	ObjectId m_objectId;

	friend class TiledObjectBodyPrivate;
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
	Q_PROPERTY(RemoteMode remoteMode READ remoteMode WRITE setRemoteMode NOTIFY remoteModeChanged FINAL)
	Q_PROPERTY(bool glowEnabled READ glowEnabled WRITE setGlowEnabled NOTIFY glowEnabledChanged FINAL)
	Q_PROPERTY(QColor glowColor READ glowColor WRITE setGlowColor NOTIFY glowColorChanged FINAL)
	Q_PROPERTY(bool overlayEnabled READ overlayEnabled WRITE setOverlayEnabled NOTIFY overlayEnabledChanged FINAL)
	Q_PROPERTY(QColor overlayColor READ overlayColor WRITE setOverlayColor NOTIFY overlayColorChanged FINAL)
	Q_PROPERTY(bool inVisibleArea READ inVisibleArea NOTIFY inVisibleAreaChanged FINAL)
	Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged FINAL)
	Q_PROPERTY(Direction currentDirection READ currentDirection WRITE setCurrentDirection NOTIFY currentDirectionChanged FINAL)
	Q_PROPERTY(Directions availableDirections READ availableDirections WRITE setAvailableDirections NOTIFY availableDirectionsChanged FINAL)


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
		jumpToSprite(sprite, m_currentDirection);
	}
	Q_INVOKABLE void jumpToSpriteLater(const char *sprite) const {
		jumpToSpriteLater(sprite, m_currentDirection);
	}

	QStringList availableSprites() const;


	static qreal toRadian(const qreal &angle);
	static qreal toDegree(const qreal &angle);
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
	static QPointF toPoint(const qreal &angle, const qreal &radius);
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

	virtual void synchronize() override;

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

	Direction currentDirection() const;
	void setCurrentDirection(const Direction &newCurrentDirection);

	Directions availableDirections() const;
	void setAvailableDirections(const Directions &newAvailableDirections);


signals:
	void remoteModeChanged();
	void glowEnabledChanged();
	void overlayEnabledChanged();
	void glowColorChanged();
	void overlayColorChanged();
	void inVisibleAreaChanged();
	void displayNameChanged();
	void currentDirectionChanged();
	void availableDirectionsChanged();
	void sceneChanged();

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
	Direction m_currentDirection = Invalid;
	Directions m_availableDirections = None;
	TiledSpriteHandler *m_spriteHandler = nullptr;
	TiledSpriteHandler *m_spriteHandlerAuxFront = nullptr;
	TiledSpriteHandler *m_spriteHandlerAuxBack = nullptr;

	friend class TiledEffect;
	friend class TiledScene;
	friend class TiledObjectBody;

};






#endif // TILEDOBJECT_H
