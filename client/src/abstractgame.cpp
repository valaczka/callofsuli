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

Q_LOGGING_CATEGORY(lcGame, "app.game")

/**
 * @brief Game::Game
 * @param client
 */

AbstractGame::AbstractGame(Client *client)
	: QObject{client}
	, m_client(client)
{
	Q_ASSERT(client);

	qCDebug(lcGame).noquote() << tr("Game created") << this;
}


/**
 * @brief Game::~Game
 */

AbstractGame::~AbstractGame()
{
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
 * @brief AbstractGame::load
 */

void AbstractGame::load()
{
	if (m_pageItem) {
		qCCritical(lcGame).noquote() << tr("A játék lapja már létezik!") << m_pageItem;
		return;
	}

	QQuickItem *page = m_client->stackPushPage("PageStart.qml");

	if (page) {
		connect(page, &QQuickItem::destroyed, this, &AbstractGame::onPageItemDestroyed);
		setPageItem(page);

		qCDebug(lcGame) << tr("Game page loaded");
	}
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


