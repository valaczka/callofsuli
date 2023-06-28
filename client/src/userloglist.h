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

#include "websocket.h"
#include <QObject>

class UserLogList : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QVariantList model READ model WRITE setModel NOTIFY modelChanged)
	Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
	Q_PROPERTY(WebSocket::API api READ api WRITE setApi NOTIFY apiChanged)
	Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)

public:
	explicit UserLogList(QObject *parent = nullptr);

	const QString &username() const;
	void setUsername(const QString &newUsername);

	const WebSocket::API &api() const;
	void setApi(const WebSocket::API &newApi);

	const QString &path() const;
	void setPath(const QString &newPath);

	const QVariantList &model() const;
	void setModel(const QVariantList &newModel);

public slots:
	void reload();

signals:
	void usernameChanged();
	void apiChanged();
	void pathChanged();
	void modelChanged();
	void modelReloaded();
	void modelReloadRequested();

private:
	void loadFromJson(const QJsonObject &obj);

	QVariantList m_model;
	QString m_username;
	WebSocket::API m_api = WebSocket::ApiGeneral;
	QString m_path = QStringLiteral("user/%1/log");
};

#endif // USERLOGLIST_H
