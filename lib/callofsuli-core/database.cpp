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
#include <QSqlRecord>


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
			delete m_mutex;
			m_mutex = nullptr;
		}

		ret.resolve();
	});

	QDefer::await(ret);

	delete m_worker;

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

	m_worker->execInThread([ret, this, path]() mutable {
		LOG_CDEBUG("db") << "Open database" << qPrintable(m_dbName) << "-" << qPrintable(path);

		QSqlDatabase db = QSqlDatabase::database(m_dbName);
		db.setDatabaseName(path);

		if (db.open()) {
			ret.resolve();
		} else {
			LOG_CERROR("db") << "Open database error" << qPrintable(m_dbName) << qPrintable(db.lastError().text());
			ret.reject();
		}
	});

	ret.done([&retValue](){
		retValue = true;
	}).fail([&retValue](){
		retValue = false;
	});;

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
		LOG_CDEBUG("db") << "Close database:" << qPrintable(m_dbName);

		QSqlDatabase::database(m_dbName).close();

		ret.resolve();
	});

	QDefer::await(ret);
}



/**
 * @brief Database::mutex
 * @return
 */

QRecursiveMutex *Database::mutex()
{
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
	bool err = false;
	const QByteArray &b = Utils::fileContent(filename, &err);

	if (err) {
		LOG_CWARNING("db") << "File read error:" << qPrintable(filename);
		return false;
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

	m_worker->execInThread([ret, this]() mutable {
		LOG_CDEBUG("db") << "Add database:" << qPrintable(m_dbName);

		m_mutex = new QRecursiveMutex;

		if (QSqlDatabase::contains(m_dbName)) {
			LOG_CERROR("db") << "Database already in use:" << qPrintable(m_dbName);
			ret.reject();
			return;
		}

		QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_dbName);

		if (!db.isValid()) {
			LOG_CERROR("db") << "Database invalid:" << qPrintable(m_dbName);
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
 * @brief Database::worker
 * @return
 */

QLambdaThreadWorker *Database::worker() const
{
	return m_worker;
}








/**
 * @brief QueryBuilder::exec
 * @return
 */

bool QueryBuilder::exec()
{
	QString q;

	auto bit = m_bind.constBegin();

	for (auto it = m_queryString.constBegin(), prev = m_queryString.constEnd(); it != m_queryString.constEnd(); prev=it, ++it) {
		switch (it->type) {
		case QueryString::Query:
			q += (it->text ? it->text : "");
			break;

		case QueryString::Bind:
			while (bit != m_bind.constEnd() && bit->type == Bind::Field)
				++bit;

			if (bit != m_bind.constEnd()) {
				if (prev != m_queryString.constEnd() && prev->type == QueryString::Bind) {
					if (bit->type == Bind::Positional)
						q += QStringLiteral(",?");
					else
						q += QStringLiteral(",").append(bit->name);
				} else {
					if (bit->type == Bind::Positional)
						q += QStringLiteral("?");
					else
						q += bit->name;
				}

				++bit;
			} else {
				LOG_CERROR("db") << "QueryBuilder error: invalid bind";
				return false;
			}
			break;

		case QueryString::FieldPlaceholder:
		{
			bool has = false;
			foreach (const Bind &b, m_bind) {
				if (b.type != Bind::Field)
					continue;

				if (has)	q += QStringLiteral(",");
				q += b.name;
				has = true;
			}
		}

			break;

		case QueryString::ValuePlaceholder:
		{
			bool has = false;
			foreach (const Bind &b, m_bind) {
				if (b.type != Bind::Field)
					continue;

				if (has)
					q += QStringLiteral(",?");
				else {
					q += QStringLiteral("?");
					has = true;
				}
			}
		}

			break;
		}
	}

	m_sqlQuery.prepare(q);

	foreach (const Bind &b, m_bind) {
		if (b.type == Bind::Positional || b.type == Bind::Field)
			m_sqlQuery.addBindValue(b.value);
		else
			m_sqlQuery.bindValue(b.name, b.value);
	}


	bool r = m_sqlQuery.exec();

	if (r)
		LOG_CTRACE("db") << "Sql query:" << qPrintable(m_sqlQuery.executedQuery().simplified());
	else {
		LOG_CTRACE("db") << "Sql query:" << qPrintable(m_sqlQuery.lastQuery().simplified());
		QUERY_LOG_ERROR(m_sqlQuery);
	}

	return r;
}


/**
 * @brief QueryBuilder::execToJsonArray
 * @param err
 * @return
 */


QJsonArray QueryBuilder::execToJsonArray(bool *err)
{
	if (!exec()) {
		if (err) *err = true;
		return QJsonArray();
	}

	QJsonArray list;

	while (m_sqlQuery.next()) {
		const QSqlRecord &rec = m_sqlQuery.record();
		QJsonObject obj;

		for (int i=0; i<rec.count(); ++i)
			obj.insert(rec.fieldName(i), rec.value(i).toJsonValue());

		list.append(obj);
	}

	return list;
}


/**
 * @brief QueryBuilder::execToJsonObject
 * @param err
 * @return
 */

QJsonObject QueryBuilder::execToJsonObject(bool *err)
{
	if (!exec()) {
		if (err) *err = true;
		return QJsonObject();
	}

	if (m_sqlQuery.size() > 1) {
		if (err) *err = true;
		LOG_CWARNING("db") << "More than one row returned";
		return QJsonObject();
	}

	QJsonObject obj;

	if (m_sqlQuery.first()) {
		const QSqlRecord &rec = m_sqlQuery.record();

		for (int i=0; i<rec.count(); ++i)
			obj.insert(rec.fieldName(i), rec.value(i).toJsonValue());
	}

	return obj;
}


/**
 * @brief QueryBuilder::sqlQuery
 * @return
 */

QSqlQuery &QueryBuilder::sqlQuery()
{
	return m_sqlQuery;
}