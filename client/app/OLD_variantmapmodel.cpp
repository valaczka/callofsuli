/* ---- Call of Suli ----
 *
 * variantmapmodel.cpp
 *
 * Created on: 2020. 11. 18.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * VariantMapModel
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "variantmapmodel.h"
#include "OLD_variantmapdata.h"


VariantMapModel::VariantMapModel(const QStringList &roleNames, QObject *parent)
	: QAbstractListModel(parent)
	, m_dataDelete(true)
	, m_data(new VariantMapData())

{
	m_data->addModel(this);

	int i = Qt::UserRole+1;

	m_roleNames[i++] = "dataObject";
	m_roleNames[i++] = "selected";

	foreach (QString role, roleNames)
		m_roleNames[i++] = role.toLatin1();
}



/**
 * @brief VariantMapModel::~VariantMapModel
 */

VariantMapModel::~VariantMapModel()
{
	if (m_data) {
		m_data->removeModel(this);

		if (m_dataDelete)
			delete m_data;

		m_data = nullptr;
	}
}



/**
 * @brief VariantMapModel::rowCount
 * @return
 */

int VariantMapModel::rowCount(const QModelIndex &) const
{
	return m_data->size();
}


/**
 * @brief VariantMapModel::data
 * @param index
 * @return
 */

QVariant VariantMapModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	if (row < 0 || row >= m_data->size())
		return QVariant();

	QByteArray roleName = m_roleNames.value(role);

	if (roleName == "dataObject")
		return m_data->at(row).second;
	if (roleName == "selected")
		return m_selected.contains(m_data->at(row).first);
	else if (!roleName.isEmpty())
		return m_data->at(row).second.value(roleName);

	return QVariant();
}



/**
 * @brief VariantMapModel::beginInsertRow
 * @param i
 */

void VariantMapModel::beginInsertRow(const int &i)
{
	QAbstractListModel::beginInsertRows(QModelIndex(), i, i);
}


/**
 * @brief VariantMapModel::endInsertRows
 */

void VariantMapModel::endInsertRows()
{
	QAbstractListModel::endInsertRows();
	emit countChanged(count());
}


/**
 * @brief VariantMapModel::beginRemoveRows
 * @param from
 * @param to
 */

void VariantMapModel::beginRemoveRows(const int &from, const int &to)
{
	QAbstractListModel::beginRemoveRows(QModelIndex(), from, to);
}



/**
 * @brief VariantMapModel::endRemoveRows
 */

void VariantMapModel::endRemoveRows()
{
	QAbstractListModel::endRemoveRows();
	emit countChanged(count());
}


/**
 * @brief VariantMapModel::getSelectedData
 * @param field
 * @return
 */

QVariantList VariantMapModel::getSelectedData(const QString &field) const
{
	QVariantList list;

	foreach (int key, m_selected)
		list.append(m_data->valueKey(key).value(field));

	return list;
}



/**
 * @brief VariantMapModel::getSelectedData
 * @param fields
 * @return
 */

QVariantList VariantMapModel::getSelectedData(const QStringList &fields) const
{
	QVariantList list;

	foreach (int key, m_selected) {
		QVariantMap m;
		foreach (QString s, fields)
			m.insert(s, m_data->valueKey(key).value(s));

		list.append(m);
	}

	return list;
}




/**
 * @brief VariantMapModel::clear
 */

void VariantMapModel::clear()
{
	unselectAll();
	m_data->clear();
}


/**
 * @brief VariantMapModel::setVariantList
 * @param list
 */

void VariantMapModel::setVariantList(const QVariantList &list, const QString &unique_field)
{
	m_data->fromMapList(list, unique_field);
}


/**
 * @brief VariantMapModel::setJsonArray
 * @param list
 * @param unique_field
 */

void VariantMapModel::setJsonArray(const QJsonArray &list, const QString &unique_field)
{
	m_data->fromJsonArray(list, unique_field);
}


