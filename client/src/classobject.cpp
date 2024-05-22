/*
 * ---- Call of Suli ----
 *
 * classobject.cpp
 *
 * Created on: 2023. 01. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ClassObject
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

#include "classobject.h"
#include "qjsonobject.h"

ClassObject::ClassObject(QObject *parent)
	: SelectableObject{parent}
{

}


/**
 * @brief ClassObject::loadFromJson
 * @param object
 * @param allField
 */

void ClassObject::loadFromJson(const QJsonObject &object, const bool &allField)
{
	if (object.contains(QStringLiteral("id")) || allField)
		setClassid(object.value(QStringLiteral("id")).toInt());

	if (object.contains(QStringLiteral("name")) || allField)
		setName(object.value(QStringLiteral("name")).toString());

	if (object.contains(QStringLiteral("dailyLimit")) || allField)
		setDailyLimit(object.value(QStringLiteral("dailyLimit")).toInt());
}



const QString &ClassObject::name() const
{
	return m_name;
}

void ClassObject::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}

int ClassObject::classid() const
{
	return m_classid;
}

void ClassObject::setClassid(int newClassid)
{
	if (m_classid == newClassid)
		return;
	m_classid = newClassid;
	emit classidChanged();
}

int ClassObject::dailyLimit() const
{
	return m_dailyLimit;
}

void ClassObject::setDailyLimit(int newDailyLimit)
{
	if (m_dailyLimit == newDailyLimit)
		return;
	m_dailyLimit = newDailyLimit;
	emit dailyLimitChanged();
}
