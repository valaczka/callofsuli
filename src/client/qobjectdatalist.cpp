/*
 * ---- Call of Suli ----
 *
 * qobjectdatalist.cpp
 *
 * Created on: 2020. 11. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * QObjectDataList
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

#include "qobjectdatalist.h"
#include "qobjectmodel.h"

QObjectDataList::QObjectDataList()
	: QList<QObject *>()
{

}

/**
 * @brief QObjectDataList<T>::addModel
 * @param model
 */

void QObjectDataList::addModel(QObjectModel *model)
{
	if (!m_models.contains(model))
		m_models.append(model);
}


/**
 * @brief QObjectDataList<T>::removeModel
 * @param model
 * @return
 */

bool QObjectDataList::removeModel(QObjectModel *model)
{
	return m_models.removeOne(model);
}

/**
 * @brief QObjectDataList<T>::append
 */

void QObjectDataList::append(QObject *object)
{
	int i = QList::size();

	foreach (QObjectModel *model, m_models)
		model->beginInsertRow(i);

	QList::append(object);

	foreach (QObjectModel *model, m_models)
		model->endInsertRows();
}


/**
 * @brief QObjectDataList<T>::insert
 * @param i
 * @param object
 */

void QObjectDataList::insert(const int &i, QObject *object)
{
	foreach (QObjectModel *model, m_models)
		model->beginInsertRow(i);

	QList::insert(i, object);

	foreach (QObjectModel *model, m_models)
		model->endInsertRows();
}


/**
 * @brief QObjectDataList<T>::clear
 */

void QObjectDataList::clear()
{
	foreach (QObjectModel *model, m_models)
		model->beginRemoveRows(0, QList::size()-1);

	QList::clear();

	foreach (QObjectModel *model, m_models)
		model->endRemoveRows();
}


/**
 * @brief QObjectDataList<T>::removeOne
 * @param object
 * @return
 */

bool QObjectDataList::removeOne(QObject *object)
{
	int index = QList::indexOf(object);

	if (index == -1)
		return false;

	foreach (QObjectModel *model, m_models)
		model->beginRemoveRows(index, index);

	QList::removeAt(index);

	foreach (QObjectModel *model, m_models)
		model->endRemoveRows();

	return true;
}


/**
 * @brief QObjectDataList<T>::removeAll
 * @param object
 * @return
 */

int QObjectDataList::removeAll(QObject *object)
{
	int ret = 0;
	while (QList::contains(object)) {
		if (QObjectDataList::removeOne(object))
			ret++;
	}
	return ret;
}


/**
 * @brief QObjectDataList<T>::update
 * @param object
 */

void QObjectDataList::update(QObject *object)
{
	foreach (QObjectModel *model, m_models)
		model->updateObject(object);
}


