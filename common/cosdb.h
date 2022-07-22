/*
 * ---- Call of Suli ----
 *
 * cossql.h
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


#ifndef COSDB_H
#define COSDB_H

#include <QtSql>
#include <QObject>

Q_DECLARE_LOGGING_CATEGORY(sql)

class CosDbWorker : public QObject
{
	Q_OBJECT

public:
	CosDbWorker(const QString &connectionName, QObject *parent = nullptr);
	~CosDbWorker();

public slots:
	void init();

	QString connectionName() const { return m_connectionName; }
	void setDatabaseName(QString databaseName) { QSqlDatabase::database(m_connectionName, false).setDatabaseName(databaseName); }
	QString databaseName() const { return QSqlDatabase::database(m_connectionName, false).databaseName(); }
	bool subscribeToNotification(const QString &table);

	bool isOpen() const { return QSqlDatabase::database(m_connectionName, false).isOpen(); }
	bool isValid() const { return QSqlDatabase::database(m_connectionName, false).isValid(); }
	void setConnectOptions(const QString &options = QString()) { QSqlDatabase::database(m_connectionName, false).setConnectOptions(options); }
	QString connectOptions() const { return QSqlDatabase::database(m_connectionName, false).connectOptions(); }
	bool databaseExists() const { return (QFile::exists(QSqlDatabase::database(m_connectionName, false).databaseName())); }

	bool open();
	void close() { QSqlDatabase::database(m_connectionName, false).close(); }
	void transaction() { QSqlDatabase::database(m_connectionName, false).transaction(); }
	void commit() { QSqlDatabase::database(m_connectionName, false).commit(); }
	void rollback() { QSqlDatabase::database(m_connectionName, false).rollback(); }

	QVariantList execQuery(QSqlQuery query, QString *errorString = nullptr, QVariant *lastInsertId = nullptr);
	QJsonArray execQueryJson(QSqlQuery query, QString *errorString = nullptr, QVariant *lastInsertId = nullptr);

	bool execSimpleQuery(QString query, const QVariantList &args = QVariantList(), QString *errorString = nullptr);
	bool execSimpleQuery(QStringList queries, QString *errorString = nullptr);
	bool execBatchQuery(QString query, const QVariantList &list, QString *errorString = nullptr);
	QVariantList execSelectQuery(QString query, const QVariantList &args = QVariantList(), QString *errorString = nullptr);
	QJsonArray execSelectQueryJson(QString query, const QVariantList &args = QVariantList(), QString *errorString = nullptr);
	QVariantMap execSelectQueryOneRow(QString query, const QVariantList &args = QVariantList(), QString *errorString = nullptr);
	int execInsertQuery(QString query, const QVariantMap &map = QVariantMap(), QString *errorString = nullptr);
	bool execUpdateQuery(QString query, const QVariantMap &map = QVariantMap(), const QVariantMap &bindValues = QVariantMap(),
						 QString *errorString = nullptr);
	bool execListQuery(QString query, const QVariantList &list, const QVariantMap &bindValues = QVariantMap(),
					   const bool &parenthesizeValues = false, QString *errorString = nullptr);

	void undoLogBegin(const QString &desc);
	QVariantMap undo(const int &floor);
	QVariantMap undoStack();


signals:
	void databaseError(const QString &text);
	void notification(QString table);

private:
	QSqlQuery simpleQuery(QString query, const QVariantList &args = QVariantList());
	QSqlQuery insertQuery(QString query, const QVariantMap &map = QVariantMap()) const;
	QSqlQuery updateQuery(QString query, const QVariantMap &map = QVariantMap(), const QVariantMap &bindValues = QVariantMap()) const;
	QSqlQuery listQuery(QString query, const QVariantList &list, const QVariantMap &bindValues = QVariantMap(),
						const bool &parenthesizeValues = false) const;


	QString m_connectionName;
	QRecursiveMutex m_mutex;
};



/**
 * @brief The CosDb class
 */


class CosDb : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString databaseName READ databaseName WRITE setDatabaseName NOTIFY databaseNameChanged)

public:
	explicit CosDb(const QString &connectionName = QString(), QObject *parent = nullptr);
	virtual ~CosDb();

	QString databaseName();

	CosDbWorker *worker() const { return m_worker; }

	bool isOpen() const;
	bool isValid() const;
	void transaction() const { QMetaObject::invokeMethod(m_worker, "transaction", Qt::BlockingQueuedConnection); }
	void commit() const { QMetaObject::invokeMethod(m_worker, "commit", Qt::BlockingQueuedConnection); }
	void rollback() const { QMetaObject::invokeMethod(m_worker, "rollback", Qt::BlockingQueuedConnection); }

	bool databaseExists() const;
	bool subscribeToNotification(const QString &table) const;

	void setConnectOptions(const QString &options = QString()) {
		QMetaObject::invokeMethod(m_worker, "setConnectOptions", Qt::BlockingQueuedConnection, Q_ARG(QString, options) );
	}
	QString connectOptions() const;

	static QString hashPassword (const QString &password, QString *salt = nullptr,
								 QCryptographicHash::Algorithm method = QCryptographicHash::Sha1);


public slots:

	bool batchQuery(const QString &query);
	bool batchQueryFromFile(const QString &filename);


	bool execSimpleQuery(QString query, const QVariantList &args = QVariantList(), QString *errorString = nullptr);
	bool execBatchQuery(QString query, const QVariantList &list, QString *errorString = nullptr);
	QVariantList execSelectQuery(QString query, const QVariantList &args = QVariantList(), QString *errorString = nullptr);
	QJsonArray execSelectQueryJson(QString query, const QVariantList &args = QVariantList(), QString *errorString = nullptr);
	QVariantMap execSelectQueryOneRow(QString query, const QVariantList &args = QVariantList(), QString *errorString = nullptr);
	int execInsertQuery(QString query, const QVariantMap &map = QVariantMap(), QString *errorString = nullptr);
	bool execUpdateQuery(QString query, const QVariantMap &map = QVariantMap(), const QVariantMap &bindValues = QVariantMap(),
						 QString *errorString = nullptr);
	bool execListQuery(QString query, const QVariantList &list, const QVariantMap &bindValues = QVariantMap(),
					   const bool &parenthesizeValues = false, QString *errorString = nullptr);


	bool createUndoTables();
	bool createTrigger(const QString &table);
	void dropUndoTables();
	void dropUndoTriggers();

	int canUndo() const { return m_canUndo; }

	void undoLogBegin(const QString &desc) {
		QMetaObject::invokeMethod(m_worker, "undoLogBegin", Qt::BlockingQueuedConnection, Q_ARG(QString, desc) );
	}
	void undoLogEnd();
	void undo(const int &floor);
	QVariantMap undoStack();
	void clearUndo();

	bool open();
	void close() { QMetaObject::invokeMethod(m_worker, "close", Qt::BlockingQueuedConnection); }
	void setDatabaseName(QString databaseName);

	QVariant get(const int &rowid, const QString &table, const QString &field);


private slots:
	void setCanUndo(int canUndo, const QString &undoString);


signals:
	void canUndoChanged(int canUndo, const QString &undoString);
	void undone();
	void databaseNameChanged(QString databaseName);


private:
	int m_canUndo;
	bool m_isOwnCreated;
	QStringList m_undoTables;
	QStringList m_undoTriggers;
	CosDbWorker *m_worker;
	QThread m_workerThread;
	QMutex m_mutex;
};

#endif // COSSQL_H
