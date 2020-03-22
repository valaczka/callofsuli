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

CosSql::CosSql(QObject *parent)
	: QObject(parent)
{
	m_db = QSqlDatabase::addDatabase("QSQLITE");
	m_dbCreated = false;
}

CosSql::~CosSql()
{
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
			qWarning().noquote() << QString(tr("Az adatbázis (%1) nem létezik!")).arg(file);
			return false;
		}
		m_dbCreated = true;
	}

	if (!m_db.open()) {
		qWarning().noquote() << QString(tr("Nem sikerült megnyitni az adatbázist: %1")).arg(file);
		return false;
	}

	qDebug().noquote() << QString(tr("Adatbázis megnyitva: %1")).arg(m_db.databaseName());

	return true;
}


/**
 * @brief CosSql::simpleQuery
 * @param query
 * @param args
 * @return
 */

QVariantList CosSql::simpleQuery(const QString &query, const QVariantList &args, QVariant *lastInsertId)
{
	QSqlQuery q(m_db);
	QVariantList r;

	qDebug() << args;

	q.prepare(query);
	for (int i=0; i<args.count(); ++i) {
		q.addBindValue(args.at(i));
	}

	bool success = true;

	if (!q.exec()) {
		qWarning().noquote() << QString(tr("SQL error: %1")).arg(q.lastError().text());
		success = false;
	}

	qDebug().noquote() << QString(tr("SQL command: %1")).arg(q.executedQuery());

	if (!success) {
		return r;
	}

	if (lastInsertId) {
		*lastInsertId = q.lastInsertId();
	}

	if (q.first()) {
		QSqlRecord rec = q.record();

		for (int i=0; i<rec.count(); ++i) {
			r << q.value(i);
		}
	}

	return r;
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

	QStringList list = query.split(';', QString::SkipEmptyParts);
	foreach (QString cmd, list) {
		QString c = cmd.simplified();

		if (c.isEmpty())
			continue;

		qDebug().noquote() << c;
		if (!q.exec(c)) {
			qWarning().noquote() << QString(tr("SQL error: %1")).arg(q.lastError().text());
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
	qDebug().noquote() << QString(tr("Batch sql query from file: %1")).arg(filename);

	QFile file(filename);

	if (!file.open(QFile::ReadOnly)) {
		qWarning().noquote() << QString(tr("Read error: %1")).arg(file.fileName());
		return false;
	}

	QString t = QString::fromUtf8(file.readAll());

	file.close();

	return batchQuery(t);
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

QSqlQuery CosSql::updateQuery(QString query, const QVariantMap &map) const
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

	return q;
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




