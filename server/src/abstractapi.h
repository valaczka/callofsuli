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
#include "qhttpserverresponse.h"
#include "databasemain.h"
#include <QPointer>


#define DEFAULT_LIMIT	50


class ServerService;
class Handler;


/**
 * @brief The AbstractAPI class
 */

class AbstractAPI : public QObject
{
	Q_OBJECT

public:
	AbstractAPI(const char *path, Handler *handler, ServerService *service);
	virtual ~AbstractAPI();

	const char *path() const;

	static QHttpServerResponse responseOk();
	static QHttpServerResponse responseOk(QJsonObject object);
	static QHttpServerResponse responseResult(const char *field, const QJsonValue &value);
	static QHttpServerResponse responseError(const char *errorStr, const QHttpServerResponse::StatusCode &code = QHttpServerResponse::StatusCode::Ok);
	static QHttpServerResponse responseErrorSql();

	static const char *apiPath();

	DatabaseMain *databaseMain() const;
	QLambdaThreadWorker *databaseMainWorker() const;

protected:

	static const char *m_apiPath;
	ServerService *m_service = nullptr;
	Handler *m_handler = nullptr;
	const char *m_path = nullptr;

	Credential::Roles m_validateRole = Credential::None;
};



// AUTHORIZATION

#define AUTHORIZE_API()	\
	const auto &credential = m_handler->authorizeRequestLog(request);\
	if (m_validateRole != Credential::None && (!credential || !(credential->roles() & m_validateRole)))\
		return responseError("unauthorized request", QHttpServerResponse::StatusCode::Unauthorized);

#define AUTHORIZE_API_X(role)	\
	const auto &credential = m_handler->authorizeRequestLog(request);\
	if ((role) != Credential::None && (!credential || !(credential->roles() & (role))))\
		return responseError("unauthorized request", QHttpServerResponse::StatusCode::Unauthorized);

#define AUTHORIZE_FUTURE_API()	\
	const auto &credential = m_handler->authorizeRequestLog(request);\
	if (m_validateRole != Credential::None && (!credential || !(credential->roles() & m_validateRole)))\
		return QtConcurrent::run(&AbstractAPI::responseError, "unauthorized request", QHttpServerResponse::StatusCode::Unauthorized);

#define AUTHORIZE_FUTURE_API_X(role)	\
	const auto &credential = m_handler->authorizeRequestLog(request);\
	if ((role) != Credential::None && (!credential || !(credential->roles() & (role))))\
		return QtConcurrent::run(&AbstractAPI::responseError, "unauthorized request", QHttpServerResponse::StatusCode::Unauthorized);



// CONTENT

#define JSON_OBJECT_GET()	const auto &jsonObject = Utils::byteArrayToJsonObject(request.body());

#define JSON_OBJECT_ASSERT() \
	JSON_OBJECT_GET(); \
	const QByteArray &mimeType = request.value(QByteArrayLiteral("Content-Type")); \
	if ((!mimeType.isEmpty() && mimeType.compare(QByteArrayLiteral("application/json")) != 0) || !jsonObject) \
		return QtConcurrent::run(&AbstractAPI::responseError, "invalid content", QHttpServerResponse::StatusCode::BadRequest);


// LAMBDA THREAD

#define LAMBDA_THREAD_BEGIN(...)	\
	QDefer ret;\
	QHttpServerResponse response(QHttpServerResponse::StatusCode::InternalServerError);\
	databaseMainWorker()->execInThread([&response, ret, this, __VA_ARGS__]() mutable {\
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());\
		QMutexLocker(databaseMain()->mutex());

#define LAMBDA_THREAD_BEGIN_NOVAR()	\
	QDefer ret;\
	QHttpServerResponse response(QHttpServerResponse::StatusCode::InternalServerError);\
	databaseMainWorker()->execInThread([&response, ret, this]() mutable {\
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());\
		QMutexLocker(databaseMain()->mutex());

#define LAMBDA_THREAD_END	\
		ret.resolve(); \
	}); \
	QDefer::await(ret); \
	return response;


#define LAMBDA_SQL_ASSERT(opt)		if (!(opt)) { response = responseErrorSql(); return ret.reject();	}
#define LAMBDA_SQL_ASSERT_ROLLBACK(opt)		if (!(opt)) { response = responseErrorSql(); db.rollback(); return ret.reject();	}
#define LAMBDA_SQL_ERROR(errStr, opt)		if (!(opt)) { response = responseError(errStr); return ret.reject();	}
#define LAMBDA_SQL_ERROR_ROLLBACK(errStr, opt)		if (!(opt)) { response = responseError(errStr); db.rollback(); return ret.reject();	}

#endif // ABSTRACTAPI_H
