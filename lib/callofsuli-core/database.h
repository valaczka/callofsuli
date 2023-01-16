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

	QRecursiveMutex *mutex();

	void queryPrepareList(QSqlQuery *q, const QString &queryString, const QVariantList &list);

	// Methods starts with "_" must be called from m_thread

	bool _batchFromData(const QString &data);
	bool _batchFromFile(const QString &filename);

	QLambdaThreadWorker *worker() const;

protected:
	bool databaseInit();

	QLambdaThreadWorker *m_worker = nullptr;
	QString m_dbName;
	QRecursiveMutex *m_mutex = nullptr;
};



#define QUERY_LOG_ERROR(q)		LOG_CERROR("db") << "Sql error:" << qPrintable(q.lastError().text());
#define QUERY_LOG_WARNING(q)	LOG_CWARNING("db") << "Sql error:" << qPrintable(q.lastError().text());



/**
 * @brief The QueryBuilder class
 */


class QueryBuilder
{
private:

	struct QueryString {
		enum Type {
			Query,
			Bind,
			FieldPlaceholder,
			ValuePlaceholder
		};

		Type type = Query;
		const char *text = nullptr;

		QueryString() {}
		QueryString(const Type &t) : type(t) {}
		QueryString(const Type &t, const char *txt) : type(t), text(txt) {}
	};

	struct Bind {
		enum Type {
			Positional,
			Named,
			Field
		};

		Type type = Positional;
		QVariant value;
		const char *name = nullptr;

		Bind() {}
		Bind(const Type &t, const QVariant &v, const char *n) : type(t), value(v), name(n) {}
		Bind(const Type &t, const QVariant &v) : type(t), value(v) {}
	};

	QSqlQuery m_sqlQuery;
	QVector<QueryString> m_queryString;
	QVector<Bind> m_bind;

public:
	explicit QueryBuilder(QSqlDatabase db) : m_sqlQuery(db) {};

	static QueryBuilder q(QSqlDatabase db) { return QueryBuilder(db); }

	QueryBuilder &addQuery(const char *q) {
		m_queryString.append({QueryString::Query, q});
		return *this;
	}

	QueryBuilder &addValue(const QVariant &v) {
		m_queryString.append(QueryString::Bind);
		m_bind.append({Bind::Positional, v});
		return *this;
	}

	QueryBuilder &addValue(const char *name, const QVariant &v) {
		m_queryString.append(QueryString::Bind);
		m_bind.append({Bind::Named, v, name});
		return *this;
	}

	QueryBuilder &addNullValue() {
		m_queryString.append(QueryString::Bind);
		m_bind.append({Bind::Positional, QVariant::Invalid});
		return *this;
	}

	QueryBuilder &addNullValue(const char *name) {
		m_queryString.append(QueryString::Bind);
		m_bind.append({Bind::Positional, QVariant::Invalid, name});
		return *this;
	}

	QueryBuilder &setFieldPlaceholder() {
		m_queryString.append(QueryString::FieldPlaceholder);
		return *this;
	}

	QueryBuilder &setValuePlaceholder() {
		m_queryString.append(QueryString::ValuePlaceholder);
		return *this;
	}

	QueryBuilder &addField(const char *name, const QVariant &v) {
		m_bind.append({Bind::Field, v, name});
		return *this;
	}

	bool exec();
	QJsonArray execToJsonArray(bool *err = nullptr);
	QJsonObject execToJsonObject(bool *err = nullptr);

	void clear() {
		m_sqlQuery.clear();
		m_queryString.clear();
		m_bind.clear();
	}

	QSqlQuery &sqlQuery();

	QVariant value(const char *field) { return m_sqlQuery.value(field); }
	QVariant value(const char *field, const QVariant &defaultValue) {
		return m_sqlQuery.value(field).isNull() ? defaultValue : m_sqlQuery.value(field) ;
	}

	void logError() const { QUERY_LOG_ERROR(m_sqlQuery); }
	void logWarning() const { QUERY_LOG_WARNING(m_sqlQuery); }
};


#endif // DATABASE_H
