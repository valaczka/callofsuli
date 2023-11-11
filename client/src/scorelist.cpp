/*
 * ---- Call of Suli ----
 *
 * scorelist.cpp
 *
 * Created on: 2023. 06. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ScoreList
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

#include "scorelist.h"
#include "application.h"
#include "user.h"
#include "utils_.h"


/**
 * @brief ScoreList::ScoreList
 * @param parent
 */

ScoreList::ScoreList(QObject *parent)
	: FetchModel{parent}
{
	setFields(Utils::getRolesFromObject(User().metaObject()));
}




/**
 * @brief ScoreList::reload
 */

void ScoreList::reload()
{
	Client *client = Application::instance()->client();

	client->send(m_api, m_path, m_apiData)
			->error(client, &Client::onHttpConnectionError)
			->fail(client, [client](const QString &err){ client->messageWarning(err, tr("Letöltési hiba")); })
			->done(this, &ScoreList::loadFromJson);

}



/**
 * @brief ScoreList::reloadFromVariantList
 * @param list
 */

void ScoreList::reloadFromVariantList(const QVariantList &list)
{
	if (limit() < 0)
		Utils::patchSListModel(m_model.get(), list, QStringLiteral("username"));
	else
		FetchModel::reloadFromVariantList(list);
}



/**
 * @brief ScoreList::refresh
 */

void ScoreList::refresh()
{
	QVariantList model;

	foreach (const QVariant &v, m_originalModel) {
		const QVariantMap &map = v.toMap();

		if (m_filterClassId > -1 && map.value(QStringLiteral("classid"), -1).toInt() != m_filterClassId)
			continue;

		model.append(map);
	}

	if (m_sortOrder != SortNone) {
		std::sort(model.begin(), model.end(), [this](const QVariant &left, const QVariant &right) -> bool {
			const QVariantMap &mLeft = left.toMap();
			const QVariantMap &mRight = right.toMap();

			const bool defaultOrder = QString::localeAwareCompare(mLeft.value(QStringLiteral("username")).toString(),
																  mRight.value(QStringLiteral("username")).toString()) < 0;

			switch (m_sortOrder) {
			case SortXP:
				if (mLeft.value(QStringLiteral("xp")).toInt() < mRight.value(QStringLiteral("xp")).toInt())
					return true;
				else if (mLeft.value(QStringLiteral("xp")).toInt() > mRight.value(QStringLiteral("xp")).toInt())
					return false;
				else
					return defaultOrder;
				break;
			case SortXPdesc:
				if (mLeft.value(QStringLiteral("xp")).toInt() > mRight.value(QStringLiteral("xp")).toInt())
					return true;
				else if (mLeft.value(QStringLiteral("xp")).toInt() < mRight.value(QStringLiteral("xp")).toInt())
					return false;
				else
					return defaultOrder;
				break;
			case SortStreak:
				if (mLeft.value(QStringLiteral("streak")).toInt() < mRight.value(QStringLiteral("streak")).toInt())
					return true;
				else if (mLeft.value(QStringLiteral("streak")).toInt() > mRight.value(QStringLiteral("streak")).toInt())
					return false;
				else
					return defaultOrder;
				break;
			case SortFullname:
				if (QString::localeAwareCompare(mLeft.value(QStringLiteral("fullName")).toString(),
												mRight.value(QStringLiteral("fullName")).toString()) < 0)
					return true;
				else if (QString::localeAwareCompare(mLeft.value(QStringLiteral("fullName")).toString(),
													 mRight.value(QStringLiteral("fullName")).toString()) > 0)
					return false;
				else
					return defaultOrder;
				break;
			case SortFullNickname:
				if (QString::localeAwareCompare(mLeft.value(QStringLiteral("fullNickName")).toString(),
												mRight.value(QStringLiteral("fullNickName")).toString()) < 0)
					return true;
				else if (QString::localeAwareCompare(mLeft.value(QStringLiteral("fullNickName")).toString(),
													 mRight.value(QStringLiteral("fullNickName")).toString()) > 0)
					return false;
				else
					return defaultOrder;
				break;
			default:
				return defaultOrder;
			}
		});
	}

	reloadFromVariantList(model);
}




/**
 * @brief ScoreList::onEventJsonReceived
 * @param event
 * @param json
 */

void ScoreList::onEventJsonReceived(const QString &, const QJsonObject &json)
{
	loadFromJson(json);
}







/**
 * @brief ScoreList::loadFromJson
 * @param obj
 */

void ScoreList::loadFromJson(const QJsonObject &obj)
{
	m_originalModel.clear();

	for (const QJsonValue &v : obj.value(QStringLiteral("list")).toArray()) {
		User u;
		u.loadFromJson(v.toObject());
		m_originalModel.append(u.toVariantMap());
	}

	refresh();
}



/**
 * @brief ScoreList::filterClassId
 * @return
 */

int ScoreList::filterClassId() const
{
	return m_filterClassId;
}

void ScoreList::setFilterClassId(int newFilterClassId)
{
	if (m_filterClassId == newFilterClassId)
		return;
	m_filterClassId = newFilterClassId;
	emit filterClassIdChanged();
	refresh();
}


const QJsonObject &ScoreList::apiData() const
{
	return m_apiData;
}

void ScoreList::setApiData(const QJsonObject &newApiData)
{
	if (m_apiData == newApiData)
		return;
	m_apiData = newApiData;
	emit apiDataChanged();
}

const QString &ScoreList::path() const
{
	return m_path;
}

void ScoreList::setPath(const QString &newPath)
{
	if (m_path == newPath)
		return;
	m_path = newPath;
	emit pathChanged();
}

const HttpConnection::API &ScoreList::api() const
{
	return m_api;
}

void ScoreList::setApi(const HttpConnection::API &newApi)
{
	if (m_api == newApi)
		return;
	m_api = newApi;
	emit apiChanged();
}



/**
 * @brief ScoreList::sortOrder
 * @return
 */

const ScoreList::SortOrder &ScoreList::sortOrder() const
{
	return m_sortOrder;
}

void ScoreList::setSortOrder(const SortOrder &newSortOrder)
{
	if (m_sortOrder == newSortOrder)
		return;
	m_sortOrder = newSortOrder;
	emit sortOrderChanged();
	refresh();
}
