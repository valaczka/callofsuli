/*
 * ---- Call of Suli ----
 *
 * database.cpp
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

#include "database.h"
#include "Logger.h"


/**
 * @brief Database::Database
 */

Database::Database(const QString &dbName)
	: m_dbName(dbName)
{
	Q_ASSERT(!dbName.isEmpty());

	LOG_CDEBUG("db") << "Create new database:" << m_dbName;

	if (!databaseInit()) {
		qFatal("%s", QObject::tr("Can't create database: %1").arg(m_dbName).toLatin1().constData());
	}
}


/**
 * @brief Database::~Database
 */

Database::~Database()
{
	LOG_CDEBUG("db") << "Remove database:" << m_dbName;

	m_worker.execInThread([this](){
		{
			QSqlDatabase db = QSqlDatabase::database(m_dbName);
			if (db.isOpen())
				db.close();
		}

		QSqlDatabase::removeDatabase(m_dbName);
	});

	LOG_CDEBUG("db") << "Destroy database:" << m_dbName;
}


/**
 * @brief Database::dbName
 * @return
 */

const QString &Database::dbName() const
{
	return m_dbName;
}


/**
 * @brief Database::databaseOpen
 * @param path
 */

QDeferred<QSqlError> Database::databaseOpen(const QString &path)
{
	QDeferred<QSqlError> ret;

	m_worker.execInThread([ret, this, path]() mutable {
		LOG_CDEBUG("db") << "Open database" << m_dbName << ": " << path;

		QSqlDatabase db = QSqlDatabase::database(m_dbName);
		db.setDatabaseName(path);

		if (db.open()) {
			ret.resolve(db.lastError());
		} else {
			ret.reject(db.lastError());
		}
	});

	return ret;
}


/**
 * @brief Database::databaseClose
 */

void Database::databaseClose()
{
	QDefer ret;

	m_worker.execInThread([ret, this]() mutable {
		LOG_CDEBUG("db") << "Close database:" << m_dbName;

		QSqlDatabase::database(m_dbName).close();

		ret.resolve();
	});

	QDefer::await(ret);
}


/**
 * @brief Database::databaseInit
 * @return
 */

bool Database::databaseInit()
{
	QDefer ret;
	bool retValue = false;

	m_worker.execInThread([ret, this]() mutable {
		LOG_CDEBUG("db") << "Add database:" << m_dbName;

		if (QSqlDatabase::contains(m_dbName)) {
			LOG_CERROR("db") << "Database already in use:" << m_dbName;
			ret.reject();
			return;
		}

		QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_dbName);

		if (!db.isValid()) {
			LOG_CERROR("db") << "Database invalid:" << m_dbName;
			ret.reject();
		} else {
			ret.resolve();
		}
	});


	ret.done([&retValue](){
		retValue = true;
	}).fail([&retValue](){
		retValue = false;
	});;

	QDefer::await(ret);

	return retValue;
}

