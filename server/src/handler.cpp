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
#include "adminapi.h"
#include "authapi.h"
#include "peerengine.h"
#include "teacherapi.h"
#include "userapi.h"
#include "serverservice.h"
#include <QtConcurrent/QtConcurrent>


/**
 * @brief Handler::Handler
 * @param parent
 */

Handler::Handler(ServerService *service, QObject *parent)
	: QObject{parent}
	, m_service(service)
{
	Q_ASSERT(m_service);

	LOG_CTRACE("service") << "Handler created" << this;
}


/**
 * @brief Handler::~Handler
 */

Handler::~Handler()
{
	LOG_CTRACE("service") << "Handler destroyed" << this;
}


/**
 * @brief Handler::httpServer
 * @return
 */

QHttpServer *Handler::httpServer() const
{
	const auto &ptr = m_service->webServer().lock().get();

	Q_ASSERT(ptr);

	return ptr->server();

}





/**
 * @brief Handler::loadRoutes
 * @return
 */

bool Handler::loadRoutes()
{
	LOG_CTRACE("service") << "Load routes";

	auto server = httpServer();

	if (!server)
		return false;

	server->route("/favicon.ico", QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		return getFavicon(request);
	});

	server->route("/content/", QHttpServerRequest::Method::Get,
				  [this](const QString &fname, const QHttpServerRequest &request){
		return getDynamicContent(fname, request);
	});


	server->setMissingHandler(server, [this](const QHttpServerRequest &request, QHttpServerResponder &responder){
		const QString &callbackPath = QStringLiteral("/")+OAuth2Authenticator::callbackPath()+QStringLiteral("/");
		const QString &requestPath = request.url().path();

		if (requestPath.startsWith(callbackPath)) {
			responder.sendResponse(std::move(getCallback(request)));
		} else if (requestPath.startsWith(AbstractAPI::apiPath())) {
			authorizeRequestLog(request);
			responder.sendResponse(std::move(AbstractAPI::responseError("invalid api request", QHttpServerResponse::StatusCode::NotFound)));
		} else
			responder.sendResponse(std::move(getStaticContent(request)));
	});


	// Load APIs

	addApi(std::make_unique<GeneralAPI>(this, m_service));
	addApi(std::make_unique<AdminAPI>(this, m_service));
	addApi(std::make_unique<AuthAPI>(this, m_service));
	addApi(std::make_unique<TeacherAPI>(this, m_service));
	addApi(std::make_unique<UserAPI>(this, m_service));

	return true;
}



/**
 * @brief Handler::logRequest
 * @param request
 */

std::optional<Credential> Handler::authorizeRequest(const QHttpServerRequest &request) const
{
	QByteArray token = request.value(QByteArrayLiteral("Authorization"));

	if (token.isEmpty())
		return std::nullopt;

	static const char* bearer = "Bearer ";

	if (token.startsWith(bearer))
		token.remove(0, strlen(bearer));

	if (!Credential::verify(token, m_service->settings()->jwtSecret(), m_service->config().get("tokenFirstIat").toInteger(0))) {
		LOG_CDEBUG("client") << "Token verification failed";
		return std::nullopt;
	}

	Credential c = Credential::fromJWT(token);

	if (!c.isValid()) {
		LOG_CDEBUG("client") << "Invalid token";
		return std::nullopt;
	}

	return c;
}


/**
 * @brief Handler::authorizeRequestLog
 * @param request
 * @return
 */

std::optional<Credential> Handler::authorizeRequestLog(const QHttpServerRequest &request) const
{
	const auto &credential = authorizeRequest(request);

	/*for (const auto &h : request.headers()) {
		LOG_CTRACE("service") << "--" << h.first << ":" << h.second;
	}*/

	const QByteArray &userAgent = request.value(QByteArrayLiteral("User-Agent"));

	QByteArray method;

	switch (request.method()) {
		case QHttpServerRequest::Method::Get: method = QByteArrayLiteral("GET"); break;
		case QHttpServerRequest::Method::Put: method = QByteArrayLiteral("PUT"); break;
		case QHttpServerRequest::Method::Post: method = QByteArrayLiteral("POST"); break;
		case QHttpServerRequest::Method::Delete: method = QByteArrayLiteral("DELETE"); break;
		case QHttpServerRequest::Method::Head: method = QByteArrayLiteral("HEAD"); break;
		case QHttpServerRequest::Method::Patch: method = QByteArrayLiteral("PATCH"); break;
		default: method = QByteArrayLiteral("INVALID"); break;
	}

	LOG_CDEBUG("service") << method.constData() << qPrintable(request.url().path())
						  << qPrintable(request.remoteAddress().toString()) << request.remotePort()
						  << (credential ? qPrintable(credential->username()) : "")
						  << (userAgent.isEmpty() ? "" : (QByteArrayLiteral("[")+userAgent+QByteArrayLiteral("]")).constData());

	if (credential) {
		const auto &list = m_service->engineHandler()->engineGet<PeerEngine>(AbstractEngine::EnginePeer);
		PeerUser user;
		user.setUsername(credential->username());
		user.setHost(request.remoteAddress());
		user.setAgent(userAgent);

		for (auto e : list) {
			e->logPeerUser(user);
		}
	}

	return credential;
}



