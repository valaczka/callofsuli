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


#include "cosdb.h"

CosDb::CosDb(const QString &connectionName, QObject *parent)
	: QObject(parent)
	, m_worker(nullptr)
	, m_workerThread()
	, m_mutex(QMutex::Recursive)
{
	m_worker = new CosDbWorker(connectionName);
	m_worker->moveToThread(&m_workerThread);
	connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

	m_workerThread.start();

	QMetaObject::invokeMethod(m_worker, "init", Qt::BlockingQueuedConnection);

	m_canUndo = -1;
	m_isOwnCreated = false;
}


/**
 * @brief CosSql::~CosSql
 */

CosDb::~CosDb()
{
	m_workerThread.quit();
	m_workerThread.wait();
}


/**
 * @brief CosDb::databaseName
 * @return
 */

QString CosDb::databaseName()
{
	QString ret;

	QMetaObject::invokeMethod(m_worker, "databaseName", Qt::BlockingQueuedConnection,
							  Q_RETURN_ARG(QString, ret)
							  );

	return ret;
}


/**
 * @brief CosDb::isOpen
 * @return
 */

bool CosDb::isOpen() const
{
	bool ret = false;

	QMetaObject::invokeMethod(m_worker, "isOpen", Qt::BlockingQueuedConnection,
							  Q_RETURN_ARG(bool, ret)
							  );

	return ret;
}


/**
 * @brief CosDb::isValid
 * @return
 */

bool CosDb::isValid() const
{
	bool ret = false;

	QMetaObject::invokeMethod(m_worker, "isValid", Qt::BlockingQueuedConnection,
							  Q_RETURN_ARG(bool, ret)
							  );

	return ret;
}


/**
 * @brief CosDb::databaseExists
 * @return
 */

bool CosDb::databaseExists() const
{
	bool ret = false;

	QMetaObject::invokeMethod(m_worker, "databaseExists", Qt::BlockingQueuedConnection,
							  Q_RETURN_ARG(bool, ret)
							  );

	return ret;
}


/**
 * @brief CosDb::connectOptions
 * @return
 */

QString CosDb::connectOptions() const
{
	QString ret;

	QMetaObject::invokeMethod(m_worker, "connectOptions", Qt::BlockingQueuedConnection,
							  Q_RETURN_ARG(QString, ret)
							  );

	return ret;
}


/**
 * @brief CosDb::setConnectOptions
 * @param options
 */










/**
 * @brief CosSql::batchQuery
 * @param query
 * @return
 */

bool CosDb::batchQuery(const QString &query)
{
	/*	QMutexLocker locker(&m_mutex);

	QSqlQuery q(m_db);

	qDebug().noquote() << "SQL batch query ----------";

	QStringList list = query.split("\n\n", Qt::SkipEmptyParts);
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
*/

	return true;
}


/**
 * @brief CosSql::batchQuery
 * @param query
 * @return
 */

bool CosDb::batchQueryFromFile(const QString &filename)
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
 * @brief CosDb::execSimpleQuery
 * @param query
 * @param args
 * @param errorString
 * @return
 */

bool CosDb::execSimpleQuery(QString query, const QVariantList &args, QString *errorString)
{
	bool ret = false;

	QMetaObject::invokeMethod(m_worker, "execSimpleQuery", Qt::BlockingQueuedConnection,
							  Q_RETURN_ARG(bool, ret),
							  Q_ARG(QString, query),
							  Q_ARG(QVariantList, args),
							  Q_ARG(QString*, errorString)
							  );

	return ret;
}


/**
 * @brief CosDb::execSelectQuery
 * @param query
 * @param args
 * @param errorString
 * @return
 */

QVariantList CosDb::execSelectQuery(QString query, const QVariantList &args, QString *errorString)
{
	QVariantList ret;

	QMetaObject::invokeMethod(m_worker, "execSelectQuery", Qt::BlockingQueuedConnection,
							  Q_RETURN_ARG(QVariantList, ret),
							  Q_ARG(QString, query),
							  Q_ARG(QVariantList, args),
							  Q_ARG(QString*, errorString)
							  );

	return ret;
}


