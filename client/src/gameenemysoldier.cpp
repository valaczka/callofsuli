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
#include "actiongame.h"
#include "gameplayer.h"
#include <QRandomGenerator>

GameEnemySoldier::GameEnemySoldier(QQuickItem *parent)
	: GameEnemy(parent)
{
	setHpProgressEnabled(true);

	connect(this, &GameEnemy::attack, this, &GameEnemySoldier::onAttack);
	connect(this, &GameEnemy::movingChanged, this, &GameEnemySoldier::onMovingChanged);
	connect(this, &GameObject::timingTimerTimeout, this, &GameEnemySoldier::onTimingTimerTimeout);
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
 * @brief GameEnemySoldier::onMovingTimerTimeout
 */

void GameEnemySoldier::onTimingTimerTimeout()
{
	if (m_terrainEnemyData.type != GameTerrain::EnemySoldier) {
		qCWarning(lcScene) << tr("Invalid enemy type");
		return;
	}

	if (m_enemyState == Dead || !isAlive())
		return;

	if (!game() || !game()->running() || !m_moving) {
		return;
	}


	if (m_enemyState == Idle) {
		m_turnElapsedMsec += m_scene->timingTimerTimeoutMsec();

		if (m_turnElapsedMsec >= m_msecBeforeTurn) {
			setFacingLeft(!facingLeft());
			setEnemyState(Move);
			m_turnElapsedMsec = -1;
		}
	} else if (m_enemyState == Move) {
		qreal posX = x();
		qreal delta = m_walkSize;

		body()->setBodyType(Box2DBody::Kinematic);

		if (facingLeft()) {
			if (posX-delta < m_terrainEnemyData.rect.left()) {
				setEnemyState(Idle);
				m_turnElapsedMsec = 0;
			} else {
				setX(posX-delta);
			}
		} else {
			if (posX+delta > m_terrainEnemyData.rect.right() - width()) {
				setEnemyState(Idle);
				m_turnElapsedMsec = 0;
			} else {
				setX(posX+delta);
			}
		}

		body()->setBodyType(Box2DBody::Dynamic);
	} else if (m_enemyState == WatchPlayer) {
		m_attackElapsedMsec += m_scene->timingTimerTimeoutMsec();

		setMsecLeftToAttack(qMax((int)m_msecBeforeAttack-m_attackElapsedMsec, 0));

		if (m_attackElapsedMsec >= m_msecBeforeAttack) {
			//setFacingLeft(!facingLeft());
			setEnemyState(Attack);
			attackPlayer();
			m_attackElapsedMsec = 0;
		}
	} else if (m_enemyState == Attack) {
		m_attackElapsedMsec += m_scene->timingTimerTimeoutMsec();

		if (m_attackElapsedMsec >= m_msecBetweenAttack) {
			//setFacingLeft(!facingLeft());
			attackPlayer();
			m_attackElapsedMsec = 0;
		}
	}
}

/**
 * @brief GameEnemySoldier::onMovingChanged
 */

void GameEnemySoldier::onMovingChanged()
{
	if (game() && game()->running() && m_moving)
		setEnemyState(Move);
}







/**
 * @brief GameEnemySoldier::turnElapsedMsec
 * @return
 */

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
 * @brief GameEnemySoldier::attackPlayer
 */

void GameEnemySoldier::attackPlayer()
{
	emit attack();

	jumpToSprite("shot");

	if (player() && player()->isAlive())
		player()->hurtByEnemy(this, true);
}


/**
 * @brief GameEnemySoldier::rayCastReport
 * @param items
 */

void GameEnemySoldier::rayCastReport(const QMultiMap<qreal, GameEntity *> &items)
{
	GamePlayer *player = nullptr;

	foreach(GameEntity *item, items) {
		GamePlayer *e = qobject_cast<GamePlayer *>(item);

		if (e && e->isAlive()) {
			player = e;
			break;
		}
	}

	setPlayer(player);

	if ((m_enemyState == Attack || m_enemyState == WatchPlayer) && !player)
		setEnemyState(Move);
	else if (player && m_enemyState != Attack && m_enemyState != WatchPlayer)
		setEnemyState(WatchPlayer);

}


/**
 * @brief GameEnemySoldier::enemyStateModified
 * @param newEnemyState
 */

void GameEnemySoldier::enemyStateModified()
{
	switch (m_enemyState) {
	case Invalid:
	case Idle:
		jumpToSprite("idle");
		break;
	case Move:
		jumpToSprite("walk");
		break;
	case WatchPlayer:
		m_attackElapsedMsec = 0;
		setMsecLeftToAttack(m_msecBeforeAttack);
		jumpToSprite("idle");
		break;

	case Attack:
		setMsecLeftToAttack(0);
		break;

	case Dead:
		jumpToSprite("idle");
		jumpToSprite("dead");
		break;
	}
}



/**
 * @brief GameEnemySoldier::onSceneConnected
 */

void GameEnemySoldier::onSceneConnected()
{
	const QJsonObject &data = m_scene->levelData().value("enemy").toObject().value("soldier").toObject();

	setRayCastElevation(data.value("rayCastElevation").toDouble());
	setRayCastLength(data.value("rayCastLength").toDouble());
	setMsecBeforeTurn(data.value("msecBeforeTurn").toDouble());
	setCastAttackFraction(data.value("castAttackFraction").toDouble());
	setMsecBeforeAttack(data.value("msecBeforeAttack").toDouble());
	setMsecBetweenAttack(data.value("msecBetweenAttack").toDouble());
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
