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

#include "box2dworld.h"
#include "libtiledquick/mapitem.h"
#include "libtiledquick/maploader.h"
#include "tiledobject.h"
#include <QQuickItem>
#include <QElapsedTimer>


class TiledGame;

#if QT_VERSION >= 0x060000

#ifndef OPAQUE_PTR_TiledGame
#define OPAQUE_PTR_TiledGame
Q_DECLARE_OPAQUE_POINTER(TiledGame*)
#endif

#endif

/**
 * @brief The TiledScene class
 */

class TiledScene : public TiledQuick::MapItem
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(Box2DWorld *world READ world CONSTANT FINAL)
	Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged FINAL)
	Q_PROPERTY(TiledGame *game READ game WRITE setGame NOTIFY gameChanged FINAL)
	Q_PROPERTY(int sceneId READ sceneId WRITE setSceneId NOTIFY sceneIdChanged FINAL)
	Q_PROPERTY(QString ambientSound READ ambientSound WRITE setAmbientSound NOTIFY ambientSoundChanged FINAL)
	Q_PROPERTY(QString backgroundMusic READ backgroundMusic WRITE setBackgroundMusic NOTIFY backgroundMusicChanged FINAL)

public:
	explicit TiledScene(QQuickItem *parent = nullptr);
	virtual ~TiledScene();

	TiledQuick::TileLayerItem *addTileLayer(Tiled::TileLayer *layer, Tiled::MapRenderer *renderer);

	Q_INVOKABLE int getDynamicZ(const QPointF &point, const int &defaultValue = 1) const;
	Q_INVOKABLE int getDynamicZ(const qreal &x, const qreal &y, const int &defaultValue = 1) const;
	Q_INVOKABLE int getDynamicZ(QQuickItem *item, const int &defaultValue = 1) const;

	Q_INVOKABLE bool load(const QUrl &url);

	void appendToObjects(TiledObject *object);
	void removeFromObjects(TiledObject *object);

	bool running() const;
	void setRunning(bool newRunning);

	void startMusic();
	void stopMusic();

	bool isGroundContainsPoint(const QPointF &point) const;

	TiledQuick::MapLoader *mapLoader() const;

	Box2DWorld *world() const;

	TiledGame *game() const;
	void setGame(TiledGame *newGame);

	int sceneId() const;
	void setSceneId(int newSceneId);

	QString ambientSound() const;
	void setAmbientSound(const QString &newAmbientSound);

	QString backgroundMusic() const;
	void setBackgroundMusic(const QString &newBackgroundMusic);

signals:
	void runningChanged();
	void gameChanged();
	void sceneIdChanged();
	void ambientSoundChanged();
	void backgroundMusicChanged();

protected:
	virtual void refresh() override;

	std::unique_ptr<TiledQuick::MapLoader> m_mapLoader;
	std::unique_ptr<Box2DWorld> m_world;
	QVector<QPointer<TiledObjectBasePolygon>> m_groundObjects;
	QString m_ambientSound;
	QString m_backgroundMusic;

	QVector<QPointer<TiledObject>> m_tiledObjects;
	QVector<QPointer<TiledObject>> m_tiledObjectsToAppend;
	QVector<QPointer<TiledObject>> m_tiledObjectsToRemove;



/// TEST POINTS
/*private:
	Q_PROPERTY(QVariantList testPoints READ testPoints WRITE setTestPoints NOTIFY testPointsChanged FINAL)
	QVariantList m_testPoints;
signals:
	void testPointsChanged();
public:
	QVariantList testPoints() const { return m_testPoints; }
	void setTestPoints(const QVariantList &list) {
		if (m_testPoints == list)
			return;
		m_testPoints = list;
		emit testPointsChanged();
	}*/
/// ----


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
		int z = 1;

		QPointF getMaxBottomRight() const;
		QPointF getMinTopLeft() const;

		bool isOver(const qreal &x, const qreal &y) const;
		bool isOver(const QPointF &point) const { return isOver(point.x(), point.y()); }
	};


	void appendDynamicZ(const QString &name, const QRectF &area);
	void setTileLayersZ();
	void onWorldStepped();
	void reorderObjectsZ();
	void repaintTilesets(Tiled::Tileset *tileset);

	TiledGame *m_game = nullptr;
	int m_sceneId = -1;

	std::vector<DynamicZ> m_dynamicZList;
	QElapsedTimer m_worldStepTimer;

	friend class TiledGame;
};

#endif // TILEDSCENE_H
