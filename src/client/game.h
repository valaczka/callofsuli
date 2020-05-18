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
#include "block.h"
#include "abstractstorage.h"

class Block;

class Game : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(Map* map READ map WRITE setMap NOTIFY mapChanged)
	Q_PROPERTY(int level READ level WRITE setLevel NOTIFY levelChanged)
	Q_PROPERTY(int missionId READ missionId WRITE setMissionId NOTIFY missionIdChanged)
	Q_PROPERTY(bool isSummary READ isSummary WRITE setIsSummary NOTIFY isSummaryChanged)
	Q_PROPERTY(GamePlayMode gamePlayMode READ gamePlayMode WRITE setGamePlayMode NOTIFY gamePlayModeChanged)

	Q_PROPERTY(GameState gameState READ gameState NOTIFY gameStateChanged)
	Q_PROPERTY(GameResult gameResult READ gameResult NOTIFY gameResultChanged)
	Q_PROPERTY(GameMode gameMode READ gameMode NOTIFY gameModeChanged)
	Q_PROPERTY(int gameId READ gameId NOTIFY gameIdChanged)

	Q_PROPERTY(quint64 maxMSec READ maxMSec NOTIFY maxMSecChanged)
	Q_PROPERTY(quint64 currentMSec READ currentMSec NOTIFY currentMSecChanged)
	Q_PROPERTY(int maxHP READ maxHP NOTIFY maxHPChanged)
	Q_PROPERTY(int currentHP READ currentHP WRITE setCurrentHP NOTIFY currentHPChanged)
	Q_PROPERTY(Intro* intro READ intro NOTIFY introChanged)
	Q_PROPERTY(Intro* outro READ outro NOTIFY outroChanged)
	Q_PROPERTY(QString missionName READ missionName NOTIFY missionNameChanged)
	Q_PROPERTY(bool showCorrect READ showCorrect NOTIFY showCorrectChanged)

	Q_PROPERTY(int currentBlockIndex READ currentBlockIndex NOTIFY currentBlockIndexChanged)
	Q_PROPERTY(int targetCount READ targetCount NOTIFY targetCountChanged)
	Q_PROPERTY(int targetDone READ targetDone NOTIFY targetDoneChanged)
	Q_PROPERTY(int targetBlockCount READ targetBlockCount NOTIFY targetBlockCountChanged)
	Q_PROPERTY(int targetBlockDone READ targetBlockDone NOTIFY targetBlockDoneChanged)

public:

	enum GameState {
		GameInvalid,
		GamePrepared,
		GameOpening,
		GameRegistered,
		GameRun,
		GameClosing,
		GameFinished
	};
	Q_ENUM(GameState)


	enum GameResult {
		GameResultInvalid,
		GameResultSucceed,						// Sikerült végigcsinálni
		GameResultFailed,						// Nem sikerült végigcsinálni
		GameResultAborted,						// Felhasználó megszakította
		GameResultTimeout,						// Lejárt az idő
		GameResultCancelled						// Érvénytelenné vált
	};
	Q_ENUM(GameResult)


	enum GameMode {
		GameLinear,
		GameChapterShuffle,
		GameFullShuffle
	};
	Q_ENUM(GameMode)

	enum GamePlayMode {
		GamePlayOffline,
		GamePlayOnline
	};
	Q_ENUM(GamePlayMode)



	Game(QObject *parent=nullptr);
	~Game();


	Map* map() const { return m_map; }
	int level() const { return m_level; }
	int missionId() const { return m_missionId; }
	bool isSummary() const { return m_isSummary; }

	GameState gameState() const { return m_gameState; }
	GameResult gameResult() const { return m_gameResult; }
	quint64 maxMSec() const { return m_maxMSec; }
	quint64 maxHP() const { return m_maxHP; }
	GameMode gameMode() const { return m_gameMode; }
	GamePlayMode gamePlayMode() const { return m_gamePlayMode; }
	Intro* intro() const { return m_intro; }
	Intro* outro() const { return m_outro; }
	QList<Block *> blocks() const { return m_blocks; }
	int currentBlockIndex() const { return m_currentBlockIndex; }
	int targetCount() const { return m_targetCount; }
	int targetDone() const { return m_targetDone; }
	int gameId() const { return m_gameId; }
	QString missionName() const { return m_missionName; }
	int targetBlockCount() const { return m_targetBlockCount; }
	int targetBlockDone() const { return m_targetBlockDone; }
	int currentMSec() const { return m_currentMSec; }
	int currentHP() const { return m_currentHP; }
	bool showCorrect() const { return m_showCorrect; }

