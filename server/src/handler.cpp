/*
 * ---- Call of Suli ----
 *
 * handler.cpp
 *
 * Created on: 2023. 03. 13.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Handler
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

#include "handler.h"
#include "Logger.h"
#include "generalapi.h"
#include "authapi.h"
#include "teacherapi.h"
#include "adminapi.h"
#include "panelapi.h"
#include "serverservice.h"


/**
 * @brief Handler::Handler
 * @param parent
 */

Handler::Handler(ServerService *service, QObject *parent)
	: HttpRequestHandler{parent}
	, m_service(service)
{
	LOG_CTRACE("client") << "Handler created" << this;

	m_apiHandlers.insert("admin", new AdminAPI(service));
	m_apiHandlers.insert("auth", new AuthAPI(service));
	m_apiHandlers.insert("general", new GeneralAPI(service));
	m_apiHandlers.insert("teacher", new TeacherAPI(service));
	m_apiHandlers.insert("panel", new PanelAPI(service));
}


/**
 * @brief Handler::~Handler
 */

Handler::~Handler()
{
	foreach (AbstractAPI *api, m_apiHandlers) {
		delete api;
		api = nullptr;
	}

	m_apiHandlers.clear();

	LOG_CTRACE("client") << "Handler destroyed" << this;
}


/**
 * @brief Handler::handle
 * @param request
 * @param response
 */

void Handler::handle(HttpRequest *request, HttpResponse *response)
{
	LOG_CDEBUG("client") << qPrintable(request->method()+QStringLiteral(":")) << qPrintable(request->uriStr())
						 << qPrintable(request->address().toString());

	const QString &method = request->method();
	const QString &uri = request->uriStr();

	if (method == QLatin1String("GET") && uri == QLatin1String("/favicon.ico")) {
		getFavicon(response);
		return;
	}


	if (uri.startsWith(QStringLiteral("/api/")))
		return handleApi(request, response);

	if (uri.startsWith(QStringLiteral("/cb/")))
		return handleOAuthCallback(request, response);

	if (method == QLatin1String("GET"))
		return getWasmContent(uri, response);


	LOG_CWARNING("client") << "Invalid request:" << request->method() << request->uriStr();
	response->setError(HttpStatus::BadRequest, tr("Invalid request"));

}



/**
 * @brief Handler::getFavicon
 * @param response
 */

void Handler::getFavicon(HttpResponse *response)
{
	QByteArray b;
	QFile f(QStringLiteral(":/wasm/cos.png"));

	if (f.exists() && f.open(QIODevice::ReadOnly)) {
		b = f.readAll();
		f.close();
		response->setStatus(HttpStatus::Ok, b);
	} else {
		response->setError(HttpStatus::NotFound, tr("Érvénytelen kérés!"));
	}
}


/**
 * @brief Handler::getWasmContent
 * @param uri
 * @param response
 */

void Handler::getWasmContent(QString uri, HttpResponse *response)
{
	QDir htmlDir(QStringLiteral(":/html"));
	QDir dir(QStringLiteral(":/wasm"));

	uri.remove(QRegExp("^/"));

	if (uri.isEmpty())
		uri = QStringLiteral("index.html");

	QString fname;
	bool isHtml = false;

	if (htmlDir.exists(uri)) {
		fname = htmlDir.absoluteFilePath(uri);
		isHtml = true;
	} else if (dir.exists(uri))
		fname = dir.absoluteFilePath(uri);

	if (!fname.isEmpty()) {
		LOG_CTRACE("client") << "HTTP response file content:" << fname;

		QByteArray b;
		QFile f(fname);
		if (f.open(QIODevice::ReadOnly)) {
			b = f.readAll();
			f.close();
		}

		if (isHtml) {
			b.replace(QByteArrayLiteral("${server:name}"), m_service->serverName().toUtf8());
		}

		response->setStatus(HttpStatus::Ok, b);
		return;
	}

	LOG_CWARNING("client") << "Invalid wasm content request:" << uri;
	response->setError(HttpStatus::NotFound, tr("Érvénytelen kérés!"));
}



/**
 * @brief Handler::handleApi
 * @param request
 * @param response
 */

void Handler::handleApi(HttpRequest *request, HttpResponse *response)
{
	if (request->method() == QLatin1String("GET") || request->method() == QLatin1String("POST")) {
		QRegularExpression exp(QStringLiteral("^/api/(\\w+)/(.*)$"));
		QRegularExpressionMatch match = exp.match(request->uriStr());

		if (match.hasMatch()) {
			const QString &api = match.captured(1);
			const QString &params = match.captured(2);

			for (auto it = m_apiHandlers.constBegin(); it != m_apiHandlers.constEnd(); ++it) {
				if (it.key() == api) {
					it.value()->handle(request, response, params);
					return;
				}
			}
		}
	}

	LOG_CWARNING("client") << "Invalid api request:" << request->uriStr();
	response->setError(HttpStatus::BadRequest, QStringLiteral("invalid request"));
}




/**
 * @brief Handler::handleOAuthCallback
 * @param request
 * @param response
 */

void Handler::handleOAuthCallback(HttpRequest *request, HttpResponse *response)
{
	QRegularExpression exp(QStringLiteral("^/cb/(\\w+)"));
	QRegularExpressionMatch match = exp.match(request->uriStr());

	if (match.hasMatch()) {
		const QString &provider = match.captured(1);

		OAuth2Authenticator *authenticator = m_service->oauth2Authenticator(provider.toUtf8());

		if (!authenticator) {
			LOG_CWARNING("client") << "Invalid provider:" << provider;
			return response->setError(HttpStatus::BadRequest, QStringLiteral("invalid provider"));
		}

		QByteArray content = QStringLiteral("<html><head><meta charset=\"UTF-8\"><title>Call of Suli</title></head><body><p>%1</p></body></html>")
				.arg(tr("A kapcsolatfelvétel sikeres, zárd be ezt lapot.")).toUtf8();

		if (authenticator->parseResponse(request->uriQuery()))
			return response->setStatus(HttpStatus::Ok, content);
		else
			return response->setError(HttpStatus::BadRequest, QStringLiteral("invalid request"));
	}

	LOG_CWARNING("client") << "Invalid request:" << request->method() << request->uriStr();
	response->setError(HttpStatus::BadRequest, QStringLiteral("invalid request"));
}

const QMap<const char *, AbstractAPI *> &Handler::apiHandlers() const
{
	return m_apiHandlers;
}


