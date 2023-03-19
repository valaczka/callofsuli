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
#include "utils.h"
#include "serverservice.h"
#include "adminapi.h"
#include "rank.h"

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
	QDefer ret;

	bool r = false;

	m_worker->execInThread([ret, this, &r]() mutable {
		LOG_CDEBUG("db") << "Prepare database:" << qPrintable(m_dbFile);

		QMutexLocker mutexlocker(mutex());

		if (!_checkSystemTable()) {
			r = false;
			ret.reject();
			return;
		}

		LOG_CDEBUG("db") << "Database prepared:" << qPrintable(m_dbFile);

		r = true;
		ret.resolve();
	});

	QDefer::await(ret);

	return r;
}


/**
 * @brief DatabaseMain::configSet
 * @param json
 */

void DatabaseMain::saveConfig(const QJsonObject &json)
{
	LOG_CTRACE("db") << "Save config";

	m_worker->execInThread([json, this]() mutable {
		QSqlDatabase db = QSqlDatabase::database(m_dbName);
		QMutexLocker mutexlocker(mutex());
		const QString &doc = QJsonDocument(json).toJson(QJsonDocument::Compact);

		QueryBuilder::q(db).addQuery("UPDATE system SET config=")
				.addValue(doc)
				.exec();

	});
}




/**
 * @brief DatabaseMain::_checkSystemTable
 * @return
 */

bool DatabaseMain::_checkSystemTable()
{
	static uint called = 1;

	LOG_CTRACE("db") << "Check system table" << called;

	if (called > 2) {
		LOG_CERROR("db") << "System table prepare infinite loop";
		return false;
	}

	called++;

	QSqlDatabase db = QSqlDatabase::database(m_dbName);

	QSqlQuery q(db);

	q.exec(QStringLiteral("SELECT versionMajor, versionMinor, serverName from system"));

	if (q.size() > 1) {
		LOG_CERROR("db") << "Corrupt database";
		return false;
	} else if (q.first()) {
		int vMajor = q.value(QStringLiteral("versionMajor")).toInt();
		int vMinor = q.value(QStringLiteral("versionMinor")).toInt();

		m_service->setServerName(q.value(QStringLiteral("serverName")).toString());

		if (Utils::versionCode(vMajor, vMinor) < Utils::versionCode()) {
			if (_upgradeTables())
				return _checkSystemTable();
			else
				return false;
		} else if (Utils::versionCode(vMajor, vMinor) == Utils::versionCode()) {
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
		db.transaction();
		if (_createTables() && _createUsers() && _createRanks()) {
			db.commit();
			return _checkSystemTable();
		} else {
			db.rollback();
			return false;
		}
	}

	return false;
}


/**
 * @brief DatabaseMain::_createTables
 * @return
 */

bool DatabaseMain::_createTables()
{
	LOG_CTRACE("db") << "Create tables";

	if (!_batchFromFile(QStringLiteral(":/sql/main.sql")))
		return false;

	QSqlDatabase db = QSqlDatabase::database(m_dbName);

	QueryBuilder q(db);

	q.addQuery("INSERT INTO system (")
			.setFieldPlaceholder()
			.addQuery(") VALUES (")
			.setValuePlaceholder()
			.addQuery(")")
			.addField("versionMajor", m_service->versionMajor())
			.addField("versionMinor", m_service->versionMinor())
			.addField("serverName", QStringLiteral("New Call of Suli server"))
			;

	if (!q.exec())
		return false;


	q.clear();

	q.addQuery("INSERT INTO classCode(")
			.setFieldPlaceholder()
			.addQuery(") VALUES (")
			.setValuePlaceholder()
			.addQuery(")")
			.addField("classid", QVariant::Invalid)
			.addField("code", AdminAPI::generateClassCode())
			;

	return q.exec();
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

	AdminAPI::User user;
	user.username = QStringLiteral("admin");
	user.familyName = QStringLiteral("Adminisztrátor");
	user.isTeacher = true;
	user.isAdmin = true;
	user.active = true;

	QDefer ret;
	bool success = false;

	AdminAPI::userAdd(this, user)
			.fail([&ret, &success]() mutable {
		success = false;
		ret.reject();
	})
			.done([this, user, &ret, &success]() mutable {
		AdminAPI::authAddPlain(this, user.username, user.username)
				.fail([&ret, &success]() mutable {
			success = false;
			ret.reject();
		})
				.done([&ret, &success]() mutable {
			success = true;
			ret.resolve();
		});
	});

	QDefer::await(ret);

	return success;
}



/**
 * @brief DatabaseMain::_createRanks
 * @return
 */

bool DatabaseMain::_createRanks()
{
	QSqlDatabase db = QSqlDatabase::database(m_dbName);

	foreach(const Rank &r, RankList::defaultRankList()) {
		QueryBuilder q(db);
		q.addQuery("INSERT INTO rank(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("level", r.level())
				.addField("sublevel", r.sublevel() < 0 ? QVariant(QVariant::Invalid) : r.sublevel())
				.addField("xp", r.xp() < 0 ? QVariant(QVariant::Invalid) : r.xp())
				.addField("name", r.name());

		if (!q.exec()) {
			return false;
		}
	}

	return true;
}
