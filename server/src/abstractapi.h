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
#include "httpRequest.h"
#include "databasemain.h"

typedef std::function<void(const QString &, const QJsonObject &, HttpResponse *)> ApiMapFunction;

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
		const char *method;
		const char *name;
		ApiMapFunction func;

		Map(const char *m, const char *n, ApiMapFunction f) : method(m), name(n), func(f) {}
	};

	virtual void handle(HttpRequest *request, HttpResponse *response, const QString &parameters);


	DatabaseMain *databaseMain() const;
	QLambdaThreadWorker *databaseMainWorker() const;


	template <typename T>
	void addMap(const char *method, const char *name, T *inst,
				void (T::*handler)(const QString &, const QJsonObject &, HttpResponse *) const)
	{
		Map m = {method, name, std::bind(handler, inst, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)};
		m_maps.append(m);
	}

	QVector<Map>::const_iterator findMap(const char *method, const char *name);

	void responseError(HttpResponse *response, const char *errorStr, const HttpStatus &status = HttpStatus::InternalServerError) const;
	void responseAnswer(HttpResponse *response, const char *field, const QJsonValue &value) const;
	void responseAnswer(HttpResponse *response, const QJsonObject &value) const;

protected:
	QVector<Map> m_maps;
	ServerService *m_service = nullptr;
};

#endif // ABSTRACTAPI_H
