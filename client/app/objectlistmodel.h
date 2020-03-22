/*
 * ---- Call of Suli ----
 *
 * objectlistmodel.h
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

#ifndef OBJECTLISTMODEL_H
#define OBJECTLISTMODEL_H

#include "qobjectlistmodel.h"
#include "objectlistmodelobject.h"

class ObjectListModel : public QObjectListModel
{
	Q_OBJECT

	Q_PROPERTY(int count READ count NOTIFY countChanged)
	Q_PROPERTY(int selectedCount READ selectedCount NOTIFY selectedCountChanged)
	Q_PROPERTY(bool processing READ processing WRITE setProcessing NOTIFY processingChanged)

public:
	explicit ObjectListModel(const QMetaObject *objectType, QObject *parent = nullptr);

	int selectedCount() const;
	void setSelectedCount(int newSelectedCount);

	int count() const;

	Q_INVOKABLE QList<QObject *> find(const char *field, const QVariant &value) const;
	Q_INVOKABLE int index(QObject *object) const;

	Q_INVOKABLE int nextIntValue(const char *property, const int &start = 0) const;

	Q_INVOKABLE QList<QObject *> getSelected() const;
	Q_INVOKABLE QVariantList getSelectedFields(const QString &field) const;

	Q_INVOKABLE virtual void appendJsonArray(const QJsonArray &array)
	{
		Q_UNUSED(array)
		qWarning() << "Missing implementation" << Q_FUNC_INFO;
	}

	Q_INVOKABLE virtual void updateJsonArray(const QJsonArray &array, const QString &unique_field)
	{
		Q_UNUSED(array)
		Q_UNUSED(unique_field)
		qWarning() << "Missing implementation" << Q_FUNC_INFO;
	}

	Q_INVOKABLE virtual QJsonArray toJsonArray() const
	{
		qWarning() << "Missing implementation" << Q_FUNC_INFO;
		return QJsonArray();
	}

	bool processing() const;
	void setProcessing(bool newProcessing);

public slots:
	void select(int i);
	void unselect(int i);
	void selectToggle(int i);
	void selectAll();
	void unselectAll();
	void selectAllToggle();

	void removeObject(const QModelIndex &index) override;
	void removeObject(int index) override;

signals:
	void selectedCountChanged();
	void countChanged();

	void processingChanged();

private:
	int m_selectedCount;
	bool m_processing;
};



/**
 * Generic class
 */

template <typename TData, typename = void>
class ObjectGenericListModel;

template <typename T>
using ObjectGenericListModel_Object_SFINAE = typename std::enable_if<std::is_base_of<ObjectListModelObject, T>::value>::type;

template <typename T>
class ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>> : public ObjectListModel
{
public:
	explicit ObjectGenericListModel(QObject *parent = nullptr);

	QList<T*> objects() const;
	T *object(const QModelIndex &index) const;
	T *object(int index) const;
	T *takeObject(const QModelIndex &index);
	T *takeObject(int index);
	T *replaceObject(const QModelIndex &index, T *object);
	T *replaceObject(int index, T *object);

	void addObject(T *object);
	void insertObject(const QModelIndex &index, T *object);
	void insertObject(int index, T*object);
	void resetModel(const QList<T*> &objects);

	static ObjectGenericListModel<T>* fromJsonArray(QObject *parent, const QJsonArray &array);
	static ObjectGenericListModel<T>* fromJsonArray(const QJsonArray &array);

	void appendJsonArray(QObject *parent, const QJsonArray &array);
	Q_INVOKABLE virtual void appendJsonArray(const QJsonArray &array) override;
	Q_INVOKABLE virtual void updateJsonArray(const QJsonArray &array, const QString &field) override;
	Q_INVOKABLE virtual QJsonArray toJsonArray() const override;

	static QJsonArray toJsonArray(QList<T*> list);

	QList<T*> getSelected() const;
	QList<T*> find(const char *field, const QVariant &value) const;


protected slots:
	void onSelectedChanged();
};





template<typename T>
void ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::onSelectedChanged()
{
	int n = 0;
	for (QObject* obj : this->QObjectListModel::objects()) {
		ObjectListModelObject *t = qobject_cast<ObjectListModelObject*>(obj);
		if (t->selected())
			n++;
	}
	this->ObjectListModel::setSelectedCount(n);
}


template<typename T>
ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::ObjectGenericListModel(QObject *parent)
	: ObjectListModel{&T::staticMetaObject, parent}
{

}

template <typename T>
QList<T*> ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::objects() const
{
	const QList<QObject*> objs = this->QObjectListModel::objects();
	QList<T*> list;
	list.reserve(objs.size());
	for(auto obj : objs)
		list.append(qobject_cast<T*>(obj));
	return list;
}

template <typename T>
T *ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::object(const QModelIndex &index) const
{
	return qobject_cast<T*>(this->QObjectListModel::object(index));
}

template <typename T>
T *ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::object(int index) const
{
	return qobject_cast<T*>(this->QObjectListModel::object(index));
}

template <typename T>
T *ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::takeObject(const QModelIndex &index)
{
	return qobject_cast<T*>(this->QObjectListModel::takeObject(index));
	emit this->countChanged();
}

