/*
 * ---- Call of Suli ----
 *
 * tiledscene.h
 *
 * Created on: 2024. 02. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledScene
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

#ifndef TILEDSCENE_H
#define TILEDSCENE_H

#include "libtiledquick/mapitem.h"
#include "tiledobject.h"
#include <QQuickItem>
#include <QElapsedTimer>
#include "chipmunk/chipmunk.h"


class TiledGame;

#ifndef OPAQUE_PTR_TiledGame
#define OPAQUE_PTR_TiledGame
Q_DECLARE_OPAQUE_POINTER(TiledGame*)
#endif


class TiledVisualItem;

#ifndef OPAQUE_PTR_TiledVisualItem
#define OPAQUE_PTR_TiledVisualItem
Q_DECLARE_OPAQUE_POINTER(TiledVisualItem*)
#endif


class TiledDebugDraw;




/**
 * @brief The TiledSceneDefinition class
 */

class TiledSceneDefinition : public QSerializer
{
	Q_GADGET

public:
	enum SceneEffect {
		EffectNone		= 0,
		EffectRain		= 1,
		EffectSnow		= 2
	};

	Q_ENUM(SceneEffect)

	TiledSceneDefinition()
		: id(-1)
		, effect(EffectNone)
	{}

	QS_SERIALIZABLE
	QS_FIELD(int, id)
	QS_FIELD(QString, file)
	QS_FIELD(QString, ambient)
	QS_FIELD(QString, music)
	QS_FIELD(SceneEffect, effect)
};



/**
 * @brief The TiledScene class
 */

class TiledScene : public TiledQuick::MapItem
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(TiledGame *game READ game WRITE setGame NOTIFY gameChanged FINAL)
	Q_PROPERTY(QString ambientSound READ ambientSound WRITE setAmbientSound NOTIFY ambientSoundChanged FINAL)
	Q_PROPERTY(QString backgroundMusic READ backgroundMusic WRITE setBackgroundMusic NOTIFY backgroundMusicChanged FINAL)
	Q_PROPERTY(QRectF onScreenArea READ onScreenArea WRITE setOnScreenArea NOTIFY onScreenAreaChanged FINAL)
	Q_PROPERTY(TiledSceneDefinition::SceneEffect sceneEffect READ sceneEffect WRITE setSceneEffect NOTIFY sceneEffectChanged FINAL)
	Q_PROPERTY(QRectF viewport READ viewport NOTIFY viewportChanged FINAL)

public:
	explicit TiledScene(QQuickItem *parent = nullptr);
	virtual ~TiledScene();

	TiledQuick::TileLayerItem *addTileLayer(Tiled::TileLayer *layer, Tiled::MapRenderer *renderer);
	TiledVisualItem *addVisualItem(const QString &displayName = {});
	TiledVisualItem *addVisualItem(Tiled::ImageLayer *layer, const QString &displayName = {});
	TiledVisualItem *addVisualItem(Tiled::TileLayer *layer, Tiled::MapRenderer *renderer);

	void addLightObject(Tiled::MapObject *object);
	QQuickItem *addLight(Tiled::MapObject * object, Tiled::MapRenderer *renderer, const qreal &opacity = 1.);

	Q_INVOKABLE int getDynamicZ(const QPointF &point, const int &defaultValue = 1) const;
	Q_INVOKABLE int getDynamicZ(const qreal &x, const qreal &y, const int &defaultValue = 1) const;
	Q_INVOKABLE int getDynamicZ(QQuickItem *item, const int &defaultValue = 1) const;

	Q_INVOKABLE bool load(const QUrl &url);

	void startMusic();
	void stopMusic();

	int sceneId() const { return m_sceneId; }

	TiledGame *game() const;
	void setGame(TiledGame *newGame);

	QString ambientSound() const;
	void setAmbientSound(const QString &newAmbientSound);

	QString backgroundMusic() const;
	void setBackgroundMusic(const QString &newBackgroundMusic);

	QRectF onScreenArea() const;
	void setOnScreenArea(const QRectF &newOnScreenArea);

	TiledSceneDefinition::SceneEffect sceneEffect() const;
	void setSceneEffect(TiledSceneDefinition::SceneEffect newSceneEffect);

	QRectF viewport() const;
	void setViewport(const QRectF &newViewport);

	Q_INVOKABLE QList<QQuickItem*> visualItems() const { return m_visualItems; }

signals:
	void scaleResetRequest();
	void runningChanged();
	void gameChanged();
	void sceneIdChanged();
	void ambientSoundChanged();
	void backgroundMusicChanged();
	void onScreenAreaChanged();
	void sceneEffectChanged();
	void viewportChanged();

protected:
	virtual void refresh() override;

	QList<QQuickItem*> m_visualItems;

	cpSpace *m_space = nullptr;
	int m_sceneId = -1;

	std::unique_ptr<Tiled::Map> m_map;
	QString m_ambientSound;
	QString m_backgroundMusic;
	TiledSceneDefinition::SceneEffect m_sceneEffect = TiledSceneDefinition::EffectNone;
	QRectF m_onScreenArea;
	QRectF m_viewport;


private:
	struct DynamicZ {
		DynamicZ(const QString &_name, const QVector<QRectF> &_areas, const int &_z)
			: name(_name)
			, areas(_areas)
			, z(_z)
		{}
		DynamicZ() {}

		QString name;
		QVector<QRectF> areas;
		int z = 0;

		QPointF getMaxBottomRight() const;
		QPointF getMinTopLeft() const;

		bool isOver(const qreal &x, const qreal &y) const;
		bool isOver(const QPointF &point) const { return isOver(point.x(), point.y()); }
	};


	void appendDynamicZ(const QString &name, const QRectF &area);
	void setTileLayersZ();
	void reorderObjectsZ(const std::vector<TiledObjectBody *> list);
	void repaintTilesets(Tiled::Tileset *tileset);

	TiledDebugDraw *m_debugDraw = nullptr;
	void debugDrawEvent(TiledDebugDraw *debugDraw);

	TiledGame *m_game = nullptr;

	std::vector<DynamicZ> m_dynamicZList;
	QList<Tiled::MapObject*> m_lightObjects;

	friend class TiledGame;
	friend class TiledDebugDraw;
};

#endif // TILEDSCENE_H
