/*
 * ---- Call of Suli ----
 *
 * userloglist.h
 *
 * Created on: 2023. 06. 12.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * UserLogList
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

#ifndef USERLOGLIST_H
#define USERLOGLIST_H

#include "httpconnection.h"
#include <QObject>

class UserLogList : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QVariantList model READ model WRITE setModel NOTIFY modelChanged)
	Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
	Q_PROPERTY(HttpConnection::API api READ api WRITE setApi NOTIFY apiChanged)
	Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
	Q_PROPERTY(QVariantList counters READ counters WRITE setCounters NOTIFY countersChanged)

public:
	explicit UserLogList(QObject *parent = nullptr);

	const QString &username() const;
	void setUsername(const QString &newUsername);

	const HttpConnection::API &api() const;
	void setApi(const HttpConnection::API &newApi);

	const QString &path() const;
	void setPath(const QString &newPath);

	const QVariantList &model() const;
	void setModel(const QVariantList &newModel);

	const QVariantList &counters() const;
	void setCounters(const QVariantList &newCounters);

public slots:
	void reload();

signals:
	void usernameChanged();
	void apiChanged();
	void pathChanged();
	void modelChanged();
	void modelReloaded();
	void modelReloadRequested();
	void countersChanged();

private:
	void loadFromJson(const QJsonObject &obj);

	QVariantList m_model;
	QVariantList m_counters;
	QString m_username;
	HttpConnection::API m_api = HttpConnection::ApiGeneral;
	QString m_path = QStringLiteral("user/%1/log");
};

#endif // USERLOGLIST_H
