/*
 * ---- Call of Suli ----
 *
 * gameactivity.cpp
 *
 * Created on: 2020. 12. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameActivity
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "gameactivity.h"
#include "gamematch.h"
#include "cosgame.h"

GameActivity::GameActivity(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassInvalid, parent)
	, m_prepared(false)
	, m_game(nullptr)
{
	CosDb *db = new CosDb("objectiveDb", this);
	//db->setDatabaseName(Client::standardPath("tmptargets.db"));
	db->setDatabaseName(":memory:");
	addDb(db, true);
}



/**
 * @brief GameActivity::~GameActivity
 */

GameActivity::~GameActivity()
{

}


/**
 * @brief GameActivity::prepare
 */

void GameActivity::prepare()
{
	if (!db()->open()) {
		emit prepareFailed();
		return;
	}

	if (!m_game || !m_game->gameMatch()) {
		qWarning() << "Invalid game";
		emit prepareFailed();
		return;
	}

	run(&GameActivity::prepareDb, QVariantMap());
}


/**
 * @brief GameActivity::createTarget
 * @param block
 * @return
 */

void GameActivity::createTarget(GameEnemy *enemy)
{
	GameEnemyData *enemyData = enemy->enemyData();
	int block = enemy->block();

	GameBlock *b = enemyData->block();

	if (b) {
		// Questions factor
		qreal questions = 1.0;

		if (m_game && m_game->gameMatch() && m_game->gameMatch()->missionLevel()) {
			questions = m_game->gameMatch()->missionLevel()->questions();
		}

		int blockQuestions = 0;

		foreach (GameEnemyData *ed, b->enemies()) {
			if (ed->targetId() != -1 && !ed->targetDestroyed())
				blockQuestions++;
		}

		if (blockQuestions >= qFloor(b->enemies().count()*questions)) {
			return;
		}
	}



	QVariantMap m = db()->execSelectQueryOneRow("SELECT id, objective FROM targets "
												"WHERE block IS NULL "
												"ORDER BY num, RANDOM() LIMIT 1");

	if (m.isEmpty())
		return;

	int target = m.value("id").toInt();

	if (!db()->execSimpleQuery("UPDATE targets SET block=?, num=num+1 WHERE id=?", {block, target})) {
		Client::clientInstance()->sendMessageError(tr("Belső hiba"), tr("Ismeretlen adatbázis hiba!"));
	} else if (enemyData) {
		enemyData->setTargetId(target);
		enemyData->setTargetDestroyed(false);
		enemyData->setObjectiveUuid(m.value("objective").toString());
		enemy->setMaxHp(1);
		enemy->setHp(1);
	}
}




/**
 * @brief GameActivity::createTargets
 * @param enemies
 * @return
 */

bool GameActivity::createTargets(QVector<GameEnemy *> enemies)
{
	while (!enemies.isEmpty()) {
		GameEnemy *e = enemies.takeAt(QRandomGenerator::global()->bounded(enemies.size()));

		createTarget(e);

		QCoreApplication::processEvents();
	}

	return true;
}






/**
 * @brief GameActivity::onEnemyKilled
 */

void GameActivity::onEnemyKilled(GameEnemy *enemy)
{
	qDebug() << "ENEMY DIED" << enemy;

	createPickable(enemy);

	if (enemy && enemy->enemyData()) {
		if (m_game && m_game->gameMatch() && !enemy->xpGained())
			m_game->gameMatch()->addXP(0.5);

		int target = enemy->enemyData()->targetId();
		enemy->enemyData()->setTargetId(-1);
		enemy->enemyData()->setObjectiveUuid(QByteArray());
		enemy->enemyData()->setPickableType(GamePickable::PickableInvalid);
		enemy->enemyData()->setPickableData(QVariantMap());

		QVariantList l;
		l.append(target);
		db()->execSimpleQuery("UPDATE targets SET block=null WHERE id=?", l);
	}
}



/**
 * @brief GameActivity::onEnemyKillMissed
 * @param enemy
 */

void GameActivity::onEnemyKillMissed(GameEnemy *enemy)
{
	if (enemy && enemy->enemyData()) {
		GameEnemyData *data = enemy->enemyData();
		int target = data->targetId();
		data->setTargetId(-1);
		data->setObjectiveUuid(QByteArray());
		QVariantList l;
		l.append(target);
		db()->execSimpleQuery("UPDATE targets SET block=null WHERE id=?", l);

		createTarget(enemy);
	}
}



/**
 * @brief GameActivity::setPrepared
 * @param prepared
 */

void GameActivity::setPrepared(bool prepared)
{
	if (m_prepared == prepared)
		return;

	m_prepared = prepared;
	emit preparedChanged(m_prepared);
}

void GameActivity::setGame(CosGame *game)
{
	if (m_game == game)
		return;

	m_game = game;
	emit gameChanged(m_game);
}




/**
 * @brief GameActivity::prepareDb
 */

void GameActivity::prepareDb(QVariantMap)
{
	try {

		if (!db()->execSimpleQuery("CREATE TABLE targets("
								   "id INTEGER NOT NULL PRIMARY KEY, "
								   "objective TEXT NOT NULL, "
								   "block INTEGER,"
								   "num INTEGER NOT NULL DEFAULT 0)"))
			throw 1;


		GameMatch *match = m_game->gameMatch();
		GameMapMissionLevel *level = match->missionLevel();

		if (!level) {
			qWarning() << "Invalid mission level";
			throw 1;
		}

		foreach (GameMapChapter *chapter, level->chapters()) {
			foreach (GameMapObjective *objective, chapter->objectives()) {
				int iFrom = 0;
				int iTo = 1;

				if (objective->storageId() != -1) {
					iFrom = 1;
					iTo = objective->storageCount()+1;
				}

				for (int i=iFrom; i<iTo; ++i) {
					QVariantMap m;
					m["objective"] = objective->uuid();
					db()->execInsertQuery("INSERT INTO targets(?k?) VALUES (?)", m);

				}
			}
		}


	}  catch (...) {
		emit prepareFailed();
		return;
	}

	emit prepareSucceed();

	setPrepared(true);
}







/**
 * @brief GameActivity::createPickable
 * @param data
 */


void GameActivity::createPickable(GameEnemy *enemy)
{
	if (!m_game || !enemy->enemyData())
		return;

	GameEnemyData *enemyData = enemy->enemyData();

	if (enemyData->pickableType() == GamePickable::PickableInvalid)
		return;

	qreal x=0, y=0;
	QVariantMap d = enemyData->pickableData();

	if (enemy) {
		Entity *p = enemy->parentEntity();

		if (p) {
			x = p->x();
			y = p->y();
		}

		Box2DBox *box = enemy->boundBox();

		if (box) {
			x += box->x()+box->width()/2;
			y += box->y()+box->height();
		}
	}

	d["bottomPoint"] = QPointF(x,y);

	QQuickItem *item = nullptr;

	QMetaObject::invokeMethod(m_game->gameScene(), "createPickable", Qt::DirectConnection,
							  Q_RETURN_ARG(QQuickItem*, item),
							  Q_ARG(int, enemyData->pickableType()),
							  Q_ARG(QVariant, d)
							  );

}
