/*
 * ---- Call of Suli ----
 *
 * game.cpp
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

#include "game.h"


/**
 * @brief Game::Game
 * @param parent
 */

Game::Game(QObject *parent)
	: AbstractActivity(parent)
{
	m_map = nullptr;
	m_level = -1;
	m_missionId = -1;
	m_isSummary = false;

	m_gameState = GameInvalid;
	m_gameResult = GameResultInvalid;
	m_maxMSec = 0;
	m_maxHP = 0;
	m_gameMode = GameLinear;
	m_intro = nullptr;
	m_outro = nullptr;

	m_currentBlockIndex = -1;
	m_targetCount = 0;
	m_targetDone = 0;

	m_gamePlayMode = GamePlayOffline;
	m_gameId = -1;

	m_timer = new QTimer(this);
	m_timer->setInterval(200);

	m_targetBlockCount = -1;
	m_targetBlockDone = -1;

	m_currentMSec = 0;
	m_currentHP = 0;
	m_showCorrect = false;

	connect(m_timer, &QTimer::timeout, this, &Game::onTimerTimeout);

	connect(this, &Game::gameRegistered, this, &Game::onRegistered);
}


/**
 * @brief Game::clientSetup
 */

void Game::clientSetup()
{
	connect(m_client, &Client::jsonGameReceived, this, &Game::onJsonReceived);
}




/**
 * @brief Game::onJsonReceived
 * @param object
 * @param binaryData
 * @param clientMsgId
 */

void Game::onJsonReceived(const QJsonObject &object, const QByteArray &binaryData, const int &clientMsgId)
{
	Q_UNUSED(object)
	Q_UNUSED(binaryData)
	Q_UNUSED(clientMsgId)

	// on game registered -> emit registered
	// on game register closed -> emit register closed
}


/**
 * @brief Game::onTimerTimeout
 */

void Game::onTimerTimeout()
{
	qint64 sec = QDateTime::currentDateTime().msecsTo(m_dateTimeEnd);

	if (sec <= 0) {
		timeout();
		setCurrentMSec(0);
		return;
	}

	setCurrentMSec(sec);
}


/**
 * @brief Game::onRegistered
 */

void Game::onRegistered()
{
	setGameState(GameRegistered);
	open();
}






/**
 * @brief Game::onPrepared
 */

void Game::playPrepared()
{
	if (m_gamePlayMode == GamePlayOnline)
		registerRequest();
	else
		open();
}



/**
 * @brief Game::timeout
 */

void Game::timeout()
{
	if (m_gameState != GameRun)
		return;

	m_timer->stop();
	setGameResult(GameResultTimeout);
	setGameState(GameClosing);
	emit gameFailed();

	if (m_gamePlayMode == GamePlayOnline) {
		// closing
	}
}




/**
 * @brief Game::nextBlockTarget
 */

void Game::nextTarget()
{
	if (m_gameState != GameRun) {
		m_client->sendMessageError(tr("Játék"), tr("Érvénytelen kérés"), "nextTarget()");
		return;
	}

	if (m_currentBlockIndex == -1) {
		setTargetDone(0);
		setTargetBlockDone(0);
		setCurrentBlockIndex(0);
		setCurrentBlockCurrentIndex(-1);
	}

	while (m_currentBlockIndex < m_blocks.count()) {
		int tc = m_blocks.value(m_currentBlockIndex)->targetCount();

		setTargetBlockCount(tc);

		int nextIndex = m_blocks.value(m_currentBlockIndex)->currentIndex()+1;

		if (nextIndex >= tc) {
			setTargetBlockDone(0);
			setTargetDone(m_targetDone+tc);

			int nextBlock = m_currentBlockIndex+1;

			if (nextBlock >= m_blocks.count()) {
				succeed();
				return;
			} else {
				setCurrentBlockIndex(nextBlock);
				setCurrentBlockCurrentIndex(-1);
				setTargetBlockCount(m_blocks.value(m_currentBlockIndex)->targetCount());
				nextIndex = -1;
			}
		} else {
			setTargetBlockDone(nextIndex);
			setCurrentBlockCurrentIndex(nextIndex);
			qDebug() << "TARGET POPULATED" << m_currentBlockIndex << nextIndex;
			AbstractStorage::Target t = getCurrentBlockCurrentTarget();
			emit targetPopulated(t.module, t.task, m_showCorrect ? t.solution : QJsonObject());
			return;
		}
	}
}


/**
 * @brief Game::succeed
 */

