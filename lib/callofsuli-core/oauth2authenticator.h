/*
 * ---- Call of Suli ----
 *
 * oauth2authenticator.h
 *
 * Created on: 2023. 01. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * OAuth2Authenticator
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

#ifndef OAUTH2AUTHENTICATOR_H
#define OAUTH2AUTHENTICATOR_H

#include <QObject>
#include <QtNetworkAuth>
#include "oauth2replyhandler.h"
#include "oauth2codeflow.h"

class OAuth2Authenticator : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString clientId READ clientId WRITE setClientId NOTIFY clientIdChanged)
	Q_PROPERTY(QString clientKey READ clientKey WRITE setClientKey NOTIFY clientKeyChanged)
	Q_PROPERTY(QHostAddress listenAddress READ listenAddress WRITE setListenAddress NOTIFY listenAddressChanged)
	Q_PROPERTY(quint16 listenPort READ listenPort WRITE setListenPort NOTIFY listenPortChanged)

public:
	enum Type {
		Invalid,
		Google
	};

	Q_ENUM(Type)

	explicit OAuth2Authenticator(const Type &type, QObject *parent = nullptr);
	virtual ~OAuth2Authenticator();

	bool listen() const;

	void addCodeFlow(OAuth2CodeFlow *flow, QObject *referenceObject);
	void removeCodeFlow(OAuth2CodeFlow *flow);
	OAuth2CodeFlow *getCodeFlowForReferenceObject(QObject *referenceObject) const;
	OAuth2CodeFlow *getCodeFlowForState(const QString &status) const;

	const QString &clientId() const;
	void setClientId(const QString &newClientId);

	const QString &clientKey() const;
	void setClientKey(const QString &newClientKey);

	const QHostAddress &listenAddress() const;
	void setListenAddress(const QHostAddress &newListenAddress);

	quint16 listenPort() const;
	void setListenPort(quint16 newListenPort);

	QNetworkAccessManager *networkManager() const;

	const QString &redirectHost() const;
	void setRedirectHost(const QString &newRedirectHost);

	AbstractReplyHandler *handler() const;
	void setHandler(AbstractReplyHandler *newHandler);

	template <typename T2>
	T2* createHandler() {
		OAuth2ReplyHandler<T2> *h = new OAuth2ReplyHandler<T2>(m_listenAddress, m_listenPort, this);
		m_handler = h;
		return h->handler();
	};

	Type type() const;

signals:
	void clientIdChanged();
	void clientKeyChanged();
	void listenAddressChanged();
	void listenPortChanged();

protected:
	AbstractReplyHandler *m_handler = nullptr;
	QNetworkAccessManager *m_networkManager = nullptr;
	QVector<QPointer<OAuth2CodeFlow>> m_codeFlowList;

	QString m_clientId;
	QString m_clientKey;
	QHostAddress m_listenAddress = QHostAddress::Any;
	quint16 m_listenPort = 0;
	QString m_redirectHost;

private:
	const Type m_type = Invalid;

};



#endif // OAUTH2AUTHENTICATOR_H
