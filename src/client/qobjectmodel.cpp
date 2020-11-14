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

#include "qobjectmodel.h"

QObjectModel::QObjectModel(const QStringList &roleNames, QObject *parent)
	: QAbstractListModel(parent)

{
	int i = Qt::UserRole+1;

	m_roleNames[i] = "dataObject";

	foreach (QString role, roleNames) {
		++i;
		m_roleNames[i] = role.toLatin1();
	}
}


/**
 * @brief QObjectModel::data
 * @param index
 * @return
 */

QVariant QObjectModel::data(const QModelIndex &index, int role) const
{
	if (index.row() < 0 || index.row() >= m_data.size())
		return QVariant();

	QByteArray roleName = m_roleNames.value(role);

	if (roleName == "dataObject")
		return QVariant::fromValue(m_data[index.row()]);
	else if (!roleName.isEmpty())
		return m_data[index.row()]->property(roleName);

	return QVariant();
}




/**
 * @brief QObjectModel::append
 * @param o
 */

void QObjectModel::append(QObject *o)
{
	int i = m_data.size();
	beginInsertRows(QModelIndex(), i, i);
	m_data.append(o);

	emit countChanged(count());

	endInsertRows();
}


/**
 * @brief QObjectModel::insert
 * @param o
 * @param i
 */

void QObjectModel::insert(QObject *o, int i)
{
	beginInsertRows(QModelIndex(), i, i);
	m_data.insert(i, o);

	emit countChanged(count());

	endInsertRows();
}


/**
 * @brief QObjectModel::get
 * @param i
 * @return
 */

QObject *QObjectModel::get(int i)
{
	Q_ASSERT(i >= 0 && i < m_data.count());
	return m_data[i];
}


/**
 * @brief QObjectModel::clear
 */

void QObjectModel::clear()
{
	beginRemoveRows(QModelIndex(), 0, m_data.count()-1);
	m_data.clear();
	emit countChanged(count());
	endRemoveRows();
}



/**
 * @brief QObjectModel::deleteAll
 */
void QObjectModel::deleteAll()
{
	beginRemoveRows(QModelIndex(), 0, m_data.count()-1);
	qDeleteAll(m_data.begin(), m_data.end());
	m_data.clear();
	emit countChanged(count());
	endRemoveRows();
}


/**
 * @brief QObjectModel::remove
 * @param i
 */

void QObjectModel::remove(int i)
{
	Q_ASSERT(i>=0 && i<m_data.size());
	beginRemoveRows(QModelIndex(), i, i);
	QObject *o = m_data.takeAt(i);
	o->deleteLater();
	emit countChanged(count());
	endRemoveRows();
}


/**
 * @brief QObjectModel::objectUpdated
 * @param row
 */

void QObjectModel::rowUpdated(const int &row)
{
	Q_ASSERT(row>=0 && row<m_data.size());
	QModelIndex i1 = index(row);

	emit dataChanged(i1, i1);
}