/**
 * @brief Handler::getFavicon
 * @param request
 * @return
 */

QHttpServerResponse Handler::getFavicon(const QHttpServerRequest &request)
{
	authorizeRequestLog(request);

	const QString &fname = QStringLiteral(":/wasm/cos.png");

	if (QFile::exists(fname))
		return QHttpServerResponse::fromFile(fname);
	else
		return getErrorPage(tr("Hiányzó fájl"));
}



/**
 * @brief Handler::getStaticContent
 * @param request
 * @return
 */

QHttpServerResponse Handler::getStaticContent(const QHttpServerRequest &request)
{
	authorizeRequestLog(request);

	QString path = request.url().path();

	QDir htmlDir(QStringLiteral(":/html"));
	QDir dir(QStringLiteral(":/wasm"));

	if (path.startsWith('/'))
		path.removeFirst();

	if (path.isEmpty())
		path = QStringLiteral("index.html");

	QString fname;
	bool isHtml = false;

	if (htmlDir.exists(path)) {
		fname = htmlDir.absoluteFilePath(path);
		isHtml = true;
	} else if (dir.exists(path))
		fname = dir.absoluteFilePath(path);

	if (!fname.isEmpty() && QFile::exists(fname)) {
		LOG_CTRACE("service") << "HTTP response file content:" << fname;

		if (isHtml) {
			QByteArray b;
			QFile f(fname);
			if (f.open(QIODevice::ReadOnly)) {
				b = f.readAll();
				f.close();
			}

			QByteArray contentType;
			const auto &server= m_service->webServer().lock();

			QByteArray hostname = QStringLiteral("callofsuli://%1:%2").arg(server ? server->redirectHost() : QStringLiteral("invalid"))
								  .arg(m_service->settings()->listenPort()).toUtf8();

			if (m_service->settings()->ssl())
				hostname.append(QByteArrayLiteral("/?ssl=1"));

			b.replace(QByteArrayLiteral("${server:name}"), m_service->serverName().toUtf8())
					.replace(QByteArrayLiteral("${server:connect}"), hostname);

			if (fname.endsWith(QStringLiteral("css")))
				contentType = QByteArrayLiteral("text/css");
			if (fname.endsWith(QStringLiteral("js")))
				contentType = QByteArrayLiteral("text/javascript");
			if (fname.endsWith(QStringLiteral("html")) || fname.endsWith(QStringLiteral("htm")))
				contentType = QByteArrayLiteral("text/html");

			return QHttpServerResponse(contentType, b, QHttpServerResponder::StatusCode::Ok);
		}

		if (QFile::exists(fname)) {
			if (fname.endsWith(QStringLiteral("html")) || fname.endsWith(QStringLiteral("htm"))) {
				QByteArray b;
				QFile f(fname);
				if (f.open(QIODevice::ReadOnly)) {
					b = f.readAll();
					f.close();
				}

				return QHttpServerResponse(QByteArrayLiteral("text/html"), b, QHttpServerResponder::StatusCode::Ok);
			} else
				return QHttpServerResponse::fromFile(fname);
		} else
			return getErrorPage(tr("A fájl nem található"));
	}

	LOG_CWARNING("service") << "Invalid static content request:" << path;
	return getErrorPage(tr("Érvénytelen útvonal"));
}




/**
 * @brief Handler::getCallback
 * @param request
 * @return
 */

QHttpServerResponse Handler::getCallback(const QHttpServerRequest &request)
{
	static const QRegularExpression exp(QStringLiteral("^/cb/(\\w+)"));
	const QRegularExpressionMatch &match = exp.match(request.url().path());

	if (match.hasMatch()) {
		const QString &provider = match.captured(1);

		const auto &ptr = m_service->oauth2Authenticator(provider.toUtf8());

		if (!ptr) {
			LOG_CWARNING("client") << "Invalid provider:" << provider;
			return AbstractAPI::responseError("invalid provider", QHttpServerResponse::StatusCode::BadRequest);
		}

		OAuth2Authenticator *authenticator = ptr->lock().get();
		OAuth2CodeFlow *flow = authenticator->parseResponse(std::move(QUrlQuery(request.url())));

		if (!flow)
			return AbstractAPI::responseError("invalid request", QHttpServerResponse::StatusCode::BadRequest);

		QByteArray content;

		if (flow->isWasm()) {
			QFile f(QStringLiteral(":/html/callback_wasm.html"));
			if (f.open(QIODevice::ReadOnly)) {
				content = f.readAll();
				f.close();
			}
		} else {
			QFile f(QStringLiteral(":/html/callback.html"));
			if (f.open(QIODevice::ReadOnly)) {
				content = f.readAll();
				f.close();
			}
		}

		if (!content.isEmpty()) {
			content.replace(QByteArrayLiteral("${server:name}"), m_service->serverName().toUtf8());
		} else if (flow->isWasm()) {
			content = QStringLiteral("<html><head><meta charset=\"UTF-8\"><title>Call of Suli</title></head><body>"
									 "<p>%1</p>"
									 "</body></html>")
					  .arg(tr("A kapcsolatfelvétel sikeres, zárd be ezt a lapot."))
					  .toUtf8();
		} else {
			content = QStringLiteral("<html><head><meta charset=\"UTF-8\"><title>Call of Suli</title></head><body>"
									 "<p>%1</p>"
									 "<p><a href=\"callofsuli://\">%2</a></p>"
									 "</body></html>")
					  .arg(tr("A kapcsolatfelvétel sikeres, zárd be ezt a lapot."), tr("Vissza az alkalmazásba"))
					  .toUtf8();
		}


		return QHttpServerResponse(QByteArrayLiteral("text/html"), content, QHttpServerResponder::StatusCode::Ok);

	}

	LOG_CWARNING("service") << "Invalid request:" << request.url().path();
	return AbstractAPI::responseError("invalid request", QHttpServerResponse::StatusCode::BadRequest);
}


