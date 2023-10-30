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


const char *AbstractAPI::m_apiPath = "/api/";


/**
 * @brief AbstractAPI::~AbstractAPI
 */

AbstractAPI::AbstractAPI(const char *path, Handler *handler, ServerService *service)
	: QObject(handler)
	, m_service(service)
	, m_handler(handler)
	, m_path(path)
{
	Q_ASSERT(handler);
	Q_ASSERT(service);

	QByteArray p = m_apiPath;
	p.append(path);

	LOG_CTRACE("service") << "Abstract API created:" << m_path << this;
}



/**
 * @brief AbstractAPI::~AbstractAPI
 */

AbstractAPI::~AbstractAPI()
{
	LOG_CTRACE("service") << "Abstract API destroyed:" << m_path << this;
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
	return m_service->databaseMainWorker();
}


/**
 * @brief AbstractAPI::apiPath
 * @return
 */

const char *AbstractAPI::apiPath()
{
	return m_apiPath;
}


/**
 * @brief AbstractAPI::responseError
 * @param errorStr
 * @return
 */

QHttpServerResponse AbstractAPI::responseError(const char *errorStr, const QHttpServerResponse::StatusCode &code)
{
	LOG_CTRACE("service") << "-> RESPONSE ERROR" << errorStr << code;
	return QHttpServerResponse(QJsonObject{
								   { QStringLiteral("error"), errorStr }
							   }, code);
}


/**
 * @brief AbstractAPI::responseErrorSql
 * @return
 */

QHttpServerResponse AbstractAPI::responseErrorSql()
{
	LOG_CTRACE("service") << "-> RESPONSE SQL ERROR";
	return QHttpServerResponse(QJsonObject{
								   { QStringLiteral("error"), QStringLiteral("sql error") }
							   }, QHttpServerResponse::StatusCode::InternalServerError);
}


/**
 * @brief AbstractAPI::path
 * @return
 */

const char *AbstractAPI::path() const
{
	return m_path;
}



/**
 * @brief AbstractAPI::responseOk
 * @return
 */

QHttpServerResponse AbstractAPI::responseOk()
{
	return responseResult("status", QStringLiteral("ok"));
}


/**
 * @brief AbstractAPI::responseOk
 * @param object
 * @return
 */

QHttpServerResponse AbstractAPI::responseOk(QJsonObject object)
{
	object.insert(QStringLiteral("status"), QStringLiteral("ok"));
	return QHttpServerResponse(object);
}


/**
 * @brief AbstractAPI::responseResult
 * @param field
 * @param value
 * @return
 */

QHttpServerResponse AbstractAPI::responseResult(const char *field, const QJsonValue &value)
{
	return QHttpServerResponse(QJsonObject{
								   { field, value }
							   }, QHttpServerResponse::StatusCode::Ok);
}

