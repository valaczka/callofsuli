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
	QString dbFile = Client::standardPath("tmptargets.db");

	if (QFile::exists(dbFile)) {
		qDebug() << tr("A fájl (%1) létezik, törlöm").arg(dbFile);
		QFile::remove(dbFile);
	}

	CosDb *db = new CosDb("objectiveDb", this);

	db->setDatabaseName(dbFile);

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

	if (!m_client || !m_game || !m_game->gameMatch()) {
		qWarning() << "Invalid client or game";
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

bool GameActivity::createTarget(GameEnemyData *enemyData, const int &block)
{
	QHash<int, QPair<int, int>> mapAllocations;

	QVariantList maplist = db()->execSelectQuery("SELECT targets.map as map, count(*) as count, maxObjective FROM targets "
												 "LEFT JOIN maps ON (maps.map=targets.map) "
												 "WHERE block IS NOT NULL GROUP by targets.map");

	foreach (QVariant v, maplist) {
		QVariantMap m = v.toMap();
		mapAllocations[m.value("map").toInt()] = qMakePair(m.value("maxObjective").toInt(), m.value("count").toInt());
	}

	qDebug() << "Allocations" << mapAllocations;

	QVariantList l;
	l.append(block);

	QVariantList list = db()->execSelectQuery("SELECT target, map, objective FROM positions "
											  "LEFT JOIN targets ON (targets.id=positions.target) "
											  "WHERE (positions.block=? OR positions.block IS NULL) AND targets.block IS NULL "
											  "ORDER BY num, RANDOM()", l);

	foreach (QVariant v, list) {
		QVariantMap m = v.toMap();

		int map = m.value("map").toInt();
		if (mapAllocations.contains(map)) {
			int maxObj = mapAllocations.value(map).first;
			int current = mapAllocations.value(map).second;

			++current;

			if (maxObj > 0 && current > maxObj) {
				continue;
			}

			mapAllocations[map] = qMakePair(maxObj, current);
		}

		int target = m.value("target").toInt();

		QVariantList ll;
		ll.append(block);
		ll.append(target);

		if (!db()->execSimpleQuery("UPDATE targets SET block=?, num=num+1 WHERE id=?", ll)) {
			m_client->sendMessageError(tr("Belső hiba"), tr("Ismeretlen adatbázis hiba!"));
		} else if (enemyData) {
			enemyData->setTargetId(target);
			enemyData->setObjectiveUuid(m.value("objective").toByteArray());
			return true;
		}
	}

	return false;
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

		if (!createTarget(e->enemyData(), e->block()))
			return false;
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
		int target = enemy->enemyData()->targetId();
		enemy->enemyData()->setTargetId(-1);
		enemy->enemyData()->setObjectiveUuid(QByteArray());
		enemy->enemyData()->setPickableType(GameEnemyData::PickableInvalid);
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

		createTarget(data, enemy->block());
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
		if (!db()->execSimpleQuery("CREATE TABLE maps("
								   "map INTEGER NOT NULL PRIMARY KEY, "
								   "maxObjective INTEGER NOT NULL DEFAULT 0)"))
			throw 1;

		if (!db()->execSimpleQuery("CREATE TABLE targets("
								   "id INTEGER NOT NULL PRIMARY KEY, "
								   "map INTEGER NOT NULL, "
								   "objective TEXT NOT NULL, "
								   "block INTEGER,"
								   "num INTEGER NOT NULL DEFAULT 0)"))
			throw 1;

		if (!db()->execSimpleQuery("CREATE TABLE positions("
								   "target INTEGER NOT NULL REFERENCES targets(id) ON UPDATE CASCADE ON DELETE CASCADE, "
								   "block INTEGER)"))
			throw 1;


		GameMatch *match = m_game->gameMatch();
		GameMap::MissionLevel *level = match->missionLevel();

		if (!level) {
			qWarning() << "Invalid mission level";
			throw 1;
		}

		int mapIdx = 1;

		foreach (GameMap::BlockChapterMap *map, level->blockChapterMaps()) {
			QVariantMap m;
			m["map"] = mapIdx;
			m["maxObjective"] = map->maxObjective();

			db()->execInsertQuery("INSERT INTO maps(?k?) VALUES (?)", m);

			foreach (GameMap::Chapter *chapter, map->chapters()) {
				foreach (GameMap::Objective *objective, chapter->objectives()) {
					QVariantList blockList;

					if (map->blocks().count()) {
						foreach (qint32 block, map->blocks()) {
							blockList.append(block);
						}
					} else {
						blockList.append(QVariant::Invalid);
					}

					QVariantMap m;
					m["map"] = mapIdx;
					m["objective"] = QString(objective->uuid());
					int idx = db()->execInsertQuery("INSERT INTO targets(?k?) VALUES (?)", m);

					foreach (QVariant v, blockList) {
						QVariantMap m;
						m["target"] = idx;
						m["block"] = v;

						db()->execInsertQuery("INSERT INTO positions(?k?) VALUES (?)", m);
					}
				}
			}

			++mapIdx;
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

	if (enemyData->pickableType() == GameEnemyData::PickableInvalid)
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
							  Q_RETURN_ARG(QQuickItem *, item),
							  Q_ARG(int, enemyData->pickableType()),
							  Q_ARG(QVariant, d)
							  );

}
