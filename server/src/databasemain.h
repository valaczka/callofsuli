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

	bool databasePrepare();

	void saveConfig(const QJsonObject &json);

private:
	bool _checkSystemTable();
	bool _createTables();
	bool _upgradeTables();
	bool _createUsers();

	QString m_dbFile;
	ServerService *m_service = nullptr;
};

#endif // DATABASEMAIN_H
