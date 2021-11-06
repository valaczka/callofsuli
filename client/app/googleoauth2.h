/*
 * ---- Call of Suli ----
 *
 * googleoauth2.h
 *
 * Created on: 2021. 11. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GoogleOAuth2
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

#ifndef GOOGLEOAUTH2_H
#define GOOGLEOAUTH2_H

#include <QObject>
#include <QtNetwork>
#include <QtNetworkAuth>
#include <QOAuth2AuthorizationCodeFlow>

class GoogleOAuth2 : public QObject
{
	Q_OBJECT

public:
	explicit GoogleOAuth2(QObject *parent = nullptr);
	~GoogleOAuth2();

	const QString id() const;
	const QString key() const;
	qint16 handlerPort() const;

public slots:
	void setClient(const QString &id, const QString &key);
	void setHandlerPort(const qint16 &port);
	void grant();

signals:
	void browserRequest(const QUrl &url);
	void authenticated(const QString &token);

private:
	QOAuth2AuthorizationCodeFlow m_oauth;
	QOAuthHttpServerReplyHandler *m_handler;
};

#endif // GOOGLEOAUTH2_H
