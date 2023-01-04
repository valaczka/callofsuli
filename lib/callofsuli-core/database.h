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


#include "Logger.h"
#include <QDeferred>
#include <QLambdaThreadWorker>
#include <QSqlDatabase>
#include <QSqlError>
#include <QObject>
#include <QSqlQuery>


/**
 * @brief The DatabaseMutexObject class
 */

class DatabaseMutexObject : public QObject
{
	Q_OBJECT

public:
	DatabaseMutexObject(QObject *parent = nullptr);
	virtual ~DatabaseMutexObject();

	QMutex &mutex();

private:
	QMutex m_mutex;
};


/**
 * @brief The Database class
 */


class Database
{
public:
	Database(const QString &dbName);
	virtual ~Database();

	const QString &dbName() const;

	virtual bool databaseOpen(const QString &path);
	virtual void databaseClose();

	QMutex *mutex();

	void queryPrepareList(QSqlQuery *q, const QString &queryString, const QVariantList &list);

	// Methods starts with "_" must be called from m_thread

	QSqlError _batchFromData(const QString &data);
	QSqlError _batchFromFile(const QString &filename);

protected:
	bool databaseInit();

	QLambdaThreadWorker m_worker;
	QString m_dbName;
	//DatabaseMutexObject *m_mutexObject = nullptr;
	QMutex *m_mutex = nullptr;
};


/**
 * @brief The QueryBuilder class
 */


class QueryBuilder : public QSqlQuery
{
public:
	explicit QueryBuilder(QSqlDatabase db) : QSqlQuery(db) {}

	QueryBuilder &addQuery(const char *q) {
		m_query.append(q);
		m_beforeIsValue = false;
		return *this;
	}

	QueryBuilder &addValue(const QVariant &v) {
		if (m_beforeIsValue)
			m_query.append(QStringLiteral(",?"));
		else
			m_query.append(QStringLiteral("?"));
		addBindValue(v);
		m_beforeIsValue = true;
		return *this;
	}

	QueryBuilder &addValue(const char *name, const QVariant &v) {
		if (m_beforeIsValue)
			m_query.append(QStringLiteral(","));

		m_query.append(name);
		bindValue(name, v);
		m_beforeIsValue = true;
		return *this;
	}

	QueryBuilder &addNullValue() {
		if (m_beforeIsValue)
			m_query.append(QStringLiteral(",?"));
		else
			m_query.append(QStringLiteral("?"));
		addBindValue(QVariant::Invalid);
		m_beforeIsValue = true;
		return *this;
	}

	bool exec() {
		prepare(m_query);
		bool r = QSqlQuery::exec();
		LOG_CTRACE("db") << "Executed query" << executedQuery();
		return r;
	}

private:
	QString m_query;
	bool m_beforeIsValue = false;
};

#define QUERY_LOG_ERROR(q)		LOG_CERROR("db") << "Sql error:" << qPrintable(q.lastError().text());
#define QUERY_LOG_WARNING(q)	LOG_WARNING("db") << "Sql error:" << qPrintable(q.lastError().text());

#endif // DATABASE_H
