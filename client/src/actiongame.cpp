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

#include "gameenemysoldier.h"
#include "gameplayer.h"
#include "gameenemy.h"
#include "actiongame.h"
#include "client.h"
#include <QRandomGenerator>
#include <QtMath>

ActionGame::ActionGame(GameMapMissionLevel *missionLevel, Client *client)
	: AbstractLevelGame(Action, missionLevel, client)
{
	qCDebug(lcGame).noquote() << tr("Action game constructed") << this;
}


/**
 * @brief ActionGame::~ActionGame
 */

ActionGame::~ActionGame()
{
	qDeleteAll(m_questions);
	qDeleteAll(m_enemies);
	qCDebug(lcGame).noquote() << tr("Action game destroyed") << this;
}


/**
 * @brief ActionGame::createQuestions
 */

void ActionGame::createQuestions()
{
	qDeleteAll(m_questions);

	QVector<Question> list = AbstractLevelGame::createQuestions();

	foreach (const Question &q, list) {
		QuestionLocation *ql = new QuestionLocation();
		ql->setQuestion(q);
		ql->setEnemy(nullptr);
		ql->setUsed(0);

		m_questions.append(ql);
	}
}


/**
 * @brief ActionGame::createLocations
 */

void ActionGame::createEnemyLocations()
{
	qDeleteAll(m_enemies);

	if (!m_scene) {
		qCWarning(lcGame).noquote() << tr("Missing scene, don't created any location");
		return;
	}

	foreach (const GameTerrain::EnemyData &e, m_scene->terrain().enemies()) {
		EnemyLocation *l = new EnemyLocation(e);

		m_enemies.append(l);

		/*GameEnemySoldier *soldier = GameEnemySoldier::create(this, e);

		soldier->setFacingLeft(QRandomGenerator::global()->generate() % 2);

		soldier->setX(e.rect.left() + e.rect.width()/2);
		soldier->setY(e.rect.bottom()-soldier->height());

		soldier->setMaxHp(QRandomGenerator::global()->bounded(1, 5));
		soldier->setHp(QRandomGenerator::global()->bounded(1, 5));

		addChildItem(soldier);

		soldier->startMovingAfter(2500);
		QCoreApplication::processEvents();*/
	}

	qCDebug(lcGame).noquote() << tr("%1 enemy places created").arg(m_enemies.size());

	emit activeEnemiesChanged();
}


/**
 * @brief ActionGame::createFixEnemies
 */

void ActionGame::createFixEnemies()
{
	if (!m_scene) {
		qCWarning(lcGame).noquote() << tr("Missing scene");
		return;
	}

	qCDebug(lcGame).noquote() << tr("Create fix enemies");

	int n = 0;

	foreach (EnemyLocation *el, m_enemies) {
		const GameTerrain::EnemyData &e = el->enemyData();
		const GameTerrain::EnemyType &type = el->enemyData().type;

		if (el->enemy())
			continue;

		if (type == GameTerrain::EnemyInvalid || type == GameTerrain::EnemySoldier)
			continue;

		/*GameEnemySoldier *soldier = GameEnemySoldier::create(m_scene, e);

		soldier->setFacingLeft(QRandomGenerator::global()->generate() % 2);

		soldier->setX(e.rect.left() + e.rect.width()/2);
		soldier->setY(e.rect.bottom()-soldier->height());

		soldier->setMaxHp(QRandomGenerator::global()->bounded(1, 5));
		soldier->setHp(QRandomGenerator::global()->bounded(1, 5));

		m_scene->addChildItem(soldier);

		soldier->startMovingAfter(2500);
		QCoreApplication::processEvents();

		el->setenemy(soldier);
*/

		++n;
	}

	qCDebug(lcGame).noquote() << tr("%1 fix enemies created").arg(n);

	emit activeEnemiesChanged();
}


/**
 * @brief ActionGame::recreateEnemies
 */

