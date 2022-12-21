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

AbstractGame::AbstractGame(const Mode &mode, Client *client)
	: QObject{client}
	, m_client(client)
	, m_mode(mode)
{
	Q_ASSERT(client);

	qCDebug(lcGame).noquote() << tr("Game created") << this;

	m_backgroundImage = "qrc:/internal/game/bg.png";
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
 * @brief AbstractGame::closeGame
 */

void AbstractGame::finishGame()
{
	qCDebug(lcGame).noquote() << tr("Finish game") << this;
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



const QString &AbstractGame::name() const
{
	return m_name;
}

void AbstractGame::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}


/**
 * @brief AbstractGame::backgroundImage
 * @return
 */

const QString &AbstractGame::backgroundImage() const
{
	/*if (m_bgImage.isEmpty() || m_imageDbName.isEmpty())
					return "qrc:/internal/game/bg.png";
			else if (m_bgImage.startsWith("qrc:/"))
					return m_bgImage;
			else
					return "image://"+m_imageDbName+"/"+m_bgImage;*/

	return m_backgroundImage;
}
