/*
 * ---- Call of Suli ----
 *
 * googleoauth2.cpp
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


#include <QDesktopServices>

#include "googleoauth2.h"

GoogleOAuth2::GoogleOAuth2(QObject *parent)
	: QObject(parent)
	, m_oauth(this)
	, m_handler(nullptr)
{
	m_oauth.setAuthorizationUrl(QUrl("https://accounts.google.com/o/oauth2/auth"));
	m_oauth.setAccessTokenUrl(QUrl("https://oauth2.googleapis.com/token"));
	m_oauth.setScope("email profile");

	connect(&m_oauth, &QOAuth2AuthorizationCodeFlow::statusChanged, this, [=](QAbstractOAuth::Status status) {
		if (status == QAbstractOAuth::Status::Granted) {
			emit authenticated(m_oauth.token(), m_oauth.expirationAt().toString("yyyy-MM-dd HH:mm:ss"), m_oauth.refreshToken());
		}
	});

	m_oauth.setModifyParametersFunction([&](QAbstractOAuth::Stage stage, QVariantMap *parameters) {
		if (stage == QAbstractOAuth::Stage::RequestingAccessToken) {
			QByteArray code = parameters->value("code").toByteArray();
			(*parameters)["code"] = QUrl::fromPercentEncoding(code);
		}
	});

	connect(&m_oauth, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &GoogleOAuth2::browserRequest);
}



/**
 * @brief GoogleOAuth2::~GoogleOAuth2
 */

GoogleOAuth2::~GoogleOAuth2()
{
	if (m_handler)
		delete m_handler;

	m_handler = nullptr;
}

/**
 * @brief GoogleOAuth2::id
 * @return
 */

const QString GoogleOAuth2::id() const
{
	return m_oauth.clientIdentifier();
}


/**
 * @brief GoogleOAuth2::key
 * @return
 */

const QString GoogleOAuth2::key() const
{
	return m_oauth.clientIdentifierSharedKey();
}


/**
 * @brief GoogleOAuth2::port
 * @return
 */

qint16 GoogleOAuth2::handlerPort() const
{
	if (m_handler)
		return m_handler->port();
	else
		return -1;
}






/**
 * @brief GoogleOAuth2::setClient
 * @param id
 * @param key
 */

void GoogleOAuth2::setClient(const QString &id, const QString &key, const qint16 &port)
{
	m_oauth.setClientIdentifier(id);
	m_oauth.setClientIdentifierSharedKey(key);
	if (!m_handler) {
		if (port > 0)
			m_handler = new QOAuthHttpServerReplyHandler(port, this);
		else
			m_handler = new QOAuthHttpServerReplyHandler(this);
	}

	m_oauth.setReplyHandler(m_handler);

	qDebug() << "Listen on port" << m_handler->port();
}



/**
 * @brief GoogleOAuth2::grant
 */

void GoogleOAuth2::grant()
{
	m_oauth.grant();
}
