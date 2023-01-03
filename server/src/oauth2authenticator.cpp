/*
 * ---- Call of Suli ----
 *
 * oauth2authenticator.cpp
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

#include "oauth2authenticator.h"
#include "serverservice.h"
#include "oauth2codeflow.h"

OAuth2Authenticator::OAuth2Authenticator(ServerService *service)
	: QObject{service}
	, m_service(service)
	, m_handler(new QOAuthHttpServerReplyHandler(this))
{
	LOG_CTRACE("oauth2") << "OAuth2Authenticator created" << this;

	setClientId("822493401756-d8ooae3ja5s9g5a1u7ld5b38es1no61f.apps.googleusercontent.com");
	setClientKey("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");
	setListenPort(15151);

}


/**
 * @brief OAuth2Authenticator::~OAuth2Authenticator
 */

OAuth2Authenticator::~OAuth2Authenticator()
{
	qDeleteAll(m_codeFlowList);
	delete m_handler;

	LOG_CTRACE("oauth2") << "OAuth2Authenticator destroyed" << this;
}


/**
 * @brief OAuth2Authenticator::listen
 */

bool OAuth2Authenticator::listen() const
{
	if (m_handler->isListening())
		m_handler->close();

	LOG_CINFO("oauth2") << "OAuth2Handler listening on" << m_listenAddress << m_listenPort;
	return m_handler->listen(m_listenAddress, m_listenPort);
}


/**
 * @brief OAuth2Authenticator::addCodeFlow
 * @param flow
 */

void OAuth2Authenticator::addCodeFlow(OAuth2CodeFlow *flow, Client *client)
{
	Q_ASSERT(flow);
	Q_ASSERT(client);

	LOG_CDEBUG("oauth2") << "Add new code flow" << flow << "to client" << client;

	connect(flow, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, client, &Client::requestOAuth2Browser);

	/***
	 * Q_SIGNALS:
	void callbackReceived(const QVariantMap &values);
	void tokensReceived(const QVariantMap &tokens);

	void replyDataReceived(const QByteArray &data);
	void callbackDataReceived(const QByteArray &data);

	*/

	//flow->setReplyHandler(m_handler);
	m_codeFlowList.append(flow);
}



/**
 * @brief OAuth2Authenticator::clientId
 * @return
 */

const QString &OAuth2Authenticator::clientId() const
{
	return m_clientId;
}

void OAuth2Authenticator::setClientId(const QString &newClientId)
{
	if (m_clientId == newClientId)
		return;
	m_clientId = newClientId;
	emit clientIdChanged();
}

const QString &OAuth2Authenticator::clientKey() const
{
	return m_clientKey;
}

void OAuth2Authenticator::setClientKey(const QString &newClientKey)
{
	if (m_clientKey == newClientKey)
		return;
	m_clientKey = newClientKey;
	emit clientKeyChanged();
}

const QHostAddress &OAuth2Authenticator::listenAddress() const
{
	return m_listenAddress;
}

void OAuth2Authenticator::setListenAddress(const QHostAddress &newListenAddress)
{
	if (m_listenAddress == newListenAddress)
		return;
	m_listenAddress = newListenAddress;
	emit listenAddressChanged();
}

quint16 OAuth2Authenticator::listenPort() const
{
	return m_listenPort;
}

void OAuth2Authenticator::setListenPort(quint16 newListenPort)
{
	if (m_listenPort == newListenPort)
		return;
	m_listenPort = newListenPort;
	emit listenPortChanged();
}




