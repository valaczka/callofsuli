/*
 * ---- Call of Suli ----
 *
 * gameenemy.cpp
 *
 * Created on: 2020. 10. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemy
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

#include <QDebug>
#include <qhash.h>
#include "gameenemydata.h"
#include "gameenemy.h"


GameEnemyData::GameEnemyData(QObject *parent)
	: QObject(parent)
	, m_boundRect()
	, m_active(false)
	, m_block(nullptr)
	, m_enemy(nullptr)
	, m_enemyType(EnemySoldier)
	, m_targetId(-1)
	, m_objectiveUuid()
	, m_pickableType(GamePickable::PickableInvalid)
	, m_pickableData()
	, m_targetDestroyed(false)
{

}


/**
 * @brief GameEnemyData::inventoryTypes
 * @return
 */

QHash<QString, GameEnemyData::InventoryType> GameEnemyData::inventoryTypes()
{
	QHash<QString, GameEnemyData::InventoryType> list;

	list["health"] = InventoryType(tr("1 HP"),
								   "qrc:/internal/game/powerup.gif",
								   GamePickable::PickableHealth,
								   QVariantMap());

	list["time1"] = InventoryType(tr("30 másodperc"),
								  "qrc:/internal/game/time-30.png",
								  GamePickable::PickableTime,
								  QVariantMap({{"text", "30"}, {"secs", 30}}));

	list["time2"] = InventoryType(tr("1 perc"),
								  "qrc:/internal/game/time-60.png",
								  GamePickable::PickableTime,
								  QVariantMap({{"text", "60"}, {"secs", 60}}));

	list["shield1"] = InventoryType(tr("1 pajzs"),
									"qrc:/internal/game/shield-green.png",
									GamePickable::PickableShield,
									QVariantMap({{"num", 1}}));

	list["shield2"] = InventoryType(tr("2 pajzs"),
									"qrc:/internal/game/shield-blue.png",
									GamePickable::PickableShield,
									QVariantMap({{"num", 2}}));

	list["shield3"] = InventoryType(tr("3 pajzs"),
									"qrc:/internal/game/shield-red.png",
									GamePickable::PickableShield,
									QVariantMap({{"num", 3}}));

	list["shield4"] = InventoryType(tr("5 pajzs"),
									"qrc:/internal/game/shield-gold.png",
									GamePickable::PickableShield,
									QVariantMap({{"num", 5}}));

	list["water"] = InventoryType(tr("1 víz"),
								   "qrc:/internal/game/water.svg",
								   GamePickable::PickableWater,
								   QVariantMap());

	list["pliers"] = InventoryType(tr("1 fogó"),
								   "qrc:/internal/game/pliers.png",
								   GamePickable::PickablePliers,
								   QVariantMap());

	list["camouflage"] = InventoryType(tr("1 álruha"),
								   "qrc:/internal/game/camouflage.png",
								   GamePickable::PickableCamouflage,
								   QVariantMap());

	list["teleporter"] = InventoryType(tr("1 teleportáló"),
								   "qrc:/internal/game/teleporter.png",
								   GamePickable::PickableTeleporter,
								   QVariantMap());

	return list;
}


/**
 * @brief GameEnemyData::inventoryInfo
 * @param module
 * @return
 */

QVariantMap GameEnemyData::inventoryInfo(const QString &module)
{
	QHash<QString, InventoryType> list = inventoryTypes();

	if (!list.contains(module)) {
		return QVariantMap({
							   { "name", QObject::tr("Érvénytelen modul!") },
							   { "icon", "image://font/Material Icons/\ue002" }
						   });
	}

	InventoryType t = list.value(module);

	return QVariantMap({
						   { "name", t.name },
						   { "icon", t.icon }
					   });
}


/**
 * @brief GameEnemyData::enemyPrivate
 * @return
 */

GameEnemy *GameEnemyData::enemyPrivate() const
{
	return m_enemy ? qvariant_cast<GameEnemy *>(m_enemy->property("entityPrivate")) : nullptr;
}





void GameEnemyData::setBoundRect(QRectF boundRect)
{
	if (m_boundRect == boundRect)
		return;

	m_boundRect = boundRect;
	emit boundRectChanged(m_boundRect);
}

void GameEnemyData::setActive(bool active)
{
	if (m_active == active)
		return;

	m_active = active;
	emit activeChanged(m_active);
}

void GameEnemyData::setBlock(GameBlock *block)
{
	if (m_block == block)
		return;

	m_block = block;
	emit blockChanged(m_block);
}

void GameEnemyData::setEnemy(QQuickItem *enemy)
{
	if (m_enemy == enemy)
		return;

	m_enemy = enemy;
	emit enemyChanged(m_enemy);
}

void GameEnemyData::setEnemyType(GameEnemyData::EnemyType enemyType)
{
	if (m_enemyType == enemyType)
		return;

	m_enemyType = enemyType;
	emit enemyTypeChanged(m_enemyType);
}


/**
 * @brief GameEnemyData::enemyDied
 */

void GameEnemyData::enemyDied()
{
	setEnemy(nullptr);
	setActive(false);
}


/**
 * @brief GameEnemyData::enemyKilled
 */

void GameEnemyData::enemyKilled(GameEnemy *)
{
	setActive(false);

	if (m_block)
		m_block->recalculateActiveEnemies();
}

void GameEnemyData::setTargetId(int targetId)
{
	if (m_targetId == targetId)
		return;

	m_targetId = targetId;
	emit targetIdChanged(m_targetId);
}

void GameEnemyData::setObjectiveUuid(QString objectiveUuid)
{
	if (m_objectiveUuid == objectiveUuid)
		return;

	m_objectiveUuid = objectiveUuid;
	emit objectiveUuidChanged(m_objectiveUuid);
}



/**
 * @brief GameEnemyData::generateTarget
 * @return
 */

Question GameEnemyData::generateQuestion()
{
	GameEnemy *e = enemyPrivate();

	GameMap *gameMap = nullptr;

	if (e) {
		CosGame *g = e->cosGame();
		if (g) {
			GameMatch *m = g->gameMatch();

			if (m) {
				gameMap = m->gameMap();
			}
		}
	}

	if (m_objectiveUuid.isEmpty() || !gameMap)
		return Question();


	GameMapObjective *objective = gameMap->objective(m_objectiveUuid);

	if (!objective) {
		return Question();
	}

	Question q(objective);

	q.generate();

	return q;
}



void GameEnemyData::setPickableType(GamePickable::PickableType pickableType)
{
	if (m_pickableType == pickableType)
		return;

	m_pickableType = pickableType;
	emit pickableTypeChanged(m_pickableType);
}

void GameEnemyData::setPickableData(QVariantMap pickableData)
{
	if (m_pickableData == pickableData)
		return;

	m_pickableData = pickableData;
	emit pickableDataChanged(m_pickableData);
}



void GameEnemyData::setTargetDestroyed(bool targetDestroyed)
{
	if (m_targetDestroyed == targetDestroyed)
		return;

	m_targetDestroyed = targetDestroyed;
	emit targetDestroyedChanged(m_targetDestroyed);
}

