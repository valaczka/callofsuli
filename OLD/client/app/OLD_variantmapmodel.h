/*
 * ---- Call of Suli ----
 *
 * variantmapmodel.h
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

#ifndef OLD_VARIANTMAPMODEL_H
#define OLD_VARIANTMAPMODEL_H

#include <QAbstractListModel>

class VariantMapData;

class VariantMapModel : public QAbstractListModel
{
	Q_OBJECT
	Q_PROPERTY(int selectedCount READ selectedCount NOTIFY selectedCountChanged)
	Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
	explicit VariantMapModel(const QStringList &roleNames = QStringList(), QObject *parent = nullptr);
	virtual ~VariantMapModel();

	Q_INVOKABLE static VariantMapModel *newModel(const QStringList &roleNames, QObject *parent = nullptr) {
		return new VariantMapModel(roleNames, parent);
	}

	int rowCount(const QModelIndex &) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QHash<int, QByteArray> roleNames() const override { return m_roleNames; }
	int selectedCount() const { return m_selected.count(); }
	int count() const;

	void beginInsertRow(const int &i);
	void endInsertRows();

	void beginRemoveRows(const int &from, const int &to);
	void endRemoveRows();

	QList<int> getSelected() const { return m_selected; }

	Q_INVOKABLE QVariantList getSelectedData(const QString &field) const;
	Q_INVOKABLE QVariantList getSelectedData(const QStringList &fields) const;

	Q_INVOKABLE void clear();
	Q_INVOKABLE void setVariantList(const QVariantList &list, const QString &unique_field);
	Q_INVOKABLE void setJsonArray(const QJsonArray &list, const QString &unique_field);
	Q_INVOKABLE void replaceList(const QVariantList &list);
	Q_INVOKABLE void replaceJsonArray(const QJsonArray &list);
	Q_INVOKABLE void appendList(const QVariantList &list);
	Q_INVOKABLE void appendJsonArray(const QJsonArray &list);

	VariantMapData *variantMapData() const { return m_data; }

public slots:
	const QVariantMap get(int i) const;
	const QVariantMap getByKey(int key) const;
	int getKey(int i) const;
	int findKey(const QString &field, const QVariant &value) const;
	void updateItem(const int &row);

	void select(int i);
	void unselect(int i);
	void selectToggle(int i);
	void selectAll();
	void unselectAll();
	void selectAllToggle();
	void selectByRole(const QString &role);

signals:
	void selectedCountChanged(int selectedCount);
	void countChanged(int count);
	void rolesChanged(QStringList roles);

private:
	bool m_dataDelete;
	VariantMapData *m_data;
	QList<int> m_selected;
	QHash<int, QByteArray> m_roleNames;
};

#endif // OLD_VARIANTMAPMODEL_H
