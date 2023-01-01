/*
 * ---- Call of Suli ----
 *
 * database.h
 *
 * Created on: 2022. 12. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Database
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

#ifndef DATABASE_H
#define DATABASE_H


#include "qloggingcategory.h"
#include <QDeferred>
#include <QLambdaThreadWorker>
#include <QSqlDatabase>
#include <QSqlError>

class Database
{
public:
	Database(const QString &dbName);
	virtual ~Database();

	const QString &dbName() const;

	virtual QDeferred<QSqlError> databaseOpen(const QString &path);

protected:
	bool databaseInit();

	QLambdaThreadWorker m_worker;
	QString m_dbName;
};

Q_DECLARE_LOGGING_CATEGORY(lcDb);

#endif // DATABASE_H
