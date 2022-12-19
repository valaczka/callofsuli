/*
 * ---- Call of Suli ----
 *
 * gameenemy.cpp
 *
 * Created on: 2022. 12. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemy
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

#include "gameenemy.h"
#include "desktopclient.h"
#include "gamescene.h"
#include "application.h"

#include <qtimer.h>

GameEnemy::GameEnemy(QQuickItem *parent)
	: GameEntity(parent)
{
	m_defaultShotSound = "qrc:/sound/sfx/enemyshot.wav";

	setCategoryFixture(CATEGORY_ENEMY);
	setCategoryRayCast(CATEGORY_PLAYER);
	setRayCastEnabled(true);

	connect(this, &GameObject::sceneConnected, this, &GameEnemy::onSceneConnected);

#ifndef Q_OS_WASM
	DesktopClient *client = qobject_cast<DesktopClient*>(Application::instance()->client());

	if (client) {
		m_soundEffect = client->newSoundEffect();
		m_soundEffect->setSource(shotSound());
		connect(this, &GameEnemy::attack, m_soundEffect, &QSoundEffect::play);
		connect(this, &GameEnemy::shotSoundChanged, m_soundEffect, &QSoundEffect::setSource);
	}
#endif
}


/**
 * @brief GameEnemy::~GameEnemy
 */

GameEnemy::~GameEnemy()
{
#ifndef Q_OS_WASM
	if (m_soundEffect)
		m_soundEffect->deleteLater();
#endif
}


/**
 * @brief GameEnemy::onSceneChanged
 */

void GameEnemy::onSceneConnected()
{
	connect(m_scene, &GameScene::zoomOverviewChanged, this, &GameEntity::setOverlayEnabled);
}




bool GameEnemy::moving() const
{
	return m_moving;
}

void GameEnemy::setMoving(bool newMoving)
{
	if (m_moving == newMoving)
		return;
	m_moving = newMoving;
	emit movingChanged();
}


/**
 * @brief GameEnemy::startMovingAfter
 * @param msec
 */

void GameEnemy::startMovingAfter(const int &msec)
{
	QTimer::singleShot(msec, this, [this]() { setMoving(true); });
}


/**
 * @brief GameEnemy::setTerrainEnemyData
 * @param newTerrainEnemyData
 */

void GameEnemy::setTerrainEnemyData(const GameTerrain::EnemyData &newTerrainEnemyData)
{
	m_terrainEnemyData = newTerrainEnemyData;
}


/**
 * @brief GameEnemy::terrainEnemyData
 * @return
 */

const GameTerrain::EnemyData &GameEnemy::terrainEnemyData() const
{
	return m_terrainEnemyData;
}

