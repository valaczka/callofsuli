/*
 * ---- Call of Suli ----
 *
 * gameplayerposition.h
 *
 * Created on: 2022. 12. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GamePlayerPosition
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

#ifndef GAMEPLAYERPOSITION_H
#define GAMEPLAYERPOSITION_H

#include "gameobject.h"
#include "gameterrain.h"
#include <QObject>

class GamePlayerPosition : public GameObject
{
	Q_OBJECT

public:
	GamePlayerPosition(const GameTerrain::PlayerPositionData &data, QQuickItem *parent = nullptr);
	~GamePlayerPosition();

	const GameTerrain::PlayerPositionData &data() const;

private:
	const GameTerrain::PlayerPositionData m_data;
};

#endif // GAMEPLAYERPOSITION_H
