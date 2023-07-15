/*
 * ---- Call of Suli ----
 *
 * campaignresultlist.cpp
 *
 * Created on: 2023. 06. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * CampaignResultList
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

#include "offsetmodel.h"
#include "application.h"
#include "client.h"

OffsetModel::OffsetModel(QObject *parent)
	: QObject{parent}
	, m_model(new QSListModel(this))
{

}


/**
 * @brief OffsetModel::~OffsetModel
 */

OffsetModel::~OffsetModel()
{
	delete m_model;
	m_model = nullptr;
}


const WebSocket::API &OffsetModel::api() const
{
	return m_api;
}

void OffsetModel::setApi(const WebSocket::API &newApi)
{
	if (m_api == newApi)
		return;
	m_api = newApi;
	emit apiChanged();
}

const QString &OffsetModel::path() const
{
	return m_path;
}

void OffsetModel::setPath(const QString &newPath)
{
	if (m_path == newPath)
		return;
	m_path = newPath;
	emit pathChanged();
}

const QJsonObject &OffsetModel::apiData() const
{
	return m_apiData;
}

void OffsetModel::setApiData(const QJsonObject &newApiData)
{
	if (m_apiData == newApiData)
		return;
	m_apiData = newApiData;
	emit apiDataChanged();
}

QSListModel *OffsetModel::model() const
{
	return m_model;
}


/**
 * @brief OffsetModel::fields
 * @return
 */

const QStringList &OffsetModel::fields() const
{
	return m_fields;
}

/**
 * @brief OffsetModel::setFields
 * @param newFields
 */

void OffsetModel::setFields(const QStringList &newFields)
{
	if (m_fields == newFields)
		return;
	m_fields = newFields;
	m_model->setRoleNames(newFields);
	emit fieldsChanged();
}


/**
 * @brief OffsetModel::limit
 * @return
 */

int OffsetModel::limit() const
{
	return m_limit;
}

void OffsetModel::setLimit(int newLimit)
{
	if (m_limit == newLimit)
		return;
	m_limit = newLimit;
	emit limitChanged();
}


/**
 * @brief OffsetModel::listField
 * @return
 */


const QString &OffsetModel::listField() const
{
	return m_listField;
}

void OffsetModel::setListField(const QString &newListField)
{
	if (m_listField == newListField)
		return;
	m_listField = newListField;
	emit listFieldChanged();
}


/**
 * @brief OffsetModel::canFetch
 * @return
 */

bool OffsetModel::canFetch() const
{
	return m_canFetch;
}

void OffsetModel::setCanFetch(bool newCanFetch)
{
	if (m_canFetch == newCanFetch)
		return;
	m_canFetch = newCanFetch;
	emit canFetchChanged();
}


/**
 * @brief OffsetModel::reload
 */

void OffsetModel::reload()
{
	if (m_fetchActive || m_api == WebSocket::ApiInvalid)
		return;

	LOG_CTRACE("client") << "Reload";

	m_offset = 0;
	m_model->clear();
	emit modelCleared();

	setCanFetch(true);

	fetch();
}



/**
 * @brief OffsetModel::loadSnapshot
 */

void OffsetModel::fetch()
{
	if (m_fetchActive || m_api == WebSocket::ApiInvalid)
		return;

	LOG_CTRACE("client") << "Fetch" << m_model;

	Client *client = Application::instance()->client();

	Q_ASSERT(client);

	QJsonObject data = m_apiData;
	data[QStringLiteral("limit")] = m_limit;
	data[QStringLiteral("offset")] = m_offset;

	client->send(m_api, m_path, data)
			->error(client, &Client::onWebSocketError)
			->error([this](const QNetworkReply::NetworkError &){ m_fetchActive = false; })
			->fail([client, this](const QString &err){
		m_fetchActive = false;
		client->messageWarning(err, tr("Letöltési hiba"));
	})
			->done(this, &OffsetModel::loadFromJson);
}



/**
 * @brief OffsetModel::getListFromJson
 * @param obj
 * @return
 */

QVariantList OffsetModel::getListFromJson(const QJsonObject &obj)
{
	return obj.value(m_listField).toArray().toVariantList();
}




/**
 * @brief OffsetModel::loadFromJson
 * @param obj
 */

void OffsetModel::loadFromJson(const QJsonObject &obj)
{
	m_fetchActive = false;

	const QVariantList &list = getListFromJson(obj);
	const int &limit = obj.value(QStringLiteral("limit")).toInt(0);
	const int &offset = obj.value(QStringLiteral("offset")).toInt(m_offset);

	if (list.isEmpty()) {
		emit snapshotEmpty();
		setCanFetch(false);
		return;
	}

	m_model->insert(m_model->count(), list);

	m_offset = offset + list.size();

	if (limit > 0 && list.size() < limit)
		setCanFetch(false);

	emit snapshotAdded();
}