void Game::succeed()
{
	if (m_gameState != GameRun)
		return;

	m_timer->stop();

	setGameResult(GameResultSucceed);
	emit gameSucceed(m_outro);
	setGameState(GameClosing);

	if (m_gamePlayMode == GamePlayOnline) {
		// closing
	}
}



/**
 * @brief Game::failed
 */

void Game::failed()
{
	if (m_gameState != GameRun)
		return;

	m_timer->stop();

	setGameResult(GameResultFailed);
	setGameState(GameClosing);
	emit gameFailed();

	if (m_gamePlayMode == GamePlayOnline) {
		// closing
	}
}





/**
 * @brief Game::~Game
 */

Game::~Game()
{
	if (m_intro)
		delete m_intro;

	if (m_outro)
		delete m_outro;

	delete m_timer;

	qDeleteAll(m_blocks.begin(), m_blocks.end());
	m_blocks.clear();

}




/**
 * @brief Game::prepare
 * @return
 */

bool Game::prepare()
{
	if (!m_map) {
		emit gamePrepareError(tr("Nincs megadva pálya!"));
		return false;
	}

	if (m_level < 1) {
		emit gamePrepareError(tr("Nincs megadva szint!"));
		return false;
	}

	if (m_missionId < 0) {
		emit gamePrepareError(tr("Nincs megadva küldetés!"));
		return false;
	}

	if (m_gameState != GameInvalid) {
		emit gamePrepareError(tr("A játék már elő van készítve!"));
		return false;
	}

	QVariantMap data = m_map->missionGet(m_missionId, m_isSummary, true, m_level);

	if (data.value("id", -1).toInt() == -1) {
		emit gamePrepareError(tr("Érvénytelen küldetés!"));
		return false;
	}

	QVariantList levels = data.value("levels").toList();

	QVariantMap foundLevel;

	foreach (QVariant v, levels) {
		QVariantMap m = v.toMap();
		if (m.value("level", -1).toInt() == m_level) {
			foundLevel = m;
			break;
		}
	}

	if (foundLevel.isEmpty()) {
		emit gamePrepareError(tr("A megadott szint nincs beállítva a játékban!"));
		return false;
	}


	setMaxMSec(foundLevel.value("sec", 0).toInt()*1000);
	setCurrentMSec(m_maxMSec);
	setMaxHP(foundLevel.value("hp", 0).toInt());
	setCurrentHP(m_maxHP);
	setShowCorrect(foundLevel.value("showCorrect").toBool());

	int mode = foundLevel.value("mode", 0).toInt();

	if (mode == 2 || m_isSummary)
		setGameMode(GameFullShuffle);
	else if (mode == 1)
		setGameMode(GameChapterShuffle);
	else
		setGameMode(GameLinear);



	int introId = data.value("introId", -1).toInt();
	int outroId = data.value("outroId", -1).toInt();
	int introLevelMin = data.value("introLevelMin", 0).toInt();
	int introLevelMax = data.value("introLevelMax", 0).toInt();
	int outroLevelMin = data.value("outroLevelMin", 0).toInt();
	int outroLevelMax = data.value("outroLevelMax", 0).toInt();

	if (introId != -1 && (introLevelMin == 0 || m_level>=introLevelMin) && (introLevelMax == 0 || m_level<=introLevelMax)) {
		m_intro = new Intro(this);
		m_intro->setType(Intro::IntroIntro);
		m_intro->setText(data.value("introText").toString());
		m_intro->setImage(data.value("introImg").toString());
		m_intro->setMedia(data.value("introMedia").toString());
		m_intro->setSeconds(data.value("introSec").toInt());
	}

	if (outroId != -1 && (outroLevelMin == 0 || m_level>=outroLevelMin) && (outroLevelMax == 0 || m_level<=outroLevelMax)) {
		m_outro = new Intro(this);
		m_outro->setType(Intro::IntroOutro);
		m_outro->setText(data.value("outroText").toString());
		m_outro->setImage(data.value("outroImg").toString());
		m_outro->setMedia(data.value("outroMedia").toString());
		m_outro->setSeconds(data.value("outroSec").toInt());
	}



	QVariantList storages = data.value("storages").toList();


	if (storages.isEmpty()) {
		emit gamePrepareError(tr("Egyetlen célpont sincs a játékban!"));
		return false;
	}


	bool hasObjective = false;

	foreach (QVariant v, storages) {
		QVariantList l = v.toMap().value("objectives").toList();

		if (!l.isEmpty())
			hasObjective = true;
	}


	if (!hasObjective) {
		emit gamePrepareError(tr("Egyetlen fegyver sincs megadva ezen a szinten!"));
		return false;
	}



	if (m_gameMode == GameFullShuffle) {
		Block *b = new Block(this);

		foreach(QVariant v, storages) {
			b->addStorage(v.toMap());
		}

		m_blocks << b;
	} else {
		while (storages.count()) {
			int idx = 0;
			int count = storages.count();

			if (count > 1 && m_gameMode == GameChapterShuffle) {
				idx = random() % count;
			}

			QVariant m = storages.takeAt(idx);

			Block *b = new Block(this);

			b->addStorage(m.toMap());

			m_blocks << b;
		}
	}


	int targetcount = 0;
	foreach (Block *b, m_blocks) {
		b->resetTargets();
		targetcount += b->targetCount();
	}

	if (!targetcount) {
		emit gamePrepareError(tr("Egyetlen feladatot sem lehet készíteni ezen a szinten!"));
		return false;
	}

	setTargetCount(targetcount);
	setTargetDone(0);
	setCurrentBlockIndex(-1);

	setGameState(GamePrepared);
	emit gamePrepared();
	return true;
}




