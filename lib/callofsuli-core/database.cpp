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
#include "utils.h"
#include <QSqlQuery>


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

	QDefer ret;

	m_worker.execInThread([ret, this]() mutable {
		{
			QSqlDatabase db = QSqlDatabase::database(m_dbName);
			if (db.isOpen())
				db.close();
		}

		QSqlDatabase::removeDatabase(m_dbName);

		if (m_mutex) {
			m_mutex->unlock();
			delete m_mutex;
			m_mutex = nullptr;
		}

		ret.resolve();
	});

	QDefer::await(ret);

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

bool Database::databaseOpen(const QString &path)
{
	QDefer ret;
	bool retValue = false;

	m_worker.execInThread([ret, this, path]() mutable {
		LOG_CDEBUG("db") << "Open database" << m_dbName << ": " << path  << QThread::currentThread();

		QSqlDatabase db = QSqlDatabase::database(m_dbName);
		db.setDatabaseName(path);

		if (db.open()) {
			ret.resolve();
		} else {
			LOG_CERROR("db") << "Open database error" << m_dbName << qPrintable(db.lastError().text());
			ret.reject();
		}
	});

	ret.done([&retValue](){
		retValue = true;
	}).fail([&retValue](){
		retValue = false;
	});;

	QDefer::await(ret);

	LOG_CTRACE("db") << "Database opened:" << m_dbName;

	return retValue;
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
 * @brief Database::mutex
 * @return
 */

QMutex *Database::mutex()
{
	/*Q_ASSERT(m_mutexObject);

	rturn m_mutexObject->mutex();*/

	return m_mutex;
}


/**
 * @brief Database::queryPrepare
 * @param q
 * @param queryString
 */

void Database::queryPrepareList(QSqlQuery *q, const QString &queryString, const QVariantList &list)
{
	q->prepare(queryString);

	foreach (const QVariant &v, list)
		q->addBindValue(v);
}


/**
 * @brief Database::batchFromData
 * @param data
 * @return
 */

QSqlError Database::_batchFromData(const QString &data)
{

	LOG_CDEBUG("db") << "Batch sql data begin -------------" << QThread::currentThread();

	//QMutexLocker mutexlocker(mutex());

	QSqlDatabase db = QSqlDatabase::database(m_dbName);

	LOG_CDEBUG("db") << "DB" << db;

	db.transaction();

	QStringList list = data.split(QStringLiteral("\n\n"), Qt::SkipEmptyParts);

	foreach (const QString &s, list) {
		QString cmd = s.simplified();
		if (cmd.isEmpty())
			continue;

		if (cmd.startsWith(QStringLiteral("---")))
			continue;

		QSqlQuery q(db);

		LOG_CTRACE("db") << qPrintable(cmd);

		if (!q.exec(s)) {
			QUERY_LOG_ERROR(q);
			const QSqlError &err = q.lastError();

			db.rollback();

			return err;
		}
	}

	db.commit();

	LOG_CDEBUG("db") << "Batch sql data success -----------";

	return QSqlError();
}


/**
 * @brief Database::batchFromFile
 * @param filename
 * @return
 */

QSqlError Database::_batchFromFile(const QString &filename)
{
	bool err = false;
	const QByteArray &b = Utils::fileContent(filename, &err);

	if (err) {
		LOG_CWARNING("db") << "File read error:" << qPrintable(filename);
		return QSqlError(QString(), QString(), QSqlError::UnknownError);
	}

	return _batchFromData(QString::fromUtf8(b));
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
		LOG_CDEBUG("db") << "Add database:" << m_dbName  << QThread::currentThread();

		m_mutex = new QMutex;

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

	LOG_CTRACE("db") << "Database created:" << m_dbName  << QThread::currentThread();

	return retValue;
}



/**
 * @brief DatabaseMutexObject::DatabaseMutexObject
 * @param parent
 */

DatabaseMutexObject::DatabaseMutexObject(QObject *parent)
	: QObject(parent)
{
	LOG_CTRACE("db") << "Mutex object created:" << this;
}


/**
 * @brief DatabaseMutexObject::~DatabaseMutexObject
 */

DatabaseMutexObject::~DatabaseMutexObject()
{
	LOG_CTRACE("db") << "Mutex object destroyed:" << this;
}


/**
 * @brief DatabaseMutexObject::mutex
 * @return
 */

QMutex &DatabaseMutexObject::mutex()
{
	return m_mutex;
}
