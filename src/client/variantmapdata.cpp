/*
 * ---- Call of Suli ----
 *
 * varianthashdata.cpp
 *
 * Created on: 2020. 11. 18.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * VariantHashData
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

#include "variantmapdata.h"
#include "variantmapmodel.h"

VariantMapData::VariantMapData()
	: _MapList()
	, m_models()
{

}


/**
 * @brief VariantHashData::~VariantHashData
 */

VariantMapData::~VariantMapData()
{

}


/**
 * @brief VariantHashData::addModel
 * @param model
 */

void VariantMapData::addModel(VariantMapModel *model)
{
	if (!m_models.contains(model))
		m_models.append(model);
}

/**
 * @brief VariantHashData::removeModel
 * @param model
 * @return
 */

bool VariantMapData::removeModel(VariantMapModel *model)
{
	return m_models.removeOne(model);
}



/**
 * @brief VariantHashData::append
 * @param map
 */

int VariantMapData::append(const QVariantMap &map)
{
	int ret = getNextKey();

	int i = size();

	foreach (VariantMapModel *model, m_models)
		model->beginInsertRow(i);

	QVector::append(qMakePair(ret, map));

	foreach (VariantMapModel *model, m_models)
		model->endInsertRows();

	return ret;
}





/**
 * @brief VariantHashData::clear
 */


void VariantMapData::clear()
{
	foreach (VariantMapModel *model, m_models)
		model->beginRemoveRows(0, QVector::size()-1);

	QVector::clear();

	foreach (VariantMapModel *model, m_models)
		model->endRemoveRows();
}



/**
 * @brief VariantMapData::removeMore
 * @param i
 * @param count
 */

void VariantMapData::removeMore(const int &index, const int &count)
{
	if (index<0 || index+count-1>=QVector::size())
		return;

	foreach (VariantMapModel *model, m_models)
		model->beginRemoveRows(index, index+count-1);

	QVector::remove(index, count);

	foreach (VariantMapModel *model, m_models)
		model->endRemoveRows();
}







/**
 * @brief VariantMapData::removeAt
 * @param i
 */

void VariantMapData::removeAt(const int &index)
{
	if (index<0 || index>=QVector::size())
		return;

	foreach (VariantMapModel *model, m_models)
		model->beginRemoveRows(index, index);

	QVector::remove(index);

	foreach (VariantMapModel *model, m_models)
		model->endRemoveRows();
}



/**
 * @brief VariantMapData::removeKey
 * @param key
 */

bool VariantMapData::removeKey(const int &key)
{
	int index = -1;
	for (int i=0; i<size(); i++) {
		if (at(i).first == key) {
			index = i;
			break;
		}
	}

	if (index == -1)
		return false;

	foreach (VariantMapModel *model, m_models)
		model->beginRemoveRows(index, index);

	QVector::remove(index);

	foreach (VariantMapModel *model, m_models)
		model->endRemoveRows();

	return true;
}




/**
 * @brief VariantMapData::key
 * @param field
 * @param value
 * @return
 */

int VariantMapData::key(const QString &field, const QVariant &value, const int &from) const
{
	for (int i=from; i<size(); i++) {
		QVariantMap m = at(i).second;
		if (m.value(field) == value)
			return at(i).first;
	}

	return -1;
}


/**
 * @brief VariantMapData::find
 * @param field
 * @param value
 * @param from
 * @return
 */

int VariantMapData::find(const QString &field, const QVariant &value, const int &from) const
{
	for (int i=from; i<size(); i++) {
		QVariantMap m = at(i).second;
		if (m.value(field) == value)
			return i;
	}

	return -1;
}





/**
 * @brief VariantMapData::index
 * @param key
 * @return
 */

int VariantMapData::keyIndex(const int &key) const
{
	for (int i=0; i<size(); i++) {
		if (at(i).first == key)
			return i;
	}

	return -1;
}


/**
 * @brief VariantMapData::value
 * @param key
 * @return
 */

QVariantMap VariantMapData::valueKey(const int &key) const
{
	for (int i=0; i<size(); i++) {
		if (at(i).first == key)
			return at(i).second;
	}

	return QVariantMap();
}






