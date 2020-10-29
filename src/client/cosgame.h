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

class CosGame : public Game
{
	Q_OBJECT

	Q_PROPERTY(QString playerCharacter READ playerCharacter WRITE setPlayerCharacter NOTIFY playerCharacterChanged)
	Q_PROPERTY(QString terrain READ terrain WRITE setTerrain NOTIFY terrainChanged)

	Q_PROPERTY(QVariantMap gameData READ gameData WRITE setGameData NOTIFY gameDataChanged)
	Q_PROPERTY(QVariantMap terrainData READ terrainData NOTIFY terrainDataChanged)

	Q_PROPERTY(QQuickItem * player READ player WRITE setPlayer NOTIFY playerChanged)
	Q_PROPERTY(int level READ level WRITE setLevel NOTIFY levelChanged)

	Q_PROPERTY(QMap<int, GameBlock *> blocks READ blocks WRITE setBlocks NOTIFY blocksChanged)

	Q_PROPERTY(QList<GameEnemyData *> enemies READ enemies WRITE setEnemies NOTIFY enemiesChanged)
	Q_PROPERTY(int activeEnemies READ activeEnemies NOTIFY activeEnemiesChanged)

	Q_PROPERTY(int currentBlock READ currentBlock WRITE setCurrentBlock NOTIFY currentBlockChanged)
	Q_PROPERTY(int previousBlock READ previousBlock WRITE setPreviousBlock NOTIFY previousBlockChanged)

	Q_PROPERTY(QList<GameLadder *> ladders READ ladders WRITE setLadders NOTIFY laddersChanged)


public:
	CosGame(QQuickItem *parent = 0);
	~CosGame();

	Q_INVOKABLE void loadTerrainData();

	void addEnemy(GameEnemyData *enemy);
	void addPlayerPosition(const int &block, const int &blockFrom, const int &x, const int &y);
	Q_INVOKABLE void recreateEnemies(QQuickItem *scene);
	Q_INVOKABLE void resetEnemy(GameEnemyData *enemyData);
	Q_INVOKABLE void setEnemiesMoving(const bool &moving);

	void addLadder(GameLadder *ladder);

	QString playerCharacter() const { return m_playerCharacter; }
	QString terrain() const { return m_terrain; }
	QVariantMap gameData() const { return m_gameData; }
	QVariantMap terrainData() const { return m_terrainData; }

	int level() const { return m_level; }
	QQuickItem * player() const { return m_player; }

	QList<GameEnemyData *> enemies() const { return m_enemies; }
	int activeEnemies() const;

	QMap<int, GameBlock *> blocks() const { return m_blocks; }

	int currentBlock() const { return m_currentBlock; }
	int previousBlock() const { return m_previousBlock; }

	QList<GameLadder *> ladders() const { return m_ladders; }

public slots:
	void placePlayer();

	void setPlayerCharacter(QString playerCharacter);
	void setTerrain(QString terrain);
	void setGameData(QVariantMap gameData);
	void setTerrainData(QVariantMap terrainData);

	void setLevel(int level);
	void setPlayer(QQuickItem * player);

	void setEnemies(QList<GameEnemyData *> enemies);
	void setBlocks(QMap<int, GameBlock *> blocks);
	void setCurrentBlock(int currentBlock);
	void setPreviousBlock(int previousBlock);

	void setLadders(QList<GameLadder *> ladders);

signals:
	void blocksLoaded();
	void enemiesRecreated();

	void playerCharacterChanged(QString playerCharacter);
	void terrainChanged(QString terrain);
	void gameDataChanged(QVariantMap gameData);
	void terrainDataChanged(QVariantMap terrainData);

	void levelChanged(int level);

	void enemiesChanged(QList<GameEnemyData *> enemies);
	void activeEnemiesChanged(int activeEnemies);
	void blocksChanged(QMap<int, GameBlock *> blocks);
	void playerChanged(QQuickItem * player);
	void currentBlockChanged(int currentBlock);
	void previousBlockChanged(int previousBlock);

	void laddersChanged(QList<GameLadder *> ladders);

private:
	void loadGameData();
	void loadBlocks();

	QString m_playerCharacter;
	QVariantMap m_gameData;
	QVariantMap m_terrainData;
	int m_level;
	QList<GameEnemyData *> m_enemies;
	QMap<int, GameBlock *> m_blocks;
	QString m_terrain;
	QQuickItem * m_player;
	int m_currentBlock;
	int m_previousBlock;
	QList<GameLadder *> m_ladders;
};

#endif // COSGAME_H
