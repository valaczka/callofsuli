/*
 * ---- Call of Suli ----
 *
 * eventstream.cpp
 *
 * Created on: 2023. 06. 18.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * EventStream
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

#include "eventstream.h"
#include "Logger.h"
#include "qdeferred.hpp"
#include "userapi.h"
#include "serverservice.h"


const QHash<QByteArray, EventStream::EventStreamType> EventStream::m_streamTypeHash = {
	{ QByteArrayLiteral("groupScore"), EventStream::EventStreamGroupScore }
};


/**
 * @brief EventStream::EventStream
 * @param httpConnection
 */

EventStream::EventStream(const EventStreamType &type, HttpConnection *httpConnection)
	: EventStream(httpConnection)
{
	add(type);
}


/**
 * @brief EventStream::EventStream
 * @param httpConnection
 */

EventStream::EventStream(HttpConnection *httpConnection)
	: HttpEventStream(httpConnection)
{

}


/**
 * @brief EventStream::EventStream
 * @param type
 * @param data
 * @param httpConnection
 */

EventStream::EventStream(const EventStreamType &type, const QVariant &data, HttpConnection *httpConnection)
	: EventStream(type, httpConnection)
{
	m_streamData.insert(type, data);
}


/**
 * @brief EventStream::~EventStream
 */

EventStream::~EventStream()
{
	m_service = nullptr;
}








/**
 * @brief EventStream::streamTypeHash
 * @return
 */

const QHash<QByteArray, EventStream::EventStreamType> &EventStream::streamTypeHash()
{
	return m_streamTypeHash;
}



/**
 * @brief EventStream::streamType
 * @param type
 * @return
 */

QByteArray EventStream::streamType(const EventStreamType &type)
{
	for (auto it = m_streamTypeHash.constBegin(); it != m_streamTypeHash.constEnd(); ++it) {
		if (it.value() == type)
			return it.key();
	}

	return QByteArray();
}



/**
 * @brief EventStream::streamType
 * @param type
 * @return
 */

EventStream::EventStreamType EventStream::streamType(const QByteArray &type)
{
	return m_streamTypeHash.value(type, EventStreamInvalid);
}



/**
 * @brief EventStream::trigger
 */

void EventStream::trigger()
{
	LOG_CTRACE("client") << "Event stream triggered:" << this;

	trigger(EventStreamGroupScore);
}


/**
 * @brief EventStream::trigger
 * @param type
 */

void EventStream::trigger(const EventStreamType &type)
{
	if (!m_streamTypes.testFlag(type))
		return;

	LOG_CTRACE("client") << "Event stream triggered:" << type << this;

	if (type == EventStreamGroupScore)
		triggerGroupScore();
}


/**
 * @brief EventStream::trigger
 * @param type
 * @param data
 */

void EventStream::trigger(const EventStreamType &type, const QVariant &data)
{
	if (!m_streamTypes.testFlag(type))
		return;

	if (m_streamData.value(type) != data)
		return;

	LOG_CTRACE("client") << "Event stream triggered:" << type << data << this;

	if (type == EventStreamGroupScore)
		triggerGroupScore();
}


/**
 * @brief EventStream::add
 * @param type
 */

void EventStream::add(const EventStreamType &type)
{
	m_streamTypes.setFlag(type);
}



/**
 * @brief EventStream::add
 * @param type
 * @param data
 */

void EventStream::add(const EventStreamType &type, const QVariant &data)
{
	m_streamTypes.setFlag(type);
	m_streamData.insert(type, data);
}





/**
 * @brief EventStream::service
 * @return
 */

ServerService *EventStream::service() const
{
	return m_service;
}

void EventStream::setService(ServerService *newService)
{
	m_service = newService;
}


/**
 * @brief EventStream::write
 * @param type
 * @param data
 * @return
 */

void EventStream::write(const EventStreamType &type, const QJsonObject &data)
{
	if (!HttpEventStream::write(streamType(type), QJsonDocument(data).toJson(QJsonDocument::Compact)))
		LOG_CWARNING("client") << "EventStream write failed" << this;
}


/**
 * @brief EventStream::streamData
 * @return
 */

const QMap<EventStream::EventStreamType, QVariant> &EventStream::streamData() const
{
	return m_streamData;
}



/**
 * @brief EventStream::streamTypes
 * @return
 */

const EventStream::EventStreamTypes &EventStream::streamTypes() const
{
	return m_streamTypes;
}

void EventStream::setStreamTypes(const EventStreamTypes &newStreamTypes)
{
	if (m_streamTypes == newStreamTypes)
		return;
	m_streamTypes = newStreamTypes;
	emit streamTypesChanged();
}





/// TRIGGERS ///



/**
 * @brief EventStream::triggerGroupScore
 */

void EventStream::triggerGroupScore()
{
	const int &id = m_streamData.value(EventStreamGroupScore, -1).toInt();

	if (id < 0) {
		LOG_CERROR("client") << "EventStreamGroupScore invalid id";
		return;
	}

	if (!m_service)
		return;

	QDeferred<QJsonArray> def = UserAPI::getGroupScore(m_service->databaseMain(), id);

	def.fail([this](const QJsonArray &) {
		write(EventStreamGroupScore, QJsonObject{{QStringLiteral("error"), true}});
	})
			.done([this](const QJsonArray &list) {
		write(EventStreamGroupScore, QJsonObject{{QStringLiteral("list"), list}});
	});
}
