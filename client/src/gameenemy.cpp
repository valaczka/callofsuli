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
#include "gamescene.h"
#include "application.h"
#include "gameplayer.h"
#include <qtimer.h>

#ifndef Q_OS_WASM
#include "desktopclient.h"
#endif

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

	connect(this, &GamePlayer::allHpLost, this, [this](){
		emit killed(this);
	});

	connect(this, &GameEnemy::isAliveChanged, this, [this](){
		if (!isAlive())
			setEnemyState(Dead);
	});
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

const GamePickable::GamePickableData &GameEnemy::pickable() const
{
	return m_pickable;
}

void GameEnemy::setPickable(const GamePickable::GamePickableData &newPickable)
{
	m_pickable = newPickable;
	emit pickableChanged();
}


/**
 * @brief GameEnemy::question
 * @return
 */

ActionGame::QuestionLocation *GameEnemy::question() const
{
	return m_question;
}

void GameEnemy::setQuestion(ActionGame::QuestionLocation *newQuestion)
{
	if (m_question == newQuestion)
		return;
	m_question = newQuestion;
	emit questionChanged();
}


/**
 * @brief GameEnemy::enemyState
 * @return
 */

const GameEnemy::EnemyState &GameEnemy::enemyState() const
{
	return m_enemyState;
}

void GameEnemy::setEnemyState(const EnemyState &newEnemyState)
{
	if (m_enemyState == newEnemyState)
		return;
	m_enemyState = newEnemyState;
	emit enemyStateChanged();

	enemyStateModified();
}


/**
 * @brief GameEnemy::msecBetweenAttack
 * @return
 */

qreal GameEnemy::msecBetweenAttack() const
{
	return m_msecBetweenAttack;
}

void GameEnemy::setMsecBetweenAttack(qreal newMsecBetweenAttack)
{
	if (qFuzzyCompare(m_msecBetweenAttack, newMsecBetweenAttack))
		return;
	m_msecBetweenAttack = newMsecBetweenAttack;
	emit msecBetweenAttackChanged();
}

qreal GameEnemy::msecBeforeAttack() const
{
	return m_msecBeforeAttack;
}

void GameEnemy::setMsecBeforeAttack(qreal newMsecBeforeAttack)
{
	if (qFuzzyCompare(m_msecBeforeAttack, newMsecBeforeAttack))
		return;
	m_msecBeforeAttack = newMsecBeforeAttack;
	emit msecBeforeAttackChanged();
}

qreal GameEnemy::castAttackFraction() const
{
	return m_castAttackFraction;
}

void GameEnemy::setCastAttackFraction(qreal newCastAttackFraction)
{
	if (qFuzzyCompare(m_castAttackFraction, newCastAttackFraction))
		return;
	m_castAttackFraction = newCastAttackFraction;
	emit castAttackFractionChanged();
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


bool GameEnemy::aimedByPlayer() const
{
	return m_aimedByPlayer;
}

void GameEnemy::setAimedByPlayer(bool newAimedByPlayer)
{
	if (m_aimedByPlayer == newAimedByPlayer)
		return;
	m_aimedByPlayer = newAimedByPlayer;
	emit aimedByPlayerChanged();
}


/**
 * @brief GameEnemy::player
 * @return
 */

GamePlayer *GameEnemy::player() const
{
	return qobject_cast<GamePlayer*>(m_player);
}

void GameEnemy::setPlayer(GamePlayer *newPlayer)
{
	if (m_player == newPlayer)
		return;
	m_player = newPlayer;
	emit playerChanged();
}

qreal GameEnemy::msecLeftToAttack() const
{
	return m_msecLeftToAttack;
}

void GameEnemy::setMsecLeftToAttack(qreal newMsecLeftToAttack)
{
	if (qFuzzyCompare(m_msecLeftToAttack, newMsecLeftToAttack))
		return;
	m_msecLeftToAttack = newMsecLeftToAttack;
	emit msecLeftToAttackChanged();
}




/**
 * @brief GameEnemy::attackByPlayer
 * @param player
 * @param questionEmpty
 */

void GameEnemy::attackByPlayer(GamePlayer *player, const bool &questionEmpty)
{
	decreaseHp();

	/*const qreal &playerX = player->x();
	const qreal &meX = x();

	if (m_enemyData->enemyType() != GameEnemyData::EnemySniper && (!isEmptyQuestion || !isAlive() || m_cosGame->gameMatch()->level() > 2))
			{
					if (playerX <= meX && !facingLeft)
							parentEntity()->setProperty("facingLeft", true);
					else if (playerX > meX && facingLeft)
							parentEntity()->setProperty("facingLeft", false);
			}*/

	if (isAlive())
		return;

	setAimedByPlayer(false);

	//setEnemyState(Dead);
}


/**
 * @brief GameEnemy::missedByPlayer
 * @param player
 */

void GameEnemy::missedByPlayer(GamePlayer *player)
{
	qCDebug(lcScene).noquote() << tr("Missed by player:") << this;

	/*qreal playerX = player->parentEntity()->x();
			qreal meX = parentEntity()->x();
			bool facingLeft = parentEntity()->property("facingLeft").toBool();

			if (playerX <= meX && !facingLeft)
					parentEntity()->setProperty("facingLeft", true);
			else if (playerX > meX && facingLeft)
					parentEntity()->setProperty("facingLeft", false);*/

	emit killMissed();

	player->hurtByEnemy(this, false);
}
