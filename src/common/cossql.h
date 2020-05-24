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


#ifndef COSSQL_H
#define COSSQL_H

#include <QtSql>
#include <QObject>


class CosSql : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int canUndo READ canUndo WRITE setCanUndo NOTIFY canUndoChanged)

public:
	explicit CosSql(const QString &connectionName = QString(), QObject *parent = nullptr);
	virtual ~CosSql();


	bool open(const QString &file, bool create = false);
	void close() { m_db.close(); }

	bool isOpen() const { return m_db.isOpen(); }
	bool isValid() const { return m_db.isValid(); }

	bool batchQuery(const QString &query);
	bool batchQueryFromFile(const QString &filename);
	QSqlQuery simpleQuery(QString query, const QVariantList &args = QVariantList());
	QSqlQuery insertQuery(QString query, const QVariantMap &map = QVariantMap()) const;
	QSqlQuery updateQuery(QString query, const QVariantMap &map = QVariantMap(), const QVariantMap &bindValues = QVariantMap()) const;
	QSqlQuery listQuery(QString query, const QVariantList &list, const QVariantMap &bindValues = QVariantMap(), const bool &parenthesizeValues = false) const;
	QVariantMap runQuery(QSqlQuery query);
	bool execQuery(QSqlQuery query);

	QVariantMap runSimpleQuery(QString query, const QVariantList &args = QVariantList()) {
		return runQuery(simpleQuery(query, args));
	}
	QVariantMap runInsertQuery(QString query, const QVariantMap &map = QVariantMap()) {
		return runQuery(insertQuery(query, map));
	}
	QVariantMap runUpdateQuery(QString query, const QVariantMap &map = QVariantMap(), const QVariantMap &bindValues = QVariantMap()) {
		return runQuery(updateQuery(query, map, bindValues));
	}

	bool execSimpleQuery(QString query, const QVariantList &args = QVariantList()) {
		return execQuery(simpleQuery(query, args));
	}

	bool execBatchQuery(QString query, const QVariantList &list);
	bool execSelectQuery(QString query, const QVariantList &args = QVariantList(), QVariantList *records = nullptr);
	bool execSelectQuery(QString query, const QVariantList &args = QVariantList(), QJsonArray *records = nullptr);
	bool execSelectQueryOneRow(QString query, const QVariantList &args = QVariantList(), QVariantMap *record = nullptr);
	bool execSelectQueryOneRow(QString query, const QVariantList &args = QVariantList(), QJsonObject *record = nullptr);

	int execInsertQuery(QString query, const QVariantMap &map = QVariantMap());

	bool execUpdateQuery(QString query, const QVariantMap &map = QVariantMap(), const QVariantMap &bindValues = QVariantMap()) {
		return execQuery(updateQuery(query, map, bindValues));
	}

	bool execListQuery(QString query, const QVariantList &list, const QVariantMap &bindValues = QVariantMap(), const bool &parenthesizeValues = false) {
		return execQuery(listQuery(query, list, bindValues, parenthesizeValues));
	}

	static QString hashPassword (const QString &password, QString *salt = nullptr,
								 QCryptographicHash::Algorithm method = QCryptographicHash::Sha1);


	bool createUndoTables();
	bool createTrigger(const QString &table);
	void undoLogBegin(const QString &desc);
	void undoLogEnd();
	QVariantMap undoStack();
	void undo(const int &floor);


	QSqlDatabase db() const { return m_db; }
	bool dbCreated() const { return m_dbCreated; }
	int canUndo() const { return m_canUndo; }


private slots:
	void setCanUndo(int canUndo)
	{
		if (m_canUndo == canUndo)
			return;

		m_canUndo = canUndo;
		emit canUndoChanged(m_canUndo);
	}


signals:
	void canUndoChanged(int canUndo);

private:
	QSqlDatabase m_db;
	bool m_dbCreated;

	int m_canUndo;
};

#endif // COSSQL_H
