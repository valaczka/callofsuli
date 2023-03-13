/*
 * ---- Call of Suli ----
 *
 * studentgroup.cpp
 *
 * Created on: 2023. 01. 29.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * StudentGroup
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

#include "studentgroup.h"
#include "qjsonobject.h"

StudentGroup::StudentGroup(QObject *parent)
	: QObject{parent}
{

}

/**
 * @brief StudentGroup::loadFromJson
 * @param object
 * @param allField
 */

void StudentGroup::loadFromJson(const QJsonObject &object, const bool &allField)
{
	if (object.contains(QStringLiteral("id")) || allField)
		setGroupid(object.value(QStringLiteral("id")).toInt());

	if (object.contains(QStringLiteral("name")) || allField)
		setName(object.value(QStringLiteral("name")).toString());

	if (object.contains(QStringLiteral("active")) || allField)
		setActive(object.value(QStringLiteral("active")).toInt());
}


/**
 * @brief StudentGroup::groupid
 * @return
 */

int StudentGroup::groupid() const
{
	return m_groupid;
}

void StudentGroup::setGroupid(int newGroupid)
{
	if (m_groupid == newGroupid)
		return;
	m_groupid = newGroupid;
	emit groupidChanged();
}

const QString &StudentGroup::name() const
{
	return m_name;
}

void StudentGroup::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}

bool StudentGroup::active() const
{
	return m_active;
}

void StudentGroup::setActive(bool newActive)
{
	if (m_active == newActive)
		return;
	m_active = newActive;
	emit activeChanged();
}
