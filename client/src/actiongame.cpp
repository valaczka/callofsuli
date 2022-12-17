/*
 * ---- Call of Suli ----
 *
 * actiongame.cpp
 *
 * Created on: 2022. 12. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ActionGame
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

#include "actiongame.h"
#include "client.h"

ActionGame::ActionGame(Client *client)
	: AbstractGame(Action, client)
{
	qCDebug(lcGame).noquote() << tr("Action game constructed") << this;
}


/**
 * @brief ActionGame::~ActionGame
 */

ActionGame::~ActionGame()
{
	qCDebug(lcGame).noquote() << tr("Action game destroyed") << this;
}


/**
 * @brief ActionGame::player
 * @return
 */

QQuickItem *ActionGame::player() const
{
	return m_player;
}

void ActionGame::setPlayer(QQuickItem *newPlayer)
{
	if (m_player == newPlayer)
		return;
	m_player = newPlayer;
	emit playerChanged();
}




/**
 * @brief ActionGame::loadPage
 * @return
 */

QQuickItem *ActionGame::loadPage()
{
	return m_client->stackPushPage("PageActionGame.qml", QVariantMap({
																		 { "game", QVariant::fromValue(this) }
																	 }));
}

bool ActionGame::running() const
{
	return m_running;
}

void ActionGame::setRunning(bool newRunning)
{
	if (m_running == newRunning)
		return;
	m_running = newRunning;
	emit runningChanged();
}


