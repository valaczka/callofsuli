/*
 * ---- Call of Suli ----
 *
 * qobjectdatalist.h
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

#ifndef QOBJECTDATALIST_H
#define QOBJECTDATALIST_H

#include <QObject>

class QObjectModel;

class QObjectDataList : public QList<QObject *>
{
public:
	QObjectDataList();

	void addModel(QObjectModel *model);
	bool removeModel(QObjectModel *model);

	void append(QObject *object);
	void insert(const int &i, QObject *object);
	void clear();

	bool removeOne(QObject *object);
	int removeAll(QObject *object);

	void update(QObject *object);

protected:
	QList<QObjectModel *> m_models;
};



#endif // QOBJECTDATALIST_H
