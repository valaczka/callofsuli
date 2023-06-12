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


QVariantList ScoreList::m_originalModel;

/**
 * @brief ScoreList::ScoreList
 * @param parent
 */

ScoreList::ScoreList(QObject *parent)
	: QObject{parent}
	, m_model(new QSListModel(this))
{
	setRoles();
	refresh();
}


/**
 * @brief ScoreList::~ScoreList
 */

ScoreList::~ScoreList()
{
	delete m_model;
	m_model = nullptr;
}

/**
 * @brief ScoreList::model
 * @return
 */

QSListModel *ScoreList::model() const
{
	return m_model;
}


/**
 * @brief ScoreList::reload
 */

void ScoreList::reload()
{
	Client *client = Application::instance()->client();

	client->send(m_api, m_path, m_apiData)
			->error(client, &Client::onWebSocketError)
			->fail([client](const QString &err){ client->messageWarning(err, tr("Letöltési hiba")); })
			->done(this, &ScoreList::loadFromJson);

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
		std::sort(model.begin(), model.end(), [this](const QVariant &left, const QVariant &right) {
			const QVariantMap &mLeft = left.toMap();
			const QVariantMap &mRight = right.toMap();

			switch (m_sortOrder) {
			case SortXP:
				return mLeft.value(QStringLiteral("xp")).toInt() < mRight.value(QStringLiteral("xp")).toInt();
				break;
			case SortXPdesc:
				return mLeft.value(QStringLiteral("xp")).toInt() > mRight.value(QStringLiteral("xp")).toInt();
				break;
			case SortStreak:
				return mLeft.value(QStringLiteral("streak")).toInt() < mRight.value(QStringLiteral("streak")).toInt();
				break;
			case SortFullname:
				return QString::localeAwareCompare(mLeft.value(QStringLiteral("fullName")).toString(),
												   mRight.value(QStringLiteral("fullName")).toString()) < 0;
				break;
			case SortFullNickname:
				return QString::localeAwareCompare(mLeft.value(QStringLiteral("fullNickName")).toString(),
												   mRight.value(QStringLiteral("fullNickName")).toString()) < 0;
				break;
			default:
				return false;
			}
		});
	}

	Utils::patchSListModel(m_model, model, QStringLiteral("username"));
}




/**
 * @brief ScoreList::setRoles
 */

void ScoreList::setRoles()
{
	m_model->setRoleNames(Utils::getRolesFromObject(User().metaObject()));
}






/**
 * @brief ScoreList::loadFromJson
 * @param obj
 */

void ScoreList::loadFromJson(const QJsonObject &obj)
{
	m_originalModel.clear();

	foreach (const QJsonValue &v, obj.value(QStringLiteral("list")).toArray()) {
		User u;
		u.loadFromJson(v.toObject());
		m_originalModel.append(u.toVariantMap());
	}

	refresh();
	emit modelReloaded();
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

const WebSocket::API &ScoreList::api() const
{
	return m_api;
}

void ScoreList::setApi(const WebSocket::API &newApi)
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
