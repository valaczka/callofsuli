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

Q_LOGGING_CATEGORY(lcGame, "app.game")

/**
 * @brief Game::Game
 * @param client
 */

AbstractGame::AbstractGame(const Mode &mode, Client *client)
	: QObject{client}
	, m_client(client)
	, m_mode(mode)
{
	Q_ASSERT(client);

	qCDebug(lcGame).noquote() << tr("Game created") << this;
}


/**
 * @brief Game::~Game
 */

AbstractGame::~AbstractGame()
{
	if (m_gameQuestion && m_gameQuestion->game() == this)
		m_gameQuestion->setGame(nullptr);

	qCDebug(lcGame).noquote() << tr("Game destroyed") << this;
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
	qCDebug(lcGame) << tr("Game page item destroyed");
	m_client->setCurrentGame(nullptr);
	deleteLater();
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

	qCDebug(lcGame).noquote() << tr("Game question set:") << m_gameQuestion;

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
		qCDebug(lcGame).noquote() << tr("Game started:") << this;
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
		qCDebug(lcGame).noquote() << tr("Game finished after %1 milliseconds:").arg(m_elapsedMsec) << this;
		return true;
	}

	return false;
}


/**
 * @brief AbstractGame::mode
 * @return
 */

const AbstractGame::Mode &AbstractGame::mode() const
{
	return m_mode;
}





/**
 * @brief AbstractGame::load
 */

bool AbstractGame::load()
{
	qCDebug(lcGame).noquote() << tr("Load game") << this;

	if (m_pageItem) {
		qCCritical(lcGame).noquote() << tr("A játék lapja már létezik!") << m_pageItem;
		return false;
	}

	QQuickItem *page = loadPage();

	if (page) {
		connect(page, &QQuickItem::destroyed, this, &AbstractGame::onPageItemDestroyed);
		setPageItem(page);

		qCDebug(lcGame) << tr("Game page loaded");
		return true;
	} else {
		qCCritical(lcGame).noquote() << tr("Game page create error");
		return false;
	}
}





/**
 * @brief AbstractGame::elapsedTimeStart
 */

void AbstractGame::elapsedTimeStart()
{
	if (m_elapsedMsec != -1 || m_elapsedTimer.isValid()) {
		qCWarning(lcGame).noquote() << tr("Elapsed timer has already been started");
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
		qCWarning(lcGame).noquote() << tr("Elapsed timer has never been started");
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
		qCWarning(lcGame).noquote() << tr("Missing game page");
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


