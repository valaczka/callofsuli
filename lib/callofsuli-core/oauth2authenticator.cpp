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
#include "Logger.h"
#include "oauth2codeflow.h"

OAuth2Authenticator::OAuth2Authenticator(const Type &type, QObject *parent)
	: QObject{parent}
	, m_networkManager(new QNetworkAccessManager(this))
	, m_type(type)
{
	Q_ASSERT(parent);

	LOG_CTRACE("oauth2") << "OAuth2Authenticator created" << m_type << this;
}


/**
 * @brief OAuth2Authenticator::~OAuth2Authenticator
 */

OAuth2Authenticator::~OAuth2Authenticator()
{
	qDeleteAll(m_codeFlowList);

	if (m_handler)
		delete m_handler;

	delete m_networkManager;

	LOG_CTRACE("oauth2") << "OAuth2Authenticator destroyed" << m_type << this;
}


/**
 * @brief OAuth2Authenticator::listen
 */

bool OAuth2Authenticator::listen() const
{
	Q_ASSERT(m_handler);
	return m_handler->listen(m_listenAddress, m_listenPort);
}


/**
 * @brief OAuth2Authenticator::addCodeFlow
 * @param flow
 */

void OAuth2Authenticator::addCodeFlow(OAuth2CodeFlow *flow, QObject *referenceObject)
{
	Q_ASSERT(flow);

	LOG_CDEBUG("oauth2") << "Add new code flow" << flow << "to object" << referenceObject;

	flow->setReplyHandler(m_handler->abstractHandler());

	m_codeFlowList.append(flow);
}


/**
 * @brief OAuth2Authenticator::removeCodeFlow
 * @param flow
 */

void OAuth2Authenticator::removeCodeFlow(OAuth2CodeFlow *flow)
{
	if (m_codeFlowList.contains(flow)) {
		LOG_CTRACE("oauth2") << "Remove and delete code flow:" << flow;
		m_codeFlowList.removeAll(flow);
		flow->deleteLater();
	} else {
		LOG_CWARNING("oauth2") << "Code flow not found:" << flow;
	}
}





/**
 * @brief OAuth2Authenticator::getCodeFlowForClient
 * @param client
 * @return
 */

OAuth2CodeFlow *OAuth2Authenticator::getCodeFlowForReferenceObject(QObject *referenceObject) const
{
	if (!referenceObject)
		return nullptr;

	foreach (OAuth2CodeFlow *f, m_codeFlowList)
		if (f->referenceObject()  == referenceObject)
			return f;

	return nullptr;
}


/**
 * @brief OAuth2Authenticator::getCodeFlowForStatus
 * @param status
 * @return
 */

OAuth2CodeFlow *OAuth2Authenticator::getCodeFlowForState(const QString &state) const
{
	foreach (OAuth2CodeFlow *f, m_codeFlowList)
		if (f->state() == state)
			return f;

	return nullptr;
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



QNetworkAccessManager *OAuth2Authenticator::networkManager() const
{
	return m_networkManager;
}



const QString &OAuth2Authenticator::redirectHost() const
{
	return m_redirectHost;
}

void OAuth2Authenticator::setRedirectHost(const QString &newRedirectHost)
{
	m_redirectHost = newRedirectHost;
}

AbstractReplyHandler *OAuth2Authenticator::handler() const
{
	return m_handler;
}

void OAuth2Authenticator::setHandler(AbstractReplyHandler *newHandler)
{
	m_handler = newHandler;
	m_handler->setAuthenticator(this);
}

OAuth2Authenticator::Type OAuth2Authenticator::type() const
{
	return m_type;
}



