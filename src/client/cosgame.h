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

#include "gamescene.h"

class GamePlayer;
class GameQuestion;

class CosGame : public Game
{
	Q_OBJECT

	Q_PROPERTY(QString playerCharacter READ playerCharacter WRITE setPlayerCharacter NOTIFY playerCharacterChanged)
	Q_PROPERTY(QString terrain READ terrain WRITE setTerrain NOTIFY terrainChanged)

	Q_PROPERTY(QVariantMap gameData READ gameData WRITE setGameData NOTIFY gameDataChanged)
	Q_PROPERTY(QVariantMap terrainData READ terrainData NOTIFY terrainDataChanged)

	Q_PROPERTY(QQuickItem * player READ player WRITE setPlayer NOTIFY playerChanged)
	Q_PROPERTY(int level READ level WRITE setLevel NOTIFY levelChanged)
	Q_PROPERTY(int startHp READ startHp WRITE setStartHp NOTIFY startHpChanged)
	Q_PROPERTY(int startBlock READ startBlock WRITE setStartBlock NOTIFY startBlockChanged)

	Q_PROPERTY(QMap<int, GameBlock *> blocks READ blocks NOTIFY blocksChanged)
	Q_PROPERTY(GameObject * playerStartPosition READ playerStartPosition WRITE setPlayerStartPosition NOTIFY playerStartPositionChanged)
	Q_PROPERTY(QList<GameLadder *> ladders READ ladders NOTIFY laddersChanged)

	Q_PROPERTY(QQuickItem * gameScene READ gameScene WRITE setGameScene NOTIFY gameSceneChanged)
	Q_PROPERTY(QQuickItem * itemPage READ itemPage WRITE setItemPage NOTIFY itemPageChanged)
	Q_PROPERTY(GameQuestion * question READ question NOTIFY questionChanged)
	Q_PROPERTY(bool running READ running NOTIFY runningChanged)


public:
	CosGame(QQuickItem *parent = 0);
	~CosGame();

	Q_INVOKABLE void loadTerrainData();
	Q_INVOKABLE GameBlock * getBlock(const int &num, const bool &create = true);

	Q_INVOKABLE void recreateEnemies();
	Q_INVOKABLE void resetEnemy(GameEnemyData *enemyData);
	Q_INVOKABLE void setEnemiesMoving(const bool &moving);
	Q_INVOKABLE void tryAttack(GamePlayer *player, GameEnemy *enemy);

	qreal deathlyAttackDistance();

	void addEnemy(GameEnemyData *enemy) { m_enemies.append(enemy); }
	void addLadder(GameLadder *ladder) { m_ladders.append(ladder); emit laddersChanged(m_ladders); }

	QString playerCharacter() const { return m_playerCharacter; }
	QString terrain() const { return m_terrain; }
	int level() const { return m_level; }
	int startHp() const { return m_startHp; }
	int startBlock() const { return m_startBlock; }

	QVariantMap gameData() const { return m_gameData; }
	QVariantMap terrainData() const { return m_terrainData; }

	QQuickItem * player() const { return m_player; }

	QMap<int, GameBlock *> blocks() const { return m_blocks; }
	QQuickItem * gameScene() const { return m_gameScene; }

	GameObject * playerStartPosition() const { return m_playerStartPosition; }
	QList<GameLadder *> ladders() const { return m_ladders; }

	bool running() const { return m_running; }

	QQuickItem * itemPage() const { return m_itemPage; }
	GameQuestion * question() const { return m_question; }

public slots:
	void resetPlayer();
	void setLastPosition();

	void setPlayerCharacter(QString playerCharacter);
	void setTerrain(QString terrain);
	void setGameData(QVariantMap gameData);
	void setTerrainData(QVariantMap terrainData);

	void setLevel(int level);
	void setPlayer(QQuickItem * player);
	void setStartHp(int startHp);
	void setGameScene(QQuickItem * gameScene);
	void setPlayerStartPosition(GameObject * playerStartPosition);
	void setStartBlock(int startBlock);
	void setRunning(bool running);
	void setItemPage(QQuickItem * itemPage);
	void setQuestion(GameQuestion * question);

private slots:
	void onLayersLoaded();
	void onPlayerDied();
	void resetRunning();
	void recalculateBlocks();

signals:
	void gameCompleted();

	void playerCharacterChanged(QString playerCharacter);
	void terrainChanged(QString terrain);
	void gameDataChanged(QVariantMap gameData);
	void terrainDataChanged(QVariantMap terrainData);

	void levelChanged(int level);
	void startHpChanged(int startHp);

	void blocksChanged(QMap<int, GameBlock *> blocks);
	void playerChanged(QQuickItem * player);
	void gameSceneChanged(QQuickItem * gameScene);
	void playerStartPositionChanged(GameObject * playerStartPosition);
	void startBlockChanged(int startBlock);
	void laddersChanged(QList<GameLadder *> allLadder);
	void runningChanged(bool running);
	void itemPageChanged(QQuickItem * itemPage);
	void questionChanged(GameQuestion * question);

private:
	void loadGameData();

	QList<GameEnemyData *> m_enemies;
	QList<GameLadder *>m_ladders;
	QString m_playerCharacter;
	QVariantMap m_gameData;
	QVariantMap m_terrainData;
	int m_level;
	QMap<int, GameBlock *> m_blocks;
	QString m_terrain;
	QQuickItem * m_player;
	QQuickItem * m_gameScene;
	int m_startHp;
	GameObject * m_playerStartPosition;
	int m_startBlock;
	bool m_running;
	QQuickItem * m_itemPage;
	GameQuestion * m_question;
};

#endif // COSGAME_H
