/*
 * ---- Call of Suli ----
 *
 * panelapi.h
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

#ifndef PANELAPI_H
#define PANELAPI_H

#include "abstractapi.h"

class PanelAPI : public AbstractAPI
{

public:
	PanelAPI(ServerService *service);

	void panel(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void panelLive(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void panelCreate(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;
};

#endif // PANELAPI_H
