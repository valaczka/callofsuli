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
#include "qdiriterator.h"
#include "qtimer.h"
#include <QRandomGenerator>

GameEnemySoldier::GameEnemySoldier(QQuickItem *parent)
	: GameEnemy(parent)
{
	setHpProgressEnabled(true);

	connect(this, &GameEnemy::attack, this, &GameEnemySoldier::onAttack);
	connect(this, &GameEnemy::killed, this, &GameEnemySoldier::onKilled);
	connect(this, &GameEnemy::movingChanged, this, &GameEnemySoldier::onMovingChanged);
	connect(this, &GameEnemySoldier::atBoundChanged, this, &GameEnemySoldier::onAtBoundChanged);

	QTimer *timer = new QTimer(this);
	timer->setInterval(QRandomGenerator::global()->bounded(3000, 10000));
	connect(timer, &QTimer::timeout, this, &GameEnemy::attack);
	timer->start();

	connect(this, &GameEnemy::killed, timer, &QTimer::deleteLater);

	connect(this, &GameObject::timingTimerTimeout, this, &GameEnemySoldier::onMovingTimerTimeout);

	connect(this, &GameObject::sceneConnected, this, &GameEnemySoldier::onSceneConnected);

}


/**
 * @brief GameEnemy::~GameEnemy
 */

GameEnemySoldier::~GameEnemySoldier()
{

}


/**
 * @brief GameEnemySoldier::onAttack
 */

void GameEnemySoldier::onAttack()
{
	qCDebug(lcScene).noquote() << tr("Enemy soldier attack") << this;

	jumpToSprite("shot");
}


/**
 * @brief GameEnemySoldier::onKilled
 */

void GameEnemySoldier::onKilled()
{
	jumpToSprite("dead");
}




/**
 * @brief GameEnemySoldier::onMovingTimerTimeout
 */

void GameEnemySoldier::onMovingTimerTimeout()
{
	if (m_terrainEnemyData.type != GameTerrain::EnemySoldier) {
		qCWarning(lcScene) << tr("Invalid enemy type");
		return;
	}

	if (!hp())
		return;

	if (!game() || !game()->running() || !m_moving) {
		jumpToSprite("idle");
		return;
	}


	/*if (m_player || m_attackRunning || !m_isAlive)
		return;*/

	if (m_atBound) {
		m_turnElapsedMsec += m_scene->timingTimerTimeoutMsec();

		if (m_turnElapsedMsec >= m_msecBeforeTurn) {
			setFacingLeft(!facingLeft());
			setAtBound(false);
			m_turnElapsedMsec = -1;
		}
	} else {
		qreal posX = x();
		qreal delta = m_walkSize;

		if (facingLeft()) {
			if (posX-delta < m_terrainEnemyData.rect.left()) {
				setAtBound(true);
				m_turnElapsedMsec = 0;
			} else {
				setX(posX-delta);
			}
		} else {
			if (posX+delta > m_terrainEnemyData.rect.right() - width()) {
				setAtBound(true);
				m_turnElapsedMsec = 0;
			} else {
				setX(posX+delta);
			}
		}
	}
}

/**
 * @brief GameEnemySoldier::onMovingChanged
 */

void GameEnemySoldier::onMovingChanged()
{
	if (game() && game()->running() && m_moving)
		jumpToSprite("walk");
}


/**
 * @brief GameEnemySoldier::onAtBoundChanged
 */

void GameEnemySoldier::onAtBoundChanged()
{
	if (!game() || !game()->running()) {
		jumpToSprite("idle");
		return;
	}

	jumpToSprite(m_atBound ? "idle" : "walk");

}




int GameEnemySoldier::turnElapsedMsec() const
{
	return m_turnElapsedMsec;
}

void GameEnemySoldier::setTurnElapsedMsec(int newTurnElapsedMsec)
{
	if (m_turnElapsedMsec == newTurnElapsedMsec)
		return;
	m_turnElapsedMsec = newTurnElapsedMsec;
	emit turnElapsedMsecChanged();
}





/**
 * @brief GameEnemySoldier::create
 * @param scene
 * @param type
 * @return
 */

GameEnemySoldier *GameEnemySoldier::create(GameScene *scene, const GameTerrain::EnemyData &enemyData, const QString &type)
{
	qCDebug(lcScene).noquote() << tr("Create enemy soldier");

	GameEnemySoldier *soldier = qobject_cast<GameEnemySoldier*>(GameObject::createFromFile("GameEnemySoldier.qml", scene));

	if (!soldier) {
		qCCritical(lcScene).noquote() << tr("Enemy soldier creation error");
		return nullptr;
	}

	soldier->setParentItem(scene);
	soldier->setScene(scene);
	soldier->createSpriteItem();

	QDirIterator it(":/soldiers", {"data.json"}, QDir::Files, QDirIterator::Subdirectories);
	QStringList list;

	while (it.hasNext())
		list.append(it.next().section('/',-2,-2));

	if (list.isEmpty()) {
		qFatal("Enemy soldier directory is empty");
	}

	if (type.isEmpty()) {
		const QString &s = list.at(QRandomGenerator::global()->bounded(list.size()));
		soldier->setDataDir(QString(":/soldiers/%1").arg(s));
	} else if (list.contains(type)) {
		soldier->setDataDir(QString(":/soldiers/%1").arg(type));
	} else {
		qCWarning(lcScene).noquote() << tr("Invalid enemy soldier type:") << type;
		soldier->setDataDir(QString(":/soldiers/%1").arg(list.first()));
	}


	soldier->loadFromJsonFile();
	soldier->setTerrainEnemyData(enemyData);

	return soldier;
}


/**
 * @brief GameEnemySoldier::rayCastReport
 * @param items
 */

void GameEnemySoldier::rayCastReport(const QMultiMap<qreal, GameEntity *> &items)
{

}



/**
 * @brief GameEnemySoldier::onSceneConnected
 */

void GameEnemySoldier::onSceneConnected()
{
	connect(m_scene, &GameScene::showEnemiesChanged, this, [this](const bool &show){
		setGlowEnabled(show);
	});
}


/**
 * @brief GameEnemySoldier::atBound
 * @return
 */

bool GameEnemySoldier::atBound() const
{
	return m_atBound;
}

void GameEnemySoldier::setAtBound(bool newAtBound)
{
	if (m_atBound == newAtBound)
		return;
	m_atBound = newAtBound;
	emit atBoundChanged();
}

int GameEnemySoldier::msecBeforeTurn() const
{
	return m_msecBeforeTurn;
}

void GameEnemySoldier::setMsecBeforeTurn(int newMsecBeforeTurn)
{
	if (m_msecBeforeTurn == newMsecBeforeTurn)
		return;
	m_msecBeforeTurn = newMsecBeforeTurn;
	emit msecBeforeTurnChanged();
}
