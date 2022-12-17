/*
 * ---- Call of Suli ----
 *
 * gamescene.h
 *
 * Created on: 2022. 12. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameScene
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

#ifndef GAMESCENE_H
#define GAMESCENE_H

#include "box2dworld.h"
#include "qloggingcategory.h"
#include <QQuickItem>
#include "gameterrain.h"
#include "gameladder.h"
#include "tiledpaintedlayer.h"

class ActionGame;
class GameObject;

class GameScene : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(ActionGame* game READ game WRITE setGame NOTIFY gameChanged)
	Q_PROPERTY(bool zoomOverview READ zoomOverview WRITE setZoomOverview NOTIFY zoomOverviewChanged)
	Q_PROPERTY(bool debugView READ debugView WRITE setDebugView NOTIFY debugViewChanged)
	Q_PROPERTY(Box2DWorld *world READ world WRITE setWorld NOTIFY worldChanged)
	Q_PROPERTY(QList<QPointer<GameLadder>> ladders READ ladders CONSTANT)
	Q_PROPERTY(bool showObjects READ showObjects WRITE setShowObjects NOTIFY showObjectsChanged)
	Q_PROPERTY(bool showEnemies READ showEnemies WRITE setShowEnemies NOTIFY showEnemiesChanged)

public:
	GameScene(QQuickItem *parent = nullptr);
	virtual ~GameScene();

	ActionGame *game() const;
	void setGame(ActionGame *newGame);

	Q_INVOKABLE void load();

	bool zoomOverview() const;
	void setZoomOverview(bool newZoomOverview);

	bool debugView() const;
	void setDebugView(bool newDebugView);

	Box2DWorld *world() const;
	void setWorld(Box2DWorld *newWorld);

	const QList<QPointer<GameLadder>> &ladders() const;

	bool showObjects() const;
	void setShowObjects(bool newShowObjects);

	bool showEnemies() const;
	void setShowEnemies(bool newShowEnemies);

public slots:
	void zoomOverviewToggle();
	void onScenePrepared();

protected:
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;

signals:
	void gameChanged();
	void zoomOverviewChanged();
	void debugViewChanged();
	void worldChanged();
	void showObjectsChanged();
	void showEnemiesChanged();

private:
	void loadTiledLayers();
	void loadGroundLayer();
	void loadLadderLayer();

	Tiled::ObjectGroup *objectLayer(const QString &name) const;

	ActionGame *m_game = nullptr;
	GameTerrain m_terrain;
	Box2DWorld *m_world = nullptr;

	QList<QPointer<GameObject>> m_grounds;
	QList<QPointer<GameLadder>> m_ladders;
	QList<QPointer<TiledPaintedLayer>> m_tiledLayers;

	bool m_zoomOverview = false;
	bool m_debugView = false;
	bool m_showObjects = false;
	bool m_showEnemies = false;
};


Q_DECLARE_LOGGING_CATEGORY(lcScene);

#endif // GAMESCENE_H
