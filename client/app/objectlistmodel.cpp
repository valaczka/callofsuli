/*
 * ---- Call of Suli ----
 *
 * objectlistmodel.cpp
 *
 * Created on: 2021. 12. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ObjectListModel
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

#include "objectlistmodel.h"

ObjectListModel::ObjectListModel(const QMetaObject *objectType, QObject *parent)
	: QObjectListModel{objectType, true, parent}
	, m_selectedCount{0}
	, m_processing(false)
{
	setEditable(true);

	auto roleIndex = Qt::UserRole + 1;
	for(auto i = 0; i < objectType->propertyCount(); i++) {
		auto prop = objectType->property(i);
		if(prop.hasNotifySignal())
			registerSignalHelper(roleIndex, prop.notifySignal());
		roleIndex++;
	}
}


/**
 * @brief ObjectListModel::ObjectListModel
 * @param objectType
 * @param isOwner
 * @param parent
 */

ObjectListModel::ObjectListModel(const QMetaObject *objectType, const bool &isOwner, QObject *parent)
	: QObjectListModel{objectType, isOwner, parent}
	, m_selectedCount{0}
	, m_processing(false)
{
	setEditable(true);

	auto roleIndex = Qt::UserRole + 1;
	for(auto i = 0; i < objectType->propertyCount(); i++) {
		auto prop = objectType->property(i);
		if(prop.hasNotifySignal())
			registerSignalHelper(roleIndex, prop.notifySignal());
		roleIndex++;
	}
}



/**
 * @brief ObjectListModel::find
 * @param field
 * @param value
 * @return
 */

QList<QObject *> ObjectListModel::find(const char *field, const QVariant &value) const
{
	QList<QObject *> objs;

	foreach (QObject* obj, objects()) {
		QVariant v = obj->property(field);
		if (!v.isValid()) {
			qWarning() << "Invalid unique_field found" << field << value;
			return QObjectList();
		}

		if (v == value)
			objs.append(obj);
	}

	return objs;
}


/**
 * @brief ObjectListModel::index
 * @param object
 * @return
 */

int ObjectListModel::index(QObject *object) const
{
	return QObjectListModel::objects().indexOf(object);
}




/**
 * @brief ObjectListModel::select
 * @param i
 */

void ObjectListModel::select(int i)
{
	ObjectListModelObject *obj = qobject_cast<ObjectListModelObject*>(object(i));
	if (obj)
		obj->setSelected(true);
}

/**
 * @brief ObjectListModel::unselect
 * @param i
 */

void ObjectListModel::unselect(int i)
{
	ObjectListModelObject *obj = qobject_cast<ObjectListModelObject*>(object(i));
	if (obj)
		obj->setSelected(false);
}


/**
 * @brief ObjectListModel::selectToggle
 * @param i
 */

void ObjectListModel::selectToggle(int i)
{
	ObjectListModelObject *obj = qobject_cast<ObjectListModelObject*>(object(i));
	if (obj)
		obj->setSelected(!obj->selected());
}

/**
 * @brief ObjectListModel::selectAll
 */

void ObjectListModel::selectAll()
{
	foreach (QObject* obj, objects()) {
		ObjectListModelObject *o = qobject_cast<ObjectListModelObject*>(obj);
		if (o)
			o->setSelected(true);
	}
}


/**
 * @brief ObjectListModel::unselectAll
 */

void ObjectListModel::unselectAll()
{
	foreach (QObject* obj, objects()) {
		ObjectListModelObject *o = qobject_cast<ObjectListModelObject*>(obj);
		if (o)
			o->setSelected(false);
	}
}


/**
 * @brief ObjectListModel::selectAllToggle
 */

void ObjectListModel::selectAllToggle()
{
	foreach (QObject* obj, objects()) {
		ObjectListModelObject *o = qobject_cast<ObjectListModelObject*>(obj);
		if (o)
			o->setSelected(!o->selected());
	}
}


/**
 * @brief ObjectListModel::nextIntValue
 * @param property
 * @return
 */

int ObjectListModel::nextIntValue(const char *property, const int &start) const
{
	int i = start;
	foreach (QObject* obj, objects()) {
		QVariant v = obj->property(property);
		if (v.isValid() && v.canConvert<int>())
			i = qMax(v.toInt()+1, i);
	}

	return i;
}


/**
 * @brief ObjectListModel::getSelected
 * @return
 */

QList<QObject *> ObjectListModel::getSelected() const
{
	QList<QObject *> list;
	foreach (QObject* obj, objects()) {
		ObjectListModelObject *o = qobject_cast<ObjectListModelObject*>(obj);
		if (o && o->selected()) {
			list.append(obj);
		}
	}
	return list;
}


/**
 * @brief ObjectListModel::getSelectedFields
 * @param field
 * @return
 */

QVariantList ObjectListModel::getSelectedFields(const QString &field) const
{
	QObjectList list = this->getSelected();
	QVariantList ret;

	for (QObject *obj : list) {
		ret.append(obj->property(field.toLatin1()));
	}

	return ret;
}


/**
 * @brief ObjectListModel::removeObject
 * @param index
 */

void ObjectListModel::removeObject(const QModelIndex &index)
{
	QObjectListModel::removeObject(index);
	emit countChanged();
}


/**
 * @brief ObjectListModel::removeObject
 * @param index
 */

void ObjectListModel::removeObject(int index)
{
	QObjectListModel::removeObject(index);
	emit countChanged();
}




/**
 * @brief ObjectListModel::selectedCount
 * @return
 */

int ObjectListModel::selectedCount() const
{
	return m_selectedCount;
}

void ObjectListModel::setSelectedCount(int newSelectedCount)
{
	if (m_selectedCount == newSelectedCount)
		return;
	m_selectedCount = newSelectedCount;
	emit selectedCountChanged();
}

int ObjectListModel::count() const
{
	return objects().size();
}

bool ObjectListModel::processing() const
{
	return m_processing;
}

void ObjectListModel::setProcessing(bool newProcessing)
{
	if (m_processing == newProcessing)
		return;
	m_processing = newProcessing;
	emit processingChanged();
}
