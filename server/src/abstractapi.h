/*
 * ---- Call of Suli ----
 *
 * apihandler.h
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

#ifndef ABSTRACTAPI_H
#define ABSTRACTAPI_H

#include <QDeferred>
#include <QLambdaThreadWorker>
#include "credential.h"
#include "httpRequest.h"
#include "databasemain.h"
#include <QPointer>
#include "httpResponse.h"

typedef std::function<void(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse>)> ApiMapFunction;

class ServerService;

/**
 * @brief The AbstractAPI class
 */

class AbstractAPI
{
public:
	AbstractAPI(ServerService *service);
	virtual ~AbstractAPI() {}

	struct Map {
		const char *regularExpression;
		ApiMapFunction func;

		Map(const char *n, ApiMapFunction f) : regularExpression(n), func(f) {}
	};

	virtual void handle(HttpRequest *request, HttpResponse *response, const QString &parameters);


	DatabaseMain *databaseMain() const;
	QLambdaThreadWorker *databaseMainWorker() const;


	template <typename T>
	void addMap(const char *regularExpression, T *inst,
				void (T::*handler)(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse>) const)
	{
		Map m = {regularExpression, std::bind(handler, inst, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)};
		m_maps.append(m);
	}


	void responseError(HttpResponse *response, const char *errorStr) const;
	void responseAnswer(HttpResponse *response, const char *field, const QJsonValue &value) const;
	void responseAnswer(HttpResponse *response, const QJsonObject &value) const;
	void responseAnswerOk(HttpResponse *response, QJsonObject value = {}) const;
	void responseErrorSql(HttpResponse *response) const {
		responseError(response, "sql error");
	}


	Credential authorize(HttpRequest *request) const;
	bool validate(const Credential &credential, const Credential::Role &role) const;
	bool validate(HttpRequest *request, const Credential::Role &role) const;

	const QVector<Map> &maps() const;

protected:
	QVector<Map> m_maps;
	ServerService *m_service = nullptr;
	Credential m_credential;
	Credential::Role m_validateRole = Credential::Role::None;
};

#endif // ABSTRACTAPI_H