/**
 * @brief CosDb::execSelectQueryOneRow
 * @param query
 * @param args
 * @param errorString
 * @return
 */

QVariantMap CosDb::execSelectQueryOneRow(QString query, const QVariantList &args, QString *errorString)
{
	QVariantMap ret;

	QMetaObject::invokeMethod(m_worker, "execSelectQueryOneRow", Qt::BlockingQueuedConnection,
							  Q_RETURN_ARG(QVariantMap, ret),
							  Q_ARG(QString, query),
							  Q_ARG(QVariantList, args),
							  Q_ARG(QString*, errorString)
							  );

	return ret;
}


/**
 * @brief CosDb::execInsertQuery
 * @param query
 * @param map
 * @param errorString
 * @return
 */

int CosDb::execInsertQuery(QString query, const QVariantMap &map, QString *errorString)
{
	int ret = -1;

	QMetaObject::invokeMethod(m_worker, "execInsertQuery", Qt::BlockingQueuedConnection,
							  Q_RETURN_ARG(int, ret),
							  Q_ARG(QString, query),
							  Q_ARG(QVariantMap, map),
							  Q_ARG(QString*, errorString)
							  );

	qDebug() << "inSERT" << ret << query << map;

	return ret;
}


/**
 * @brief CosDb::execUpdateQuery
 * @param query
 * @param map
 * @param bindValues
 * @param errorString
 * @return
 */

bool CosDb::execUpdateQuery(QString query, const QVariantMap &map, const QVariantMap &bindValues, QString *errorString)
{
	bool ret = false;

	QMetaObject::invokeMethod(m_worker, "execUpdateQuery", Qt::BlockingQueuedConnection,
							  Q_RETURN_ARG(bool, ret),
							  Q_ARG(QString, query),
							  Q_ARG(QVariantMap, map),
							  Q_ARG(QVariantMap, bindValues),
							  Q_ARG(QString*, errorString)
							  );

	return ret;

}


/**
 * @brief CosDb::execListQuery
 * @param query
 * @param list
 * @param bindValues
 * @param parenthesizeValues
 * @param errorString
 * @return
 */

bool CosDb::execListQuery(QString query, const QVariantList &list, const QVariantMap &bindValues,
						  const bool &parenthesizeValues, QString *errorString)
{
	bool ret = false;

	QMetaObject::invokeMethod(m_worker, "execListQuery", Qt::BlockingQueuedConnection,
							  Q_RETURN_ARG(bool, ret),
							  Q_ARG(QString, query),
							  Q_ARG(QVariantList, list),
							  Q_ARG(QVariantMap, bindValues),
							  Q_ARG(bool, parenthesizeValues),
							  Q_ARG(QString*, errorString)
							  );

	return ret;
}





/**
 * @brief CosSql::simpleQuery
 * @param query
 * @param args
 * @param lastInsertId
 * @return
 */

