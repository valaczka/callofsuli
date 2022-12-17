/*
 * ---- Call of Suli ----
 *
 * gameladder.cpp
 *
 * Created on: 2022. 12. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameLadder
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

#include "gameladder.h"



GameLadder::GameLadder(QQuickItem *parent)
	: GameObject(parent)
{

}

GameLadder::~GameLadder()
{

}




/**
 * @brief GameLadder::boundRect
 * @return
 */

const QRectF &GameLadder::boundRect() const
{
	return m_boundRect;
}

void GameLadder::setBoundRect(const QRectF &newBoundRect)
{
	if (m_boundRect == newBoundRect)
		return;
	m_boundRect = newBoundRect;
	emit boundRectChanged();
}

bool GameLadder::active() const
{
	return m_active;
}

void GameLadder::setActive(bool newActive)
{
	if (m_active == newActive)
		return;
	m_active = newActive;
	emit activeChanged();
}

int GameLadder::blockTop() const
{
	return m_blockTop;
}

void GameLadder::setBlockTop(int newBlockTop)
{
	if (m_blockTop == newBlockTop)
		return;
	m_blockTop = newBlockTop;
	emit blockTopChanged();
}

int GameLadder::blockBottom() const
{
	return m_blockBottom;
}

void GameLadder::setBlockBottom(int newBlockBottom)
{
	if (m_blockBottom == newBlockBottom)
		return;
	m_blockBottom = newBlockBottom;
	emit blockBottomChanged();
}