/**
 * @brief Game::setMap
 * @param map
 */

void Game::setMap(Map *map)
{
	if (m_map == map)
		return;

	m_map = map;
	emit mapChanged(m_map);
}


void Game::setLevel(int level)
{
	if (m_level == level)
		return;

	m_level = level;
	emit levelChanged(m_level);
}

void Game::setMissionId(int missionId)
{
	if (m_missionId == missionId)
		return;

	m_missionId = missionId;
	emit missionIdChanged(m_missionId);
}

void Game::setIsSummary(bool isSummary)
{
	if (m_isSummary == isSummary)
		return;

	m_isSummary = isSummary;
	emit isSummaryChanged(m_isSummary);
}


/**
 * @brief Game::abort
 */

void Game::abort()
{
	if (m_gameState == GameInvalid || m_gameState == GameFinished)
		return;

	setGameResult(GameResultAborted);
	setGameState(GameClosing);
	emit gameFailed();

	if (m_gamePlayMode == GamePlayOnline) {
		// closing
	}

}




/**
 * @brief Game::recordRequest
 */

void Game::registerRequest()
{
	if (m_gamePlayMode != GamePlayOnline) {
		m_client->sendMessageError(tr("Játék"), tr("Érvénytelen kérés"), "registerRequest()");
		return;
	}

	// send request
}


/**
 * @brief Game::registerCloseRequest
 */

void Game::registerCloseRequest()
{
	if (m_gamePlayMode != GamePlayOnline) {
		m_client->sendMessageError(tr("Játék"), tr("Érvénytelen kérés"), "registerCloseRequest()");
		return;
	}

	// send request
}




/**
 * @brief Game::open
 */

void Game::open()
{
	if ((m_gamePlayMode == GamePlayOnline && m_gameState != GameRegistered) ||
		(m_gamePlayMode == GamePlayOffline && m_gameState != GamePrepared)) {
		m_client->sendMessageError(tr("Játék"), tr("Érvénytelen kérés"), "open()");
		return;
	}

	setGameState(GameOpening);

	if (m_intro) {
		emit introPopulated(m_intro);				// utána még start()
	} else {
		start();
	}

}




/**
 * @brief Game::start
 */

void Game::start()
{
	if (m_gameState != GameOpening) {
		m_client->sendMessageError(tr("Játék"), tr("Érvénytelen kérés"), "start()");
		return;
	}

	m_dateTimeEnd = QDateTime::currentDateTime().addMSecs(m_maxMSec);
	m_timer->start();

	setGameState(GameRun);

	nextTarget();
}







/**
 * @brief Game::targetDone
 * @param data
 */

void Game::check(const QJsonObject &data)
{
	AbstractStorage::Target t = getCurrentBlockCurrentTarget();

	bool solution = t.solutionFunc(t, data);

	if (solution) {
		emit solutionCorrect();
		nextTarget();
	} else {
		emit solutionFail();

		if (m_currentHP > 1) {
			setCurrentHP(m_currentHP-1);
			m_blocks[m_currentBlockIndex]->resetTargets();
			setCurrentBlockCurrentIndex(-1);
			setTargetBlockDone(0);
			nextTarget();
		} else {
			setCurrentHP(0);
			failed();
		}
	}
}





/**
 * @brief Game::close
 */

