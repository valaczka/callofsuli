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

#ifndef TILEDMAPOBJECT_H
#define TILEDMAPOBJECT_H

#include "box2dbody.h"
#include "tiledscene.h"
#include <QQuickItem>
#include <QSerializer>
#include <libtiled/mapobject.h>


class TiledMapObjectSprite : public QSerializer
{
	Q_GADGET

public:
	TiledMapObjectSprite()
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
 * @brief The TiledMapObjectSpriteList class
 */

class TiledMapObjectSpriteList : public QSerializer
{
	Q_GADGET

public:
	TiledMapObjectSpriteList()
	{}

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, TiledMapObjectSprite, sprites)
};




/**
 * @brief The TiledMapObject class
 */

class TiledMapObject : public QQuickItem
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(TiledScene *scene READ scene WRITE setScene NOTIFY sceneChanged FINAL)
	Q_PROPERTY(Box2DBody *body READ body CONSTANT FINAL)

public:
	TiledMapObject(QQuickItem *parent = 0);
	virtual ~TiledMapObject();

	static TiledMapObject *createFromMapObject(const Tiled::MapObject *object, Tiled::MapRenderer *renderer,
											   QQuickItem *parent = nullptr);
	static TiledMapObject *createFromPolygon(const QPolygonF &polygon, Tiled::MapRenderer *renderer,
											 QQuickItem *parent = nullptr);

	Q_INVOKABLE void bodyComplete() { m_body->componentComplete(); }
	Q_INVOKABLE void jumpToSprite(const QString &sprite) const;

	TiledScene *scene() const;
	void setScene(TiledScene *newScene);

	Box2DBody *body() const;

	Box2DFixture *defaultFixture() const;

	QPolygonF screenPolygon() const;

signals:
	void sceneChanged();
	void bodyChanged();

protected:
	virtual void onSceneConnected() {}
	bool appendSprite(const QString &path, const TiledMapObjectSprite &sprite);
	bool appendSpriteList(const QString &path, const TiledMapObjectSpriteList &spriteList);

protected slots:
	virtual void onCurrentSpriteChanged(QString sprite);
	void createVisual();

protected:
	TiledScene *m_scene = nullptr;
	std::unique_ptr<Box2DBody> m_body;
	std::unique_ptr<Box2DFixture> m_defaultFixture;
	QPolygonF m_screenPolygon;

	QQuickItem *m_visualItem = nullptr;
	QQuickItem *m_spriteSequence = nullptr;
	QStringList m_availableSprites;


};

#endif // TILEDMAPOBJECT_H
