/*
 * ---- Call of Suli ----
 *
 * game.h
 *
 * Created on: 2020. 05. 10.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Game
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

#ifndef GAME_H
#define GAME_H

#include <QObject>
#include "abstractactivity.h"
#include "map.h"
#include "intro.h"

class Game : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(Map* map READ map WRITE setMap NOTIFY mapChanged)
	Q_PROPERTY(int level READ level WRITE setLevel NOTIFY levelChanged)
	Q_PROPERTY(int missionId READ missionId WRITE setMissionId NOTIFY missionIdChanged)
	Q_PROPERTY(bool isSummary READ isSummary WRITE setIsSummary NOTIFY isSummaryChanged)

	Q_PROPERTY(GameState gameState READ gameState WRITE setGameState NOTIFY gameStateChanged)
	Q_PROPERTY(GameResult gameResult READ gameResult WRITE setGameResult NOTIFY gameResultChanged)
	Q_PROPERTY(GameMode gameMode READ gameMode WRITE setGameMode NOTIFY gameModeChanged)

	Q_PROPERTY(int maxSec READ maxSec WRITE setMaxSec NOTIFY maxSecChanged)
	Q_PROPERTY(int maxHP READ maxHP WRITE setMaxHP NOTIFY maxHPChanged)
	Q_PROPERTY(Intro* intro READ intro WRITE setIntro NOTIFY introChanged)
	Q_PROPERTY(Intro* outro READ outro WRITE setOutro NOTIFY outroChanged)


public:

	enum GameState {
		GameInvalid,
		GamePrepared,
		GameRun,
		GameFinished
	};
	Q_ENUM(GameState)


	enum GameResult {
		GameInactive,
		GameActive,
		GameTimeout,
		GameComplete,
		GameCompletePerfect
	};
	Q_ENUM(GameResult)


	enum GameMode {
		GameLinear,
		GameChapterShuffle,
		GameFullShuffle
	};
	Q_ENUM(GameMode)


	Game(QObject *parent=nullptr);
	~Game();

	Map* map() const { return m_map; }
	int level() const { return m_level; }
	int missionId() const { return m_missionId; }
	bool isSummary() const { return m_isSummary; }

	GameState gameState() const { return m_gameState; }
	GameResult gameResult() const { return m_gameResult; }
	int maxSec() const { return m_maxSec; }
	int maxHP() const { return m_maxHP; }
	GameMode gameMode() const { return m_gameMode; }
	Intro* intro() const { return m_intro; }
	Intro* outro() const { return m_outro; }

public slots:

	bool prepare();

	void setMap(Map* map);
	void setLevel(int level);
	void setMissionId(int missionId);
	void setIsSummary(bool isSummary);


private slots:

	void setGameState(GameState gameState);
	void setGameResult(GameResult gameResult);
	void setMaxSec(int maxSec);
	void setMaxHP(int maxHP);
	void setGameMode(GameMode gameMode);
	void setIntro(Intro* intro);
	void setOutro(Intro* outro);


signals:

	void mapChanged(Map* map);
	void levelChanged(int level);
	void missionIdChanged(int missionId);
	void isSummaryChanged(bool isSummary);

	void gameStateChanged(GameState gameState);
	void gameResultChanged(GameResult gameResult);
	void maxSecChanged(int maxSec);
	void maxHPChanged(int maxHP);
	void gameModeChanged(GameMode gameMode);
	void introChanged(Intro* intro);
	void outroChanged(Intro* outro);

private:
	Map* m_map;
	int m_level;
	int m_missionId;
	bool m_isSummary;
	GameState m_gameState;
	GameResult m_gameResult;
	int m_maxSec;
	int m_maxHP;
	GameMode m_gameMode;
	Intro* m_intro;
	Intro* m_outro;
};

#endif // GAME_H
