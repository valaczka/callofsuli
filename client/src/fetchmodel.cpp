/*
 * ---- Call of Suli ----
 *
 * fetchmodel.cpp
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

#include "fetchmodel.h"
#include "qjsonarray.h"


/**
 * @brief FetchModel::FetchModel
 * @param parent
 */

FetchModel::FetchModel(QObject *parent)
	: QObject{parent}
	, m_model(new QSListModel())
{

}


/**
 * @brief FetchModel::~FetchModel
 */

FetchModel::~FetchModel()
{
}


/**
 * @brief FetchModel::model
 * @return
 */

QSListModel *FetchModel::model() const
{
	return m_model.get();
}

const QStringList &FetchModel::fields() const
{
	return m_fields;
}

void FetchModel::setFields(const QStringList &newFields)
{
	if (m_fields == newFields)
		return;
	m_fields = newFields;
	m_model->setRoleNames(newFields);
	emit fieldsChanged();
}

int FetchModel::limit() const
{
	return m_limit;
}

void FetchModel::setLimit(int newLimit)
{
	if (m_limit == newLimit)
		return;
	m_limit = newLimit;
	emit limitChanged();

	m_offset = 0;
	m_model->clear();
	setCanFetch(true);
	fetch();
}

bool FetchModel::canFetch() const
{
	return m_canFetch;
}

void FetchModel::setCanFetch(bool newCanFetch)
{
	if (m_canFetch == newCanFetch)
		return;
	m_canFetch = newCanFetch;
	emit canFetchChanged();
}


/**
 * @brief FetchModel::fetch
 */

void FetchModel::fetch()
{
	if (!m_canFetch)
		return;

	const QVariantList &list = m_originalModel.mid(m_offset, m_limit);

	if (list.isEmpty()) {
		setCanFetch(false);
		return;
	}

	m_model->insert(m_model->count(), list);

	m_offset = m_offset + list.size();

	if (m_limit > 0 && list.size() < m_limit)
		setCanFetch(false);
}



/**
 * @brief FetchModel::reloadFromJsonArray
 * @param list
 */

void FetchModel::reloadFromJsonArray(const QJsonArray &list)
{
	reloadFromVariantList(list.toVariantList());
}



/**
 * @brief FetchModel::reloadFromVariantList
 * @param list
 */

void FetchModel::reloadFromVariantList(const QVariantList &list)
{
	m_offset = 0;
	m_model->clear();

	m_originalModel = list;

	setCanFetch(true);

	emit modelReloaded();

	fetch();
}


