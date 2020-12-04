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


class CosDb : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString databaseName READ databaseName WRITE setDatabaseName NOTIFY databaseNameChanged)

public:
	explicit CosDb(const QString &connectionName = QString(), QObject *parent = nullptr);
	virtual ~CosDb();

	QSqlDatabase db() const { return m_db; }
	QString databaseName() const { return m_db.databaseName(); }

	bool isOpen() const { return m_db.isOpen(); }
	bool isValid() const { return m_db.isValid(); }

	bool databaseExists() const { return (QFile::exists(m_db.databaseName())); }

	void setConnectOptions(const QString &options = QString()) { m_db.setConnectOptions(options); }
	QString connectOptions() const { return m_db.connectOptions(); }

	static QString hashPassword (const QString &password, QString *salt = nullptr,
								 QCryptographicHash::Algorithm method = QCryptographicHash::Sha1);


public slots:

	bool batchQuery(const QString &query);
	bool batchQueryFromFile(const QString &filename);


	bool execSimpleQuery(QString query, const QVariantList &args = QVariantList(), QString *errorString = nullptr);
	bool execBatchQuery(QString query, const QVariantList &list, QString *errorString = nullptr);
	QVariantList execSelectQuery(QString query, const QVariantList &args = QVariantList(), QString *errorString = nullptr);
	QVariantMap execSelectQueryOneRow(QString query, const QVariantList &args = QVariantList(), QString *errorString = nullptr);
	int execInsertQuery(QString query, const QVariantMap &map = QVariantMap(), QString *errorString = nullptr);
	bool execUpdateQuery(QString query, const QVariantMap &map = QVariantMap(), const QVariantMap &bindValues = QVariantMap(), QString *errorString = nullptr);
	bool execListQuery(QString query, const QVariantList &list, const QVariantMap &bindValues = QVariantMap(), const bool &parenthesizeValues = false,
					   QString *errorString = nullptr);


	bool createUndoTables();
	bool createTrigger(const QString &table);
	void dropUndoTables();
	void dropUndoTriggers();

	int canUndo() const { return m_canUndo; }

	void undoLogBegin(const QString &desc);
	void undoLogEnd();
	void undo(const int &floor);
	QVariantMap undoStack();

	bool open();
	void close() { m_db.close(); }
	void setDatabaseName(QString databaseName);

	QVariant get(const int &rowid, const QString &table, const QString &field);

protected slots:
	virtual bool databaseInit() { return true; };

private slots:
	void setCanUndo(int canUndo, const QString &undoString);

	QSqlQuery simpleQuery(QString query, const QVariantList &args = QVariantList());
	QSqlQuery insertQuery(QString query, const QVariantMap &map = QVariantMap()) const;
	QSqlQuery updateQuery(QString query, const QVariantMap &map = QVariantMap(), const QVariantMap &bindValues = QVariantMap()) const;
	QSqlQuery listQuery(QString query, const QVariantList &list, const QVariantMap &bindValues = QVariantMap(), const bool &parenthesizeValues = false) const;

	QVariantList execQuery(QSqlQuery query, QString *errorString = nullptr, QVariant *lastInsertId = nullptr);

signals:
	void canUndoChanged(int canUndo, const QString &undoString);
	void undone();
	void databaseError(const QString &text);
	void databaseNameChanged(QString databaseName);

protected:
	QSqlDatabase m_db;

private:
	int m_canUndo;
	bool m_isOwnCreated;
	QStringList m_undoTables;
	QStringList m_undoTriggers;
	QMutex m_mutex;
};

#endif // COSSQL_H
