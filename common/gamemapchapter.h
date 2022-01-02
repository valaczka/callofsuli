/*
 * ---- Call of Suli ----
 *
 * gamemapchapter.h
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

#ifndef GAMEMAPCHAPTER_H
#define GAMEMAPCHAPTER_H

#include <QObject>
#include "gamemapobjective.h"

class GameMapNew;

class GameMapChapter : public QObject
{
	Q_OBJECT

	Q_PROPERTY(qint32 id READ id WRITE setId NOTIFY idChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(QList<GameMapObjective *> objectives READ objectives WRITE setObjectives NOTIFY objectivesChanged)
	Q_PROPERTY(GameMapNew *gameMap READ gameMap WRITE setGameMap NOTIFY gameMapChanged)

public:
	explicit GameMapChapter(QObject *parent = nullptr);
	virtual ~GameMapChapter();

	QJsonObject toJsonObject() const;

	qint32 id() const;
	void setId(qint32 newId);

	const QString &name() const;
	void setName(const QString &newName);

	const QList<GameMapObjective *> &objectives() const;
	void setObjectives(const QList<GameMapObjective *> &newObjectives);

	GameMapNew *gameMap() const;
	void setGameMap(GameMapNew *newGameMap);

	void objectiveAdd(GameMapObjective *objective);
	int objectiveRemove(GameMapObjective *objective);

signals:
	void idChanged();
	void nameChanged();
	void objectivesChanged();

	void gameMapChanged();

private:
	friend class GameMapNew;
	bool objectivesFromStream(QDataStream &stream, GameMapNew *map);
	void objectivesToStream(QDataStream &stream) const;

	qint32 m_id;
	QString m_name;
	QList<GameMapObjective *> m_objectives;
	GameMapNew *m_gameMap;
};

#endif // GAMEMAPCHAPTER_H
