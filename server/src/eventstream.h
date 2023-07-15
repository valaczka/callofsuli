/*
 * ---- Call of Suli ----
 *
 * eventstream.h
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

#ifndef EVENTSTREAM_H
#define EVENTSTREAM_H

#include "httpEventStream.h"
#include "qjsonobject.h"

class ServerService;


/**
 * @brief The EventStream class
 */

class EventStream : public HttpEventStream
{
	Q_OBJECT

	Q_PROPERTY(EventStreamTypes streamTypes READ streamTypes WRITE setStreamTypes NOTIFY streamTypesChanged)

public:
	enum EventStreamType {
		EventStreamInvalid = 0,
		EventStreamGroupScore = 1
	};

	Q_ENUM(EventStreamType)

	Q_DECLARE_FLAGS(EventStreamTypes, EventStreamType)
	Q_FLAG(EventStreamTypes)


	EventStream(HttpConnection *httpConnection);
	EventStream(const EventStreamType &type, HttpConnection *httpConnection);
	EventStream(const EventStreamType &type, const QVariant &data, HttpConnection *httpConnection);
	virtual ~EventStream();

	static const QHash<QByteArray, EventStreamType> &streamTypeHash();
	static QByteArray streamType(const EventStreamType &type);
	static EventStream::EventStreamType streamType(const QByteArray &type);

	void trigger();
	void trigger(const EventStreamType &type);
	void trigger(const EventStreamType &type, const QVariant &data);

	void add(const EventStreamType &type);
	void add(const EventStreamType &type, const QVariant &data);

	const EventStreamTypes &streamTypes() const;
	void setStreamTypes(const EventStreamTypes &newStreamTypes);

	const QMap<EventStreamType, QVariant> &streamData() const;

	ServerService *service() const;
	void setService(ServerService *newService);

	void write(const EventStreamType &type, const QJsonObject &data);

signals:
	void streamTypesChanged();

private:
	void triggerGroupScore();

	EventStreamTypes m_streamTypes = EventStreamInvalid;
	QMap<EventStreamType, QVariant> m_streamData;

	static const QHash<QByteArray, EventStreamType> m_streamTypeHash;

	ServerService *m_service = nullptr;
};


Q_DECLARE_OPERATORS_FOR_FLAGS(EventStream::EventStreamTypes);

#endif // EVENTSTREAM_H
