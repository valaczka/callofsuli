/*
 * ---- Call of Suli ----
 *
 * varianthashdata.h
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

#ifndef OLD_VARIANTMAPDATA_H
#define OLD_VARIANTMAPDATA_H

#include <QHash>
#include <QVariant>
#include <QJsonArray>
#include <QJsonObject>
#include <OLD_variantmapmodel.h>

class VariantMapModel;

typedef QPair<int, QVariantMap> _MapPair;
typedef QVector<_MapPair> _MapList;

class VariantMapData : public _MapList
{
public:
	VariantMapData();
	virtual ~VariantMapData();

	void addModel(VariantMapModel *model);
	bool removeModel(VariantMapModel *model);

	int append(const QVariantMap &map);
	void clear();

	void removeMore(const int &index, const int &count);
	void removeAt(const int &index);
	bool removeKey(const int &key);

	int key(const QString &field, const QVariant &value, const int &from = 0) const;
	int find(const QString &field, const QVariant &value, const int &from = 0) const;
	int keyIndex(const int &key) const;
	QVariantMap valueKey(const int &key) const;

	void update(const int &index, const QVariantMap &map);
	void updateKey(const int &key, const QVariantMap &map);
	void updateValue(const int &index, const QString &field, const QVariant &value);
	void updateValueByKey(const int &key, const QString &field, const QVariant &value);

	int getNextKey() const;
	int getNextId(const QString &idField = "id");

	void fromMapList(const QVariantList &list, const QString &unique_field);
	void fromJsonArray(const QJsonArray &list, const QString &unique_field);
	void appendMapList(const QVariantList &list);
	void appendMapList(const QJsonArray &list);

private:
	QList<VariantMapModel *> m_models;
};

#endif // VARIANTHASHDATA_H