QSqlQuery CosDbWorker::simpleQuery(QString query, const QVariantList &args)
{
	QSqlDatabase m_db = QSqlDatabase::database(m_connectionName, false);

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

QSqlQuery CosDbWorker::insertQuery(QString query, const QVariantMap &map) const
{
	QStringList keys = map.keys();

	QStringList l;

	foreach (QString k, keys) {
		l << QString(":").append(k);
	}

	QSqlDatabase m_db = QSqlDatabase::database(m_connectionName, false);

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

QSqlQuery CosDbWorker::updateQuery(QString query, const QVariantMap &map, const QVariantMap &bindValues) const
{
	QStringList keys = map.keys();

	QStringList l;

	foreach (QString k, keys) {
		l << QString(k).append("=:").append(k);
	}

	QSqlDatabase m_db = QSqlDatabase::database(m_connectionName, false);

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

QSqlQuery CosDbWorker::listQuery(QString query, const QVariantList &list, const QVariantMap &bindValues, const bool &parenthesizeValues) const
{
	QStringList l;

	QSqlDatabase m_db = QSqlDatabase::database(m_connectionName, false);

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
 * @brief CosSql::query
 * @param query
 * @return
 */

QVariantList CosDbWorker::execQuery(QSqlQuery query, QString *errorString, QVariant *lastInsertId)
{
	bool success = true;
	QVariantList records;

	if (!query.exec()) {
		QString errText = query.lastError().text();
		qDebug().noquote() << tr("SQL command: ")+query.executedQuery();
		qWarning().noquote() << tr("SQL error: ")+errText;
		success = false;

		if (errorString)
			(*errorString) = errText;
	} else {
#ifdef COS_SQL_DEBUG
		qDebug().noquote() << tr("SQL command: ")+query.executedQuery();
#endif
	}

	if (!success) {
		query.finish();
		return records;
	}

	if (lastInsertId)
		(*lastInsertId) = query.lastInsertId();


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

	query.finish();

	return records;
}


/**
 * @brief CosSql::execSimpleQuery
 * @param query
 * @param args
 * @param errorString
 * @return
 */

bool CosDbWorker::execSimpleQuery(QString query, const QVariantList &args, QString *errorString)
{
	QMutexLocker locker(&m_mutex);
	QString err;

	execQuery(simpleQuery(query, args), &err);

	if (errorString)
		(*errorString) = err;

	return err.isEmpty();
}


/**
 * @brief CosSql::execBatchQuery
 * @param query
 * @param args
 * @return
 */

bool CosDbWorker::execBatchQuery(QString query, const QVariantList &list, QString *errorString)
{
	QMutexLocker locker(&m_mutex);

	QSqlDatabase m_db = QSqlDatabase::database(m_connectionName, false);

	m_db.transaction();

	QSqlQuery q(m_db);
	q.prepare(query);

	foreach (QVariant v, list) {
		q.addBindValue(v.toList());
	}

	if (!q.execBatch()) {
		QString errText = q.lastError().text();
		qWarning().noquote() << tr("SQL error: ")+errText+": "+q.executedQuery();
		if (errorString)
			(*errorString) = errText;
		m_db.rollback();

		return false;
	}

	m_db.commit();

	qDebug().noquote() << tr("SQL command: ")+q.executedQuery();

	return true;
}


/**
 * @brief CosDb::execSelectQuery
 * @param query
 * @param args
 * @param errorString
 * @return
 */

QVariantList CosDbWorker::execSelectQuery(QString query, const QVariantList &args, QString *errorString)
{
	QMutexLocker locker(&m_mutex);
	QVariantList l = execQuery(simpleQuery(query, args), errorString);

	return l;
}

/**
 * @brief CosSql::execSelectQueryOneRow
 * @param query
 * @param args
 * @param errorString
 * @return
 */

QVariantMap CosDbWorker::execSelectQueryOneRow(QString query, const QVariantList &args, QString *errorString)
{
	QMutexLocker locker(&m_mutex);
	QVariantList list = execQuery(simpleQuery(query, args), errorString);


	if (list.size() > 1) {
		if (errorString)
			(*errorString) = "multiple records";
		return QVariantMap();
	} else if (list.size() < 1) {
		return QVariantMap();
	}

	return list.value(0).toMap();
}



/**
 * @brief CosSql::execInsertQuery
 * @param query
 * @param map
 * @return
 */

int CosDbWorker::execInsertQuery(QString query, const QVariantMap &map, QString *errorString)
{
	QVariant nextId = -1;
	QMutexLocker locker(&m_mutex);

	QSqlDatabase m_db = QSqlDatabase::database(m_connectionName, false);

	execQuery(insertQuery(query, map), errorString, &nextId);

	return nextId.toInt();
}


/**
 * @brief CosSql::execUpdateQuery
 * @param query
 * @param map
 * @param bindValues
 * @param errorString
 * @return
 */

bool CosDbWorker::execUpdateQuery(QString query, const QVariantMap &map, const QVariantMap &bindValues, QString *errorString)
{
	QString err;

	QMutexLocker locker(&m_mutex);
	execQuery(updateQuery(query, map, bindValues), &err);


	if (errorString)
		(*errorString) = err;

	return err.isEmpty();
}


/**
 * @brief CosSql::execListQuery
 * @param query
 * @param list
 * @param bindValues
 * @param parenthesizeValues
 * @param errorString
 * @return
 */

bool CosDbWorker::execListQuery(QString query, const QVariantList &list, const QVariantMap &bindValues,
								const bool &parenthesizeValues, QString *errorString)
{
	QString err;

	QMutexLocker locker(&m_mutex);
	execQuery(listQuery(query, list, bindValues, parenthesizeValues), &err);


	if (errorString)
		(*errorString) = err;

	return err.isEmpty();
}



/**
 * @brief CosSql::hashPassword
 * @param password
 * @param salt
 * @return
 */

QString CosDb::hashPassword(const QString &password, QString *salt, QCryptographicHash::Algorithm method)
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
	pwd.append(password.toUtf8()).append(_salt.toUtf8());

	QByteArray hash = QCryptographicHash::hash(pwd, method);
	return QString::fromLatin1(hash.toHex());
}




/**
 * @brief CosSql::createUndoTable
 * @return
 */

bool CosDb::createUndoTables()
{
	QMutexLocker locker(&m_mutex);

	if (!execSimpleQuery("CREATE TABLE IF NOT EXISTS undoSettings(lastStep INTEGER, active BOOL)"))
		return false;

	m_undoTables.append("undoSettings");

	if (!execSimpleQuery("INSERT INTO undoSettings(lastStep, active) VALUES(0, false)"))
		return false;

	if (!execSimpleQuery("CREATE TABLE IF NOT EXISTS undoStep(id INTEGER PRIMARY KEY, desc TEXT)"))
		return false;

	m_undoTables.append("undoStep");

	if (!execSimpleQuery("CREATE TABLE IF NOT EXISTS undoLog(seq INTEGER PRIMARY KEY, stepid INTEGER NOT NULL REFERENCES undoStep(id) ON UPDATE CASCADE ON DELETE CASCADE, cmd TEXT)"))
		return false;

	m_undoTables.append("undoLog");

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

bool CosDb::createTrigger(const QString &table)
{
	QMutexLocker locker(&m_mutex);

	QString cmd = "CREATE TRIGGER IF NOT EXISTS _"+table+"_it AFTER INSERT ON "+table+" WHEN 1=(SELECT active FROM undoSettings) BEGIN\n";
	cmd += "INSERT INTO undoLog(stepid, cmd) VALUES((SELECT MAX(id) FROM undoStep), \n";
	cmd += "'DELETE FROM "+table+" WHERE rowid='||new.rowid);\n";
	cmd += "END;";

	if (!execSimpleQuery(cmd))
		return false;

	m_undoTriggers.append("_"+table+"_it");


	QVariantList l;
	l << table;
	QVariantList list;

	list = execSelectQuery("SELECT name from pragma_table_info(?)", l);

	if (list.isEmpty())
		return false;


	QStringList p;
	foreach (QVariant v, list) {
		QString field = v.toMap().value("name").toString();
		p << field+"='||quote(old."+field+")||'";

	}
	QString cmd2 = "CREATE TRIGGER IF NOT EXISTS _"+table+"_ut AFTER UPDATE ON "+table+" WHEN 1=(SELECT active FROM undoSettings) BEGIN\n";
	cmd2 += "INSERT INTO undoLog(stepid, cmd) VALUES((SELECT MAX(id) FROM undoStep), \n";
	cmd2 += "'UPDATE "+table+" SET "+p.join(",")+" WHERE rowid='||old.rowid);\n";
	cmd2 += "END;";

	if (!execSimpleQuery(cmd2))
		return false;

	m_undoTriggers.append("_"+table+"_ut");


	QStringList p2, p3;
	p2 << "rowid";
	p3 << "'||old.rowid||'";
	foreach (QVariant v, list) {
		QString field = v.toMap().value("name").toString();
		p2 << field;
		p3 << "'||quote(old."+field+")||'";
	}
	QString cmd3 = "CREATE TRIGGER IF NOT EXISTS _"+table+"_dt BEFORE DELETE ON "+table+" WHEN 1=(SELECT active FROM undoSettings) BEGIN\n";
	cmd3 += "INSERT INTO undoLog(stepid, cmd) VALUES((SELECT MAX(id) FROM undoStep), \n";
	cmd3 += "'INSERT INTO "+table+"("+p2.join(",")+") VALUES ("+p3.join(",")+")');\n";
	cmd3 += "END;";

	if (!execSimpleQuery(cmd3))
		return false;

	m_undoTriggers.append("_"+table+"_dt");

	return true;
}


/**
 * @brief CosDb::dropUndoTables
 */

void CosDb::dropUndoTables()
{
	QMutexLocker locker(&m_mutex);
	foreach (QString t, m_undoTables) {
		execSimpleQuery("DROP TABLE IF EXISTS "+t);
	}

	m_undoTables.clear();
}


/**
 * @brief CosDb::dropUndoTriggers
 */

void CosDb::dropUndoTriggers()
{
	QMutexLocker locker(&m_mutex);
	foreach (QString t, m_undoTriggers) {
		execSimpleQuery("DROP TRIGGER IF EXISTS "+t);
	}

	m_undoTriggers.clear();
}




/**
 * @brief CosSql::undoLogBegin
 * @param desc
 * @return
 */

void CosDbWorker::undoLogBegin(const QString &desc)
{
	QMutexLocker locker(&m_mutex);

	QSqlDatabase m_db = QSqlDatabase::database(m_connectionName, false);

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

void CosDb::undoLogEnd()
{
	execSimpleQuery("UPDATE undoSettings SET active=false");

	QVariantMap r =	execSelectQueryOneRow("SELECT COALESCE(MAX(id),-1) as id, desc FROM undoStep");

	setCanUndo(r.value("id",-1).toInt(), r.value("desc").toString());
}



void CosDb::undo(const int &floor)
{
	/*QMutexLocker locker(&m_mutex);

	QVariantMap r = m_worker->undo(floor);

	setCanUndo(r.value("id",-1).toInt(), r.value("desc").toString());

	emit undone();*/
}


/**
 * @brief CosDb::undoStack
 * @return
 */

QVariantMap CosDb::undoStack()
{

}



/**
 * @brief CosSql::undoStack
 * @return
 */

QVariantMap CosDbWorker::undoStack()
{
	QVariantMap ret;

	QMutexLocker locker(&m_mutex);

	QSqlDatabase m_db = QSqlDatabase::database(m_connectionName, false);

	m_db.transaction();

	ret["lastStep"] = execSelectQueryOneRow("SELECT lastStep FROM undoSettings").value("lastStep");
	ret["minStep"] = execSelectQueryOneRow("SELECT COALESCE(MIN(id),-1) as minStep FROM undoStep").value("minStep");
	ret["maxStep"] = execSelectQueryOneRow("SELECT COALESCE(MAX(id),-1) as maxStep FROM undoStep").value("maxStep");

	QVariantList l;
	l = execSelectQuery("SELECT id, desc FROM undoStep ORDER BY id DESC");
	ret["steps"] = l;

	m_db.commit();

	return ret;
}


/**
 * @brief CosDb::clearUndo
 */

void CosDb::clearUndo()
{
	QMutexLocker locker(&m_mutex);

	execSimpleQuery("DELETE FROM undoStep");
	setCanUndo(-1, "");
}


/**
 * @brief CosDb::open
 * @return
 */

bool CosDb::open()
{
	bool ret = false;

	QMetaObject::invokeMethod(m_worker, "open", Qt::BlockingQueuedConnection,
							  Q_RETURN_ARG(bool, ret)
							  );

	return ret;
}





/**
 * @brief CosSql::undo
 * @param step
 */

QVariantMap CosDbWorker::undo(const int &floor)
{
	QMutexLocker locker(&m_mutex);

	QSqlDatabase m_db = QSqlDatabase::database(m_connectionName, false);

	m_db.exec("PRAGMA foreign_keys = OFF;");

	m_db.transaction();


	QVariantMap m = execSelectQueryOneRow("SELECT lastStep FROM undoSettings");

	int ceil = m.value("lastStep", 0).toInt();

	QVariantList l;
	l << floor;
	l << ceil;

	QVariantList steps = execSelectQuery("SELECT cmd FROM undoLog WHERE stepid>? AND stepid<=? ORDER BY stepid DESC, seq", l);

	foreach (QVariant v, steps) {
		QString cmd = v.toMap().value("cmd").toString();

		execSimpleQuery(cmd);
	}

	execSimpleQuery("DELETE FROM undoStep WHERE id>? AND id<=?", l);

	QVariantList l2;
	l2 << floor;
	execSimpleQuery("UPDATE undoSettings SET lastStep=?", l2);

	QVariantMap r = execSelectQueryOneRow("SELECT COALESCE(MAX(id),-1) as id, desc FROM undoStep");


	m_db.commit();

	m_db.exec("PRAGMA foreign_keys = ON;");

	return r;
}





/**
 * @brief CosSql::openDatabase
 * @return
 */

bool CosDbWorker::open()
{
	QMutexLocker locker(&m_mutex);

	QSqlDatabase m_db = QSqlDatabase::database(m_connectionName, false);

	bool isOwn = false;

	QString dbName = m_db.databaseName();

	if (dbName.isEmpty() || dbName == ":memory:") {
		qInfo() << m_db.connectionName() << tr("Nincs megadva fájl, belső memóriában készül adatbázis");

		if (!m_db.open()) {
			qWarning() << tr("Nem sikerült megnyitni az adatbázist: ") << dbName;
			qWarning() << m_db.lastError();
			emit databaseError(m_db.lastError().text());
			return false;
		}
	} else {
		if (!m_db.isOpen()) {
			if (!QFile::exists(dbName)) {
				qDebug() << tr("Új adatbázis létrehozása") << dbName;

				isOwn = true;
			}

			if (!m_db.open()) {
				qWarning() << tr("Nem sikerült megnyitni az adatbázist: ") << dbName;
				qWarning() << m_db.lastError();
				emit databaseError(m_db.lastError().text());
				return false;
			}

			/*if (isOwn) {
				qWarning() << tr("Nem lehet létrehozni az adatbázist:") << dbName;
				emit databaseError(tr("Nem lehet létrehozni az adatbázist: ")+dbName);

				qInfo() << tr("Az adatbázis félkész, törlöm:") << dbName;
				m_db.close();

				if (dbName != ":memory:" && !QFile::remove(dbName)) {
					qWarning() << tr("Nem sikerült törölni a hibás adatbázist: ") << dbName;
				}

				return false;
			}*/
		}
	}


	QSqlQuery q(m_db);

	q.exec("PRAGMA foreign_keys = ON");
	q.finish();

	qDebug() << q.isActive() << q.isValid() << q.lastError();

	qDebug().noquote() << tr("Adatbázis megnyitva: ") << (dbName.isEmpty() ? ":memory:" : dbName);

	return true;
}


/**
 * @brief CosSql::setDatabaseName
 * @param databaseName
 */

void CosDb::setDatabaseName(QString databaseName)
{
	QMetaObject::invokeMethod(m_worker, "setDatabaseName", Qt::BlockingQueuedConnection,
							  Q_ARG(QString, databaseName));

	emit databaseNameChanged(databaseName);
}


/**
 * @brief CosDb::get
 * @param rowid
 * @param field
 * @return
 */

QVariant CosDb::get(const int &rowid, const QString &table, const QString &field)
{
	QVariantList l;
	l << rowid;
	QVariantMap m = execSelectQueryOneRow("SELECT "+field+" FROM "+table+" WHERE rowid=?", l);

	if (!m.isEmpty())
		return m.value(field);

	return QVariant::Invalid;
}


/**
 * @brief CosSql::setCanUndo
 * @param canUndo
 */

void CosDb::setCanUndo(int canUndo, const QString &undoString)
{
	if (m_canUndo == canUndo)
		return;

	m_canUndo = canUndo;
	emit canUndoChanged(m_canUndo, undoString);
}







/**
 * @brief CosDbWorker::CosDbWorker
 * @param parent
 */


CosDbWorker::CosDbWorker(const QString &connectionName, QObject *parent)
	: QObject(parent)
	, m_connectionName(connectionName)
	, m_mutex()
{

}




/**
 * @brief CosDbWorker::~CosDbWorker
 */

CosDbWorker::~CosDbWorker()
{
	QSqlDatabase::removeDatabase(m_connectionName);
}




/**
 * @brief CosDbWorker::init
 */

void CosDbWorker::init()
{
	if (m_connectionName.isEmpty())
		m_connectionName = "defaultCosDbWorker";

	QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
}

