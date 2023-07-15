/*
 * ---- Call of Suli ----
 *
 * grade.cpp
 *
 * Created on: 2023. 04. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Grade
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

#include "grade.h"
#include "qjsonobject.h"

Grade::Grade(QObject *parent)
	: SelectableObject{parent}
{

}


/**
 * @brief Grade::loadFromJson
 * @param object
 * @param allField
 */

void Grade::loadFromJson(const QJsonObject &object, const bool &allField)
{
	if (object.contains(QStringLiteral("id")) || allField)
		setGradeid(object.value(QStringLiteral("id")).toInt());

	if (object.contains(QStringLiteral("shortname")) || allField)
		setShortname(object.value(QStringLiteral("shortname")).toString());

	if (object.contains(QStringLiteral("longname")) || allField)
		setLongname(object.value(QStringLiteral("longname")).toString());

	if (object.contains(QStringLiteral("value")) || allField)
		setValue(object.value(QStringLiteral("value")).toInt());
}



/**
 * @brief Grade::gradeid
 * @return
 */

int Grade::gradeid() const
{
	return m_gradeid;
}

void Grade::setGradeid(int newGradeid)
{
	if (m_gradeid == newGradeid)
		return;
	m_gradeid = newGradeid;
	emit gradeidChanged();
}

const QString &Grade::shortname() const
{
	return m_shortname;
}

void Grade::setShortname(const QString &newShortname)
{
	if (m_shortname == newShortname)
		return;
	m_shortname = newShortname;
	emit shortnameChanged();
}

const QString &Grade::longname() const
{
	return m_longname;
}

void Grade::setLongname(const QString &newLongname)
{
	if (m_longname == newLongname)
		return;
	m_longname = newLongname;
	emit longnameChanged();
}

int Grade::value() const
{
	return m_value;
}

void Grade::setValue(int newValue)
{
	if (m_value == newValue)
		return;
	m_value = newValue;
	emit valueChanged();
}
