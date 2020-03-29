/*
 * ---- Call of Suli ----
 *
 * cossql.cpp
 *
 * Created on: 2020. 03. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * CosSql
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  Call of Suli is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "cossql.h"

CosSql::CosSql(const QString &connectionName, QObject *parent)
	: QObject(parent)
{
	if (connectionName.isEmpty())
		m_db = QSqlDatabase::addDatabase("QSQLITE");
	else
		m_db = QSqlDatabase::addDatabase("QSQLITE", connectionName);

	m_dbCreated = false;

	qDebug() << "m_db" << m_db;
}

CosSql::~CosSql()
{
	if (m_db.isOpen()) {
		qDebug().noquote() << tr("Adatbázis bezárása: ")+m_db.databaseName();
	}
	qDebug() << "m_db" << m_db << "destroy";
}


/**
 * @brief CosSql::open
 * @param filename
 * @param create
 * @return
 */

bool CosSql::open(const QString &file, bool create)
{
	m_db.setDatabaseName(file);

	if (!QFile::exists(file)){
		if (!create) {
			qWarning().noquote() << tr("Az adatbázis nem létezik: ")+file;
			return false;
		}
		m_dbCreated = true;
	}

	if (!m_db.open()) {
		qWarning().noquote() << tr("Nem sikerült megnyitni az adatbázist: ")+file;
		return false;
	}

	m_db.exec("PRAGMA foreign_keys = ON");

	qDebug().noquote() << tr("Adatbázis megnyitva: ")+m_db.databaseName();

	return true;
}



/**
 * @brief CosSql::batchQuery
 * @param query
 * @return
 */

bool CosSql::batchQuery(const QString &query)
{
	QSqlQuery q(m_db);

	qDebug().noquote() << "SQL batch query ----------";

	QStringList list = query.split("\n\n", QString::SkipEmptyParts);
	foreach (QString cmd, list) {
		QString c = cmd.simplified();

		if (c.isEmpty())
			continue;

		qDebug().noquote() << c;
		if (!q.exec(c)) {
			qWarning().noquote() << tr("SQL error: ")+q.lastError().text();
			return false;
		}
	}

	qDebug().noquote() << "SQL batch query end ------";

	return true;
}


/**
 * @brief CosSql::batchQuery
 * @param query
 * @return
 */

bool CosSql::batchQueryFromFile(const QString &filename)
{
	qDebug().noquote() << tr("Batch sql query from file: ")+filename;

	QFile file(filename);

	if (!file.open(QFile::ReadOnly)) {
		qWarning().noquote() << tr("Read error: ")+file.fileName();
		return false;
	}

	QString t = QString::fromUtf8(file.readAll());

	file.close();

	return batchQuery(t);
}


/**
 * @brief CosSql::simpleQuery
 * @param query
 * @param args
 * @param lastInsertId
 * @return
 */

QSqlQuery CosSql::simpleQuery(QString query, const QVariantList &args)
{
	QSqlQuery q(m_db);
	QVariantList r;

	q.prepare(query);
	for (int i=0; i<args.count(); ++i) {
		q.addBindValue(args.at(i));
	}

	return q;
}


/**
 * @brief CosSql::insertQuery
 * @param query
 * @param map
 * @return
 */

QSqlQuery CosSql::insertQuery(QString query, const QVariantMap &map) const
{
	QStringList keys = map.keys();

	QStringList l;

	foreach (QString k, keys) {
		l << QString(":").append(k);
	}

	QSqlQuery q(m_db);
	q.prepare(query
			  .replace(QString("?k?"), keys.join(","))
			  .replace(QString("?"), l.join(","))
			  );

	QMapIterator<QString, QVariant> i(map);
	while (i.hasNext()) {
		i.next();
		q.bindValue(QString(":").append(i.key()), i.value());
	}

	return q;
}



/**
 * @brief CosSql::updateQuery
 * @param query
 * @param map
 * @return
 */

QSqlQuery CosSql::updateQuery(QString query, const QVariantMap &map, const QVariantMap &bindValues) const
{
	QStringList keys = map.keys();

	QStringList l;

	foreach (QString k, keys) {
		l << QString(k).append("=:").append(k);
	}

	QSqlQuery q(m_db);
	q.prepare(query.replace(QString("?"), l.join(",")));

	QMapIterator<QString, QVariant> i(map);
	while (i.hasNext()) {
		i.next();
		q.bindValue(QString(":").append(i.key()), i.value());
	}

	QStringList bindKeys = bindValues.keys();
	foreach (QString k, bindKeys) {
		q.bindValue(k, bindValues[k]);
	}

	return q;
}


