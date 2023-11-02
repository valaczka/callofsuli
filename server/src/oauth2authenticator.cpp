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
#include "serverservice.h"

const char* OAuth2Authenticator::m_callbackPath = "cb";

OAuth2Authenticator::OAuth2Authenticator(const char *type, ServerService *service)
	: QObject{service}
	, m_codeFlowList(std::make_shared<QVector<std::shared_ptr<OAuth2CodeFlow> > >())
	, m_handler(new OAuth2ReplyHandler(this))
	, m_type(type)
	, m_service(service)
{
	Q_ASSERT(m_service);

	LOG_CTRACE("oauth2") << "OAuth2Authenticator created" << m_type << this;
}


/**
 * @brief OAuth2Authenticator::~OAuth2Authenticator
 */

OAuth2Authenticator::~OAuth2Authenticator()
{
	LOG_CTRACE("oauth2") << "OAuth2Authenticator destroyed" << m_type << this;
}


/**
 * @brief OAuth2Authenticator::addCodeFlow_
 * @return
 */

std::weak_ptr<OAuth2CodeFlow> OAuth2Authenticator::addCodeFlow()
{
	auto f = std::make_shared<OAuth2CodeFlow>(this);

	f->setReplyHandler(m_handler.get());

	m_codeFlowList->append(std::move(f));

	const auto &flow = m_codeFlowList->last();

	setCodeFlow(flow);

	LOG_CDEBUG("oauth2") << "Add new code flow" << flow.get()->state();

	return flow;
}




/**
 * @brief OAuth2Authenticator::removeCodeFlow
 * @param flow
 */

void OAuth2Authenticator::removeCodeFlow(OAuth2CodeFlow *flow)
{
	LOG_CTRACE("oauth2") << "Remove code flow:" << flow;

	for (auto it = m_codeFlowList->constBegin(); it != m_codeFlowList->constEnd(); ) {
		if (it->get() == flow) {
			it = m_codeFlowList->erase(it);
		} else
			++it;
	}
}





/**
 * @brief OAuth2Authenticator::getCodeFlowForStatus
 * @param status
 * @return
 */

std::optional<std::weak_ptr<OAuth2CodeFlow>> OAuth2Authenticator::getCodeFlowForState(const QString &state) const
{
	foreach (const auto &f, (*m_codeFlowList.get()))
		if (f->state() == state)
			return std::weak_ptr<OAuth2CodeFlow>(f);

	return std::nullopt;
}




ServerService *OAuth2Authenticator::service() const
{
	return m_service;
}



/**
 * @brief OAuth2Authenticator::updateUserInfoFromIdToken
 * @param data
 */

AdminAPI::User OAuth2Authenticator::getUserInfoFromIdToken(const QJsonObject &data) const
{
	AdminAPI::User user;

	bool emailVerified = data.value(QStringLiteral("email_verified")).toBool();

	if (!emailVerified) {
		LOG_CDEBUG("oauth2") << "Email not verified";
		return user;
	}

	const QString &email = data.value(QStringLiteral("email")).toString();

	if (email.isEmpty()) {
		LOG_CDEBUG("oauth2") << "Missing email";
		return user;
	}

	user.username = email;
	user.familyName = data.value(QStringLiteral("family_name")).toString();
	user.givenName = data.value(QStringLiteral("given_name")).toString();
	user.picture = data.value(QStringLiteral("picture")).toString();

	return user;
}


/**
 * @brief OAuth2Authenticator::callbackPath
 * @return
 */

const char *OAuth2Authenticator::callbackPath()
{
	return m_callbackPath;
}

const ServerSettings::OAuth &OAuth2Authenticator::oauth() const
{
	return m_oauth;
}

void OAuth2Authenticator::setOAuth(const ServerSettings::OAuth &newOauth)
{
	m_oauth = newOauth;
}





/**
 * @brief OAuth2Authenticator::type
 * @return
 */

const char *OAuth2Authenticator::type() const
{
	return m_type;
}





/**
 * @brief OAuth2ReplyHandler::OAuth2ReplyHandler
 * @param service
 */

OAuth2ReplyHandler::OAuth2ReplyHandler(OAuth2Authenticator *authenticator)
	: QAbstractOAuthReplyHandler(authenticator)
	, m_authenticator(authenticator)
{
	Q_ASSERT(m_authenticator);

	LOG_CTRACE("oauth2") << "Reply handler created" << this;
}


/**
 * @brief OAuth2ReplyHandler::~OAuth2ReplyHandler
 */

OAuth2ReplyHandler::~OAuth2ReplyHandler()
{
	LOG_CTRACE("oauth2") << "Reply handler destroyed" << this;
}



/**
 * @brief OAuth2ReplyHandler::callback
 * @return
 */

QString OAuth2ReplyHandler::callback() const
{
	Q_ASSERT(m_authenticator);

	const QUrl url(QString::fromLatin1("%1://%2:%3/%4/%5")
				   .arg(m_authenticator->service()->settings()->ssl() ? QStringLiteral("https") : QStringLiteral("http"))
				   .arg(m_authenticator->service()->webServer().lock()->redirectHost())
				   .arg(m_authenticator->service()->settings()->listenPort())
				   .arg(m_authenticator->callbackPath())
				   .arg(m_authenticator->oauth().path)
				   );
	return url.toString(QUrl::EncodeDelimiters);
}


