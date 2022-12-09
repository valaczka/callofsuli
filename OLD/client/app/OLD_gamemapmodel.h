/*
 * ---- Call of Suli ----
 *
 * gamemapmodel.h
 *
 * Created on: 2021. 09. 25.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameMapModel
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

#ifndef OLD_GAMEMAPMODEL_H
#define OLD_GAMEMAPMODEL_H

#include "OLD_variantmapmodel.h"
#include "gamemap.h"


class GameMapModel : public VariantMapModel
{
	Q_OBJECT

	Q_PROPERTY(int levelCount READ levelCount WRITE setlevelCount NOTIFY levelCountChanged)
	Q_PROPERTY(QVariantMap missionData READ missionData WRITE setMissionData NOTIFY missionDataChanged)


public:
	explicit GameMapModel(QObject *parent = nullptr);
	virtual ~GameMapModel();

	struct MissionLevel {
		QString uuid;
		int level;
		bool deathmatch;
		bool success;
		int num;

		MissionLevel() : uuid(), level(-1), deathmatch(false), success(false), num(-1) {}

		QVariantMap toMap() const;
	};

	struct Mission {
		QString uuid;
		QString name;
		QVector<MissionLevel> levels;
		QString medalImage;

		Mission() : uuid(), name(), levels(), medalImage() {}

		QVariantMap toMap() const;
	};

	struct User {
		QString firstname;
		QString lastname;
		int xp;
		QVariantMap userInfo;
		QVector<MissionLevel> levelData;
		bool active;

		User() : firstname(), lastname(), xp(0), userInfo(), levelData(), active(true) {}

		MissionLevel getLevelData(const QString &uuid, const int &level, const bool &deathmatch) const;
		MissionLevel getLevelData(const MissionLevel &missionLevel) const;
	};

	void setGameMap(GameMap *map);

	void appendUser(const QString &username, const User &user);
	void appendUser(const QString &username, const QString &firstname, const QString &lastname, const bool &active, const int &xp);

	void setUser(const QString &username, const QString &firstname, const QString &lastname, const bool &active, const int &xp);
	void setUser(const QString &username, const int &xp);
	void setUser(const QJsonObject &data);
	void setUserLevelData(const QJsonObject &data);

	void refreshUsers();

	int levelCount() const { return m_levelCount; }
	QVariantMap missionData() const { return m_missionData; }


public slots:
	void loadFromServer(const QJsonObject &jsonData, const QByteArray &);
	void setlevelCount(int levelCount);
	void setMissionData(QVariantMap missionData);

signals:
	void levelCountChanged(int levelCount);
	void missionDataChanged(QVariantMap missionData);

private:
	void updateUser(const QString &username);
	QVariantList getUserData(const User &user) const;

	QVector<Mission> m_missions;
	QHash<QString, User> m_users;
	int m_levelCount;
	QVariantMap m_missionData;
};



#endif // OLD_GAMEMAPMODEL_H
