/*
 * ---- Call of Suli ----
 *
 * game.cpp
 *
 * Created on: 2022. 12. 11.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Game
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

#include "abstractgame.h"
#include "client.h"
#include "gamequestion.h"
#include "Logger.h"

/**
 * @brief Game::Game
 * @param client
 */

AbstractGame::AbstractGame(const GameMap::GameMode &mode, Client *client)
	: QObject{client}
	, m_client(client)
	, m_mode(mode)
{
	Q_ASSERT(client);

	LOG_CTRACE("game") << "Game created" << this;
}


/**
 * @brief Game::~Game
 */

AbstractGame::~AbstractGame()
{
	if (m_gameQuestion && m_gameQuestion->game() == this)
		m_gameQuestion->setGame(nullptr);

	LOG_CTRACE("game") << "Game destroyed" << this;
}


/**
 * @brief AbstractGame::pageItem
 * @return
 */

QQuickItem *AbstractGame::pageItem() const
{
	return m_pageItem;
}






/**
 * @brief AbstractGame::onPageItemDestroyed
 */

void AbstractGame::onPageItemDestroyed()
{
	setPageItem(nullptr);
	LOG_CTRACE("game") << "Game page item destroyed";
	gameDestroy();
}

void AbstractGame::setFinishState(const FinishState &newFinishState)
{
	m_finishState = newFinishState;
}


/**
 * @brief AbstractGame::readyToDestroy
 * @return
 */

bool AbstractGame::readyToDestroy() const
{
	return m_readyToDestroy;
}


/**
 * @brief AbstractGame::setReadyToDestroy
 * @param newReadyToDestroy
 */

void AbstractGame::setReadyToDestroy(bool newReadyToDestroy)
{
	m_readyToDestroy = newReadyToDestroy;
	gameDestroy();
}


/**
 * @brief AbstractGame::finishState
 * @return
 */

const AbstractGame::FinishState &AbstractGame::finishState() const
{
	return m_finishState;
}


/**
 * @brief AbstractGame::elapsedMsec
 * @return
 */

int AbstractGame::elapsedMsec() const
{
	if (m_elapsedMsec == -1 && m_elapsedTimer.isValid())
		return m_elapsedTimer.elapsed();

	return m_elapsedMsec;
}


/**
 * @brief AbstractGame::gameQuestion
 * @return
 */

GameQuestion *AbstractGame::gameQuestion() const
{
	return m_gameQuestion;
}

void AbstractGame::setGameQuestion(GameQuestion *newGameQuestion)
{
	if (m_gameQuestion == newGameQuestion)
		return;
	m_gameQuestion = newGameQuestion;
	emit gameQuestionChanged();

	LOG_CTRACE("game") << "Game question set:" << m_gameQuestion;

	if (m_gameQuestion)
		connectGameQuestion();
}


/**
 * @brief AbstractGame::gameStart
 * @return
 */

bool AbstractGame::gameStart()
{
	if (gameStartEvent()) {
		elapsedTimeStart();
		LOG_CTRACE("game") << "Game started:" << this;
		return true;
	}

	return false;
}


/**
 * @brief AbstractGame::gameFinish
 * @return
 */

bool AbstractGame::gameFinish()
{
	if (gameFinishEvent()) {
		elapsedTimeStop();
		LOG_CTRACE("game") << "Game finished after" << m_elapsedMsec << "milliseconds:" << this;

		emit gameFinished(m_finishState);
		return true;
	}

	return false;
}


/**
 * @brief AbstractGame::gameDestroy
 */

void AbstractGame::gameDestroy()
{
	if (!m_pageItem && m_readyToDestroy)
		this->deleteLater();
}


/**
 * @brief AbstractGame::mode
 * @return
 */

const GameMap::GameMode &AbstractGame::mode() const
{
	return m_mode;
}





/**
 * @brief AbstractGame::load
 */

bool AbstractGame::load()
{
	LOG_CTRACE("game") << "Load game" << this;

	if (m_pageItem) {
		LOG_CERROR("game") << "Game page already exists" << m_pageItem;
		return false;
	}

	QQuickItem *page = loadPage();

	if (page) {
		connect(page, &QQuickItem::destroyed, this, &AbstractGame::onPageItemDestroyed);
		setPageItem(page);

		LOG_CDEBUG("game") << "Game page loaded";
		return true;
	} else {
		LOG_CERROR("game") << "Game page create error";
		return false;
	}
}





/**
 * @brief AbstractGame::elapsedTimeStart
 */

void AbstractGame::elapsedTimeStart()
{
	if (m_elapsedMsec != -1 || m_elapsedTimer.isValid()) {
		LOG_CWARNING("game") << "Elapsed timer has already been started";
		return;
	}

	m_elapsedTimer.start();
}


/**
 * @brief AbstractGame::elapsedTimeStop
 */

void AbstractGame::elapsedTimeStop()
{
	if (!m_elapsedTimer.isValid()) {
		LOG_CWARNING("game") << "Elapsed timer has never been started";
		m_elapsedMsec = -1;
		return;
	}

	m_elapsedMsec = m_elapsedTimer.elapsed();
}




/**
 * @brief AbstractGame::setPageItem
 * @param newPageItem
 */

void AbstractGame::setPageItem(QQuickItem *newPageItem)
{
	if (m_pageItem == newPageItem)
		return;
	m_pageItem = newPageItem;
	emit pageItemChanged();
}





GameMap *AbstractGame::map() const
{
	return m_map;
}

void AbstractGame::setMap(GameMap *newMap)
{
	if (m_map == newMap)
		return;
	m_map = newMap;
	emit mapChanged();
}


/**
 * @brief AbstractGame::unloadPageItem
 */

void AbstractGame::unloadPageItem()
{
	if (!m_pageItem) {
		LOG_CWARNING("game") << "Missing game page";
		return;
	}

	QVariant v = m_pageItem->property("stackViewIndex");

	if (v.isValid()) {
		m_client->stackPop(v.toInt()-1, true);
	} else {
		m_client->stackPop(-1, true);
	}
}


/**
 * @brief AbstractGame::addStatistics
 * @param stat
 */

void AbstractGame::addStatistics(const Statistics &stat)
{
	m_statistics.append(stat);
}



/**
 * @brief AbstractGame::addStatistics
 * @param uuid
 * @param success
 * @param elapsed
 */

void AbstractGame::addStatistics(const QString &uuid, const bool &success, const int &elapsed)
{
	Statistics s;
	s.objective = uuid;
	s.success = success;
	s.elapsed = elapsed;
	m_statistics.append(s);
}


/**
 * @brief AbstractGame::takeStatistics
 * @return
 */

QJsonArray AbstractGame::takeStatistics()
{
	QJsonArray r;

	foreach (const Statistics &s, m_statistics) {
		QJsonObject o;
		o[QStringLiteral("map")] = m_map ? m_map->uuid() : "";
		o[QStringLiteral("mode")] = m_mode;
		o[QStringLiteral("objective")] = s.objective;
		o[QStringLiteral("success")] = s.success;
		o[QStringLiteral("elapsed")] = s.elapsed;
		r.append(o);
	}

	m_statistics.clear();

	return r;

}


