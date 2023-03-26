/*
 * ---- Call of Suli ----
 *
 * teacherapi.h
 *
 * Created on: 2023. 03. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherAPI
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

#ifndef TEACHERAPI_H
#define TEACHERAPI_H

#include "abstractapi.h"
#include "qjsonarray.h"

class TeacherAPI : public AbstractAPI
{
public:
	TeacherAPI(ServerService *service);

	void groupOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void groups(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;
	void groupCreate(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void groupUpdate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void groupDeleteOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		groupDelete({match.captured(1).toInt()}, response);
	}
	void groupDelete(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const {
		groupDelete(data.value(QStringLiteral("list")).toArray(), response);
	}
	void groupDelete(const QJsonArray &list, const QPointer<HttpResponse> &response) const;



	void groupClassAddOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		groupClassAdd(match.captured(1).toInt(), {match.captured(2).toInt()}, response);
	}
	void groupClassAdd(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const {
		groupClassAdd(match.captured(1).toInt(), data.value(QStringLiteral("list")).toArray(), response);
	}
	void groupClassAdd(const int &id, const QJsonArray &list, const QPointer<HttpResponse> &response) const;

	void groupClassRemoveOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		groupClassRemove(match.captured(1).toInt(), {match.captured(2).toInt()}, response);
	}
	void groupClassRemove(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const {
		groupClassRemove(match.captured(1).toInt(), data.value(QStringLiteral("list")).toArray(), response);
	}
	void groupClassRemove(const int &id, const QJsonArray &list, const QPointer<HttpResponse> &response) const;
	void groupClassExclude(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;



	void groupUserAddOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		groupUserAdd(match.captured(1).toInt(), {match.captured(2)}, response);
	}
	void groupUserAdd(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const {
		groupUserAdd(match.captured(1).toInt(), data.value(QStringLiteral("list")).toArray(), response);
	}
	void groupUserAdd(const int &id, const QJsonArray &list, const QPointer<HttpResponse> &response) const;

	void groupUserRemoveOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		groupUserRemove(match.captured(1).toInt(), {match.captured(2)}, response);
	}
	void groupUserRemove(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const {
		groupUserRemove(match.captured(1).toInt(), data.value(QStringLiteral("list")).toArray(), response);
	}
	void groupUserRemove(const int &id, const QJsonArray &list, const QPointer<HttpResponse> &response) const;
	void groupUserExclude(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;


	void panelOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void panels(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;
	void panelGrab(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void panelRelease(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void panelUpdate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;
};

#endif // TEACHERAPI_H
