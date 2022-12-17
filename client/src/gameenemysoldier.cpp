/*
 * ---- Call of Suli ----
 *
 * gameenemysoldier.cpp
 *
 * Created on: 2022. 12. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemySoldier
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

#include "gameenemysoldier.h"
#include "gamescene.h"

GameEnemySoldier::GameEnemySoldier(QQuickItem *parent)
	: GameEnemy(parent)
{
	qCDebug(lcScene).noquote() << tr("Enemy soldier created") << this;
}


/**
 * @brief GameEnemy::~GameEnemy
 */

GameEnemySoldier::~GameEnemySoldier()
{
	qCDebug(lcScene).noquote() << tr("Enemy soldier destroyed") << this;
}
