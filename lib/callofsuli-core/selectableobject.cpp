/*
 * ---- Call of Suli ----
 *
 * selectableobject.cpp
 *
 * Created on: 2023. 01. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * SelectableObject
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

#include "selectableobject.h"
#include "qjsonobject.h"
#include "qmetaobject.h"

SelectableObject::SelectableObject(QObject *parent)
	: QObject{parent}
{

}

bool SelectableObject::selected() const
{
	return m_selected;
}

void SelectableObject::setSelected(bool newSelected)
{
	if (m_selected == newSelected)
		return;
	m_selected = newSelected;
	emit selectedChanged();
}


/**
 * @brief SelectableObject::toVariantMap
 * @return
 */

QVariantMap SelectableObject::toVariantMap() const
{
	QVariantMap map;

	const QMetaObject* meta = this->metaObject();

	for (int i = 0 ; i < meta->propertyCount(); i++) {
		const QMetaProperty &property = meta->property(i);

		if (!property.isValid() || !property.isReadable() || !property.isStored())
			continue;

		const QString &p = property.name();
		const QVariant &value = this->property(p.toUtf8());

		const MapConvertFunc &f = m_mapConvertFuncs.value(p);

		SelectableObject *obj = qvariant_cast<SelectableObject*>(value);

		if (f)
			map.insert(p, f(value));
		else if (obj)
			map.insert(p, obj->toVariantMap());
		else
			map.insert(p, value);
	}

	return map;
}




/**
 * @brief SelectableObject::toJsonObject
 * @return
 */

QJsonObject SelectableObject::toJsonObject() const
{
	QJsonObject map;

	const QMetaObject* meta = this->metaObject();

	for (int i = 0 ; i < meta->propertyCount(); i++) {
		const QMetaProperty &property = meta->property(i);

		if (!property.isValid() || !property.isReadable() || !property.isStored())
			continue;

		const QString &p = property.name();
		const QVariant &value = this->property(p.toUtf8());

		const JsonConvertFunc &f = m_jsonConvertFuncs.value(p);

		SelectableObject *obj = qvariant_cast<SelectableObject*>(value);

		if (f)
			map.insert(p, f(value));
		else if (obj)
			map.insert(p, obj->toJsonObject());
		else
			map.insert(p, value.toJsonValue());
	}

	return map;
}
