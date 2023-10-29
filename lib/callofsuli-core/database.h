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
#include "utils.h"



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

	QRecursiveMutex *mutex() const;

	// Methods starts with "_" must be called from m_thread

	bool _batchFromData(const QString &data);
	bool _batchFromFile(const QString &filename);

	QLambdaThreadWorker *worker() const;


	// Database upgrade

	struct Upgrade
	{
		enum Type {
			UpgradeFromData,
			UpgradeFromFile
		};

		int fromVersionMajor = 0;
		int fromVersionMinor = 0;
		int toVersionMajor = 0;
		int toVersionMinor = 0;
		Type type = UpgradeFromData;
		QString content;
	};

	virtual bool performUpgrade(const QVector<Upgrade> &upgrades, const QString &defaultUpgradeData,
								const quint32 &fromVersionMajor, const quint32 &fromVersionMinor,
								const quint32 &toVersionMajor = Utils::versionMajor(),
								const quint32 &toVersionMinor = Utils::versionMinor());

protected:
	bool databaseInit();

	std::unique_ptr<QLambdaThreadWorker> m_worker;
	QString m_dbName;
	std::unique_ptr<QRecursiveMutex> m_mutex;
};



#define QUERY_LOG_ERROR(q)		LOG_CERROR("db") << "Sql error:" << qPrintable(q.lastError().text());
#define QUERY_LOG_WARNING(q)	LOG_CWARNING("db") << "Sql error:" << qPrintable(q.lastError().text());



typedef std::function<QJsonValue(const QVariant&)> FieldConvertFunc;


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
			ValuePlaceholder,
			CombinedPlaceholder
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
			Field,
			List
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

	QueryBuilder &addList(const QVariantList &v) {
		m_queryString.append(QueryString::Bind);
		m_bind.append({Bind::List, v});
		return *this;
	}

	template <typename T>
	QueryBuilder &addNullValue() {
		m_queryString.append(QueryString::Bind);
		m_bind.append({Bind::Positional, QVariant(QMetaType::fromType<T>())});
		return *this;
	}

	template <typename T>
	QueryBuilder &addNullValue(const char *name) {
		m_queryString.append(QueryString::Bind);
		m_bind.append({Bind::Positional, QVariant(QMetaType::fromType<T>()), name});
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

	QueryBuilder &setCombinedPlaceholder() {
		m_queryString.append(QueryString::CombinedPlaceholder);
		return *this;
	}

	QueryBuilder &addField(const char *name, const QVariant &v) {
		m_bind.append({Bind::Field, v, name});
		return *this;
	}

	bool exec();
	std::optional<QJsonArray> execToJsonArray();
	std::optional<QJsonObject> execToJsonObject();
	std::optional<bool> execCheckExists();
	std::optional<QVariant> execInsert();
	std::optional<int> execInsertAsInt();

	std::optional<QJsonArray> execToJsonArray(const QMap<QString, FieldConvertFunc> &map);
	std::optional<QJsonObject> execToJsonObject(const QMap<QString, FieldConvertFunc> &map);
	std::optional<QVariant> execToValue(const char *field);
	std::optional<QVariant> execToValue(const char *field, const QVariant &defaultValue);

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

	int fieldCount() const;

	void logError() const { QUERY_LOG_ERROR(m_sqlQuery); }
	void logWarning() const { QUERY_LOG_WARNING(m_sqlQuery); }
};



#endif // DATABASE_H
