/*
 * ---- Call of Suli ----
 *
 * mapplaymission.h
 *
 * Created on: 2023. 01. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapPlayMission
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

#ifndef MAPPLAYMISSION_H
#define MAPPLAYMISSION_H

#include <QObject>
#include <QOlm/QOlm.hpp>
#include "gamemap.h"

class MapPlayMissionLevel;
using MapPlayMissionLevelList = qolm::QOlm<MapPlayMissionLevel>;

class MapPlayMission : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(MapPlayMissionLevelList* levels READ levels WRITE setLevels NOTIFY levelsChanged)

public:
	explicit MapPlayMission(QObject *parent = nullptr);

	const QString &name() const;
	void setName(const QString &newName);

	MapPlayMissionLevelList *levels() const;
	void setLevels(MapPlayMissionLevelList *newLevels);

signals:
	void nameChanged();
	void levelsChanged();

private:
	QString m_name;
	MapPlayMissionLevelList *m_levels = nullptr;
};


using MapPlayMissionList = qolm::QOlm<MapPlayMission>;

Q_DECLARE_METATYPE(MapPlayMissionList*)

/*class MapPlayMissionList : public qolm::QOlm<MapPlayMission>
{
	Q_OBJECT

public:
	MapPlayMissionList(QObject *parent = nullptr, const QList<QByteArray> & exposedRoles = {},
					   const QByteArray & displayRole = {}):
		QOlm<MapPlayMission>(parent, exposedRoles, displayRole)
	{
	}
};*/



class MapPlayMissionLevel : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int level READ level WRITE setLevel NOTIFY levelChanged)
	Q_PROPERTY(bool deathmatch READ deathmatch WRITE setDeathmatch NOTIFY deathmatchChanged)
	Q_PROPERTY(GameMapMissionLevel* mapLevel READ mapLevel WRITE setMapLevel NOTIFY mapLevelChanged)


public:
	MapPlayMissionLevel(QObject *parent = nullptr) : QObject(parent) { }

	int level() const;
	void setLevel(int newLevel);
	bool deathmatch() const;
	void setDeathmatch(bool newDeathmatch);

	GameMapMissionLevel *mapLevel() const;
	void setMapLevel(GameMapMissionLevel *newMapLevel);

signals:
	void levelChanged();
	void deathmatchChanged();

	void mapLevelChanged();

private:
	int m_level;
	bool m_deathmatch;
	GameMapMissionLevel *m_mapLevel = nullptr;
};

Q_DECLARE_METATYPE(MapPlayMissionLevelList*)

#endif // MAPPLAYMISSION_H
