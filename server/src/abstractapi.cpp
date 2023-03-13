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
	QString func;
	QString params;
	QJsonObject data;

	QRegularExpression exp(QStringLiteral("^(\\w+)/(.*)$"));
	const QRegularExpressionMatch &match = exp.match(parameters);

	if (match.hasMatch()) {
		func = match.captured(1);
		params = match.captured(2);
	} else {
		QRegularExpression exp(QStringLiteral("^(\\w+)$"));
		const QRegularExpressionMatch &match = exp.match(parameters);

		if (match.hasMatch()) {
			func = match.captured(1);
		}
	}

	if (func.isEmpty()) {
		LOG_CWARNING("client") << "Invalid api request:" << request->uriStr();
		return response->setError(HttpStatus::BadRequest, QStringLiteral("invalid request"));
	}


	auto it = findMap(request->method().toLatin1(), func.toLatin1());

	if (it == m_maps.constEnd()) {
		LOG_CWARNING("client") << "Invalid api function:" << request->uriStr();
		return response->setError(HttpStatus::BadRequest, QStringLiteral("invalid function"));
	}

	const QString &d = request->parseBodyStr();

	if (!d.isEmpty()) {
		if (request->mimeType().compare("application/json", Qt::CaseInsensitive) != 0)
			return response->setError(HttpStatus::BadRequest, QStringLiteral("Request body content type must be application/json"));

		const QJsonDocument &jsonDocument = request->parseJsonBody();
		if (jsonDocument.isNull())
			return response->setError(HttpStatus::BadRequest, QStringLiteral("invalid json"));

		data = jsonDocument.object();
	}

	LOG_CTRACE("client") << "Handle API request:" << func << params << data;

	it->func(params, data, response);
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
 * @brief AbstractAPI::findMap
 * @param method
 * @param name
 * @return
 */

QVector<AbstractAPI::Map>::const_iterator AbstractAPI::findMap(const char *method, const char *name)
{
	QVector<AbstractAPI::Map>::const_iterator it = m_maps.constBegin();

	for (; it != m_maps.constEnd(); ++it) {
		if (strcmp(it->method, method) == 0 && strcmp(it->name, name) == 0)
			return it;
	}

	return it;
}


/**
 * @brief AbstractAPI::responseError
 * @param reponse
 * @param errorStr
 */

void AbstractAPI::responseError(HttpResponse *response, const char *errorStr, const HttpStatus &status) const
{
	if (!response) {
		LOG_CTRACE("client") << "Reponse null";
		return;
	}

	QJsonDocument doc{QJsonObject{
		{ QStringLiteral("error"), errorStr }
	}};

	response->setStatus(status, doc);
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




