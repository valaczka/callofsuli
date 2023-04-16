/*
 * ---- Call of Suli ----
 *
 * studentapi.h
 *
 * Created on: 2023. 04. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * StudentAPI
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

#ifndef USERAPI_H
#define USERAPI_H

#include "abstractapi.h"

class UserAPI : public AbstractAPI
{
public:
	UserAPI(ServerService *service);


	void groups(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;

	void campaigns(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;
	void campaignOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;

	void maps(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;
	void mapOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;

};

#endif // USERAPI_H
