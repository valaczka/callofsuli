/*
 * ---- Call of Suli ----
 *
 * userloglist.cpp
 *
 * Created on: 2023. 06. 12.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * UserLogList
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

#include "userloglist.h"
#include "application.h"
#include "rank.h"
#include "server.h"
#include "gamemap.h"

UserLogList::UserLogList(QObject *parent)
	: QObject{parent}
{

}



/**
 * @brief UserLogList::username
 * @return
 */

const QString &UserLogList::username() const
{
	return m_username;
}

void UserLogList::setUsername(const QString &newUsername)
{
	if (m_username == newUsername)
		return;
	m_username = newUsername;
	emit usernameChanged();
}


/**
 * @brief UserLogList::api
 * @return
 */

const WebSocket::API &UserLogList::api() const
{
	return m_api;
}

void UserLogList::setApi(const WebSocket::API &newApi)
{
	if (m_api == newApi)
		return;
	m_api = newApi;
	emit apiChanged();
}

const QString &UserLogList::path() const
{
	return m_path;
}

void UserLogList::setPath(const QString &newPath)
{
	if (m_path == newPath)
		return;
	m_path = newPath;
	emit pathChanged();
}


/**
 * @brief UserLogList::reload
 */

void UserLogList::reload()
{
	if (m_username.isEmpty()) {
		LOG_CWARNING("client") << "Empty username";
		return;
	}

	emit modelReloadRequested();

	Client *client = Application::instance()->client();

	client->send(m_api, m_path.arg(m_username))
			->error(client, &Client::onWebSocketError)
			->fail(client, [client](const QString &err){ client->messageWarning(err, tr("Letöltési hiba")); })
			->done(this, &UserLogList::loadFromJson);
}



/**
 * @brief UserLogList::loadFromJson
 * @param obj
 */


void UserLogList::loadFromJson(const QJsonObject &obj)
{
	QVariantList model;

	Server *server = Application::instance()->client()->server();


	// Rank

	foreach (const QJsonValue &v, obj.value(QStringLiteral("ranklog")).toArray()) {
		const QJsonObject &o = v.toObject();
		QVariantMap m;

		m[QStringLiteral("timestamp")] = QDateTime::fromSecsSinceEpoch(o.value(QStringLiteral("timestamp")).toInt());
		m[QStringLiteral("xp")] = o.value(QStringLiteral("xp")).toInt();

		Rank rank;

		if (server)
			rank = server->rank(o.value(QStringLiteral("rankid")).toInt());

		m[QStringLiteral("rank")] = rank.toJson().toVariantMap();

		model.append(m);
	}

	// Streak

	foreach (const QJsonValue &v, obj.value(QStringLiteral("streaklog")).toArray()) {
		const QJsonObject &o = v.toObject();
		QVariantMap m;

		m[QStringLiteral("timestamp")] = QDateTime::fromSecsSinceEpoch(o.value(QStringLiteral("ended_on")).toInt());
		m[QStringLiteral("streak")] = o.value(QStringLiteral("streak")).toInt();

		model.append(m);
	}


	/// SORT

	std::sort(model.begin(), model.end(), [](const QVariant &left, const QVariant &right) -> bool {
		const QVariantMap &mLeft = left.toMap();
		const QVariantMap &mRight = right.toMap();

		const QDateTime &dLeft = mLeft.value(QStringLiteral("timestamp")).toDateTime();
		const QDateTime &dRight = mRight.value(QStringLiteral("timestamp")).toDateTime();

		if (dLeft > dRight)
			return true;

		if (dLeft < dRight)
			return false;

		return mLeft.contains(QStringLiteral("rank"));
	});

	setModel(model);

	emit modelReloaded();


	/// COUNTERS

	QMap<GameMap::GameMode, QMap<QString, int>> modeMap;

	foreach (const QJsonValue &v, obj.value(QStringLiteral("durations")).toArray()) {
		const QJsonObject &o = v.toObject();
		const GameMap::GameMode &mode = o.value(QStringLiteral("mode")).toVariant().value<GameMap::GameMode>();

		if (mode == GameMap::Invalid) {
			LOG_CWARNING("client") << "Invalid GameMap::GameMode" << o;
			continue;
		}

		modeMap[mode][QStringLiteral("duration")] = o.value(QStringLiteral("duration")).toInt();
	}

	foreach (const QJsonValue &v, obj.value(QStringLiteral("trophies")).toArray()) {
		const QJsonObject &o = v.toObject();
		const GameMap::GameMode &mode = o.value(QStringLiteral("mode")).toVariant().value<GameMap::GameMode>();

		if (mode == GameMap::Invalid) {
			LOG_CWARNING("client") << "Invalid GameMap::GameMode" << o;
			continue;
		}

		modeMap[mode][QStringLiteral("trophy")] = o.value(QStringLiteral("trophy")).toInt();
	}

	QVariantList counterList;

	for (auto it=modeMap.constBegin(); it != modeMap.constEnd(); ++it) {
		QVariantMap record;
		record.insert(QStringLiteral("mode"), it.key());

		for (auto ii=it.value().constBegin(); ii != it.value().constEnd(); ++ii) {
			record.insert(ii.key(), ii.value());
		}

		counterList.append(record);
	}

	setCounters(counterList);
}


/**
 * @brief UserLogList::counters
 * @return
 */

const QVariantList &UserLogList::counters() const
{
	return m_counters;
}

void UserLogList::setCounters(const QVariantList &newCounters)
{
	/*if (m_counters == newCounters)			// Nem kell, mert különben üres lista esetén nem tűnnek el a placeholderek
		return;*/
	m_counters = newCounters;
	emit countersChanged();
}



/**
 * @brief UserLogList::model
 * @return
 */

const QVariantList &UserLogList::model() const
{
	return m_model;
}

void UserLogList::setModel(const QVariantList &newModel)
{
	if (m_model == newModel)
		return;
	m_model = newModel;
	emit modelChanged();
}
