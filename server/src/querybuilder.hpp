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

#ifndef QUERYBUILDER_H
#define QUERYBUILDER_H


#include "qjsonarray.h"
#include "qjsonobject.h"
#include "qsqlrecord.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QObject>
#include <QSqlQuery>



#ifndef DB_LOG_ERROR
#   include "Logger.h"
#   define DB_LOG_ERROR()   LOG_CERROR("db")
#endif

#ifndef DB_LOG_WARNING
#   include "Logger.h"
#   define DB_LOG_WARNING()   LOG_CWARNING("db")
#endif

#ifndef DB_LOG_DEBUG
#   include "Logger.h"
#   define DB_LOG_DEBUG()   LOG_CDEBUG("db")
#endif

#ifndef DB_LOG_TRACE
#   include "Logger.h"
#   define DB_LOG_TRACE()   LOG_CTRACE("db")
#endif


#define QUERY_LOG_ERROR(q)		DB_LOG_ERROR() << "Sql error:" << qPrintable(q.lastError().text());
#define QUERY_LOG_WARNING(q)	DB_LOG_WARNING() << "Sql error:" << qPrintable(q.lastError().text());



typedef std::function<QJsonValue(const QVariant&)> FieldConvertFunc;
typedef std::function<QVariant(const QVariant&)> FieldConvertVariantFunc;


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
		QByteArray name;

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

	template <typename T>
	QueryBuilder &addNullField(const char *name) {
		m_bind.append({Bind::Field, QVariant(QMetaType::fromType<T>()), name});
		return *this;
	}

	bool exec();
	std::optional<QJsonArray> execToJsonArray();
	std::optional<QJsonObject> execToJsonObject();
	bool execCheckExists();
	std::optional<QVariant> execInsert();
	std::optional<int> execInsertAsInt();

	std::optional<QJsonArray> execToJsonArray(const QMap<QString, FieldConvertFunc> &map);
	std::optional<QJsonObject> execToJsonObject(const QMap<QString, FieldConvertFunc> &map);
	std::optional<QVariantList> execToVariantList(const QMap<QString, FieldConvertVariantFunc> &map);
	std::optional<QVariant> execToValue(const char *field);
	std::optional<QVariant> execToValue(const char *field, const QVariant &defaultValue);

	void clear() {
		m_sqlQuery.clear();
		m_queryString.clear();
		m_bind.clear();
	}

	QSqlQuery &sqlQuery() { return m_sqlQuery; }

	QVariant value(const char *field) { return m_sqlQuery.value(field); }
	QVariant value(const char *field, const QVariant &defaultValue) {
		return m_sqlQuery.value(field).isNull() ? defaultValue : m_sqlQuery.value(field) ;
	}

	int fieldCount() const;

	void logError() const { QUERY_LOG_ERROR(m_sqlQuery); }
	void logWarning() const { QUERY_LOG_WARNING(m_sqlQuery); }
};





