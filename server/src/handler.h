/*
 * ---- Call of Suli ----
 *
 * handler.h
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

#ifndef HANDLER_H
#define HANDLER_H

#include "qhttpserver.h"
#include <QObject>
#include "abstractapi.h"

class ServerService;

/**
 * @brief The Handler class
 */

class Handler : public QObject
{
	Q_OBJECT

public:
	explicit Handler(ServerService *service, QObject *parent = nullptr);
	virtual ~Handler();

	QHttpServer* httpServer() const;
	bool loadRoutes();

	std::optional<Credential> authorizeRequest(const QHttpServerRequest &request) const;
	std::optional<Credential> authorizeRequestLog(const QHttpServerRequest &request) const;
	QHttpServerResponse getErrorPage(const QString &errorString, const QHttpServerResponse::StatusCode &code = QHttpServerResponse::StatusCode::NotFound);

	AbstractAPI *api(const char *path) const;

	template <typename T,
			  typename = std::enable_if< std::is_base_of<AbstractAPI, T>::value>::type>
	T *api(const char *path) const {
		return dynamic_cast<T*>(api(path));
	}

	bool verifyPeer(const QHttpServerRequest &request) const;

private:
	QHttpServerResponse getFavicon(const QHttpServerRequest &request);
	QHttpServerResponse getStaticContent(const QHttpServerRequest &request);
	QHttpServerResponse getCallback(const QHttpServerRequest &request);
	QHttpServerResponse getDynamicContent(const QString &fname, const QHttpServerRequest &request);

	void addApi(std::unique_ptr<AbstractAPI> api);

	ServerService *m_service = nullptr;

	std::map<const char*, std::unique_ptr<AbstractAPI>> m_apis;
};

#endif // HANDLER_H
