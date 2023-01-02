/*
 * ---- Call of Suli ----
 *
 * websocketmessage.cpp
 *
 * Created on: 2022. 12. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * WebSocketMessage
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

#include "websocketmessage.h"
#include <QDataStream>

const quint32 WebSocketMessage::m_versionMajor = COS_VERSION_MAJOR;
const quint32 WebSocketMessage::m_versionMinor = COS_VERSION_MINOR;

int WebSocketMessage::m_msgNumberSequence = 0;

/**
 * @brief WebSocketMessage::WebSocketMessage
 */

WebSocketMessage::WebSocketMessage()
{

}


/**
 * @brief WebSocketMessage::createHello
 * @return
 */

WebSocketMessage WebSocketMessage::createHello()
{
	WebSocketMessage m;
	m.m_opCode = Hello;
	m.m_data = QJsonObject({
							   {QStringLiteral("version"), QStringLiteral("%1.%2").arg(m_versionMajor).arg(m_versionMinor)},
							   {QStringLiteral("versionMajor"), (int) m_versionMajor},
							   {QStringLiteral("versionMinor"), (int) m_versionMinor}
						   });
	return m;
}


/**
 * @brief WebSocketMessage::createEvent
 * @param data
 * @param binaryData
 * @return
 */

WebSocketMessage WebSocketMessage::createEvent(const QJsonObject &data, const QByteArray &binaryData)
{
	WebSocketMessage m;
	m.m_opCode = Event;
	m.m_data = data;
	m.m_binaryData = binaryData;
	return m;
}


/**
 * @brief WebSocketMessage::createRequest
 * @param data
 * @param binaryData
 * @return
 */

WebSocketMessage WebSocketMessage::createRequest(const QJsonObject &data, const QByteArray &binaryData)
{
	WebSocketMessage m;
	m.m_opCode = Request;
	m.m_requestMsgNumber = nextMsgNumber();
	m.m_data = data;
	m.m_binaryData = binaryData;
	return m;
}


/**
 * @brief WebSocketMessage::createResponse
 * @param data
 * @param binaryData
 * @return
 */

WebSocketMessage WebSocketMessage::createResponse(const QJsonObject &data, const QByteArray &binaryData)
{
	WebSocketMessage m;
	m.m_opCode = RequestResponse;
	m.m_requestMsgNumber = m_msgNumber;
	m.m_data = data;
	m.m_binaryData = binaryData;
	return m;
}

/**
 * @brief WebSocketMessage::versionMajor
 * @return
 */

quint32 WebSocketMessage::versionMajor()
{
	return m_versionMajor;
}


/**
 * @brief WebSocketMessage::versionMinor
 * @return
 */

quint32 WebSocketMessage::versionMinor()
{
	return m_versionMinor;
}


/**
 * @brief WebSocketMessage::versionCode
 * @return
 */

quint32 WebSocketMessage::versionCode()
{
	return (1000*m_versionMajor)+m_versionMinor;
}


/**
 * @brief WebSocketMessage::versionCode
 * @param major
 * @param minor
 * @return
 */

quint32 WebSocketMessage::versionCode(const int &major, const int &minor)
{
	return (1000*major)+minor;
}


/**
 * @brief WebSocketMessage::opCode
 * @return
 */

const WebSocketMessage::WebSocketOpCode &WebSocketMessage::opCode() const
{
	return m_opCode;
}


/**
 * @brief WebSocketMessage::data
 * @return
 */

const QJsonObject &WebSocketMessage::data() const
{
	return m_data;
}

void WebSocketMessage::setData(const QJsonObject &newData)
{
	m_data = newData;
}


/**
 * @brief WebSocketMessage::binaryData
 * @return
 */

const QByteArray &WebSocketMessage::binaryData() const
{
	return m_binaryData;
}

void WebSocketMessage::setBinaryData(const QByteArray &newBinaryData)
{
	m_binaryData = newBinaryData;
}




/**
 * @brief WebSocketMessage::requestMsgNumber
 * @return
 */

int WebSocketMessage::requestMsgNumber() const
{
	return m_requestMsgNumber;
}



/**
 * @brief WebSocketMessage::headerObject
 * @return
 */

QJsonObject WebSocketMessage::headerObject() const
{
	QJsonObject o;

	o.insert(QStringLiteral("op"), m_opCode);
	o.insert(QStringLiteral("d"), m_data);

	if (m_opCode == Request)
		o.insert(QStringLiteral("msg"), m_msgNumber);

	if (m_opCode == RequestResponse)
		o.insert(QStringLiteral("request"), m_requestMsgNumber);

	if (!m_binaryData.isEmpty())
		o.insert(QStringLiteral("size"), m_binaryData.size());

	return o;
}