inline bool QueryBuilder::exec()
{
	QByteArray q;

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
							q += QByteArrayLiteral(",?");
						else if (bit->type == Bind::List)
							q += QByteArrayLiteral(",?").repeated(bit->value.toList().size());
						else if (bit->type == Bind::Named)
							q += QByteArrayLiteral(",").append(bit->name);
					} else {
						if (bit->type == Bind::Positional)
							q += QByteArrayLiteral("?");
						else if (bit->type == Bind::List)
							q += QByteArrayLiteral("?")+QByteArrayLiteral(",?").repeated(bit->value.toList().size()-1);
						else if (bit->type == Bind::Named)
							q += bit->name;
					}

					++bit;
				} else {
					DB_LOG_ERROR() << "QueryBuilder error: invalid bind";
					return false;
				}
				break;

			case QueryString::FieldPlaceholder:
			{
				bool has = false;
				foreach (const Bind &b, m_bind) {
					if (b.type != Bind::Field)
						continue;

					if (has)	q += QByteArrayLiteral(",");
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
						q += QByteArrayLiteral(",?");
					else {
						q += QByteArrayLiteral("?");
						has = true;
					}
				}
			}

				break;

			case QueryString::CombinedPlaceholder:
			{
				bool has = false;
				foreach (const Bind &b, m_bind) {
					if (b.type != Bind::Field)
						continue;

					if (has)
						q += QByteArrayLiteral(",")+b.name+QByteArrayLiteral("=?");
					else {
						q += b.name+QByteArrayLiteral("=?");
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
		else if (b.type == Bind::List) {
			foreach (const QVariant &v, b.value.toList())
				m_sqlQuery.addBindValue(v);
		}else
			m_sqlQuery.bindValue(b.name, b.value);
	}


	bool r = m_sqlQuery.exec();

	if (r)
		DB_LOG_TRACE() << "Sql query:" << qPrintable(m_sqlQuery.executedQuery().simplified());
	else {
		DB_LOG_TRACE() << "Sql query:" << qPrintable(m_sqlQuery.lastQuery().simplified());
		QUERY_LOG_ERROR(m_sqlQuery);
	}

	return r;
}




inline std::optional<QJsonArray> QueryBuilder::execToJsonArray()
{
	if (!exec())
		return std::nullopt;

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




inline std::optional<QJsonObject> QueryBuilder::execToJsonObject()
{
	if (!exec())
		return std::nullopt;

	if (m_sqlQuery.size() > 1) {
		DB_LOG_WARNING() << "More than one row returned";
		return std::nullopt;
	}

	QJsonObject obj;

	if (m_sqlQuery.first()) {
		const QSqlRecord &rec = m_sqlQuery.record();

		for (int i=0; i<rec.count(); ++i)
			obj.insert(rec.fieldName(i), rec.value(i).toJsonValue());
	}

	return obj;
}




inline bool QueryBuilder::execCheckExists()
{
	if (!exec()) return false;

	if (!m_sqlQuery.first())
		return false;

	return true;
}




inline std::optional<QVariant> QueryBuilder::execInsert()
{
	if (!exec())
		return std::nullopt;

	return m_sqlQuery.lastInsertId();
}




inline std::optional<int> QueryBuilder::execInsertAsInt()
{
	const std::optional<QVariant> &v = execInsert();

	if (v)
		return v.value().toInt();
	else
		return std::nullopt;
}




inline std::optional<QJsonArray> QueryBuilder::execToJsonArray(const QMap<QString, FieldConvertFunc> &map)
{
	if (!exec()) return std::nullopt;

	QJsonArray list;

	while (m_sqlQuery.next()) {
		const QSqlRecord &rec = m_sqlQuery.record();
		QJsonObject obj;

		for (int i=0; i<rec.count(); ++i) {
			const QString &f = rec.fieldName(i);
			if (map.contains(f)) {
				const FieldConvertFunc &func = map.value(f);
				const QJsonValue &v = std::invoke(func, rec.value(i));
				if (!v.isNull()) obj.insert(f, v);
			} else {
				obj.insert(rec.fieldName(i), rec.value(i).toJsonValue());
			}
		}

		list.append(obj);
	}

	return list;
}



inline std::optional<QJsonObject> QueryBuilder::execToJsonObject(const QMap<QString, FieldConvertFunc> &map)
{
	if (!exec()) return std::nullopt;

	if (m_sqlQuery.size() > 1) {
		DB_LOG_WARNING() << "More than one row returned";
		return std::nullopt;
	}

	QJsonObject obj;

	if (m_sqlQuery.first()) {
		const QSqlRecord &rec = m_sqlQuery.record();

		for (int i=0; i<rec.count(); ++i) {
			const QString &f = rec.fieldName(i);
			if (map.contains(f)) {
				const FieldConvertFunc &func = map.value(f);
				const QJsonValue &v = std::invoke(func, rec.value(i));
				if (!v.isNull()) obj.insert(f, v);
			} else {
				obj.insert(rec.fieldName(i), rec.value(i).toJsonValue());
			}
		}
	}

	return obj;
}



/**
 * @brief QueryBuilder::execToVariantList
 * @param map
 * @return
 */

inline std::optional<QVariantList> QueryBuilder::execToVariantList(const QMap<QString, FieldConvertVariantFunc> &map)
{
	if (!exec()) return std::nullopt;

	QVariantList list;

	while (m_sqlQuery.next()) {
		const QSqlRecord &rec = m_sqlQuery.record();
		QVariantMap obj;

		for (int i=0; i<rec.count(); ++i) {
			const QString &f = rec.fieldName(i);
			if (map.contains(f)) {
				const FieldConvertVariantFunc &func = map.value(f);
				const QVariant &v = std::invoke(func, rec.value(i));
				if (!v.isNull()) obj.insert(f, v);
			} else {
				obj.insert(rec.fieldName(i), rec.value(i).toJsonValue());
			}
		}

		list.append(obj);
	}

	return list;
}



inline std::optional<QVariant> QueryBuilder::execToValue(const char *field)
{
	if (!exec()) return std::nullopt;

	if (m_sqlQuery.size() > 1) {
		DB_LOG_WARNING() << "More than one row returned";
		return std::nullopt;
	}

	if (m_sqlQuery.first())
		return value(field);
	else
		return std::nullopt;

}



inline std::optional<QVariant> QueryBuilder::execToValue(const char *field, const QVariant &defaultValue)
{
	if (!exec()) return std::nullopt;

	if (m_sqlQuery.size() > 1) {
		DB_LOG_WARNING() << "More than one row returned";
		return std::nullopt;
	}

	if (m_sqlQuery.first())
		return value(field, defaultValue);
	else
		return defaultValue;
}



inline int QueryBuilder::fieldCount() const
{
	int r = 0;

	foreach (const Bind &b, m_bind)
		if (b.type == Bind::Field)
			++r;

	return r;
}



#endif // QUERYBUILDER_H
