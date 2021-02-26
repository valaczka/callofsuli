/*
 * ---- Call of Suli ----
 *
 * cosgame.h
 *
 * Created on: 2020. 10. 21.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * CosGame
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef COSGAME_H
#define COSGAME_H

#include "gameenemydata.h"
#include "gameblock.h"
#include "gameladder.h"
#include <game.h>

#include "gamematch.h"
#include "gamescene.h"
#include "gameterrain.h"
#include "gamepickable.h"
#include "../common/gamemap.h"

class GamePlayer;
class GamePickable;
class GameQuestion;
class GameActivity;
struct GameInventoryPickable;

class CosGame : public Game
{
	Q_OBJECT

	Q_PROPERTY(QVariantMap gameData READ gameData WRITE setGameData NOTIFY gameDataChanged)
	Q_PROPERTY(QQuickItem * player READ player WRITE setPlayer NOTIFY playerChanged)

	Q_PROPERTY(GameObject * playerStartPosition READ playerStartPosition WRITE setPlayerStartPosition NOTIFY playerStartPositionChanged)
	Q_PROPERTY(int ladderCount READ ladderCount)
	Q_PROPERTY(int activeEnemies READ activeEnemies NOTIFY activeEnemiesChanged)

	Q_PROPERTY(GameMatch * gameMatch READ gameMatch WRITE setGameMatch NOTIFY gameMatchChanged)
	Q_PROPERTY(QVariantMap levelData READ levelData NOTIFY levelDataChanged)

	Q_PROPERTY(GameTerrain * terrainData READ terrainData NOTIFY terrainDataChanged)
	Q_PROPERTY(QQuickItem * gameScene READ gameScene WRITE setGameScene NOTIFY gameSceneChanged)
	Q_PROPERTY(QQuickItem * itemPage READ itemPage WRITE setItemPage NOTIFY itemPageChanged)
	Q_PROPERTY(GameQuestion * question READ question NOTIFY questionChanged)
	Q_PROPERTY(GameActivity * activity READ activity WRITE setActivity NOTIFY activityChanged)
	Q_PROPERTY(bool running READ running NOTIFY runningChanged)
	Q_PROPERTY(int msecLeft READ msecLeft NOTIFY msecLeftChanged)

	Q_PROPERTY(GamePickable * pickable READ pickable NOTIFY pickableChanged)

	Q_PROPERTY(QString backgroundMusicFile READ backgroundMusicFile WRITE setBackgroundMusicFile NOTIFY backgroundMusicFileChanged)

	Q_PROPERTY(bool isPrepared READ isPrepared WRITE setIsPrepared NOTIFY isPreparedChanged)
	Q_PROPERTY(bool isStarted READ isStarted WRITE setIsStarted NOTIFY isStartedChanged)


public:
	CosGame(QQuickItem *parent = 0);
	~CosGame();

	Q_INVOKABLE void loadScene();

	Q_INVOKABLE bool loadTerrainData();

	Q_INVOKABLE void recreateEnemies();
	Q_INVOKABLE void resetEnemy(GameEnemyData *enemyData);
	Q_INVOKABLE void setEnemiesMoving(const bool &moving);
	Q_INVOKABLE void tryAttack(GamePlayer *player, GameEnemy *enemy);

	qreal deathlyAttackDistance();


	QVariantMap gameData() const { return m_gameData; }
	QQuickItem * player() const { return m_player; }
	QQuickItem * gameScene() const { return m_gameScene; }
	GameObject * playerStartPosition() const { return m_playerStartPosition; }
	bool running() const { return m_running; }

	GameTerrain * terrainData() const { return m_terrainData; }
	void addTerrainData(QList<TiledPaintedLayer *> *tiledLayers, QQuickItem *tiledLayersParent);

	QQuickItem * itemPage() const { return m_itemPage; }
	GameQuestion * question() const { return m_question; }

	int ladderCount() const { return m_terrainData ? m_terrainData->ladders().count() : 0; }
	Q_INVOKABLE GameLadder * ladderAt(int i) { return m_terrainData ? m_terrainData->ladders().value(i) : nullptr; }

	GameMatch * gameMatch() const { return m_gameMatch; }

	bool isPrepared() const { return m_isPrepared; }
	bool isStarted() const { return m_isStarted; }

	QVariantMap levelData() const;
	GameActivity * activity() const { return m_activity; }
	int msecLeft() const { return m_msecLeft; }
	int activeEnemies() const { return m_activeEnemies; }
	QString backgroundMusicFile() const { return m_backgroundMusicFile; }

	GamePickable * pickable() const { return m_pickableList.isEmpty() ? nullptr : m_pickableList.at(0); }

public slots:
	void resetPlayer();
	void setLastPosition();
	void startGame();
	void abortGame();

	void increaseHp();
	void addSecs(const int &secs);
	void increaseShield(const int &num);

	void setGameData(QVariantMap gameData);

	void addPickable(GamePickable *p);
	void removePickable(GamePickable *p);
	void pickPickable();

	void setPlayer(QQuickItem * player);
	void setGameScene(QQuickItem * gameScene);
	void setPlayerStartPosition(GameObject * playerStartPosition);
	void setRunning(bool running);
	void setItemPage(QQuickItem * itemPage);
	void setGameMatch(GameMatch * gameMatch);
	void setIsPrepared(bool isPrepared);
	void setActivity(GameActivity * activity);
	void setActiveEnemies(int activeEnemies);
	void setIsStarted(bool isStarted);
	void setBackgroundMusicFile(QString backgroundMusicFile);

private slots:
	void onPlayerDied();
	void resetRunning();
	void recalculateActiveEnemies();
	void onTimerTimeout();
	void onGameMatchTimerTimeout();
	void onGameStarted();
	void onGameFinishedSuccess();
	void onGameFinishedLost();

signals:
	void gameStarted();
	void gameCompleted();
	void gameTimeout();
	void gameLost();
	void gameAbortRequest();

	void gameSceneLoaded();
	void gameSceneLoadFailed();

	void gameMessageSent(QString message, int colorCode = 0);

	void playerCharacterChanged(QString playerCharacter);
	void terrainChanged(QString terrain);
	void gameDataChanged(QVariantMap gameData);

	void levelChanged(int level);
	void startHpChanged(int startHp);

	void playerChanged(QQuickItem * player);
	void gameSceneChanged(QQuickItem * gameScene);
	void playerStartPositionChanged(GameObject * playerStartPosition);
	void startBlockChanged(int startBlock);
	void runningChanged(bool running);
	void itemPageChanged(QQuickItem * itemPage);
	void questionChanged(GameQuestion * question);
	void terrainDataChanged(GameTerrain * terrainData);
	void gameMatchChanged(GameMatch * gameMatch);
	void isPreparedChanged(bool isPrepared);
	void levelDataChanged(QVariantMap levelData);
	void activityChanged(GameActivity * activity);
	void msecLeftChanged(int msecLeft);
	void activeEnemiesChanged(int activeEnemies);
	void isStartedChanged(bool isStarted);
	void backgroundMusicFileChanged(QString backgroundMusicFile);
	void pickableChanged(GamePickable * pickable);

private:
	void loadGameData();
	QStringList loadSoldierData();
	void loadPickables();
	void setPickables(QVector<GameEnemyData *> *enemyList, const int &block);

	QVariantMap m_gameData;
	QQuickItem * m_player;
	QQuickItem * m_gameScene;
	GameObject * m_playerStartPosition;
	bool m_running;
	QQuickItem * m_itemPage;
	GameQuestion * m_question;
	GameTerrain * m_terrainData;
	GameMatch * m_gameMatch;
	bool m_isPrepared;
	GameActivity * m_activity;
	QTimer *m_timer;
	int m_msecLeft;
	int m_activeEnemies;
	QTimer *m_matchTimer;
	bool m_isStarted;
	QString m_backgroundMusicFile;
	QMap<int, QVector<GameInventoryPickable>> m_inventoryPickableList;
	QVector<GamePickable *> m_pickableList;
};


/**
 * @brief The GameInventoryPickable struct
 */

struct GameInventoryPickable {
	GameInventoryPickable(GameEnemyData::PickableType type, QVariantMap data, int count) :
		type(type), data(data), count(count) {};

	GameInventoryPickable(GameEnemyData::PickableType type, int count) :
		type(type), data(), count(count) {};

	GameInventoryPickable(GameEnemyData::PickableType type) :
		type(type), data(), count(1) {};

	/*friend inline bool operator== (const GameInventoryPickable &b1, const GameInventoryPickable &b2) {
		return b1.type == b2.type &&
				b1.data == b2.data &&
				b1.count == b2.count;
	}*/

	GameEnemyData::PickableType type;
	QVariantMap data;
	int count;

};

#endif // COSGAME_H
