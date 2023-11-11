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
#include "gameplayerposition.h"
#include <QQuickItem>
#include <QStack>
#include "gameladder.h"
#include "gameterrainmap.h"
#include "qmutex.h"



class ActionGame;
class GameObject;

#if QT_VERSION >= 0x060000

#ifndef OPAQUE_PTR_ActionGame
#define OPAQUE_PTR_ActionGame
  Q_DECLARE_OPAQUE_POINTER(ActionGame*)
#endif

#ifndef OPAQUE_PTR_GameObject
#define OPAQUE_PTR_GameObject
  Q_DECLARE_OPAQUE_POINTER(GameObject*)
#endif

#endif

#define TIMING_TIMER_TIMEOUT_MSEC	30

/**
 * @brief The GameScene class
 */

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
	Q_PROPERTY(QQuickItem* mouseArea READ mouseArea WRITE setMouseArea NOTIFY mouseAreaChanged)
	Q_PROPERTY(SceneState sceneState READ sceneState WRITE setSceneState NOTIFY sceneStateChanged)
	Q_PROPERTY(QQuickItem* messageList READ messageList WRITE setMessageList NOTIFY messageListChanged)
	Q_PROPERTY(QString imageTerrain READ imageTerrain NOTIFY imageTerrainChanged)
	Q_PROPERTY(QString imageOver READ imageOver NOTIFY imageOverChanged)

public:
	GameScene(QQuickItem *parent = nullptr);
	virtual ~GameScene();

	/**
	 * @brief The SceneState enum
	 */

	enum SceneState {
		ScenePrepare,
		ScenePreview,
		ScenePlay
	};

	Q_ENUM(SceneState)

	ActionGame *game() const;
	void setGame(ActionGame *newGame);

	Q_INVOKABLE void load();
	Q_INVOKABLE void playSoundPlayerVoice(const QString &source);
	Q_INVOKABLE void playSound(const QString &source);
	Q_INVOKABLE void playSoundVoiceOver(const QString &source);
	Q_INVOKABLE void playSoundMusic(const QString &source);
	Q_INVOKABLE void stopSoundMusic(const QString &source);

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

	QQuickItem *mouseArea() const;
	void setMouseArea(QQuickItem *newMouseArea);

	const QJsonObject &gameData() const;
	QJsonObject levelData(int level = -1) const;

	Q_INVOKABLE void createPlayer();

	SceneState gameState() const;
	void setGameState(SceneState newGameState);

	SceneState sceneState() const;
	void setSceneState(SceneState newSceneState);

	const GameTerrainMap &terrain() const;

	GameTerrain::PlayerPositionData getPlayerPosition();

	QQuickItem *messageList() const;
	void setMessageList(QQuickItem *newMessageList);

	const QString &imageTerrain() const;
	const QString &imageOver() const;

	void gameObjectAdd(GameObject *object);

public slots:
	void zoomOverviewToggle();
	void onScenePrepared();
	void onSceneStepSuccess();
	void onSceneLoadFailed();
	void onSceneAnimationReady();
	void activateLaddersInBlock(const int &block);
	void setPlayerPosition(GamePlayerPosition *position);

protected:
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;

signals:
	void sceneLoadFailed();
	void sceneStepSuccess();
	void sceneStarted();
	void gameChanged();
	void zoomOverviewChanged(bool zoom);
	void debugViewChanged();
	void worldChanged();
	void showObjectsChanged(bool show);
	void showEnemiesChanged(bool show);
	void mouseAreaChanged();
	void sceneStateChanged();
	void messageListChanged();
	void imageTerrainChanged();
	void imageOverChanged();

private:
	void loadGameData();
	void loadTiledLayers();
	void loadGroundLayer();
	void loadLadderLayer();
	void loadTerrainObjectsLayer();
	void loadPlayerPositionLayer();
	void loadPickablesLayer();

	Tiled::ObjectGroup *objectLayer(const QString &name) const;


private:
	void onTimerTimeout();

	void fpsSet();
	void fpsIncrease();
	void fpsDecrease();

	ActionGame *m_game = nullptr;
	GameTerrainMap m_terrain;
	Box2DWorld *m_world = nullptr;
	QQuickItem *m_mouseArea = nullptr;
	QQuickItem *m_messageList = nullptr;

	QElapsedTimer m_elapsedTimer;
	const int m_timingTimerTimeoutMsec = TIMING_TIMER_TIMEOUT_MSEC;

	QElapsedTimer m_performanceTimer;
	const QVector<double> m_fpsList = {60., 30., 25., 24., 18., 12.};
	int m_fpsIndex = 0;

	QList<QPointer<GameObject>> m_grounds;
	QList<QPointer<GameLadder>> m_ladders;
	QList<QPointer<QQuickItem>> m_childItems;
	QStack<QPointer<GamePlayerPosition>> m_playerPositions;

	QVector<QPointer<GameObject>> m_gameObjects;

	bool m_zoomOverview = false;
	bool m_debugView = false;
	bool m_showObjects = false;
	bool m_showEnemies = false;

	QJsonObject m_gameData;

	SceneState m_sceneState = ScenePrepare;
	int m_sceneLoadSteps = 0;
};


#endif // GAMESCENE_H
