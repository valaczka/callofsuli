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
#include "utils_.h"
#include "querybuilder.hpp"
#include "serverservice.h"
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

bool DatabaseMain::databasePrepare(const QString importDb)
{
	if (!databaseMapsPrepare())
		return false;

	if (!databaseStatPrepare())
		return false;

	QDefer ret;

	bool r = false;

	m_worker->execInThread([ret, this, &r, importDb]() mutable {
		LOG_CDEBUG("db") << "Prepare database:" << qPrintable(m_dbFile);

		QMutexLocker mutexlocker(mutex());

		if (!_checkSystemTable(importDb)) {
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
 * @brief DatabaseMain::databaseAttach
 * @return
 */

bool DatabaseMain::databaseAttach()
{
	QDefer ret;

	bool r = false;

	m_worker->execInThread([ret, this, &r]() mutable {
		LOG_CDEBUG("db") << "Attach databases:" << qPrintable(m_dbFile);

		QSqlDatabase db = QSqlDatabase::database(m_dbName);

		QMutexLocker mutexlocker(mutex());

		if (!QueryBuilder::q(db).addQuery("ATTACH ").addValue(m_dbMapsFile).addQuery(" AS mapdb").exec()) {
			r = false;
			ret.reject();
			return;
		}

		if (!QueryBuilder::q(db).addQuery("ATTACH ").addValue(m_dbStatFile).addQuery(" AS statdb").exec()) {
			r = false;
			ret.reject();
			return;
		}

		LOG_CDEBUG("db") << "Attach succesful";

		r = true;
		ret.resolve();
	});

	QDefer::await(ret);

	return r;
}



/**
 * @brief DatabaseMain::databaseUpgrade
 * @param major
 * @param minor
 * @return
 */

bool DatabaseMain::databaseUpgrade(const int &major, const int &minor)
{
	for (int i=0; i<=2; ++i) {
		QDefer ret;

		std::unique_ptr<Database> db;

		if (i==1) {
			db.reset(new Database(QStringLiteral("mapsDb")));
			db->databaseOpen(m_dbMapsFile);
		} else if (i==2) {
			db.reset(new Database(QStringLiteral("statDb")));
			db->databaseOpen(m_dbStatFile);
		}

		QLambdaThreadWorker *worker = db ? db->worker() : m_worker.get();

		worker->execInThread([ret, this, &major, &minor, ptr = db.get(), i]() mutable {
			QMutexLocker mutexlocker(ptr ? ptr->mutex() : mutex());

			int vMajor = 0;
			int vMinor = 0;

			{
				QSqlDatabase db = QSqlDatabase::database(ptr ? ptr->dbName() : m_dbName);

				if (QueryBuilder q(db); q.addQuery("SELECT versionMajor, versionMinor FROM system").exec()) {
					if (!q.sqlQuery().first()) {
						LOG_CERROR("db") << "Corrupt database";
						return ret.reject();
					}

					vMajor = q.value("versionMajor").toInt();
					vMinor = q.value("versionMinor").toInt();
				}
			}

			if (Utils::versionCode(vMajor, vMinor) < Utils::versionCode(major, minor)) {
				if (!_upgradeTables(ptr ? ptr : this, i, vMajor, vMinor, major, minor)) {
					LOG_CERROR("db") << "Upgrade failed";
					return ret.reject();
				}
			}

			ret.resolve();
		});

		QDefer::await(ret);

		if (db)
			db->databaseClose();

		if (ret.state() != RESOLVED) {
			return false;
		}
	}

	return true;
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
 * @brief DatabaseMain::databaseMapsPrepare
 * @return
 */

bool DatabaseMain::databaseMapsPrepare()
{
	Database mapsDb = Database(QStringLiteral("mapsDb"));
	mapsDb.databaseOpen(m_dbMapsFile);

	QDefer ret;

	bool r = false;

	mapsDb.worker()->execInThread([ret, this, &r, &mapsDb]() mutable {
		LOG_CDEBUG("db") << "Prepare maps database:" << qPrintable(m_dbMapsFile);

		QMutexLocker mutexlocker(mapsDb.mutex());

		if (!_checkMapsSystemTable(&mapsDb)) {
			r = false;
			ret.reject();
			return;
		}

		LOG_CDEBUG("db") << "Database maps prepared:" << qPrintable(m_dbMapsFile);

		r = true;
		ret.resolve();
	});

	QDefer::await(ret);

	mapsDb.databaseClose();

	return r;
}





/**
 * @brief DatabaseMain::databaseStatPrepare
 * @return
 */

bool DatabaseMain::databaseStatPrepare()
{
	Database statDb = Database(QStringLiteral("statDb"));
	statDb.databaseOpen(m_dbStatFile);

	QDefer ret;

	bool r = false;

	statDb.worker()->execInThread([ret, this, &r, &statDb]() mutable {
		LOG_CDEBUG("db") << "Prepare stat database:" << qPrintable(m_dbStatFile);

		QMutexLocker mutexlocker(statDb.mutex());

		if (!_checkStatSystemTable(&statDb)) {
			r = false;
			ret.reject();
			return;
		}

		LOG_CDEBUG("db") << "Database stat prepared:" << qPrintable(m_dbStatFile);

		r = true;
		ret.resolve();
	});

	QDefer::await(ret);

	statDb.databaseClose();

	return r;
}






/**
 * @brief DatabaseMain::_checkSystemTable
 * @return
 */

bool DatabaseMain::_checkSystemTable(const QString &dbImport)
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

	q.exec(QStringLiteral("SELECT versionMajor, versionMinor, serverName FROM system"));

	if (q.size() > 1) {
		LOG_CERROR("db") << "Corrupt database";
		return false;
	} else if (q.first()) {
		int vMajor = q.value(QStringLiteral("versionMajor")).toInt();
		int vMinor = q.value(QStringLiteral("versionMinor")).toInt();

		q.finish();

		m_service->setServerName(q.value(QStringLiteral("serverName")).toString());

		if (Utils::versionCode(vMajor, vMinor) < Utils::versionCode()) {
			if (_upgradeTables(this, 0, vMajor, vMinor))
				return _checkSystemTable();
			else
				return false;
		} else if (Utils::versionCode(vMajor, vMinor) == Utils::versionCode()) {
			return true;
		} else {
			LOG_CWARNING("db") << "Database is newer than system service";
			return true;
		}
	} else {

		if (!dbImport.isEmpty()) {
			if (_createSytemTables() && _createRanksAndGrades() && _databaseImport(dbImport)) {
				return _checkSystemTable();
			} else {
				LOG_CERROR("db") << "Database import failed:" << qPrintable(dbImport);
				return false;
			}
		}

		db.transaction();

		if (_createSytemTables() && _createUsers() && _createRanksAndGrades()) {
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
 * @brief DatabaseMain::_checkMapsSystemTable
 * @return
 */

bool DatabaseMain::_checkMapsSystemTable(Database *mapsDb)
{
	Q_ASSERT(mapsDb);

	static uint called = 1;

	LOG_CTRACE("db") << "Check maps system table" << called;

	if (called > 2) {
		LOG_CERROR("db") << "Maps system table prepare infinite loop";
		return false;
	}

	called++;

	QSqlDatabase db = QSqlDatabase::database(mapsDb->dbName());

	QSqlQuery q(db);

	q.exec(QStringLiteral("SELECT versionMajor, versionMinor from system"));

	if (q.size() > 1) {
		LOG_CERROR("db") << "Corrupt database";
		return false;
	} else if (q.first()) {
		int vMajor = q.value(QStringLiteral("versionMajor")).toInt();
		int vMinor = q.value(QStringLiteral("versionMinor")).toInt();

		q.finish();

		if (Utils::versionCode(vMajor, vMinor) < Utils::versionCode()) {
			if (_upgradeTables(mapsDb, 1, vMajor, vMinor))
				return _checkMapsSystemTable(mapsDb);
			else
				return false;
		} else if (Utils::versionCode(vMajor, vMinor) == Utils::versionCode()) {
			return true;
		} else {
			LOG_CWARNING("db") << "Maps database is newer than system service";
			return true;
		}
	} else {
		db.transaction();
		if (_createMapsTables(mapsDb)) {
			db.commit();
			return _checkMapsSystemTable(mapsDb);
		} else {
			db.rollback();
			return false;
		}
	}

	return false;
}



/**
 * @brief DatabaseMain::_checkStatSystemTable
 * @param statDb
 * @return
 */

bool DatabaseMain::_checkStatSystemTable(Database *statDb)
{
	Q_ASSERT(statDb);

	static uint called = 1;

	LOG_CTRACE("db") << "Check stat system table" << called;

	if (called > 2) {
		LOG_CERROR("db") << "Stat system table prepare infinite loop";
		return false;
	}

	called++;

	QSqlDatabase db = QSqlDatabase::database(statDb->dbName());

	QSqlQuery q(db);

	q.exec(QStringLiteral("SELECT versionMajor, versionMinor from system"));

	if (q.size() > 1) {
		LOG_CERROR("db") << "Corrupt database";
		return false;
	} else if (q.first()) {
		int vMajor = q.value(QStringLiteral("versionMajor")).toInt();
		int vMinor = q.value(QStringLiteral("versionMinor")).toInt();

		q.finish();

		if (Utils::versionCode(vMajor, vMinor) < Utils::versionCode()) {
			if (_upgradeTables(statDb, 2, vMajor, vMinor))
				return _checkStatSystemTable(statDb);
			else
				return false;
		} else if (Utils::versionCode(vMajor, vMinor) == Utils::versionCode()) {
			return true;
		} else {
			LOG_CWARNING("db") << "Stat database is newer than system service";
			return true;
		}
	} else {
		db.transaction();
		if (_createStatTables(statDb)) {
			db.commit();
			return _checkStatSystemTable(statDb);
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

bool DatabaseMain::_createSytemTables()
{
	LOG_CTRACE("db") << "Create tables";

	if (!_batchFromFile(QStringLiteral(":/sql/main.sql")))
		return false;

	QSqlDatabase db = QSqlDatabase::database(m_dbName);

	if (!QueryBuilder::q(db)
			.addQuery("INSERT INTO system (")
			.setFieldPlaceholder()
			.addQuery(") VALUES (")
			.setValuePlaceholder()
			.addQuery(")")
			.addField("versionMajor", m_service->versionMajor())
			.addField("versionMinor", m_service->versionMinor())
			.addField("serverName", QStringLiteral("New Call of Suli server"))
			.addField("config", QStringLiteral("{\"tokenFirstIat\":%1}").arg(QDateTime::currentSecsSinceEpoch()))
			.exec())
		return false;


	QueryBuilder::q(db)
			.addQuery("INSERT INTO classCode(")
			.setFieldPlaceholder()
			.addQuery(") VALUES (")
			.setValuePlaceholder()
			.addQuery(")")
			.addField("classid", QVariant(QMetaType::fromType<int>()))
			.addField("code", AdminAPI::generateClassCode())
			.exec();


	return true;
}




/**
 * @brief DatabaseMain::_createMapsTables
 * @return
 */

bool DatabaseMain::_createMapsTables(Database *db)
{
	Q_ASSERT(db);

	LOG_CTRACE("db") << "Create maps tables";

	if (!db->_batchFromFile(QStringLiteral(":/sql/maps.sql")))
		return false;

	QSqlDatabase d = QSqlDatabase::database(db->dbName());

	QueryBuilder q(d);

	q.addQuery("INSERT INTO system (")
			.setFieldPlaceholder()
			.addQuery(") VALUES (")
			.setValuePlaceholder()
			.addQuery(")")
			.addField("versionMajor", m_service->versionMajor())
			.addField("versionMinor", m_service->versionMinor())
			;

	if (!q.exec())
		return false;

	return true;
}




/**
 * @brief DatabaseMain::_createStatTables
 * @param db
 * @return
 */

bool DatabaseMain::_createStatTables(Database *db)
{
	Q_ASSERT(db);

	LOG_CTRACE("db") << "Create stat tables";

	if (!db->_batchFromFile(QStringLiteral(":/sql/stat.sql")))
		return false;

	QSqlDatabase d = QSqlDatabase::database(db->dbName());

	QueryBuilder q(d);

	q.addQuery("INSERT INTO system (")
			.setFieldPlaceholder()
			.addQuery(") VALUES (")
			.setValuePlaceholder()
			.addQuery(")")
			.addField("versionMajor", m_service->versionMajor())
			.addField("versionMinor", m_service->versionMinor())
			;

	if (!q.exec())
		return false;

	return true;
}



/**
 * @brief DatabaseMain::_upgradeTables
 * @return
 */


bool DatabaseMain::_upgradeTables(Database *db, const int &dbType, int fromMajor, int fromMinor, int toMajor, int toMinor)
{
	Q_ASSERT(db);
	Q_ASSERT(dbType >= 0 && dbType <= 2);

	if (toMajor == -1 || toMinor == -1) {
		toMajor = m_service->versionMajor();
		toMinor = m_service->versionMinor();
	}

	static const QVector<Upgrade> mainList = {
		Upgrade {3, 4, 3, 5, Database::Upgrade::UpgradeFromFile, QStringLiteral(":/sql/main_3.4_3.5.sql") },
		Upgrade {3, 5, 3, 6, Database::Upgrade::UpgradeFromFile, QStringLiteral(":/sql/main_3.5_3.6.sql") },
		Upgrade {4, 0, 4, 1, Database::Upgrade::UpgradeFromFile, QStringLiteral(":/sql/main_4.0_4.1.sql") },
		Upgrade {4, 1, 4, 2, Database::Upgrade::UpgradeFromFile, QStringLiteral(":/sql/main_4.1_4.2.sql") },
		Upgrade {4, 3, 4, 4, Database::Upgrade::UpgradeFromFile, QStringLiteral(":/sql/main_4.3_4.4.sql") },
		Upgrade {4, 4, 4, 5, Database::Upgrade::UpgradeFromFile, QStringLiteral(":/sql/main_4.4_4.5.sql") },
	};

	static const QVector<Upgrade> mapsList = {
		//Upgrade {3, 5, 3, 6, Database::Upgrade::UpgradeFromFile, QStringLiteral(":/sql/maps_3.5_3.6.sql") }
	};

	static const QVector<Upgrade> statList = {

	};

	return db->performUpgrade(dbType == 2 ? statList : dbType == 1 ? mapsList : mainList,
							  QStringLiteral("UPDATE system SET versionMajor=%1, versionMinor=%2")
							  .arg(toMajor).arg(toMinor),
							  fromMajor, fromMinor,
							  toMajor, toMinor);
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

	if (!AdminAPI::userAdd(this, user)) {
		LOG_CERROR("db") << "Admin user create error";
		return false;
	}

	if (!AdminAPI::authAddPlain(this, user.username, user.username)) {
		LOG_CERROR("db") << "Admin user auth error";
		return false;
	}

	return true;
}



/**
 * @brief DatabaseMain::_createRanks
 * @return
 */

bool DatabaseMain::_createRanksAndGrades()
{
	QSqlDatabase db = QSqlDatabase::database(m_dbName);

	foreach(const Rank &r, RankList::defaultRankList()) {
		if (!QueryBuilder::q(db)
				.addQuery("INSERT INTO rank(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("level", r.level())
				.addField("sublevel", r.sublevel() < 0 ? QVariant(QMetaType::fromType<int>()) : r.sublevel())
				.addField("xp", r.xp() < 0 ? QVariant(QMetaType::fromType<int>()) : r.xp())
				.addField("name", r.name())
				.exec())
			return false;

	}


	/// Grades

	struct Grade {
		QString shortname;
		QString longname;
		int value;
	};

	QVector<Grade> grades = {
		{ tr("1"), tr("elégtelen"), 1 },
		{ tr("2"), tr("elégséges"), 2 },
		{ tr("3"), tr("közepes"), 3 },
		{ tr("4"), tr("jó"), 4 },
		{ tr("5"), tr("jeles"), 5 },
		{ tr("5*"), tr("kitűnő"), 6 },
	};

	foreach (const Grade &g, grades) {
		if (!QueryBuilder::q(db)
				.addQuery("INSERT INTO grade(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("shortname", g.shortname)
				.addField("longname", g.longname)
				.addField("value", g.value)
				.exec())
			return false;
	}

	return true;
}





/**
 * @brief DatabaseMain::_databaseImport
 * @param dbFile
 * @return
 */

bool DatabaseMain::_databaseImport(const QString &dbFile)
{
	LOG_CINFO("db") << "Import databases:" << qPrintable(dbFile);

	QSqlDatabase db = QSqlDatabase::database(m_dbName);

	QMutexLocker mutexlocker(mutex());

	if (!QueryBuilder::q(db).addQuery("ATTACH ").addValue(dbFile).addQuery(" AS importdb").exec())
		return false;

	if (!QueryBuilder::q(db)
			.addQuery("INSERT INTO class(id, name) SELECT id, name FROM importdb.class")
			.exec())
		return false;

	if (!QueryBuilder::q(db)
			.addQuery("INSERT INTO classCode(classid, code) SELECT classid, code FROM importdb.classRegistration")
			.exec())
		return false;

	if (!QueryBuilder::q(db)
			.addQuery("INSERT INTO user(username, familyName, givenName, active, classid, isTeacher, isAdmin, nickname, character, picture) "
					  "SELECT username, firstname, lastname, active, classid, isTeacher, isAdmin, nickname, character, picture FROM importdb.user")
			.exec())
		return false;

	if (!QueryBuilder::q(db)
			.addQuery("INSERT INTO auth(username, password, salt, oauth) SELECT username, password, salt, "
					  "CASE WHEN oauthToken IS NOT NULL THEN 'google' ELSE NULL END FROM importdb.auth")
			.exec())
		return false;

	if (!QueryBuilder::q(db)
			.addQuery("INSERT INTO game(username, timestamp, mapid, missionid, level, deathmatch, success, duration, mode) "
					  "SELECT username, timestamp, mapid, missionid, level, deathmatch, success, duration*1000, 1 FROM importdb.game")
			.exec())
		return false;

	if (!QueryBuilder::q(db)
			.addQuery("INSERT INTO score(username, xp) "
					  "SELECT username, SUM(xp) FROM importdb.score GROUP BY username")
			.exec())
		return false;

	if (!QueryBuilder::q(db).addQuery("DETACH importdb").exec())
		return false;


	LOG_CINFO("db") << "Import succesful";

	return true;
}

const QString &DatabaseMain::dbStatFile() const
{
	return m_dbStatFile;
}

void DatabaseMain::setDbStatFile(const QString &newDbStatFile)
{
	m_dbStatFile = newDbStatFile;
}


/**
 * @brief DatabaseMain::dbMapsFile
 * @return
 */

const QString &DatabaseMain::dbMapsFile() const
{
	return m_dbMapsFile;
}

void DatabaseMain::setDbMapsFile(const QString &newDbMapsFile)
{
	m_dbMapsFile = newDbMapsFile;
}
