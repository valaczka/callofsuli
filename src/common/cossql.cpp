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
	m_canUndo = -1;

	qDebug() << "m_db" << m_db;
}

CosSql::~CosSql()
{
	if (m_db.isOpen()) {
		qDebug().noquote() << tr("Adatbázis bezárása: ")+m_db.databaseName();
		m_db.close();
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
		qWarning().noquote() << m_db.lastError();
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
		q.bindValue(k, bindValues.value(k));
	}

	return q;
}


/**
 * @brief CosSql::listQuery
 * @param query
 * @param map
 * @param bindValues
 * @return
 */

QSqlQuery CosSql::listQuery(QString query, const QVariantList &list, const QVariantMap &bindValues, const bool &parenthesizeValues) const
{
	QStringList l;

	QSqlQuery q(m_db);

	for (int i=0; i<list.count(); ++i) {
		QString key = QString(":l%1").arg(i);
		l << (parenthesizeValues ? "("+key+")" : key);
	}

	q.prepare(query.replace(QString("?l?"), l.join(",")));

	for (int i=0; i<list.count(); ++i) {
		QString key = QString(":l%1").arg(i);
		q.bindValue(key, list.at(i));
	}

	QStringList bindKeys = bindValues.keys();
	foreach (QString k, bindKeys) {
		q.bindValue(k, bindValues.value(k));
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

	if (r.value("error").toBool()) {
		qWarning() << tr("SQL query error: ")+r.value("errorString").toString();
		return false;
	}

	return true;
}


/**
 * @brief CosSql::execBatchQuery
 * @param query
 * @param args
 * @return
 */

bool CosSql::execBatchQuery(QString query, const QVariantList &list)
{
	m_db.transaction();

	QSqlQuery q(m_db);
	q.prepare(query);

	foreach (QVariant v, list) {
		q.addBindValue(v.toList());
	}

	if (!q.execBatch()) {
		QString errText = q.lastError().text();
		qWarning().noquote() << tr("SQL error: ")+errText+": "+q.executedQuery();
		m_db.rollback();
		return false;
	}

	m_db.commit();

	qDebug().noquote() << tr("SQL command: ")+q.executedQuery();
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

	if (r.value("error").toBool()) {
		qWarning() << tr("SQL query error: ")+r.value("errorString").toString();
		return false;
	}

	if (records) {
		foreach (QVariant q, r.value("records").toList())
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

	if (r.value("error").toBool()) {
		qWarning() << tr("SQL query error: ")+r.value("errorString").toString();
		return false;
	}

	if (records) {
		foreach (QJsonValue q, r.value("records").toJsonArray())
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

	if (list.count() != 1)
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

	if (m.value("error").toBool()) {
		qWarning() << tr("SQL query error: ")+m.value("errorString").toString();
		return -1;
	}

	return m.value("lastInsertId").toInt();
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


/**
 * @brief CosSql::createUndoTable
 * @return
 */

bool CosSql::createUndoTables()
{
	if (!execSimpleQuery("CREATE TABLE undoSettings(lastStep INTEGER, active BOOL)"))
		return false;

	if (!execSimpleQuery("INSERT INTO undoSettings(lastStep, active) VALUES(0, false)"))
		return false;

	if (!execSimpleQuery("CREATE TABLE undoStep(id INTEGER PRIMARY KEY, desc TEXT)"))
		return false;

	if (!execSimpleQuery("CREATE TABLE undoLog(seq INTEGER PRIMARY KEY, stepid INTEGER NOT NULL REFERENCES undoStep(id) ON UPDATE CASCADE ON DELETE CASCADE, cmd TEXT)"))
		return false;

	return true;
}

/**
  DELETE FROM undoStep WHERE id>lastStep
  INSERT INTO undoStep(desc) VALUES(???)

  **/

/**
 * @brief CosSql::createTrigger
 * @param table
 * @return
 */

bool CosSql::createTrigger(const QString &table)
{

	QString cmd = "CREATE TRIGGER _"+table+"_it AFTER INSERT ON "+table+" WHEN 1=(SELECT active FROM undoSettings) BEGIN\n";
	cmd += "INSERT INTO undoLog(stepid, cmd) VALUES((SELECT MAX(id) FROM undoStep), \n";
	cmd += "'DELETE FROM "+table+" WHERE rowid='||new.rowid);\n";
	cmd += "END;";

	if (!execSimpleQuery(cmd))
		return false;


	QVariantList l;
	l << table;
	QVariantList list;

	if (!execSelectQuery("SELECT name from pragma_table_info(?)", l, &list))
		return false;


	QStringList p;
	foreach (QVariant v, list) {
		QString field = v.toMap().value("name").toString();
		p << field+"='||quote(old."+field+")||'";

	}
	QString cmd2 = "CREATE TRIGGER _"+table+"_ut AFTER UPDATE ON "+table+" WHEN 1=(SELECT active FROM undoSettings) BEGIN\n";
	cmd2 += "INSERT INTO undoLog(stepid, cmd) VALUES((SELECT MAX(id) FROM undoStep), \n";
	cmd2 += "'UPDATE "+table+" SET "+p.join(",")+" WHERE rowid='||old.rowid);\n";
	cmd2 += "END;";

	if (!execSimpleQuery(cmd2))
		return false;



	QStringList p2, p3;
	p2 << "rowid";
	p3 << "'||old.rowid||'";
	foreach (QVariant v, list) {
		QString field = v.toMap().value("name").toString();
		p2 << field;
		p3 << "'||quote(old."+field+")||'";
	}
	QString cmd3 = "CREATE TRIGGER _"+table+"_dt BEFORE DELETE ON "+table+" WHEN 1=(SELECT active FROM undoSettings) BEGIN\n";
	cmd3 += "INSERT INTO undoLog(stepid, cmd) VALUES((SELECT MAX(id) FROM undoStep), \n";
	cmd3 += "'INSERT INTO "+table+"("+p2.join(",")+") VALUES ("+p3.join(",")+")');\n";
	cmd3 += "END;";

	if (!execSimpleQuery(cmd3))
		return false;

	return true;
}


/**
 * @brief CosSql::undoLogBegin
 * @param desc
 * @return
 */

void CosSql::undoLogBegin(const QString &desc)
{
	m_db.transaction();

	execSimpleQuery("DELETE FROM undoStep WHERE id>(SELECT lastStep FROM undoSettings)");

	QVariantMap m;
	m["desc"] = desc;
	int stepid = execInsertQuery("INSERT INTO undoStep(?k?) VALUES(?)", m);
	QVariantList l;
	l << stepid;
	execSimpleQuery("UPDATE undoSettings SET lastStep=?, active=true", l);

	m_db.commit();
}



/**
 * @brief CosSql::undoLogEnd
 * @return
 */

void CosSql::undoLogEnd()
{
	execSimpleQuery("UPDATE undoSettings SET active=false");

	QVariantMap r;
	execSelectQueryOneRow("SELECT COALESCE(MAX(id),-1) as id FROM undoStep", QVariantList(), &r);
	setCanUndo(r.value("id",-1).toInt());
}


/**
 * @brief CosSql::undoStack
 * @return
 */

QVariantMap CosSql::undoStack()
{
	QVariantMap ret;

	m_db.transaction();

	execSelectQueryOneRow("SELECT lastStep FROM undoSettings", QVariantList(), &ret);
	execSelectQueryOneRow("SELECT COALESCE(MIN(id),-1) as minStep FROM undoStep", QVariantList(), &ret);
	execSelectQueryOneRow("SELECT COALESCE(MAX(id),-1) as maxStep FROM undoStep", QVariantList(), &ret);

	QVariantList l;
	execSelectQuery("SELECT id, desc FROM undoStep ORDER BY id DESC", QVariantList(), &l);
	ret["steps"] = l;

	m_db.commit();

	return ret;
}




/**
 * @brief CosSql::undo
 * @param step
 */

void CosSql::undo(const int &floor)
{
	m_db.transaction();

	QVariantMap m;
	execSelectQueryOneRow("SELECT lastStep FROM undoSettings", QVariantList(), &m);

	int ceil = m.value("lastStep", 0).toInt();

	QVariantList l;
	l << floor;
	l << ceil;

	QVariantList steps;
	execSelectQuery("SELECT cmd FROM undoLog WHERE stepid>? AND stepid<=? ORDER BY stepid DESC, seq", l, &steps);

	foreach (QVariant v, steps) {
		QString cmd = v.toMap().value("cmd").toString();

		execSimpleQuery(cmd);
	}

	execSimpleQuery("DELETE FROM undoStep WHERE id>? AND id<=?", l);

	QVariantList l2;
	l2 << floor;
	execSimpleQuery("UPDATE undoSettings SET lastStep=?", l2);

	QVariantMap r;
	execSelectQueryOneRow("SELECT COALESCE(MAX(id),-1) as id FROM undoStep", QVariantList(), &r);
	setCanUndo(r.value("id",-1).toInt());

	m_db.commit();

	emit undone();
}


void CosSql::setCanUndo(int canUndo)
{
	if (m_canUndo == canUndo)
		return;

	m_canUndo = canUndo;
	emit canUndoChanged(m_canUndo);
}






