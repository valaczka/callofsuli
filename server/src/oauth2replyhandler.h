/*
 * ---- Call of Suli ----
 *
 * oauth2replyhandler.h
 *
 * Created on: 2023. 01. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * OAuth2ReplyHandler
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

#ifndef OAUTH2REPLYHANDLER_H
#define OAUTH2REPLYHANDLER_H


#include "qoauthhttpserverreplyhandler.h"
#include <QAbstractOAuthReplyHandler>

class OAuth2Authenticator;


class _Handler : public QOAuthHttpServerReplyHandler
{
	Q_OBJECT

public:
	explicit _Handler(OAuth2Authenticator *authenticator, const QHostAddress &address, quint16 port, QObject *parent = nullptr)
		: QOAuthHttpServerReplyHandler(address, port, parent)
		, m_authenticator(authenticator)
	{}

	QString callback() const override;

private:
	OAuth2Authenticator *m_authenticator = nullptr;

};



/**
 * @brief The OAuth2ReplyHandler class
 */


class OAuth2ReplyHandler : public QObject
{
	Q_OBJECT

public:
	explicit OAuth2ReplyHandler(OAuth2Authenticator *authenticator);
	virtual ~OAuth2ReplyHandler();

	bool listen(const QHostAddress &address, quint16 port);

	OAuth2Authenticator *authenticator() const;
	QOAuthHttpServerReplyHandler *handler() const;

private slots:
	void onCallbackReceived(const QVariantMap &data);

private:
	QOAuthHttpServerReplyHandler *m_handler = nullptr;
	OAuth2Authenticator *const m_authenticator = nullptr;
};

#endif // OAUTH2REPLYHANDLER_H