public slots:

	bool prepare();
	void playPrepared();
	void abort();
	void start();
	void check(const QJsonObject &data);
	void close();

	void setMap(Map* map);
	void setLevel(int level);
	void setMissionId(int missionId);
	void setIsSummary(bool isSummary);
	void setGamePlayMode(GamePlayMode gamePlayMode);

private slots:

	void clientSetup() override;
	void onJsonReceived(const QJsonObject &object, const QByteArray &binaryData, const int &clientMsgId);
	void onTimerTimeout();
	void onRegistered();
	void open();
	void registerRequest();
	void registerCloseRequest();

	void timeout();
	void nextTarget();
	void succeed();
	void failed();

	void setGameState(GameState gameState);
	void setGameResult(GameResult gameResult);
	void setMaxMSec(quint64 maxMSec);
	void setMaxHP(int maxHP);
	void setGameMode(GameMode gameMode);
	void setIntro(Intro* intro);
	void setOutro(Intro* outro);
	void setCurrentBlockIndex(int currentBlockIndex);
	void setTargetCount(int targetCount);
	void setTargetDone(int targetDone);
	void setGameId(int gameId);
	void setMissionName(QString missionName);
	void setTargetBlockCount(int targetBlockCount);
	void setTargetBlockDone(int targetBlockDone);
	void setCurrentMSec(quint64 currentMSec);
	void setCurrentHP(int currentHP);
	void setShowCorrect(bool showCorrect);

private:
	void setCurrentBlockCurrentIndex(const int &index);
	AbstractStorage::Target getCurrentBlockCurrentTarget();


signals:

	void gamePrepareError(const QString &errorString);
	void gamePrepared();
	void gameRegistered();
	void gameStarted();
	void gameSucceed(Intro *outro);
	void gameFailed();
	void gameRegisterClosed();

	void targetPopulated(const QString &module, const QJsonObject &task, const QJsonObject &solution);
	void introPopulated(Intro *intro);

	void solutionCorrect();
	void solutionFail();

	void mapChanged(Map* map);
	void levelChanged(int level);
	void missionIdChanged(int missionId);
	void isSummaryChanged(bool isSummary);

	void gameStateChanged(GameState gameState);
	void gameResultChanged(GameResult gameResult);
	void maxMSecChanged(quint64 maxMSec);
	void maxHPChanged(int maxHP);
	void gameModeChanged(GameMode gameMode);
	void gamePlayModeChanged(GamePlayMode gamePlayMode);
	void introChanged(Intro* intro);
	void outroChanged(Intro* outro);
	void currentBlockIndexChanged(int currentBlockIndex);
	void targetCountChanged(int targetCount);
	void targetDoneChanged(int targetDone);
	void gameIdChanged(int gameId);
	void missionNameChanged(QString missionName);
	void targetBlockCountChanged(int targetBlockCount);
	void targetBlockDoneChanged(int targetBlockDone);
	void currentMSecChanged(quint64 currentMSec);
	void currentHPChanged(int currentHP);
	void showCorrectChanged(bool showCorrect);

private:
	Map* m_map;
	int m_level;
	int m_missionId;
	bool m_isSummary;
	GameState m_gameState;
	GameResult m_gameResult;
	quint64 m_maxMSec;
	int m_maxHP;
	GameMode m_gameMode;
	Intro* m_intro;
	Intro* m_outro;
	QList<Block *> m_blocks;
	int m_currentBlockIndex;
	int m_targetCount;
	int m_targetDone;
	GamePlayMode m_gamePlayMode;
	int m_gameId;
	QString m_missionName;
	QTimer *m_timer;
	QDateTime m_dateTimeEnd;
	int m_targetBlockCount;
	int m_targetBlockDone;
	quint64 m_currentMSec;
	int m_currentHP;
	bool m_showCorrect;
};

#endif // GAME_H
