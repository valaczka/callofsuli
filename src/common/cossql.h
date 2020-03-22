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

public:
	explicit CosSql(QObject *parent = nullptr);
	virtual ~CosSql();


	bool open(const QString &file, bool create = false);
	void close() { m_db.close(); }
	QVariantList simpleQuery(const QString &query, const QVariantList &args = QVariantList(), QVariant *lastInsertId = nullptr);
	bool batchQuery(const QString &query);
	bool batchQueryFromFile(const QString &filename);
	QSqlQuery insertQuery(QString query, const QVariantMap &map = QVariantMap()) const;
	QSqlQuery updateQuery(QString query, const QVariantMap &map = QVariantMap()) const;


	static QString hashPassword (const QString &password, QString *salt = nullptr,
								 QCryptographicHash::Algorithm method = QCryptographicHash::Sha1);

	QSqlDatabase db() { return m_db; }
	bool dbCreated() const { return m_dbCreated; }

private:
	QSqlDatabase m_db;
	bool m_dbCreated;

signals:

};

#endif // COSSQL_H