void Game::close()
{
	if (m_gameState == GameClosing)
		setGameState(GameFinished);
}



/**
 * @brief Game::setCurrentBlockCurrentIndex
 * @param index
 */

void Game::setCurrentBlockCurrentIndex(const int &index)
{
	if (m_currentBlockIndex >= 0 && m_currentBlockIndex < m_blocks.count()) {
		m_blocks[m_currentBlockIndex]->setCurrentIndex(index);
	}
}


/**
 * @brief Game::getCurrentBlockCurrentTarget
 * @return
 */

AbstractStorage::Target Game::getCurrentBlockCurrentTarget()
{
	if (m_currentBlockIndex >= 0 && m_currentBlockIndex < m_blocks.count()) {
		return m_blocks[m_currentBlockIndex]->currentTarget();
	}

	return AbstractStorage::Target();
}





void Game::setGameId(int gameId)
{
	if (m_gameId == gameId)
		return;

	m_gameId = gameId;
	emit gameIdChanged(m_gameId);
}

void Game::setGamePlayMode(Game::GamePlayMode gamePlayMode)
{
	if (m_gamePlayMode == gamePlayMode)
		return;

	m_gamePlayMode = gamePlayMode;
	emit gamePlayModeChanged(m_gamePlayMode);
}

void Game::setShowCorrect(bool showCorrect)
{
	if (m_showCorrect == showCorrect)
		return;

	m_showCorrect = showCorrect;
	emit showCorrectChanged(m_showCorrect);
}


void Game::setCurrentMSec(quint64 currentSec)
{
	if (m_currentMSec == currentSec)
		return;

	m_currentMSec = currentSec;
	emit currentMSecChanged(m_currentMSec);
}

void Game::setCurrentHP(int currentHP)
{
	if (m_currentHP == currentHP)
		return;

	m_currentHP = currentHP;
	emit currentHPChanged(m_currentHP);
}



void Game::setTargetBlockCount(int targetBlockCount)
{
	if (m_targetBlockCount == targetBlockCount)
		return;

	m_targetBlockCount = targetBlockCount;
	emit targetBlockCountChanged(m_targetBlockCount);
}

void Game::setTargetBlockDone(int targetBlockDone)
{
	if (m_targetBlockDone == targetBlockDone)
		return;

	m_targetBlockDone = targetBlockDone;
	emit targetBlockDoneChanged(m_targetBlockDone);
}

void Game::setMissionName(QString missionName)
{
	if (m_missionName == missionName)
		return;

	m_missionName = missionName;
	emit missionNameChanged(m_missionName);
}



void Game::setTargetCount(int targetCount)
{
	if (m_targetCount == targetCount)
		return;

	m_targetCount = targetCount;
	emit targetCountChanged(m_targetCount);
}

void Game::setTargetDone(int targetDone)
{
	if (m_targetDone == targetDone)
		return;

	m_targetDone = targetDone;
	emit targetDoneChanged(m_targetDone);
}

void Game::setCurrentBlockIndex(int currentBlock)
{
	if (m_currentBlockIndex == currentBlock)
		return;

	m_currentBlockIndex = currentBlock;
	emit currentBlockIndexChanged(m_currentBlockIndex);
}


void Game::setIntro(Intro *intro)
{
	if (m_intro == intro)
		return;

	m_intro = intro;
	emit introChanged(m_intro);
}

void Game::setOutro(Intro *outro)
{
	if (m_outro == outro)
		return;

	m_outro = outro;
	emit outroChanged(m_outro);
}

void Game::setMaxMSec(quint64 maxSec)
{
	if (m_maxMSec == maxSec)
		return;

	m_maxMSec = maxSec;
	emit maxMSecChanged(m_maxMSec);
}

void Game::setMaxHP(int maxHP)
{
	if (m_maxHP == maxHP)
		return;

	m_maxHP = maxHP;
	emit maxHPChanged(m_maxHP);
}


void Game::setGameMode(Game::GameMode gameMode)
{
	if (m_gameMode == gameMode)
		return;

	m_gameMode = gameMode;
	emit gameModeChanged(m_gameMode);
}

void Game::setGameState(Game::GameState gameState)
{
	if (m_gameState == gameState)
		return;

	m_gameState = gameState;
	emit gameStateChanged(m_gameState);
}

void Game::setGameResult(Game::GameResult gameResult)
{
	if (m_gameResult == gameResult)
		return;

	m_gameResult = gameResult;
	emit gameResultChanged(m_gameResult);
}