void ActionGame::recreateEnemies()
{
	if (!m_scene) {
		qCWarning(lcGame).noquote() << tr("Missing scene");
		return;
	}

	qCDebug(lcGame).noquote() << tr("Recreate enemies");

	QList<GameEnemy *> soldiers;

	foreach (EnemyLocation *el, m_enemies) {
		const GameTerrain::EnemyData &e = el->enemyData();
		const GameTerrain::EnemyType &type = el->enemyData().type;

		if (el->enemy())
			continue;

		if (type != GameTerrain::EnemySoldier || m_closedBlocks.contains(e.block))
			continue;

		GameEnemySoldier *soldier = GameEnemySoldier::create(m_scene, e);

		soldier->setFacingLeft(QRandomGenerator::global()->generate() % 2);

		soldier->setX(e.rect.left() + e.rect.width()/2);
		soldier->setY(e.rect.bottom()-soldier->height());

		m_scene->addChildItem(soldier);

		soldier->startMovingAfter(2500);

		el->setEnemy(soldier);

		connect(soldier, &GameEntity::killed, this, &ActionGame::onEnemyDied);

		QCoreApplication::processEvents();

		soldiers.append(soldier);
	}

	qCDebug(lcGame).noquote() << tr("%1 new enemies created").arg(soldiers.size());

	linkQuestionToEnemies(soldiers);
	linkPickablesToEnemies(soldiers);



	qCDebug(lcGame).noquote() << "ENEMIES -----------------------------";

	foreach (EnemyLocation *el, m_enemies) {
		qCDebug(lcGame).noquote() << (el->enemy() ? QString("  * [%1]").arg(el->enemy()->hp()) : "  - [ ]") <<
									 QString("(%1)").arg(el->enemyData().block) <<
									 (el->enemy() && el->enemy()->question() ? el->enemy()->question()->question().uuid() : "") <<
									 (el->enemy() && el->enemy()->hasPickable() ? ("("+el->enemy()->pickable().id+")") : "");
	}

	qCDebug(lcGame).noquote() << "END ENEMIES -------------------------";

	emit activeEnemiesChanged();
}



/**
 * @brief ActionGame::createInventory
 */

void ActionGame::createInventory()
{
	m_inventory.clear();

	if (!m_missionLevel) {
		qCWarning(lcGame).noquote() << tr("Missing game map, don't created any pickable");
		return;
	}

	qCDebug(lcGame).noquote() << tr("Create inventory");

	const QHash<QString, GamePickable::GamePickableData> &list = GamePickable::pickableDataHash();

	foreach (GameMapInventory *inventory, m_missionLevel->inventories()) {
		if (!list.contains(inventory->module())) {
			qCWarning(lcGame).noquote() << tr("Invalid pickable module:") << inventory->module();
			continue;
		}

		Inventory inv(list.value(inventory->module()), inventory->block());
		m_inventory.append(inv);
	}

	qCDebug(lcGame).noquote() << tr("Inventory loaded: %1 items").arg(m_inventory.size());

	foreach (const Inventory &inv, m_inventory) {
		qCDebug(lcGame).noquote() << tr("- %1 (block: %2)").arg(inv.first.name).arg(inv.second);
	}
}


/**
 * @brief ActionGame::linkQuestionToEnemies
 * @param enemies
 */

void ActionGame::linkQuestionToEnemies(QList<GameEnemy *> enemies)
{
	qCDebug(lcGame).noquote() << tr("Link questions to enemies");

	typedef QVector<QuestionLocation *> QVL;

	QMap<int, QVL> usedList;

	foreach (QuestionLocation *ql, m_questions) {
		if (!ql->enemy()) {
			const int &used = ql->used();
			if (usedList.contains(used)) {
				usedList[used].append(ql);
			} else {
				usedList.insert(used, {ql});
			}
		}
	}

	QList<GameEnemy *> notUsed = enemies;

	for (auto it = usedList.begin(); it != usedList.end(); ++it) {
		qCDebug(lcGame).noquote() << tr("Link questions used %1 times").arg(it.key()) << it.value().size();

		while (!it.value().isEmpty() && !enemies.isEmpty()) {
			QuestionLocation *ql = it.value().takeAt(QRandomGenerator::global()->bounded(it.value().size()));

			while (!enemies.isEmpty()) {
				GameEnemy *e = enemies.takeAt(QRandomGenerator::global()->bounded(enemies.size()));

				const qreal &block = e->terrainEnemyData().block;

				qreal blockMax = 0;
				int blockQuestions = 0;

				foreach (EnemyLocation *el, m_enemies) {
					if (el->enemyData().block != block) {
						continue;
					}

					++blockMax;

					if (el->enemy()->question())
						++blockQuestions;
				}

				if (blockQuestions >= qFloor(blockMax * m_missionLevel->questions())) {
					qCDebug(lcGame).noquote() << tr("Block %1 oversize, skip.").arg(block);
					continue;
				}


				ql->setEnemy(e);
				ql->setUsed(ql->used()+1);
				e->setQuestion(ql);
				e->setMaxHp(1);
				e->setHp(1);
				notUsed.removeAll(e);

				break;
			}
		}

		if (enemies.isEmpty())
			break;
	}


	// Set HP

	const QString &levelKey = QString::number((int)m_missionLevel->level());

	foreach (GameEnemy *enemy, notUsed) {
		int hp = enemy->dataObject().value("hp").toObject().value(levelKey).toInt(3);

		enemy->setMaxHp(hp);
		enemy->setHp(hp);
	}

}


