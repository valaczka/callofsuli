/*
 * ---- Call of Suli ----
 *
 * gamemapmission.h
 *
 * Created on: 2022. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameMapMission
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

#ifndef GAMEMAPMISSION_H
#define GAMEMAPMISSION_H

#include <QObject>
#include "gamemapmissionlevel.h"

class GameMapMission;
class GameMapNew;

class GameMapMissionLock;

class GameMapMission : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
	Q_PROPERTY(QList<GameMapMissionLevel *> levels READ levels WRITE setLevels NOTIFY levelsChanged)
	Q_PROPERTY(QList<GameMapMissionLock *> locks READ locks WRITE setLocks NOTIFY locksChanged)
	Q_PROPERTY(QString medalImage READ medalImage WRITE setMedalImage NOTIFY medalImageChanged)
	Q_PROPERTY(GameMapNew *gameMap READ gameMap WRITE setGameMap NOTIFY gameMapChanged)

public:
	explicit GameMapMission(QObject *parent = nullptr);
	virtual ~GameMapMission();

	const QString &uuid() const;
	void setUuid(const QString &newUuid);

	const QString &name() const;
	void setName(const QString &newName);

	const QString &description() const;
	void setDescription(const QString &newDescription);

	const QList<GameMapMissionLevel *> &levels() const;
	void setLevels(const QList<GameMapMissionLevel *> &newLevels);

	const QList<GameMapMissionLock *> &locks() const;
	void setLocks(const QList<GameMapMissionLock *> &newLocks);

	const QString &medalImage() const;
	void setMedalImage(const QString &newMedalImage);

	GameMapNew *gameMap() const;
	void setGameMap(GameMapNew *newGameMap);

signals:
	void uuidChanged();
	void nameChanged();
	void descriptionChanged();
	void levelsChanged();
	void locksChanged();
	void medalImageChanged();

	void gameMapChanged();

private:
	QString m_uuid;
	QString m_name;
	QString m_description;
	QList<GameMapMissionLevel *> m_levels;
	QList<GameMapMissionLock *> m_locks;
	QString m_medalImage;
	GameMapNew *m_gameMap;
};



/**
 * @brief The GameMapMissionLock class
 */


class GameMapMissionLock : public QObject
{
	Q_OBJECT

	Q_PROPERTY(GameMapMission *mission READ mission WRITE setMission NOTIFY missionChanged)
	Q_PROPERTY(qint32 level READ level WRITE setLevel NOTIFY levelChanged)
	Q_PROPERTY(GameMapMission *parentMission READ parentMission WRITE setParentMission NOTIFY parentMissionChanged)

public:
	explicit GameMapMissionLock(QObject *parent = nullptr);
	virtual ~GameMapMissionLock();

	GameMapMission *mission() const;
	void setMission(GameMapMission *newMission);

	qint32 level() const;
	void setLevel(qint32 newLevel);

	GameMapMission *parentMission() const;
	void setParentMission(GameMapMission *newParentMission);

signals:
	void missionChanged();
	void levelChanged();
	void parentMissionChanged();

private:
	GameMapMission *m_mission;
	qint32 m_level;
	GameMapMission *m_parentMission;
};


#endif // GAMEMAPMISSION_H
