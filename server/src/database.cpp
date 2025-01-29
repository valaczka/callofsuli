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
#include "utils_.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include "querybuilder.hpp"


/**
 * @brief Database::Database
 */

Database::Database(const QString &dbName)
	: m_worker(new QLambdaThreadWorker())
	, m_dbName(dbName)
{
	Q_ASSERT(!dbName.isEmpty());

	LOG_CTRACE("db") << "Database created:" << qPrintable(m_dbName);

	if (!databaseInit())
		LOG_CFATAL("db") << "Can't create database:" << qPrintable(m_dbName);
}


/**
 * @brief Database::~Database
 */

Database::~Database()
{
	LOG_CTRACE("db") << "Remove database:" << qPrintable(m_dbName);

	QDefer ret;

	m_worker->execInThread([ret, this]() mutable {
		{
			QSqlDatabase db = QSqlDatabase::database(m_dbName);
			if (db.isOpen())
				db.close();
		}

		QSqlDatabase::removeDatabase(m_dbName);

		if (m_mutex) {
			m_mutex->unlock();
		}

		ret.resolve();
	});

	QDefer::await(ret);

	LOG_CTRACE("db") << "Database destroyed:" << qPrintable(m_dbName);
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

bool Database::databaseOpen(const QString &path)
{
	QDefer ret;
	bool retValue = false;

	m_worker->execInThread([ret, this, path, &retValue]() mutable {
		LOG_CDEBUG("db") << "Open database" << qPrintable(m_dbName) << "-" << qPrintable(path);

		QSqlDatabase db = QSqlDatabase::database(m_dbName);
		db.setDatabaseName(path);

		if (db.open()) {
			QSqlQuery q(db);
			q.exec(QStringLiteral("PRAGMA foreign_keys = ON"));
			retValue = true;
			ret.resolve();
		} else {
			LOG_CERROR("db") << "Open database error" << qPrintable(m_dbName) << qPrintable(db.lastError().text());
			retValue = false;
			ret.reject();
		}
	});


	QDefer::await(ret);

	LOG_CTRACE("db") << "Database opened:" << qPrintable(m_dbName);

	return retValue;
}


/**
 * @brief Database::databaseClose
 */

void Database::databaseClose()
{
	QDefer ret;

	m_worker->execInThread([ret, this]() mutable {
		auto db = QSqlDatabase::database(m_dbName);

		if (db.isOpen()) {
			LOG_CDEBUG("db") << "Close database:" << qPrintable(m_dbName);
			QSqlDatabase::database(m_dbName).close();
		}

		ret.resolve();
	});

	QDefer::await(ret);
}



/**
 * @brief Database::mutex
 * @return
 */

QRecursiveMutex *Database::mutex() const
{
	return m_mutex.get();
}




/**
 * @brief Database::batchFromData
 * @param data
 * @return
 */

bool Database::_batchFromData(const QString &data)
{
	LOG_CTRACE("db") << "Batch sql data begin -------------";

	QMutexLocker mutexlocker(mutex());

	QSqlDatabase db = QSqlDatabase::database(m_dbName);

	QStringList list = data.split(QStringLiteral("\n\n"), Qt::SkipEmptyParts);

	foreach (const QString &s, list) {
		QString cmd = s.simplified();
		if (cmd.isEmpty())
			continue;

		if (cmd.startsWith(QStringLiteral("---")))
			continue;

		if (!QueryBuilder::q(db).addQuery(s.toUtf8()).exec()) {
			return false;
		}
	}

	LOG_CTRACE("db") << "Batch sql data success -----------";

	return true;
}


/**
 * @brief Database::batchFromFile
 * @param filename
 * @return
 */

bool Database::_batchFromFile(const QString &filename)
{
	const auto &b = Utils::fileContent(filename);

	if (!b) {
		LOG_CWARNING("db") << "File read error:" << qPrintable(filename);
		return false;
	}

	return _batchFromData(QString::fromUtf8(b.value()));
}



/**
 * @brief Database::databaseInit
 * @return
 */

bool Database::databaseInit()
{
	QDefer ret;
	bool retValue = false;

	m_worker->execInThread([ret, this, &retValue]() mutable {
		LOG_CDEBUG("db") << "Add database:" << qPrintable(m_dbName);

		m_mutex = std::make_unique<QRecursiveMutex>();

		if (QSqlDatabase::contains(m_dbName)) {
			LOG_CERROR("db") << "Database already in use:" << qPrintable(m_dbName);
			ret.reject();
			return;
		}

		QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_dbName);

		if (!db.isValid()) {
			LOG_CERROR("db") << "Database invalid:" << qPrintable(m_dbName);
			retValue = false;
			ret.reject();
		} else {
			retValue = true;
			ret.resolve();
		}
	});

	QDefer::await(ret);

	LOG_CTRACE("db") << "Database created:" << m_dbName  << QThread::currentThread();

	return retValue;
}


/**
 * @brief Database::worker
 * @return
 */

QLambdaThreadWorker *Database::worker() const
{
	return m_worker.get();
}



/**
 * @brief Database::performUpgrade
 * @param toVersionMajor
 * @param toVersionMinor
 * @return
 */

bool Database::performUpgrade(const QVector<Upgrade> &upgrades,
							  const QString &defaultUpgradeData,
							  const quint32 &fromVersionMajor, const quint32 &fromVersionMinor,
							  const quint32 &toVersionMajor, const quint32 &toVersionMinor)
{
	LOG_CINFO("db") << qPrintable(QObject::tr("Upgrade database %1 v%2.%3 -> v%4.%5")
								  .arg(m_dbName)
								  .arg(fromVersionMajor).arg(fromVersionMinor)
								  .arg(toVersionMajor).arg(toVersionMinor)
								  );

	quint32 version = Utils::versionCode(fromVersionMajor, fromVersionMinor);
	const quint32 toVersion = Utils::versionCode(toVersionMajor, toVersionMinor);

	QMutexLocker mutexlocker(mutex());

	QSqlDatabase db = QSqlDatabase::database(m_dbName);

	////db.transaction();

	while (version < toVersion) {
		quint32 nextVersion = 0;

		foreach (const Upgrade &u, upgrades) {
			const quint32 from = Utils::versionCode(u.fromVersionMajor, u.fromVersionMinor);
			const quint32 to = Utils::versionCode(u.toVersionMajor, u.toVersionMinor);

			if (from < version || (nextVersion != 0 && to > nextVersion))
				continue;

			LOG_CDEBUG("db") << "- upgrade:" << from << "->" << to;

			if (nextVersion == 0 || to < nextVersion)
				nextVersion = to;

			switch (u.type) {
				case Upgrade::UpgradeFromData:
					if (!_batchFromData(u.content)) {
						///db.rollback();
						return false;
					}
					break;

				case Upgrade::UpgradeFromFile:
					if (!_batchFromFile(u.content)) {
						///db.rollback();
						return false;
					}
					break;
			}
		}

		if (nextVersion == 0 || nextVersion == toVersion) {
			version = toVersion;

			LOG_CDEBUG("db") << "- default upgrade";

			if (!defaultUpgradeData.isEmpty() && !_batchFromData(defaultUpgradeData)) {
				///db.rollback();
				return false;
			}
		} else {
			version = nextVersion;
		}
	};

	////db.commit();

	LOG_CINFO("db") << qPrintable(QObject::tr("Upgrade database %1 successfull").arg(m_dbName));

	return true;
}