/**
 * @brief ActionGame::linkPickablesToEnemies
 * @param enemies
 */

void ActionGame::linkPickablesToEnemies(QList<GameEnemy *> enemies)
{
	qCDebug(lcGame).noquote() << tr("Link pickables to enemies:") << enemies.size();

	QVector<int> eBlocks;

	foreach (EnemyLocation *el, m_enemies) {
		if (!el->enemy())
			continue;

		int b = el->enemyData().block;

		if (!eBlocks.contains(b))
			eBlocks.append(b);
	}

	int n = 0;

	foreach (const int &block, eBlocks) {
		QVector<GameEnemy *> elist;

		foreach (GameEnemy *e, enemies) {
			if (!e->hasPickable() && e->terrainEnemyData().block == block)
				elist.append(e);
		}

		while (!elist.isEmpty()) {
			QVector<int> idx;
			for (int i=0; i<m_inventory.size(); ++i) {
				if (m_inventory.at(i).second == block)
					idx.append(i);
			}

			if (idx.isEmpty())
				break;

			int x = idx.takeAt(QRandomGenerator::global()->bounded(idx.size()));
			GameEnemy *e = elist.takeAt(QRandomGenerator::global()->bounded(elist.size()));
			enemies.removeAll(e);

			Inventory data = m_inventory.takeAt(x);

			e->setPickable(data.first);
			++n;
		}
	}


	// To everywhere

	QVector<GameEnemy *> elist;

	foreach (GameEnemy *e, enemies) {
		if (!e->hasPickable())
			elist.append(e);
	}

	while (!elist.isEmpty() && !m_inventory.isEmpty()) {
		Inventory data = m_inventory.takeAt(QRandomGenerator::global()->bounded(m_inventory.size()));
		GameEnemy *e = elist.takeAt(QRandomGenerator::global()->bounded(elist.size()));

		e->setPickable(data.first);
		++n;
	}

	qCDebug(lcGame).noquote() << tr("%1 pickables linked").arg(n);
}


/**
 * @brief ActionGame::tryAttack
 * @param player
 * @param enemy
 */

void ActionGame::tryAttack(GamePlayer *player, GameEnemy *enemy)
{
	if (!player) {
		qCWarning(lcGame).noquote() << tr("Invalid player to try attack");
		return;
	}

	if (!enemy) {
		qCWarning(lcGame).noquote() << tr("Invalid enemy to try attack");
		return;
	}

	qCDebug(lcGame).noquote() << tr("Try attack");

	enemy->decreaseHp();

	/*if (m_question) {
					qWarning() << "Question already exists";
					return;
			}

			m_question = new GameQuestion(this, player, enemy, this);

			if (m_gameMatch)
					connect(m_question, &GameQuestion::xpGained, m_gameMatch, &GameMatch::addXP);

			connect(m_question, &GameQuestion::finished, this, [=]() {
					m_question->deleteLater();
					m_question = nullptr;
					emit questionChanged(nullptr);
			});

			emit questionChanged(m_question);

			m_question->run();*/

}


/**
 * @brief ActionGame::player
 * @return
 */

GamePlayer *ActionGame::player() const
{
	return qobject_cast<GamePlayer*>(m_player);
}


/**
 * @brief ActionGame::setPlayer
 * @param newPlayer
 */

void ActionGame::setPlayer(GamePlayer *newPlayer)
{
	if (m_player == newPlayer)
		return;
	m_player = newPlayer;
	emit playerChanged();

	if (!m_player)
		return;

	qCDebug(lcGame).noquote() << tr("Setup new player");

	m_player->setMaxHp(startHP());
	m_player->setHp(startHP());
	connect(m_player, &GamePlayer::died, this, &ActionGame::onPlayerDied);

}




