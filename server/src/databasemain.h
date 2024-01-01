/*
 * ---- Call of Suli ----
 *
 * databasemain.h
 *
 * Created on: 2023. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * DatabaseMain
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

#ifndef DATABASEMAIN_H
#define DATABASEMAIN_H

#include <database.h>

class ServerService;

class DatabaseMain : public QObject, public Database
{
	Q_OBJECT

public:
	explicit DatabaseMain(ServerService *service);
	virtual ~DatabaseMain();

	const QString &dbFile() const;
	void setDbFile(const QString &newDbFile);

	bool databasePrepare(const QString importDb = QString());
	bool databaseAttach();
	bool databaseUpgrade(const int &major, const int &minor);

	void saveConfig(const QJsonObject &json);

	const QString &dbMapsFile() const;
	void setDbMapsFile(const QString &newDbMapsFile);

	const QString &dbStatFile() const;
	void setDbStatFile(const QString &newDbStatFile);

private:
	bool databaseMapsPrepare();
	bool databaseStatPrepare();

	bool _checkSystemTable(const QString &dbImport = QString());
	bool _checkMapsSystemTable(Database *mapsDb);
	bool _checkStatSystemTable(Database *statDb);
	bool _createSytemTables();
	bool _createMapsTables(Database *db);
	bool _createStatTables(Database *db);
	bool _upgradeTables(Database *db, const int &dbType, int fromMajor, int fromMinor, int toMajor = -1, int toMinor = -1);
	/*bool _upgradeMapsTables(Database *mapsDb, int fromMajor, int fromMinor, int toMajor = -1, int toMinor = -1);
	bool _upgradeStatTables(Database *statDb, int fromMajor, int fromMinor, int toMajor = -1, int toMinor = -1);*/
	bool _createUsers();
	bool _createRanksAndGrades();
	bool _databaseImport(const QString &dbFile);

	QString m_dbFile;
	QString m_dbMapsFile;
	QString m_dbStatFile;
	ServerService *m_service = nullptr;
};

#endif // DATABASEMAIN_H
