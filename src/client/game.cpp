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
	m_gameResult = GameInactive;
	m_maxSec = 0;
	m_maxHP = 0;
	m_gameMode = GameLinear;
	m_intro = nullptr;
	m_outro = nullptr;

	m_targetCount = 0;
	m_targetDone = 0;

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

	qDeleteAll(m_blocks.begin(), m_blocks.end());
	m_blocks.clear();

}




/**
 * @brief Game::prepare
 * @return
 */

bool Game::prepare()
{
	Q_ASSERT(m_client);

	if (!m_map) {
		m_client->sendMessageWarning(tr("Játék"), tr("Nincs megadva pálya!"));
		return false;
	}

	if (m_level < 1) {
		m_client->sendMessageWarning(tr("Játék"), tr("Nincs megadva szint!"));
		return false;
	}

	if (m_missionId < 0) {
		m_client->sendMessageWarning(tr("Játék"), tr("Nincs megadva küldetés!"));
		return false;
	}

	if (m_gameState != GameInvalid) {
		m_client->sendMessageWarning(tr("Játék"), tr("A játék már elő van készítve!"));
		return false;
	}

	QVariantMap data = m_map->missionGet(m_missionId, m_isSummary, true, m_level);

	if (data.value("id", -1).toInt() == -1) {
		m_client->sendMessageWarning(tr("Játék"), tr("Érvénytelen küldetés!"));
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
		m_client->sendMessageWarning(tr("Játék"), tr("A megadott szint nincs beállítva a játékban!"));
		return false;
	}


	setMaxSec(foundLevel.value("sec", 0).toInt());
	setMaxHP(foundLevel.value("hp", 0).toInt());
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
		m_client->sendMessageWarning(tr("Játék"), tr("Egyetlen célpont sincs a játékban!"));
		return false;
	}


	bool hasObjective = false;

	foreach (QVariant v, storages) {
		QVariantList l = v.toMap().value("objectives").toList();

		if (!l.isEmpty())
			hasObjective = true;
	}


	if (!hasObjective) {
		m_client->sendMessageWarning(tr("Játék"), tr("Egyetlen fegyver sincs megadva ezen a szinten!"));
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
		m_client->sendMessageWarning(tr("Játék"), tr("Egyetlen feladatot sem lehet készíteni ezen a szinten!"));
		return false;
	}

	qDebug() << "TARGETS" << targetcount;

	setTargetCount(targetcount);
	setTargetDone(0);
	setCurrentBlock(0);

	setGameState(GamePrepared);
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

void Game::setCurrentBlock(int currentBlock)
{
	if (m_currentBlock == currentBlock)
		return;

	m_currentBlock = currentBlock;
	emit currentBlockChanged(m_currentBlock);
}


void Game::setBlocks(QList<Block*> blocks)
{
	if (m_blocks == blocks)
		return;

	m_blocks = blocks;
	emit blocksChanged(m_blocks);
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

void Game::setMaxSec(int maxSec)
{
	if (m_maxSec == maxSec)
		return;

	m_maxSec = maxSec;
	emit maxSecChanged(m_maxSec);
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