/**
 * @brief CosSql::runQuery
 * @param query
 * @param lastInsertId
 * @return
 */

QVariantMap CosSql::runQuery(QSqlQuery query)
{
	bool success = true;

	QVariantMap r;

	if (!query.exec()) {
		QString errText = query.lastError().text();
		r["error"] = true;
		r["errorString"] = errText;

		qWarning().noquote() << tr("SQL error: ")+errText;
		success = false;
	}

	qDebug().noquote() << tr("SQL command: ")+query.executedQuery();

	if (!success) {
		return r;
	}

	r["error"] = false;
	r["lastInsertId"] = query.lastInsertId();

	QVariantList records;

	while (query.next()) {
		QSqlRecord rec = query.record();
		QVariantMap rr;

		for (int i=0; i<rec.count(); ++i) {
			QString key = rec.fieldName(i);
			if (key.isEmpty())
				key = QString("#key%1").arg(i);

			rr[key] = query.value(i);
		}
		records << rr;
	}

	r["records"] = records;

	return r;
}


/**
 * @brief CosSql::query
 * @param query
 * @return
 */

bool CosSql::execQuery(QSqlQuery query)
{
	QVariantMap r = runQuery(query);

	if (r["error"].toBool()) {
		qWarning() << tr("SQL query error: ")+r["errorString"].toString();
		return false;
	}

	return true;
}



/**
 * @brief CosSql::execSelectQuery
 * @param query
 * @param args
 * @param records
 * @return
 */

bool CosSql::execSelectQuery(QString query, const QVariantList &args, QVariantList *records)
{
	QVariantMap r = runSimpleQuery(query, args);

	if (r["error"].toBool()) {
		qWarning() << tr("SQL query error: ")+r["errorString"].toString();
		return false;
	}

	if (records) {
		foreach (QVariant q, r["records"].toList())
			(*records).append(q);
	}

	return true;
}


/**
 * @brief CosSql::execSelectQuery
 * @param query
 * @param args
 * @param records
 * @return
 */

bool CosSql::execSelectQuery(QString query, const QVariantList &args, QJsonArray *records)
{
	QVariantMap r = runSimpleQuery(query, args);

	if (r["error"].toBool()) {
		qWarning() << tr("SQL query error: ")+r["errorString"].toString();
		return false;
	}

	if (records) {
		foreach (QJsonValue q, r["records"].toJsonArray())
			(*records).append(q);
	}

	return true;
}


/**
 * @brief CosSql::execSelectQueryOneRow
 * @param query
 * @param args
 * @param records
 * @return
 */

bool CosSql::execSelectQueryOneRow(QString query, const QVariantList &args, QVariantMap *record)
{
	QVariantList list;

	if (!execSelectQuery(query, args, &list))
		return false;

	if (list.count()>1)
		return false;

	if (record && list.count()) {
		QVariantMap map = list.value(0).toMap();
		QStringList keys = map.keys();
		foreach (QString k, keys) {
			(*record)[k] = map.value(k);
		}
	}

	return true;
}


/**
 * @brief CosSql::execSelectQueryOneRow
 * @param query
 * @param args
 * @param record
 * @return
 */

bool CosSql::execSelectQueryOneRow(QString query, const QVariantList &args, QJsonObject *record)
{
	QVariantList list;

	if (!execSelectQuery(query, args, &list))
		return false;

	if (list.count()>1)
		return false;

	if (record && list.count()) {
		QJsonObject map = list.value(0).toJsonObject();
		QStringList keys = map.keys();
		foreach (QString k, keys) {
			(*record)[k] = map.value(k);
		}
	}

	return true;
}


/**
 * @brief CosSql::execInsertQuery
 * @param query
 * @param map
 * @return
 */

int CosSql::execInsertQuery(QString query, const QVariantMap &map)
{
	QSqlQuery q = insertQuery(query, map);
	QVariantMap m = runQuery(q);

	if (m["error"].toBool()) {
		qWarning() << tr("SQL query error: ")+m["errorString"].toString();
		return -1;
	}

	return m["lastInsertId"].toInt();
}



/**
 * @brief CosSql::hashPassword
 * @param password
 * @param salt
 * @return
 */

QString CosSql::hashPassword(const QString &password, QString *salt, QCryptographicHash::Algorithm method)
{
	QString _salt;
	if (salt && !salt->isEmpty()) {
		_salt = *salt;
	} else {
		_salt = QUuid::createUuid().toString(QUuid::Id128);
		if (salt)
			*salt = _salt;
	}

	QByteArray pwd;
	pwd.append(password).append(_salt);

	QByteArray hash = QCryptographicHash::hash(pwd, method);
	return QString::fromLatin1(hash.toHex());
}




