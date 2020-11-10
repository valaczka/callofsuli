/* * ---- Call of Suli ----
 *
 * cosmessage.cpp
 *
 * Created on: 2020. 11. 08.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * CosMessage
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

#include "cosmessage.h"
#include "../version/buildnumber.h"

#include <QDebug>

int CosMessage::m_msgId = 0;

/**
 * @brief CosMessage::CosMessage
 * @param message
 */

CosMessage::CosMessage(const QByteArray &message)
	: m_messageType(MessageInvalid)
	, m_messageError(NoError)
	, m_serverError(ServerNoError)
	, m_serverErrorDetails()
	, m_binaryData()
	, m_binaryDataExpectedSize(0)
	, m_receivedFrameSize(0)
	, m_jsonAuth()
	, m_clientState(ClientInvalid)
	, m_clientRole()
	, m_peerMsgId(0)
{
	if (!message.isEmpty())
		appendFrame(message);
}


/**
 * @brief CosMessage::CosMessage
 * @param messageType
 */

CosMessage::CosMessage(const CosMessage::CosMessageType &messageType)
	: m_messageType(messageType)
	, m_messageError(NoError)
	, m_serverError(ServerNoError)
	, m_serverErrorDetails()
	, m_binaryData()
	, m_binaryDataExpectedSize(0)
	, m_receivedFrameSize(0)
	, m_jsonAuth()
	, m_clientState(ClientInvalid)
	, m_clientRole()
	, m_peerMsgId(0)
{
	if (m_msgId == INT_MAX)
		m_msgId = 0;

	m_msgId++;

}


/**
 * @brief CosMessage::CosMessage
 * @param serverError
 */

CosMessage::CosMessage(const CosMessage::CosMessageServerError &serverError, const QString &details)
	: m_messageType(MessageServerError)
	, m_messageError(NoError)
	, m_serverError(serverError)
	, m_serverErrorDetails(details)
	, m_binaryData()
	, m_binaryDataExpectedSize(0)
	, m_receivedFrameSize(0)
	, m_jsonAuth()
	, m_clientState(ClientInvalid)
	, m_clientRole()
	, m_peerMsgId(0)
{
	if (m_msgId == INT_MAX)
		m_msgId = 0;

	m_msgId++;
}


/**
 * @brief CosMessage::versionMajor
 * @return
 */

inline int CosMessage::versionMajor()
{
	return _VERSION_MAJOR;
}

/**
 * @brief CosMessage::versionMinor
 * @return
 */

inline int CosMessage::versionMinor()
{
	return _VERSION_MINOR;
}


/**
 * @brief CosMessage::versionNumber
 * @return
 */

inline quint32 CosMessage::versionNumber()
{
	return (_VERSION_MAJOR*100)+_VERSION_MINOR;
}


/**
 * @brief CosMessage::appendFrame
 * @param frame
 * @param isLastFrame
 * @return
 */

CosMessage &CosMessage::appendFrame(const QByteArray &frame)
{
	if (m_messageType != MessageBinaryData || !m_binaryDataExpectedSize) {
		QDataStream stream(frame);

		stream.startTransaction();

		quint32 magic;

		stream >> magic;

		if (magic != 0x434F53) {			// COS
			m_messageType = MessageInvalid;
			m_messageError = BadMessageFormat;
		}

		quint32 version;

		stream >> version;

		if (version < versionNumber()) {
			m_messageType = MessageInvalid;
			m_messageError = MessageTooOld;
		}

		if (version > versionNumber()) {
			m_messageType = MessageInvalid;
			m_messageError = MessageTooNew;
		}

		stream.setVersion(QDataStream::Qt_5_11);

		CosMessageType messageType = MessageInvalid;

		stream >> m_peerMsgId;
		stream >> messageType;
		stream >> m_clientState;
		stream >> m_clientRole;
		stream >> m_jsonAuth;

		switch (messageType) {
			case MessageServerError:
				m_messageType = MessageServerError;
				stream >> m_serverError;
				stream >> m_serverErrorDetails;
				break;
			case MessageBinaryData:
				m_messageType = MessageBinaryData;
				stream >> m_binaryDataExpectedSize;
				stream >> m_binaryData;
				break;

			default:
				m_messageType = MessageInvalid;
				m_messageError = InvalidMessageType;
				break;
		}

		if (!stream.commitTransaction()) {
			m_messageError = NoBinaryData;
		}
	}

	m_receivedFrameSize += frame.size();


	return *this;
}




/**
 * @brief CosMessage::binaryData
 * @return
 */

QByteArray CosMessage::binaryData() const
{
	return m_binaryData;
}


/**
 * @brief CosMessage::setBinaryData
 * @param binaryData
 */

void CosMessage::setBinaryData(const QByteArray &binaryData)
{
	m_messageType = MessageBinaryData;
	m_binaryData = binaryData;
}



void CosMessage::setServerError(const CosMessageServerError &serverError)
{
	m_messageType = MessageServerError;
	m_serverError = serverError;
}





/**
 * @brief operator <<
 * @param stream
 * @param cosMessage
 * @return
 */

QDataStream &operator<<(QDataStream &stream, const CosMessage &cosMessage)
{
	stream << (quint32) 0x434F53;			// COS
	stream << (quint32) CosMessage::versionNumber();

	stream.setVersion(QDataStream::Qt_5_11);

	stream << cosMessage.m_msgId;
	stream << cosMessage.m_messageType;
	stream << cosMessage.m_clientState;
	stream << cosMessage.m_clientRole;
	stream << cosMessage.m_jsonAuth;

	switch (cosMessage.m_messageType) {
		case CosMessage::MessageServerError:
			stream << cosMessage.m_serverError;
			stream << cosMessage.m_serverErrorDetails;
			break;
		case CosMessage::MessageBinaryData:
			stream << (quint32) cosMessage.m_binaryData.size();
			stream << cosMessage.m_binaryData;
			break;
		default:
			break;
	}

	return stream;
}


/**
 * @brief operator <<
 * @param stream
 * @param cosMessage
 * @return
 */

QDebug operator<<(QDebug stream, const CosMessage &cosMessage)
{
	QDebugStateSaver saver(stream);

	stream << "ID" << cosMessage.m_msgId;
	stream << "PEERID" << cosMessage.m_peerMsgId;
	stream << "VERSION" << CosMessage::versionNumber();
	stream << "MESSAGE TYPE" << cosMessage.m_messageType;
	stream << "CLIENT" << cosMessage.m_clientState << cosMessage.m_clientRole << cosMessage.m_jsonAuth;
	stream << "MESSAGE ERROR" << cosMessage.m_messageError;
	stream << "MESSAGE SERVER ERROR" << cosMessage.m_serverError << cosMessage.m_serverErrorDetails;
	stream << "BINARY DATA" << cosMessage.m_binaryData.size();
	stream << "BINARY DATA EXPECTED SIZE" << cosMessage.m_binaryDataExpectedSize;
	stream << "BINARY DATA RECEIVED SIZE" << cosMessage.m_receivedFrameSize;

	return stream;
}
