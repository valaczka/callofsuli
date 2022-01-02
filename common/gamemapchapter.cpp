/*
 * ---- Call of Suli ----
 *
 * gamemapchapter.cpp
 *
 * Created on: 2022. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameMapChapter
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

#include "gamemapchapter.h"
#include "gamemapnew.h"
#include <QDataStream>
#include <QJsonArray>

GameMapChapter::GameMapChapter(QObject *parent)
	: QObject{parent}
	, m_id(0)
	, m_name()
	, m_objectives()
	, m_gameMap(nullptr)
{

}


/**
 * @brief GameMapChapter::~GameMapChapter
 */

GameMapChapter::~GameMapChapter()
{
	foreach (GameMapObjective *s, m_objectives)
		s->setChapter(nullptr);

	qDeleteAll(m_objectives.begin(), m_objectives.end());
	m_objectives.clear();

	if (m_gameMap)
		m_gameMap->chapterRemove(this);
}


/**
 * @brief GameMapChapter::toJsonObject
 * @return
 */

QJsonObject GameMapChapter::toJsonObject() const
{
	QJsonObject obj;

	obj["id"] = m_id;
	obj["name"] = m_name;

	QJsonArray objectiveArray;

	foreach (GameMapObjective *s, m_objectives)
		objectiveArray.append(s->toJsonObject());

	obj["objectives"] = objectiveArray;

	return obj;
}


/**
 * @brief GameMapChapter::id
 * @return
 */

qint32 GameMapChapter::id() const
{
	return m_id;
}

void GameMapChapter::setId(qint32 newId)
{
	if (m_id == newId)
		return;
	m_id = newId;
	emit idChanged();
}

const QString &GameMapChapter::name() const
{
	return m_name;
}

void GameMapChapter::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}

const QList<GameMapObjective *> &GameMapChapter::objectives() const
{
	return m_objectives;
}

void GameMapChapter::setObjectives(const QList<GameMapObjective *> &newObjectives)
{
	if (m_objectives == newObjectives)
		return;
	m_objectives = newObjectives;
	emit objectivesChanged();
}

GameMapNew *GameMapChapter::gameMap() const
{
	return m_gameMap;
}

void GameMapChapter::setGameMap(GameMapNew *newGameMap)
{
	if (m_gameMap == newGameMap)
		return;
	m_gameMap = newGameMap;
	emit gameMapChanged();
}

/**
 * @brief GameMapChapter::objectiveAdd
 * @param chapter
 */

void GameMapChapter::objectiveAdd(GameMapObjective *objective)
{
	m_objectives.append(objective);
	objective->setChapter(this);
}


/**
 * @brief GameMapChapter::objectiveRemove
 * @param objective
 * @return
 */

int GameMapChapter::objectiveRemove(GameMapObjective *objective)
{
	int n = m_objectives.removeAll(objective);
	objective->setChapter(nullptr);
	return n;
}


/**
 * @brief GameMapChapter::objectivesFromStream
 * @param stream
 * @return
 */

bool GameMapChapter::objectivesFromStream(QDataStream &stream, GameMapNew *map)
{
	Q_ASSERT(map);

	quint32 size = 0;
	stream >> size;

	for (quint32 i=0; i<size; i++) {
		QByteArray uuid;
		QByteArray module;
		qint32 storageId = -1;
		qint32 storageCount = 0;
		QVariantMap data;
		stream >> uuid >> module >> storageId >> data >> storageCount;

		GameMapObjective *o = new GameMapObjective(this);
		o->setUuid(uuid);
		o->setModule(module);
		o->setStorageCount(storageCount);
		o->setData(data);

		GameMapStorage *s = nullptr;

		if (storageId != -1) {
			s = map->storage(storageId);
			if (!s) {
				qWarning() << "Storage not found" << s;
			}
		}

		o->setStorage(s);

		objectiveAdd(o);
	}

	if (m_objectives.size() != (int) size)
		return false;

	return true;
}



/**
 * @brief GameMapChapter::objectivesToStream
 * @param stream
 */

void GameMapChapter::objectivesToStream(QDataStream &stream) const
{
	stream << (quint32) m_objectives.size();

	foreach (GameMapObjective *o, m_objectives) {
		stream << o->uuid().toLatin1();
		stream << o->module().toLatin1();
		stream << (o->storage() ? o->storage()->id() : (qint32) -1);
		stream << o->data();
		stream << o->storageCount();
	}
}
