/*
 * ---- Call of Suli ----
 *
 * fetchmodel.h
 *
 * Created on: 2023. 07. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * FetchModel
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

#ifndef FETCHMODEL_H
#define FETCHMODEL_H

#include "qslistmodel.h"
#include <QObject>

class FetchModel : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QSListModel *model READ model CONSTANT)
	Q_PROPERTY(QStringList fields READ fields WRITE setFields NOTIFY fieldsChanged)

	Q_PROPERTY(int limit READ limit WRITE setLimit NOTIFY limitChanged)
	Q_PROPERTY(bool canFetch READ canFetch WRITE setCanFetch NOTIFY canFetchChanged)

public:
	explicit FetchModel(QObject *parent = nullptr);
	virtual ~FetchModel();

	QSListModel *model() const;

	const QStringList &fields() const;
	void setFields(const QStringList &newFields);

	int limit() const;
	void setLimit(int newLimit);

	bool canFetch() const;
	void setCanFetch(bool newCanFetch);

public slots:
	void fetch();

	virtual void reload() {}
	virtual void reloadFromJsonArray(const QJsonArray &list);
	virtual void reloadFromVariantList(const QVariantList &list);

signals:
	void modelReloaded();
	void fieldsChanged();
	void limitChanged();
	void canFetchChanged();

protected:
	QSListModel *m_model = nullptr;

private:
	QVariantList m_originalModel;
	QStringList m_fields;
	int m_limit = 10;
	int m_offset = 0;
	bool m_canFetch;
};

#endif // FETCHMODEL_H
