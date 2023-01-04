/*
 * ---- Call of Suli ----
 *
 * databasemain.cpp
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

#include "databasemain.h"
#include "Logger.h"
#include "qsqlquery.h"
#include "websocketmessage.h"
#include "serverservice.h"

DatabaseMain::DatabaseMain(ServerService *service)
	: QObject(service)
	, Database(QStringLiteral("mainDb"))
	, m_service(service)
{

}

DatabaseMain::~DatabaseMain()
{

}

const QString &DatabaseMain::dbFile() const
{
	return m_dbFile;
}

void DatabaseMain::setDbFile(const QString &newDbFile)
{
	m_dbFile = newDbFile;
}


/**
 * @brief DatabaseMain::databaseCreate
 */

bool DatabaseMain::databasePrepare()
{
	LOG_CTRACE("db") << "PREPARE"  << QThread::currentThread();
	QDefer ret;

	bool r = false;

	m_worker.execInThread([ret, this]() mutable {
		LOG_CDEBUG("db") << "Prepare database:" << qPrintable(m_dbFile)  << QThread::currentThread();

		QMutexLocker mutexlocker(mutex());

		if (!_checkSystemTable()) {
			ret.reject();
			return;
		}

		LOG_CDEBUG("db") << "Database prepared:" << qPrintable(m_dbFile);

		ret.resolve();
	});

	ret.done([&r](){ r = true; })
			.fail([&r](){ r = false; });


	QDefer::await(ret);

	return r;
}



void DatabaseMain::test()
{
	LOG_CDEBUG("db") << "TEST STARTED" << this;

	m_worker.execInThread([this]() mutable {
		LOG_CDEBUG("db") << "Test in thread started:" << m_dbName;

		QMutexLocker mutexlocker(mutex());

		LOG_CDEBUG("db") << "Mutex lock ok:" << m_dbName;

		QThread::sleep(10);

		LOG_CDEBUG("db") << "Wait end:" << m_dbName;

	});

}


/**
 * @brief DatabaseMain::_checkSystemTable
 * @return
 */

bool DatabaseMain::_checkSystemTable()
{
	static uint called = 1;

	LOG_CTRACE("db") << "Check system table" << called  << QThread::currentThread();

	if (called > 2) {
		LOG_CERROR("db") << "System table prepare infinite loop";
		return false;
	}

	called++;

	QSqlDatabase db = QSqlDatabase::database(m_dbName);

	QSqlQuery q(db);

	q.exec(QStringLiteral("SELECT versionMajor, versionMinor, serverName from system"));

	LOG_CTRACE("db") << "Query executed";

	if (q.size() > 1) {
		LOG_CERROR("db") << "Corrupt database";
		return false;
	} else if (q.first()) {
		int vMajor = q.value(QStringLiteral("versionMajor")).toInt();
		int vMinor = q.value(QStringLiteral("versionMinor")).toInt();

		m_service->setServerName(q.value(QStringLiteral("serverName")).toString());

		if (WebSocketMessage::versionCode(vMajor, vMinor) < WebSocketMessage::versionCode()) {
			if (_upgradeTables())
				return _checkSystemTable();
			else
				return false;
		} else if (WebSocketMessage::versionCode(vMajor, vMinor) == WebSocketMessage::versionCode()) {
			return true;
		} else {
#ifdef QT_DEBUG
			LOG_CINFO("db") << "Database is newer than system service, skipped in debug";
			return true;
#else
			LOG_CERROR("db") << "Database is newer than system service";
			return false;
#endif
		}
	} else {
		LOG_CTRACE("db") << "Failed";

		if (_createTables() && _createUsers())
			return _checkSystemTable();
		else
			return false;
	}

	return false;
}


/**
 * @brief DatabaseMain::_createTables
 * @return
 */

bool DatabaseMain::_createTables()
{
	LOG_CTRACE("db") << "Create tables"  << QThread::currentThread();

	LOG_CTRACE("db") << "RETURN" << _batchFromFile(QStringLiteral(":/sql/main.sql"));

	QSqlDatabase db = QSqlDatabase::database(m_dbName);

	QueryBuilder q(db);

	q.addQuery("INSERT INTO system (versionMajor, versionMinor, serverName) VALUES (")
			.addValue(m_service->versionMajor())
			.addValue(m_service->versionMinor())
			.addValue(QStringLiteral("New Call of Suli server"))
			.addQuery(")");

	if (!q.exec()) {
		QUERY_LOG_ERROR(q);
		return false;
	}

	return true;
}



bool DatabaseMain::_upgradeTables()
{
	LOG_CERROR("db") << "Missing implementation";
	return false;
}


/**
 * @brief DatabaseMain::_createUsers
 * @return
 */

bool DatabaseMain::_createUsers()
{
	QSqlDatabase db = QSqlDatabase::database(m_dbName);

	struct Users {
		QString username;
		QString password;
		QString firstname;
		bool isAdmin;
		bool isTeacher;
		bool isPanel;

		Users(const QString &u,
			  const QString &p,
			  const QString &f,
			  bool ia,
			  bool it,
			  bool ip
			  ) :
			username(u),
			password(p),
			firstname(f),
			isAdmin(ia),
			isTeacher(it),
			isPanel(ip)
		{}
	};

	QVector<Users> users;

	users.append({
					 QStringLiteral("admin"),
					 QStringLiteral("admin"),
					 QStringLiteral("Adminisztrátor"),
					 true,
					 false,
					 false
				 });

#ifdef QT_DEBUG
	for (int i=1; i<6; ++i) {
		users.append({
						 QStringLiteral("student%1").arg(i),
						 QStringLiteral("student"),
						 QStringLiteral("Tanuló %1").arg(i),
						 false,
						 false,
						 false
					 });
	}

	for (int i=1; i<4; ++i) {
		users.append({
						 QStringLiteral("teacher%1").arg(i),
						 QStringLiteral("teacher"),
						 QStringLiteral("Tanár %1").arg(i),
						 false,
						 true,
						 false
					 });
	}

	for (int i=1; i<3; ++i) {
		users.append({
						 QStringLiteral("panel%1").arg(i),
						 QStringLiteral("panel"),
						 QStringLiteral("Panel %1").arg(i),
						 false,
						 false,
						 true
					 });
	}
#endif

	db.transaction();

	foreach (const Users &u, users) {
		QSqlQuery q(db);
		queryPrepareList(&q, QStringLiteral("INSERT INTO user(username, firstname, active, isAdmin, isTeacher, isPanel) "
											"VALUES (?,?,?,?,?,?)"),
						 {
							 u.username,
							 u.firstname,
							 true,
							 u.isAdmin,
							 u.isTeacher,
							 u.isPanel
						 });

		if (!q.exec()) {
			QUERY_LOG_ERROR(q);
			db.rollback();
			return false;
		}

		q.clear();

		QString salt;
		QString pwd = Credential::hashString(u.password, &salt);

		queryPrepareList(&q, QStringLiteral("INSERT INTO auth(username, password, salt) "
											"VALUES (?,?,?)"),
						 {
							 u.username,
							 pwd,
							 salt
						 });

		if (!q.exec()) {
			QUERY_LOG_ERROR(q);
			db.rollback();
			return false;
		}
	}

	db.commit();

	return true;
}
