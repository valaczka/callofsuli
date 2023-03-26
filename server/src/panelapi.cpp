/*
 * ---- Call of Suli ----
 *
 * panelapi.cpp
 *
 * Created on: 2023. 03. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * PanelAPI
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

#include "panelapi.h"
#include "serverservice.h"


/**
 * @brief PanelAPI::PanelAPI
 * @param service
 */

PanelAPI::PanelAPI(ServerService *service)
	: AbstractAPI(service)
{
	m_validateRole = Credential::Panel;

	addMap("^/*$", this, &PanelAPI::panelCreate);
	addMap("^(\\d+)/*$", this, &PanelAPI::panel);
	addMap("^(\\d+)/live/*$", this, &PanelAPI::panelLive);
}




/**
 * @brief PanelAPI::panel
 * @param match
 * @param response
 */

void PanelAPI::panel(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	LOG_CTRACE("client") << "Get panel" << id;

	Panel *panel = m_service->panel(id);

	if (!panel)
		return responseError(response, "invalid uuid");

	responseAnswer(response, panel->getEventData());
}




/**
 * @brief PanelAPI::panelLive
 * @param match
 * @param response
 */

void PanelAPI::panelLive(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	LOG_CTRACE("client") << "Get panel" << id;

	Panel *panel = m_service->panel(id);

	if (!panel)
		return responseError(response, "invalid uuid");

	if (panel->stream())
		return responseError(response, "stream used");

	HttpConnection *conn = qobject_cast<HttpConnection*>(response->parent());
	HttpEventStream *stream = new HttpEventStream(conn);
	conn->setEventStream(stream);
	m_service->addEventStream(stream);
	panel->setStream(stream);

	response->setStatus(HttpStatus::Ok);

	panel->sendConfig();
}




/**
 * @brief PanelAPI::panelCreate
 * @param response
 */

void PanelAPI::panelCreate(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const
{
	Panel *panel = new Panel(m_service);
	m_service->addPanel(panel);

	LOG_CTRACE("client") << "Create panel" << panel->id();

	responseAnswerOk(response, QJsonObject({{QStringLiteral("id"), panel->id()}}));
}