/**
 * @brief VariantMapModel::replaceList
 * @param list
 */

void VariantMapModel::replaceList(const QVariantList &list)
{
	m_data->clear();
	m_data->appendMapList(list);
}

/**
 * @brief VariantMapModel::replaceJsonArray
 * @param list
 */

void VariantMapModel::replaceJsonArray(const QJsonArray &list)
{
	m_data->clear();
	m_data->appendMapList(list);
}


/**
 * @brief VariantMapModel::appendList
 * @param list
 */

void VariantMapModel::appendList(const QVariantList &list)
{
	m_data->appendMapList(list);
}


/**
 * @brief VariantMapModel::appendJsonArray
 * @param list
 */

void VariantMapModel::appendJsonArray(const QJsonArray &list)
{
	m_data->appendMapList(list);
}


/**
 * @brief VariantMapModel::count
 * @return
 */

int VariantMapModel::count() const
{
	return m_data->size();
}






/**
 * @brief VariantMapModel::get
 * @param i
 * @return
 */

const QVariantMap VariantMapModel::get(int i) const
{
	if (i<0 || i>=m_data->size())
		return QVariantMap();
	else
		return m_data->at(i).second;
}


/**
 * @brief VariantMapModel::getByKey
 * @param key
 * @return
 */

const QVariantMap VariantMapModel::getByKey(int key) const
{
	return m_data->valueKey(key);
}


/**
 * @brief VariantMapModel::getKey
 * @param i
 * @return
 */

int VariantMapModel::getKey(int i) const
{
	if (i<0 || i>=m_data->size())
		return -1;
	else
		return m_data->at(i).first;
}


/**
 * @brief VariantMapModel::findKey
 * @param field
 * @param value
 * @return
 */

int VariantMapModel::findKey(const QString &field, const QVariant &value) const
{
	return m_data->key(field, value);
}





/**
 * @brief VariantMapModel::updateItem
 * @param index
 * @param map
 */

void VariantMapModel::updateItem(const int &row)
{
	QModelIndex i1 = index(row);

	emit dataChanged(i1, i1);
}




/**
 * @brief VariantMapModel::select
 * @param i
 */

void VariantMapModel::select(int i)
{
	int key = m_data->at(i).first;
	if (key != -1 && !m_selected.contains(key)) {
		m_selected.append(key);
		updateItem(i);
		emit selectedCountChanged(selectedCount());
	}
}


/**
 * @brief VariantMapModel::unselect
 * @param i
 */

void VariantMapModel::unselect(int i)
{
	int key = m_data->at(i).first;
	if (key != -1 && m_selected.contains(key)) {
		m_selected.removeAll(key);
		updateItem(i);
		emit selectedCountChanged(selectedCount());
	}
}




/**
 * @brief VariantMapModel::selectToggle
 * @param i
 */

void VariantMapModel::selectToggle(int i)
{
	int key = m_data->at(i).first;
	if (m_selected.contains(key))
		unselect(i);
	else
		select(i);
}


/**
 * @brief VariantMapModel::selectAll
 */

void VariantMapModel::selectAll()
{
	for (int i=0; i<m_data->size(); i++)
		select(i);
	emit selectedCountChanged(selectedCount());
}


/**
 * @brief VariantMapModel::unselectAll
 */

void VariantMapModel::unselectAll()
{
	m_selected.clear();
	emit selectedCountChanged(selectedCount());
}


/**
 * @brief VariantMapModel::selectAllToggle
 */

void VariantMapModel::selectAllToggle()
{
	if (m_selected.size() < m_data->size())
		selectAll();
	else
		unselectAll();
}


/**
 * @brief VariantMapModel::selectByRole
 * @param role
 */

void VariantMapModel::selectByRole(const QString &role)
{
	unselectAll();

	for (int i=0; i<m_data->size(); i++) {
		QVariantMap content = m_data->at(i).second;

		if (content.value(role, false).toBool())
			select(i);
	}
}