/**
 * @brief ActionGame::loadPage
 * @return
 */

QQuickItem *ActionGame::loadPage()
{
	QQuickItem *page = m_client->stackPushPage("PageActionGame.qml", QVariantMap({
																					 { "game", QVariant::fromValue(this) }
																				 }));

	const QVariant &scene = page->property("scene");

	setScene(qvariant_cast<GameScene*>(scene));
	return page;
}


/**
 * @brief ActionGame::closedBlocks
 * @return
 */

const QVector<int> &ActionGame::closedBlocks() const
{
	return m_closedBlocks;
}



GameScene *ActionGame::scene() const
{
	return m_scene;
}

void ActionGame::setScene(GameScene *newScene)
{
	if (m_scene == newScene)
		return;
	m_scene = newScene;
	emit sceneChanged();
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


/**
 * @brief ActionGame::EnemyLocation::~EnemyLocation
 */

ActionGame::EnemyLocation::EnemyLocation(const GameTerrain::EnemyData &enemyData)
	: m_enemyData(enemyData)
{

}



ActionGame::EnemyLocation::~EnemyLocation()
{
	if (m_enemy)
		m_enemy->deleteLater();
}



const GameTerrain::EnemyData &ActionGame::EnemyLocation::enemyData() const
{
	return m_enemyData;
}

void ActionGame::EnemyLocation::setEnemyData(const GameTerrain::EnemyData &newEnemyData)
{
	m_enemyData = newEnemyData;
}

GameEnemy *ActionGame::EnemyLocation::enemy() const
{
	return qobject_cast<GameEnemy*>(m_enemy);
}

void ActionGame::EnemyLocation::setEnemy(GameEnemy *newEnemy)
{
	m_enemy = newEnemy;
}


/**
 * @brief ActionGame::activeEnemies
 * @return
 */

int ActionGame::activeEnemies() const
{
	int n = 0;

	foreach (EnemyLocation *e, m_enemies) {
		if (e->enemy())
			++n;
	}

	return n;
}


/**
 * @brief ActionGame::onPlayerDied
 */

void ActionGame::onPlayerDied(GameEntity *)
{
	qCDebug(lcGame).noquote() << tr("Player died");
	setPlayer(nullptr);

	if (m_deathmatch) {
		emit missionFailed();
		qDebug() << "!!!!!";
		return;
	}

	recreateEnemies();

	m_scene->createPlayer();
}




/**
 * @brief ActionGame::onEnemyDied
 */

void ActionGame::onEnemyDied(GameEntity *entity)
{
	GameEnemy *enemy = qobject_cast<GameEnemy*>(entity);

	if (!enemy) {
		qCCritical(lcGame).noquote() << tr("Invalid enemy entity:") << entity;
		return;
	}

	int block = -1;

	foreach (EnemyLocation *el, m_enemies) {
		if (el->enemy() == enemy) {
			block = el->enemyData().block;
			el->setEnemy(nullptr);
			break;
		}
	}

	emit activeEnemiesChanged();

	if (!activeEnemies()) {
		emit missionCompleted();
		qCDebug(lcGame).noquote() << tr("Mission completed");
		return;
	}

	// Close blocks

	foreach (EnemyLocation *el, m_enemies) {
		if (el->enemyData().block == block && el->enemy()) {
			return;
		}
	}

	if (m_closedBlocks.contains(block))
		return;

	m_closedBlocks.append(block);

	qCDebug(lcGame).noquote() << tr("Block closed:") << block;

	if (m_scene)
		m_scene->activateLaddersInBlock(block);

}



/**
 * @brief ActionGame::QuestionLocation::question
 * @return
 */


const Question &ActionGame::QuestionLocation::question() const
{
	return m_question;
}

void ActionGame::QuestionLocation::setQuestion(const Question &newQuestion)
{
	m_question = newQuestion;
}


int ActionGame::QuestionLocation::used() const
{
	return m_used;
}

void ActionGame::QuestionLocation::setUsed(int newUsed)
{
	m_used = newUsed;
}

GameEnemy *ActionGame::QuestionLocation::enemy() const
{
	return qobject_cast<GameEnemy*>(m_enemy);
}

void ActionGame::QuestionLocation::setEnemy(GameEnemy *newEnemy)
{
	m_enemy = newEnemy;
}