/**
 * @brief Handler::getDynamicContent
 * @param request
 * @return
 */

QHttpServerResponse Handler::getDynamicContent(const QString &fname, const QHttpServerRequest &request)
{
	authorizeRequestLog(request);

	const QString &file = m_service->settings()->dataDir().absoluteFilePath(QStringLiteral("content/")+fname);

	if (QFile::exists(file))
		return QHttpServerResponse::fromFile(file);
	else
		return getErrorPage(tr("Hiányzó fájl"));
}



/**
 * @brief Handler::addApi
 * @param api
 */

void Handler::addApi(std::unique_ptr<AbstractAPI> api)
{
	LOG_CTRACE("service") << "Add API" << api->path() << api.get();
	m_apis.insert({api->path(), std::move(api)});
}



/**
 * @brief Handler::getErrorPage
 * @param errorString
 * @param code
 * @return
 */

QHttpServerResponse Handler::getErrorPage(const QString &message, const QHttpServerResponse::StatusCode &code)
{
	QByteArray b;
	QFile f(QStringLiteral(":/html/html_error.html"));

	if (f.open(QIODevice::ReadOnly)) {
		b = f.readAll();
		f.close();
	}

	const auto &server= m_service->webServer().lock();

	QByteArray hostname = QStringLiteral("callofsuli://%1:%2").arg(server ? server->redirectHost() : QStringLiteral("invalid"))
						  .arg(m_service->settings()->listenPort()).toUtf8();

	if (m_service->settings()->ssl())
		hostname.append(QByteArrayLiteral("/?ssl=1"));


	QByteArray str;

	switch (code) {
		case QHttpServerResponder::StatusCode::Ok:
			str = tr("Ok").toUtf8();
			break;
		case QHttpServerResponder::StatusCode::NotFound:
			str = tr("A kért oldal nem található").toUtf8();
			break;
		case QHttpServerResponder::StatusCode::Unauthorized:
			str = tr("Azonosítás szükséges").toUtf8();
			break;
		default:
			str = tr("Hibás kérés").toUtf8();
			break;
	}

	b.replace(QByteArrayLiteral("${server:name}"), m_service->serverName().toUtf8())
			.replace(QByteArrayLiteral("${server:connect}"), hostname)
			.replace(QByteArrayLiteral("${statusCode}"), QByteArray::number(static_cast<int>(code)))
			.replace(QByteArrayLiteral("${message}"), message.toUtf8())
			.replace(QByteArrayLiteral("${statusStr}"), str)
			;

	return QHttpServerResponse(QByteArrayLiteral("text/html"), b, QHttpServerResponder::StatusCode::Ok);
}



/**
 * @brief Handler::api
 * @param path
 * @return
 */

AbstractAPI *Handler::api(const char *path) const
{
	for (const auto &[p, ptr] : m_apis) {
		if (std::strcmp(path, p) == 0) {
			return ptr.get();
		}
	}

	return nullptr;
}


/**
 * @brief Handler::verifyPeer
 * @param request
 * @return
 */

bool Handler::verifyPeer(const QHttpServerRequest &request) const
{
	const auto &message = request.body();

	if (!m_service->settings()->verifyPeer())
		return true;

	const QByteArray &userAgentSign = request.value(QByteArrayLiteral("User-Agent-Sign"));


	const auto &list = m_service->agentSignatures();

	for (const QByteArray &key : list) {
		if (QMessageAuthenticationCode::hash(message, key, QCryptographicHash::Sha3_256).toBase64() == userAgentSign) {
			return true;
		}
	}

	LOG_CWARNING("service") << "Peer verification failed"
						  << qPrintable(request.remoteAddress().toString()) << request.remotePort()
						  << (userAgentSign.isEmpty() ? "" : (QByteArrayLiteral("[")+userAgentSign+QByteArrayLiteral("]")).constData());

	return false;
}


