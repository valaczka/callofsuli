/*
 * ---- Call of Suli ----
 *
 * studentmap.cpp
 *
 * Created on: 2023. 04. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * StudentMap
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

#include "studentmap.h"

StudentMap::StudentMap(QObject *parent)
	: BaseMap{parent}
{

}


/**
 * @brief StudentMap::loadFromJson
 * @param object
 * @param allField
 */

void StudentMap::loadFromJson(const QJsonObject &object, const bool &allField)
{
	/*if (object.contains(QStringLiteral("version")) || allField)
		setVersion(object.value(QStringLiteral("version")).toInt());

	if (object.contains(QStringLiteral("draftVersion")) || allField)
		setDraftVersion(object.value(QStringLiteral("draftVersion")).toInt());

	if (object.contains(QStringLiteral("cache")) || allField)
		setCache(object.value(QStringLiteral("cache")).toObject());

	if (object.contains(QStringLiteral("lastModified")) || allField)
		setLastModified(QDateTime::fromSecsSinceEpoch(object.value(QStringLiteral("lastModified")).toInteger()));

	if (object.contains(QStringLiteral("lastEditor")) || allField)
		setLastEditor(object.value(QStringLiteral("lastEditor")).toString());*/

	BaseMap::loadFromJson(object, allField);
}