template <typename T>
T *ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::takeObject(int index)
{
	return qobject_cast<T*>(this->QObjectListModel::takeObject(index));
	emit this->countChanged();
}

template<typename T>
T *ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::replaceObject(const QModelIndex &index, T *object)
{
	return qobject_cast<T*>(this->QObjectListModel::replaceObject(index, object));
}

template<typename T>
T *ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::replaceObject(int index, T *object)
{
	return qobject_cast<T*>(this->QObjectListModel::replaceObject(index, object));
}


template <typename T>
void ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::addObject(T *object)
{
	QObject::connect(object, &ObjectListModelObject::selectedChanged, this, &ObjectGenericListModel<T>::onSelectedChanged);
	this->QObjectListModel::addObject(object);
	emit this->countChanged();
}

template <typename T>
void ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::insertObject(const QModelIndex &index, T *object)
{
	this->QObjectListModel::insertObject(index, object);
	emit this->countChanged();
}

template <typename T>
void ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::insertObject(int index, T *object)
{
	this->QObjectListModel::insertObject(index, object);
	emit this->countChanged();
}

template <typename T>
void ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::resetModel(const QList<T*> &objects)
{
	QObjectList list;
	list.reserve(objects.size());
	for(auto obj : objects)
		list.append(obj);
	this->QObjectListModel::resetModel(std::move(list));
	emit this->countChanged();
}



template<typename T>
void ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::updateJsonArray(const QJsonArray &array, const QString &field)
{
	Q_ASSERT(!field.isEmpty());

	setProcessing(true);

	QObjectList unused_objects = this->QObjectListModel::objects();

	foreach (QJsonValue v, array) {
		QJsonObject o = v.toObject();

		QObjectList matches = this->ObjectListModel::find(field.toLatin1(), o.value(field.toLatin1()).toVariant());

		if (matches.size() > 1) {
			for (QObject *obj : matches) {
				unused_objects.removeAll(obj);
				QObject *t = this->QObjectListModel::takeObject(this->QObjectListModel::index(obj));
				t->deleteLater();
			}

			matches.clear();
		}

		T *newObj = ObjectListModelObject::fromJsonObject<T>(o, this->parent() ? this->parent() : this);

		if (newObj) {
			if (!matches.isEmpty()) {
				T *obj = qobject_cast<T*>(matches.at(0));
				unused_objects.removeAll(obj);
				obj->updateProperties(newObj);
				newObj->deleteLater();
			} else {
				this->addObject(newObj);
			}

			QCoreApplication::processEvents();
		}
	}

	for (QObject *obj : unused_objects) {
		QObject *t = this->QObjectListModel::takeObject(this->QObjectListModel::index(obj));
		t->deleteLater();
	}

	setProcessing(false);
}

template<typename T>
void ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::appendJsonArray(const QJsonArray &array)
{
	appendJsonArray(this->parent(), array);
}

template<typename T>
void ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::appendJsonArray(QObject *parent, const QJsonArray &array)
{
	foreach (QJsonValue v, array) {
		QJsonObject o = v.toObject();
		if (o.isEmpty())
			continue;
		T *obj = ObjectListModelObject::fromJsonObject<T>(o, parent);
		if (!obj)
			continue;
		this->addObject(obj);
	}
}

template<typename T>
ObjectGenericListModel<T> *ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::fromJsonArray(const QJsonArray &array)
{
	return fromJsonArray(nullptr, array);
}

template<typename T>
ObjectGenericListModel<T> *ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::fromJsonArray(QObject *parent, const QJsonArray &array)
{
	ObjectGenericListModel<T> *model = new ObjectGenericListModel<T>(parent);

	foreach (QJsonValue v, array) {
		QJsonObject o = v.toObject();
		if (o.isEmpty())
			continue;
		T *obj = ObjectListModelObject::fromJsonObject<T>(o, parent);
		if (!obj)
			continue;
		model->addObject(obj);
		QCoreApplication::processEvents();
	}

	return model;
}

template<typename T>
QJsonArray ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::toJsonArray() const
{
	QJsonArray array;
	const QList<T*> objs = objects();
	for(T* obj : objs) {
		QtJsonSerializer::JsonSerializer serializer;
		QJsonValue json = serializer.serialize(obj);
		array.append(json);
	}
	return array;
}



template<typename T>
QJsonArray ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::toJsonArray(QList<T *> list)
{
	QJsonArray array;
	for(T* obj : list) {
		QtJsonSerializer::JsonSerializer serializer;
		QJsonValue json = serializer.serialize(obj);
		array.append(json);
	}
	return array;
}


template<typename T>
QList<T *> ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::getSelected() const
{
	const QList<QObject*> objs = this->ObjectListModel::getSelected();
	QList<T*> list;
	list.reserve(objs.size());
	for(auto obj : objs)
		list.append(qobject_cast<T*>(obj));
	return list;
}



template<typename T>
QList<T *> ObjectGenericListModel<T, ObjectGenericListModel_Object_SFINAE<T>>::find(const char *field, const QVariant &value) const
{
	Q_ASSERT(field);

	const QList<QObject*> objs = this->ObjectListModel::find(field, value);
	QList<T*> list;
	list.reserve(objs.size());
	for(auto obj : objs)
		list.append(qobject_cast<T*>(obj));
	return list;
}

#endif // OBJECTLISTMODEL_H
