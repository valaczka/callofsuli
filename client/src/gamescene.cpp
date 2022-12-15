/*
 * ---- Call of Suli ----
 *
 * gamescene.cpp
 *
 * Created on: 2022. 12. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameScene
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

#include "gamescene.h"
#include "actiongame.h"
#include "gameterrain.h"


Q_LOGGING_CATEGORY(lcScene, "app.scene")

/**
 * @brief GameScene::GameScene
 * @param parent
 */

GameScene::GameScene(QQuickItem *parent)
	: QQuickItem(parent)
{
	qCDebug(lcScene).noquote() << tr("Scene created") << this;

	setImplicitWidth(200);
	setImplicitHeight(200);
}


/**
 * @brief GameScene::~GameScene
 */

GameScene::~GameScene()
{
	qCDebug(lcScene).noquote() << tr("Scene destroyed") << this;
}


/**
 * @brief GameScene::game
 * @return
 */

ActionGame *GameScene::game() const
{
	return m_game;
}

void GameScene::setGame(ActionGame *newGame)
{
	if (m_game == newGame)
		return;
	m_game = newGame;
	emit gameChanged();
}


/**
 * @brief GameScene::loadTerrain
 * @param terrain
 */

void GameScene::loadTerrain(const QString &terrainName)
{
	if (!m_terrain.loadMap(terrainName)) {
		return;
	}

	if (!m_terrain.loadTiledLayers(this))
		return;

	setImplicitWidth(m_terrain.width());
	setImplicitHeight(m_terrain.height());

	qCDebug(lcScene).noquote() << "RESIZE" << m_terrain.width() << m_terrain.height();
}

