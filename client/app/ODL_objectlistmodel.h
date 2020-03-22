/*
 * ---- Call of Suli ----
 *
 * objectlistmodel.h
 *
 * Created on: 2021. 12. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#ifndef ODL_OBJECTLISTMODEL_H
#define ODL_OBJECTLISTMODEL_H

#include "qgenericlistmodel.h"
#include "objectlistmodelobject.h"
#include <QJsonArray>


template <typename TData, typename = void>
class ObjectListModel;

template <typename TObject>
using ObjectListModel_Object_SFINAE = typename std::enable_if<std::is_base_of<ObjectListModelObject, TObject>::value>::type;

template <typename TObject>
class ObjectListModel<TObject, ObjectListModel_Object_SFINAE<TObject>> : public QGenericListModel<TObject>
{
public:
	explicit ObjectListModel(QObject *parent = nullptr);

	void addObject(TObject *object);

	Q_INVOKABLE QJsonArray toJsonArray(const bool &withSelectedState = false) const;
	Q_INVOKABLE static ObjectListModel<TObject>* fromJsonArray(QObject *parent, const QJsonArray &array);
	Q_INVOKABLE static ObjectListModel<TObject>* fromJsonArray(const QJsonArray &array)
	{
		return fromJsonArray(nullptr, array);
	}

	Q_INVOKABLE void appendJsonArray(QObject *parent, const QJsonArray &array);
	Q_INVOKABLE void appendJsonArray(const QJsonArray &array)
	{
		appendJsonArray(this->parent(), array);
	}

	Q_INVOKABLE void updateJsonArray(const QJsonArray &array, const char *unique_field);

	QList<TObject *> find(const char *field, const QVariant &value);

private slots:
	void onSelectedChanged();

};


/**
 * @brief ::onSelectedChanged
 */

template<typename TObject>
void ObjectListModel<TObject, ObjectListModel_Object_SFINAE<TObject>>::onSelectedChanged()
{
	qDebug() << "ONSELECTEDCHANGED";
	this->QObjectListModel::setSelectedCount(5);
}


/**
 * @brief ::addObject
 */

template<typename TObject>
void ObjectListModel<TObject, ObjectListModel_Object_SFINAE<TObject>>::addObject(TObject *object)
{
	Q_ASSERT(object);

	QObject::connect(object, &TObject::selectedChanged, this, &ObjectListModel<TObject>::onSelectedChanged);

	this->QObjectListModel::addObject(object);
}


/**
 * @brief ::find
 * @param unique_field
 */

template<typename TObject>
QList<TObject *> ObjectListModel<TObject, ObjectListModel_Object_SFINAE<TObject> >::find(const char *field, const QVariant &value)
{
	Q_ASSERT(field);

	QList<TObject *> objects;

	const QObjectList objs = this->QObjectListModel::objects();

	for (QObject* obj : objs) {
		QVariant v = obj->property(field);
		if (!v.isValid()) {
			qWarning() << "Invalid unique_field found" << field << value;
			return QList<TObject *>();
		}

		if (v == value)
			objects.append(static_cast<TObject *>(obj));
	}

	return objects;
}






/**
 * @brief ObjectListModel::ObjectListModel
 * @param parent
 */

template<typename TObject>
ObjectListModel<TObject, ObjectListModel_Object_SFINAE<TObject>>::ObjectListModel(QObject *parent)
	: QGenericListModel<TObject> (true, parent)
{
	this->setEditable(true);
}




/**
 * @brief ::toJsonArray
 * @param withSelectedState
 */

template<typename TObject>
QJsonArray ObjectListModel<TObject, ObjectListModel_Object_SFINAE<TObject>>::toJsonArray(const bool &withSelectedState) const
{
	QJsonArray array;
	const QList<TObject *> objs = this->QGenericListModel<TObject>::objects();
	for(TObject* obj : objs) {
		QJsonObject json = obj->toJsonObject(withSelectedState);
		array.append(json);
	}
	return array;
}



/**
 * @brief ::fromJsonArray
 * @param parent
 * @param array
 * @param unique_field
 * @param withSelected
 */

template<typename TObject>
ObjectListModel<TObject> *ObjectListModel<TObject, ObjectListModel_Object_SFINAE<TObject>>::fromJsonArray(QObject *parent, const QJsonArray &array)
{
	ObjectListModel<TObject> *model = new ObjectListModel<TObject>(parent);

	foreach (QJsonValue v, array) {
		QJsonObject o = v.toObject();
		if (o.isEmpty())
			continue;
		TObject *obj = ObjectListModelObject::fromJsonObject<TObject>(o);
		if (!obj)
			continue;
		obj->setParent(parent);
		model->ObjectListModel<TObject>::addObject(obj);
	}

	return model;
}



/**
 * @brief ::appenJsonArray
 * @param array
 */

template<typename TObject>
void ObjectListModel<TObject, ObjectListModel_Object_SFINAE<TObject>>::appendJsonArray(QObject *parent, const QJsonArray &array)
{
	foreach (QJsonValue v, array) {
		QJsonObject o = v.toObject();
		if (o.isEmpty())
			continue;
		TObject *obj = ObjectListModelObject::fromJsonObject<TObject>(o);
		if (!obj)
			continue;
		obj->setParent(parent);
		this->ObjectListModel<TObject>::addObject(obj);
	}
}


/**
 * @brief ::updateJsonArray
 * @param array
 * @param unique_field
 */

template<typename TObject>
void ObjectListModel<TObject, ObjectListModel_Object_SFINAE<TObject>>::updateJsonArray(const QJsonArray &array, const char *unique_field)
{
	Q_ASSERT(unique_field);

	QList<TObject *> unused_objects = this->QGenericListModel<TObject>::objects();

	foreach (QJsonValue v, array) {
		QJsonObject o = v.toObject();

		QList<TObject *> matches = this->find(unique_field, o.value(unique_field).toVariant());

		if (matches.size() > 1) {
			for (TObject *obj : matches) {
				unused_objects.removeAll(obj);
				QObject *t = this->QObjectListModel::takeObject(this->QObjectListModel::index(obj));
				t->deleteLater();
			}

			matches.clear();
		}

		if (!matches.isEmpty()) {
			TObject *obj = matches.at(0);
			unused_objects.removeAll(obj);
			obj->updateJsonObject(o);
		} else {
			TObject *obj = ObjectListModelObject::fromJsonObject<TObject>(o);
			if (obj) {
				obj->setParent(this->parent());
				this->ObjectListModel<TObject>::addObject(obj);
			}
		}
	}

	for (TObject *obj : unused_objects) {
		QObject *t = this->QObjectListModel::takeObject(this->QObjectListModel::index(obj));
		t->deleteLater();
	}
}


#endif // ODL_OBJECTLISTMODEL_H
