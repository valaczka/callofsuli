/*
 * ---- Call of Suli ----
 *
 * objectlistmodelobject.cpp
 *
 * Created on: 2021. 12. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ObjectListModelObject
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

#include "objectlistmodelobject.h"


ObjectListModelObject::ObjectListModelObject(QObject *parent)
	: QObject{parent}
	, m_selected(false)
{

}

/**
 * @brief ObjectListModelObject::~ObjectListModelObject
 */

ObjectListModelObject::~ObjectListModelObject()
{

}



/**
 * @brief ObjectListModelObject::selected
 * @return
 */

bool ObjectListModelObject::selected() const
{
	return m_selected;
}

/**
 * @brief ObjectListModelObject::setSelected
 * @param newSelected
 */

void ObjectListModelObject::setSelected(bool newSelected)
{
	if (m_selected == newSelected)
		return;
	m_selected = newSelected;
	emit selectedChanged();
}



/**
 * @brief ObjectListModelObject::toJsonObject
 * @return
 */

QJsonObject ObjectListModelObject::toJsonObject() const
{
	QJsonObject jsonObject;

	const QMetaObject *metaobject = this->metaObject();

	int count = metaobject->propertyCount();
	for (int i=0; i<count; ++i) {
		QMetaProperty metaproperty = metaobject->property(i);
		if (metaproperty.isReadable() && metaproperty.isStored() && metaproperty.name() != QStringLiteral("objectName"))
			jsonObject.insert(metaproperty.name(), QJsonValue::fromVariant(metaproperty.read(this)));
	}

	return jsonObject;
}




/**
 * @brief ObjectListModelObject::updateProperties
 * @param other
 */

void ObjectListModelObject::updateProperties(QObject *other)
{
	const QMetaObject *metaobject = this->metaObject();

	int count = metaobject->propertyCount();
	for (int i=0; i<count; ++i) {
		QMetaProperty metaproperty = metaobject->property(i);
		if (metaproperty.isWritable())
			metaproperty.write(this, metaproperty.read(other));
	}
}


/**
 * @brief ObjectListModelObject::fromJsonObjectPrivate
 * @param objectType
 * @param jsonObject
 * @param parent
 * @return
 */

QObject *ObjectListModelObject::fromJsonObjectPrivate(const QMetaObject objectType, const QJsonObject &jsonObject, QObject *parent)
{
	QObject* object = objectType.newInstance(Q_ARG(QObject*, parent));

	int count = objectType.propertyCount();
	for (int i=0; i<count; ++i) {
		QMetaProperty metaproperty = objectType.property(i);
		const char *propName = metaproperty.name();

		if (jsonObject.contains(propName) && metaproperty.isWritable())
			metaproperty.write(object, jsonObject.value(propName).toVariant());
	}

	return object;
}