/**
	 * @brief VariantMapData::update
	 * @param object
	 */


void VariantMapData::update(const int &index, const QVariantMap &map)
{
	if (index<0 || index>=size())
		return;

	_MapPair *p = data();
	p[index].second = map;

	foreach (VariantMapModel *model, m_models)
		model->updateItem(index);
}


/**
 * @brief VariantMapData::updateKey
 * @param key
 * @param map
 */

void VariantMapData::updateKey(const int &key, const QVariantMap &map)
{
	int index = keyIndex(key);

	if (index == -1)
		return;

	_MapPair *p = data();
	p[index].second = map;

	foreach (VariantMapModel *model, m_models)
		model->updateItem(index);
}


/**
 * @brief VariantMapData::updateValue
 * @param index
 * @param field
 * @param value
 */

void VariantMapData::updateValue(const int &index, const QString &field, const QVariant &value)
{
	if (index<0 || index>=size())
		return;

	_MapPair *p = data();
	p[index].second[field] = value;

	foreach (VariantMapModel *model, m_models)
		model->updateItem(index);
}


/**
 * @brief VariantMapData::updateValueByKey
 * @param key
 * @param field
 * @param value
 */

void VariantMapData::updateValueByKey(const int &key, const QString &field, const QVariant &value)
{
	updateValue(keyIndex(key), field, value);
}



/**
 * @brief VariantMapData::getNextId
 * @return
 */

int VariantMapData::getNextKey() const
{
	int nextKey = 1;

	for (int i=0; i<size(); i++) {
		_MapPair p = at(i);
		if (p.first >= nextKey)
			nextKey = p.first+1;
	}

	return nextKey;
}


/**
 * @brief VariantMapData::getNextId
 * @param idField
 * @return
 */

int VariantMapData::getNextId(const QString &idField)
{
	int nextId = 0;

	for (int i=0; i<size(); i++) {
		QVariantMap m = at(i).second;
		int n = m.value(idField, 0).toInt();
		if (n >= nextId)
			nextId = n+1;
	}

	return nextId;
}




/**
	 * @brief VariantMapData::fromVariantList
	 * @param list
	 * @param unique_key
	 */

void VariantMapData::fromMapList(const QVariantList &list, const QString &unique_field)
{
	QList<int> usedKeys;

	foreach (QVariant v, list) {
		QVariantMap m = v.toMap();

		int index = find(unique_field, m.value(unique_field));

		if (index == -1) {
			int newK = VariantMapData::append(m);
			usedKeys.append(newK);
		} else {
			VariantMapData::update(index, m);
			usedKeys.append(at(index).first);
		}
	}

	for (int i=0; i<size(); i++) {
		int kk = at(i).first;
		if (!usedKeys.contains(kk)) {
			removeKey(kk);
			i = -1;
		}
	}
}


/**
	 * @brief VariantMapData::fromJsonArray
	 * @param list
	 * @param unique_key
	 */

void VariantMapData::fromJsonArray(const QJsonArray &list, const QString &unique_field)
{
	QList<int> usedKeys;

	foreach (QJsonValue v, list) {
		QVariantMap m = v.toObject().toVariantMap();

		int index = find(unique_field, m.value(unique_field));

		if (index == -1) {
			int newK = VariantMapData::append(m);
			usedKeys.append(newK);
		} else {
			VariantMapData::update(index, m);
			usedKeys.append(at(index).first);
		}
	}

	for (int i=0; i<size(); i++) {
		int kk = at(i).first;
		if (!usedKeys.contains(kk)) {
			removeKey(kk);
			i = -1;
		}
	}
}




/**
	 * @brief VariantMapData::appendMapList
	 * @param list
	 */

void VariantMapData::appendMapList(const QVariantList &list)
{
	foreach (QVariant v, list)
		VariantMapData::append(v.toMap());

}


/**
	 * @brief VariantMapData::appendMapList
	 * @param list
	 */

void VariantMapData::appendMapList(const QJsonArray &list)
{
	foreach (QJsonValue v, list)
		VariantMapData::append(v.toObject().toVariantMap());
}
