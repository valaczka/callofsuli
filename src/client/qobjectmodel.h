/*
 * ---- Call of Suli ----
 *
 * qobjectmodel.h
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

#ifndef QOBJECTMODEL_H
#define QOBJECTMODEL_H

#include <QAbstractListModel>
#include "qobjectdatalist.h"


class QObjectModel : public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(int selectedCount READ selectedCount NOTIFY selectedCountChanged)

public:
	explicit QObjectModel(QObjectDataList *dataList = nullptr, const QStringList &roleNames = QStringList(), QObject *parent = nullptr);
	~QObjectModel();

	int rowCount(const QModelIndex &) const;
	QVariant data(const QModelIndex &index, int role) const;
	QHash<int, QByteArray> roleNames() const { return m_roleNames; }
	int selectedCount() const { return m_selected.count(); }

	void beginInsertRow(const int &i);
	void endInsertRows();

	void beginRemoveRows(const int &from, const int &to);
	void endRemoveRows();

	QObjectList getSelected() const { return m_selected; }

public slots:
	QObject* get(int i) const;
	void updateObject(QObject *o);

	void select(QObject *o);
	void select(int i);
	void unselect(QObject *o);
	void unselect(int i);
	void selectToggle(QObject *o);
	void selectToggle(int i);
	void selectAll();
	void unselectAll();
	void selectAllToggle();

signals:
	void selectedCountChanged(int selectedCount);

private:
	QObjectDataList *m_data;
	QObjectList m_selected;
	QHash<int, QByteArray> m_roleNames;
};

#endif // QOBJECTMODEL_H
