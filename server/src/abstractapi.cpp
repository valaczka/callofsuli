/*
 * ---- Call of Suli ----
 *
 * apihandler.cpp
 *
 * Created on: 2023. 03. 13.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ApiHandler
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

#include "abstractapi.h"
#include "Logger.h"
#include "serverservice.h"

AbstractAPI::AbstractAPI(ServerService *service)
	: m_service(service)
{
	Q_ASSERT(m_service);
	Q_ASSERT(m_service->databaseMain());
}


/**
 * @brief AbstractAPI::handle
 * @param request
 * @param response
 * @param method
 * @param parameters
 */

void AbstractAPI::handle(HttpRequest *request, HttpResponse *response, const QString &parameters)
{
	if (m_validateRole != Credential::Role::None) {
		LOG_CTRACE("client") << "Validate role:" << m_validateRole;

		const Credential &credential = authorize(request);

		if (!credential.isValid()) {
			LOG_CWARNING("client") << "Unauthorized request:" << request->uriStr();
			return response->setError(HttpStatus::Unauthorized, QStringLiteral("invalid token"));
		}

		if (!validate(request, m_validateRole)) {
			LOG_CWARNING("client") << "Permission denied:" << m_validateRole << request->uriStr();
			return response->setError(HttpStatus::Forbidden, QStringLiteral("permission denied"));
		}
	}

	QJsonObject data;

	QVector<AbstractAPI::Map>::const_iterator it = m_maps.constBegin();

	QRegularExpressionMatch match;

	for (; it != m_maps.constEnd(); ++it) {
		if (strcmp(it->method, request->method().toLatin1()) == 0) {
			QRegularExpression r(it->regularExpression);
			QRegularExpressionMatch m = r.match(parameters);
			if (m.hasMatch()) {
				match = m;
				break;
			}
		}
	}

	if (it == m_maps.constEnd()) {
		LOG_CWARNING("client") << "Invalid api function:" << request->uriStr();
		return responseError(response, "invalid function");
	}

	const QString &d = request->parseBodyStr();

	if (!d.isEmpty()) {
		if (request->mimeType().compare("application/json", Qt::CaseInsensitive) != 0)
			return responseError(response, "Request body content type must be application/json");

		const QJsonDocument &jsonDocument = request->parseJsonBody();
		if (jsonDocument.isNull())
			return responseError(response, "invalid json");

		data = jsonDocument.object();
	}

	LOG_CTRACE("client") << "Handle API request:" << parameters << data;

	it->func(match, data, response);
}




/**
 * @brief AbstractAPI::databaseMain
 * @return
 */

DatabaseMain *AbstractAPI::databaseMain() const
{
	return m_service->databaseMain();
}


/**
 * @brief AbstractAPI::databaseMainWorker
 * @return
 */

QLambdaThreadWorker *AbstractAPI::databaseMainWorker() const
{
	return m_service->databaseMain()->worker();
}





/**
 * @brief AbstractAPI::responseError
 * @param reponse
 * @param errorStr
 */

void AbstractAPI::responseError(HttpResponse *response, const char *errorStr) const
{
	if (!response) {
		LOG_CTRACE("client") << "Reponse null";
		return;
	}

	QJsonDocument doc{QJsonObject{
		{ QStringLiteral("error"), errorStr }
	}};

	response->setStatus(HttpStatus::Ok, doc);
}


/**
 * @brief AbstractAPI::responseAnswer
 * @param response
 * @param field
 * @param value
 */

void AbstractAPI::responseAnswer(HttpResponse *response, const char *field, const QJsonValue &value) const
{
	if (!response) {
		LOG_CTRACE("client") << "Reponse null";
		return;
	}

	responseAnswer(response, QJsonObject{
					   { field, value }
				   });
}


/**
 * @brief AbstractAPI::responseAnswer
 * @param response
 * @param value
 */

void AbstractAPI::responseAnswer(HttpResponse *response, const QJsonObject &value) const
{
	if (!response) {
		LOG_CTRACE("client") << "Reponse null";
		return;
	}

	response->setStatus(HttpStatus::Ok, QJsonDocument{value});
}




/**
 * @brief AbstractAPI::authorize
 * @param request
 * @return
 */

Credential AbstractAPI::authorize(HttpRequest *request) const
{
	LOG_CTRACE("client") << "Authorize request";

	const QString &token = request->headerDefault(QStringLiteral("Authorization"), QString());

	if (token.isEmpty()) {
		LOG_CTRACE("client") << "Missing token";
		return Credential();
	}

	if (!Credential::verify(token, m_service->settings()->jwtSecret())) {
		LOG_CDEBUG("client") << "Token verification failed";
		return Credential();
	}

	Credential c = Credential::fromJWT(token);

	if (!c.isValid())
		LOG_CDEBUG("client") << "Invalid token";

	return c;
}


/**
 * @brief AbstractAPI::validate
 * @param credential
 * @param role
 * @return
 */

bool AbstractAPI::validate(const Credential &credential, const Credential::Role &role) const
{
	return (credential.isValid() && credential.roles().testFlag(role));
}


/**
 * @brief AbstractAPI::validate
 * @param request
 * @param role
 * @return
 */

bool AbstractAPI::validate(HttpRequest *request, const Credential::Role &role) const
{
	return validate(authorize(request), role);
}