/**
 * @brief WebSocketMessage::toByteArray
 * @return
 */

QByteArray WebSocketMessage::toByteArray() const
{
	QByteArray s;
	QDataStream writeStream(&s, QIODevice::WriteOnly);
	writeStream << (*this);

	return s;
}



/**
 * @brief WebSocketMessage::nextMsgNumber
 * @return
 */

int WebSocketMessage::nextMsgNumber()
{
	if (m_msgNumberSequence == INT_MAX)
		m_msgNumberSequence = 0;

	return ++m_msgNumberSequence;
}


/**
 * @brief WebSocketMessage::expectedDataSize
 * @return
 */

int WebSocketMessage::expectedDataSize() const
{
	return m_expectedDataSize;
}


/**
 * @brief WebSocketMessage::error
 * @return
 */

const WebSocketMessage::WebSocketError &WebSocketMessage::error() const
{
	return m_error;
}




/**
 * @brief WebSocketMessage::fromByteArray
 * @return
 */

WebSocketMessage WebSocketMessage::fromByteArray(const QByteArray &binaryData, const bool &isFrame)
{
	WebSocketMessage m;

	QDataStream stream(binaryData);

	stream.setVersion(QDataStream::Qt_5_11);

	stream.startTransaction();

	quint32 magic;

	stream >> magic;

	if (magic != 0x434F53) {                        // COS
		stream.abortTransaction();
		m.m_error = InvalidMessage;
		return m;
	}

	QJsonObject headerObject;

	stream >> headerObject;

	if (headerObject.isEmpty()) {
		stream.abortTransaction();
		m.m_error = InvalidHeaderObject;
		return m;
	}

	WebSocketOpCode opCode = headerObject.value(QStringLiteral("op")).toVariant().value<WebSocketOpCode>();

	if (opCode == Invalid) {
		stream.abortTransaction();
		m.m_error = InvalidOpCode;
		return m;
	}

	if (opCode == Request) {
		int n = headerObject.value(QStringLiteral("msg")).toInt();

		m.m_msgNumber = n;

		if (n <= 0) {
			stream.abortTransaction();
			m.m_error = MissingRequestNumber;
			return m;
		}
	} else if (opCode == RequestResponse) {
		int n = headerObject.value(QStringLiteral("request")).toInt();

		m.m_requestMsgNumber = n;

		if (n <= 0) {
			stream.abortTransaction();
			m.m_error = MissingRequestNumber;
			return m;
		}
	}

	m.m_data = headerObject.value(QStringLiteral("d")).toObject();

	int bSize = headerObject.value(QStringLiteral("size")).toInt();

	m.m_expectedDataSize = bSize;

	if (bSize > 0) {
		QByteArray bData;
		stream >> bData;

		if (!isFrame && bData.isEmpty()) {
			stream.abortTransaction();
			m.m_error = MissingBinaryData;
			return m;
		}

		m.m_binaryData = bData;
	}

	if (!stream.commitTransaction() && !isFrame) {
		stream.abortTransaction();
		m.m_error = UncompletedTransaction;
		return m;
	}

	m.m_opCode = opCode;

	return m;
}



/**
 * @brief operator <<
 * @param stream
 * @param message
 * @return
 */


QDataStream &operator<<(QDataStream &stream, const WebSocketMessage &message)
{
	stream.setVersion(QDataStream::Qt_5_11);

	stream << (quint32) 0x434F53;                   // COS
	stream << message.headerObject();

	if (!message.m_binaryData.isEmpty())
		stream << message.m_binaryData;

	return stream;
}



/**
 * @brief operator <<
 * @param stream
 * @param message
 * @return
 */

QDebug operator<<(QDebug stream, const WebSocketMessage &message)
{
	QDebugStateSaver saver(stream);

	stream.nospace().noquote() << QStringLiteral("WebSocketMessage(") ;

	if (message.m_error != WebSocketMessage::NoError) {
		stream.nospace().noquote() << message.m_error;
	} else {
		stream.nospace().noquote() << message.m_opCode << QStringLiteral(", ") << message.headerObject();

		if (!message.m_binaryData.isEmpty())
			stream.nospace().noquote() << QStringLiteral(", QByteArray(") << message.m_binaryData.size() << QStringLiteral(")");
	}

	stream.nospace().noquote() << QStringLiteral(")");

	return stream;
}
