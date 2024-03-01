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

#include "box2dbody.h"
#include <QQuickItem>
#include <QSerializer>
#include <libtiled/mapobject.h>


class TiledObjectSprite : public QSerializer
{
	Q_GADGET

public:
	TiledObjectSprite()
		: duration(0)
		, durationVariation(0)
		, frameCount(0)
		, frameDuration(0)
		//, frameDurationVariation(0)
		, frameHeight(0)
		, frameWidth(0)
		, frameX(0)
		, frameY(0)
		//, frameRate(0)
		//, frameRateVariation(0)
	{}

	QS_SERIALIZABLE
	QS_FIELD(int, duration)
	QS_FIELD(int, durationVariation)
	QS_FIELD(int, frameCount)
	QS_FIELD(int, frameDuration)
	//QS_FIELD(int, frameDurationVariation)
	QS_FIELD(int, frameHeight)
	QS_FIELD(int, frameWidth)
	QS_FIELD(int, frameX)
	QS_FIELD(int, frameY)
	//QS_FIELD(qreal, frameRate)
	//QS_FIELD(qreal, frameRateVariation)
	QS_FIELD(QString, name)
	QS_FIELD(QJsonObject, to)
};




/**
 * @brief The TiledObjectSpriteList class
 */

class TiledObjectSpriteList : public QSerializer
{
	Q_GADGET

public:
	TiledObjectSpriteList()
	{}

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, TiledObjectSprite, sprites)
};




/**
 * @brief The TiledMapObjectAlterableSprite class
 */

class TiledMapObjectAlterableSprite : public QSerializer
{
	Q_GADGET

public:
	TiledMapObjectAlterableSprite()
	{}

	QS_SERIALIZABLE
	QS_QT_DICT(QMap, QString, QString, alterations)
	QS_COLLECTION_OBJECTS(QList, TiledObjectSprite, sprites)
};





/**
 * @brief The TiledObjectAlterableSpriteList class
 */

class TiledObjectAlterableSpriteList : public QSerializer
{
	Q_GADGET

public:
	TiledObjectAlterableSpriteList()
	{}

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, TiledMapObjectAlterableSprite, list)
};



class TiledScene;

#if QT_VERSION >= 0x060000

#ifndef OPAQUE_PTR_TiledScene
#define OPAQUE_PTR_TiledScene
  Q_DECLARE_OPAQUE_POINTER(TiledScene*)
#endif

#endif





/**
 * @brief The TiledObjectBase class
 */

class TiledObjectBase : public QQuickItem
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(TiledScene *scene READ scene WRITE setScene NOTIFY sceneChanged FINAL)
	Q_PROPERTY(Box2DBody *body READ body CONSTANT FINAL)

public:
	explicit TiledObjectBase(QQuickItem *parent = 0);
	virtual ~TiledObjectBase();

	Q_INVOKABLE void bodyComplete() { m_body->componentComplete(); }
	virtual void worldStep() {}

	TiledScene *scene() const;
	void setScene(TiledScene *newScene);

	Box2DBody *body() const;
	Box2DFixture *defaultFixture() const;

signals:
	void sceneChanged();

protected:
	virtual void onSceneConnected() {}

protected:
	TiledScene *m_scene = nullptr;
	std::unique_ptr<Box2DBody> m_body;
	std::unique_ptr<Box2DFixture> m_defaultFixture;
};





/**
 * @brief The TiledObject class
 */

class TiledObject : public TiledObjectBase
{
	Q_OBJECT
	QML_ELEMENT

public:
	explicit TiledObject(QQuickItem *parent = 0);

	static TiledObject *createFromMapObject(const Tiled::MapObject *object, Tiled::MapRenderer *renderer,
											   QQuickItem *parent = nullptr);
	static TiledObject *createFromPolygon(const QPolygonF &polygon, Tiled::MapRenderer *renderer,
											 QQuickItem *parent = nullptr);

	static QPolygonF toPolygonF(const Tiled::MapObject *object, Tiled::MapRenderer *renderer);

	Q_INVOKABLE void jumpToSprite(const QString &sprite) const;

	QPolygonF screenPolygon() const;
	QStringList availableSprites() const;
	QStringList availableAlterations() const;

signals:

protected:
	bool appendSprite(const QString &source, const TiledObjectSprite &sprite);
	bool appendSpriteList(const QString &source, const TiledObjectSpriteList &spriteList);
	bool appendSprite(const TiledMapObjectAlterableSprite &sprite, const QString &path = QStringLiteral(""));
	bool appendSpriteList(const TiledObjectAlterableSpriteList &spriteList, const QString &path = QStringLiteral(""));
	static QString getSpriteName(const QString &sprite, const QString &alteration = QStringLiteral(""));

protected slots:
	virtual void onCurrentSpriteChanged(QString sprite);
	void createVisual();

protected:
	QPolygonF m_screenPolygon;

	QQuickItem *m_visualItem = nullptr;
	QQuickItem *m_spriteSequence = nullptr;
	QStringList m_availableSprites;
	QStringList m_availableAlterations;
};

#endif // TILEDOBJECT_H
