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

#include "httpRequestHandler.h"
#include "abstractapi.h"

class ServerService;

/**
 * @brief The Handler class
 */

class Handler : public HttpRequestHandler
{
	Q_OBJECT

public:
	explicit Handler(ServerService *service, QObject *parent = nullptr);
	virtual ~Handler();

	void handle(HttpRequest *request, HttpResponse *response);

	const QMap<const char *, AbstractAPI *> &apiHandlers() const;

private:
	void getFavicon(HttpResponse *response);
	void getWasmContent(QString uri, HttpResponse *response);
	void handleApi(HttpRequest *request, HttpResponse *response);
	void handleOAuthCallback(HttpRequest *request, HttpResponse *response);

	QMap<const char*, AbstractAPI*> m_apiHandlers;
	ServerService *m_service = nullptr;
};

#endif // HANDLER_H
