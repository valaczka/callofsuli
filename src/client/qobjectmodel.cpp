/*
 * ---- Call of Suli ----
 *
 * qobjectmodel.cpp
 *
 * Created on: 2020. 11. 14.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * QObjectModel
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

#include <QtDebug>
#include "qobjectmodel.h"
#include "qobjectdatalist.h"

QObjectModel::QObjectModel(QObjectDataList *dataList, const QStringList &roleNames, QObject *parent)
	: QAbstractListModel(parent)
	, m_data(dataList)

{
	Q_ASSERT(dataList);

	m_data->addModel(this);

	int i = Qt::UserRole+1;

	m_roleNames[i++] = "dataObject";
	m_roleNames[i++] = "selected";

	foreach (QString role, roleNames)
		m_roleNames[i++] = role.toLatin1();
}


/**
 * @brief QObjectModel::~QObjectModel
 */

QObjectModel::~QObjectModel()
{
	m_data->removeModel(this);
}


/**
 * @brief QObjectModel::rowCount
 * @return
 */

int QObjectModel::rowCount(const QModelIndex &) const
{
	return m_data->size();
}


/**
 * @brief QObjectModel::data
 * @param index
 * @return
 */

QVariant QObjectModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	if (row < 0 || row >= m_data->size())
		return QVariant();

	QByteArray roleName = m_roleNames.value(role);

	if (roleName == "dataObject")
		return QVariant::fromValue(m_data->at(row));
	if (roleName == "selected")
		return m_selected.contains(m_data->at(row));
	else if (!roleName.isEmpty())
		return m_data->at(row)->property(roleName);

	return QVariant();
}



/**
 * @brief QObjectModel::beginInsertRow
 * @param i
 */

void QObjectModel::beginInsertRow(const int &i)
{
	QAbstractListModel::beginInsertRows(QModelIndex(), i, i);
}


/**
 * @brief QObjectModel::endInsertRows
 */

void QObjectModel::endInsertRows()
{
	QAbstractListModel::endInsertRows();
}


/**
 * @brief QObjectModel::beginRemoveRows
 * @param from
 * @param to
 */

void QObjectModel::beginRemoveRows(const int &from, const int &to)
{
	QAbstractListModel::beginRemoveRows(QModelIndex(), from, to);
	for (int i=from; i<=to; i++) {
		QObject *o = m_data->at(i);
		m_selected.removeAll(o);
		emit selectedCountChanged(selectedCount());
	}
}



/**
 * @brief QObjectModel::endRemoveRows
 */

void QObjectModel::endRemoveRows()
{
	QAbstractListModel::endRemoveRows();
}






/**
 * @brief QObjectModel::get
 * @param i
 * @return
 */

QObject *QObjectModel::get(int i) const
{
	if (i<0 || i>=m_data->count())
		return nullptr;
	else
		return m_data->at(i);
}






/**
 * @brief QObjectModel::updateObject
 * @param o
 */

void QObjectModel::updateObject(QObject *o)
{
	int from = 0;

	do {
		int row = m_data->indexOf(o, from);

		if (row == -1)
			return;

		QModelIndex i1 = index(row);

		emit dataChanged(i1, i1);

		from = row+1;
	} while (true);
}




/**
 * @brief QObjectModel::select
 * @param o
 */

void QObjectModel::select(QObject *o)
{
	if (m_data->contains(o) && !m_selected.contains(o)) {
		m_selected.append(o);
		emit selectedCountChanged(selectedCount());
		updateObject(o);
	}
}


/**
 * @brief QObjectModel::select
 * @param i
 */

void QObjectModel::select(int i)
{
	QObject *o = get(i);
	if (o) select (o);
}


/**
 * @brief QObjectModel::unselect
 * @param o
 */

void QObjectModel::unselect(QObject *o)
{
	if (m_data->contains(o) && m_selected.contains(o)) {
		m_selected.removeAll(o);
		emit selectedCountChanged(selectedCount());
		updateObject(o);
	}
}


/**
 * @brief QObjectModel::unselect
 * @param i
 */

void QObjectModel::unselect(int i)
{
	QObject *o = get(i);
	if (o) unselect (o);
}


/**
 * @brief QObjectModel::selectToggle
 * @param o
 */

void QObjectModel::selectToggle(QObject *o)
{
	if (m_data->contains(o)) {
		if (m_selected.contains(o))
			unselect(o);
		else
			select(o);
	}
}


/**
 * @brief QObjectModel::selectToggle
 * @param i
 */

void QObjectModel::selectToggle(int i)
{
	QObject *o = get(i);
	if (o) selectToggle(o);
}


/**
 * @brief QObjectModel::selectAll
 */

void QObjectModel::selectAll()
{
	for (int i=0; i<m_data->size(); i++) {
		QObject *o = m_data->at(i);
		select(o);
	}
}


/**
 * @brief QObjectModel::unselectAll
 */

void QObjectModel::unselectAll()
{
	for (int i=0; i<m_data->size(); i++) {
		QObject *o = m_data->at(i);
		unselect(o);
	}
}


/**
 * @brief QObjectModel::selectAllToggle
 */

void QObjectModel::selectAllToggle()
{
	if (m_selected.size() < m_data->size())
		selectAll();
	else
		unselectAll();
}









