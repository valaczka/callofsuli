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
 * @param messageType
 */

void CosMessage::increaseMsgId()
{
	if (m_msgId == INT_MAX)
		m_msgId = 0;

	m_msgId++;
}













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
	, m_clientRole()
	, m_cosClass(ClassInvalid)
	, m_cosFunc()
	, m_jsonData()
	, m_peerMsgId(0)
	, m_responsedMsgId(0)
{
	if (!message.isEmpty())
		appendFrame(message);
}

CosMessage::CosMessage()
	: m_messageType(MessageInvalid)
	, m_messageError(NoError)
	, m_serverError(ServerNoError)
	, m_serverErrorDetails()
	, m_binaryData()
	, m_binaryDataExpectedSize(0)
	, m_receivedFrameSize(0)
	, m_jsonAuth()
	, m_clientRole()
	, m_cosClass(ClassInvalid)
	, m_cosFunc()
	, m_jsonData()
	, m_peerMsgId(0)
	, m_responsedMsgId(0)
{

}


CosMessage::CosMessage(const CosMessage::CosMessageType &messageType, const CosMessage &orig)
	: m_messageType(messageType)
	, m_messageError(NoError)
	, m_serverError(ServerNoError)
	, m_serverErrorDetails()
	, m_binaryData()
	, m_binaryDataExpectedSize(0)
	, m_receivedFrameSize(0)
	, m_jsonAuth()
	, m_clientRole()
	, m_cosClass(ClassInvalid)
	, m_cosFunc()
	, m_jsonData()
	, m_peerMsgId(0)
	, m_responsedMsgId(0)
{
	increaseMsgId();

	if (orig.valid())
		m_responsedMsgId = orig.peerMsgId();
}

CosMessage::CosMessage(const CosMessage::CosMessageError &messageError, const CosMessage &orig)
	: m_messageType(MessageOther)
	, m_messageError(messageError)
	, m_serverError(ServerNoError)
	, m_serverErrorDetails()
	, m_binaryData()
	, m_binaryDataExpectedSize(0)
	, m_receivedFrameSize(0)
	, m_jsonAuth()
	, m_clientRole()
	, m_cosClass(ClassInvalid)
	, m_cosFunc()
	, m_jsonData()
	, m_peerMsgId(0)
	, m_responsedMsgId(0)
{
	increaseMsgId();

	if (orig.valid())
		m_responsedMsgId = orig.peerMsgId();
}


/**
 * @brief CosMessage::CosMessage
 * @param serverError
 */

CosMessage::CosMessage(const CosMessage::CosMessageServerError &serverError, const QString &details, const CosMessage &orig)
	: m_messageType(MessageServerError)
	, m_messageError(NoError)
	, m_serverError(serverError)
	, m_serverErrorDetails(details)
	, m_binaryData()
	, m_binaryDataExpectedSize(0)
	, m_receivedFrameSize(0)
	, m_jsonAuth()
	, m_clientRole()
	, m_cosClass(ClassInvalid)
	, m_cosFunc()
	, m_jsonData()
	, m_peerMsgId(0)
	, m_responsedMsgId(0)
{
	increaseMsgId();

	if (orig.valid())
		m_responsedMsgId = orig.peerMsgId();
}


/**
 * @brief CosMessage::CosMessage
 * @param jsonData
 * @param orig
 */

CosMessage::CosMessage(const QJsonObject &jsonData, const CosClass &cosClass, const QString &cosFunc, const CosMessage &orig)
	: m_messageType(MessageJson)
	, m_messageError(NoError)
	, m_serverError(ServerNoError)
	, m_serverErrorDetails()
	, m_binaryData()
	, m_binaryDataExpectedSize(0)
	, m_receivedFrameSize(0)
	, m_jsonAuth()
	, m_clientRole()
	, m_cosClass(cosClass)
	, m_cosFunc(cosFunc)
	, m_jsonData(jsonData)
	, m_peerMsgId(0)
	, m_responsedMsgId(0)
{
	increaseMsgId();

	if (orig.valid())
		m_responsedMsgId = orig.peerMsgId();
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
	return _VERSION_MAJOR;//(_VERSION_MAJOR*100)+_VERSION_MINOR;
}


/**
 * @brief CosMessage::send
 * @param socket
 */

void CosMessage::send(QWebSocket *socket)
{
	Q_ASSERT(socket);

	QByteArray s;
	QDataStream writeStream(&s, QIODevice::WriteOnly);
	writeStream << (*this);

	qDebug() << "SEND" << (*this);

	socket->sendBinaryMessage(s);
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
		stream >> m_responsedMsgId;
		stream >> messageType;
		stream >> m_clientRole;
		stream >> m_jsonAuth;
		stream >> m_cosClass;
		stream >> m_cosFunc;

		switch (messageType) {
			case MessageServerError:
				m_messageType = MessageServerError;
				stream >> m_serverError;
				stream >> m_serverErrorDetails;
				break;
			case MessageJson:
				m_messageType = MessageJson;
				stream >> m_jsonData;
				break;
			case MessageBinaryData:
				m_messageType = MessageBinaryData;
				stream >> m_jsonData;
				stream >> m_binaryDataExpectedSize;
				stream >> m_binaryData;
				break;
			case MessageOther:
				m_messageType = MessageOther;
				stream >> m_messageError;
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
	if (serverError != ServerNoError)
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
	stream << cosMessage.m_responsedMsgId;
	stream << cosMessage.m_messageType;
	stream << cosMessage.m_clientRole;
	stream << cosMessage.m_jsonAuth;
	stream << cosMessage.m_cosClass;
	stream << cosMessage.m_cosFunc;

	switch (cosMessage.m_messageType) {
		case CosMessage::MessageJson:
			stream << cosMessage.jsonData();
			break;
		case CosMessage::MessageServerError:
			stream << cosMessage.m_serverError;
			stream << cosMessage.m_serverErrorDetails;
			break;
		case CosMessage::MessageBinaryData:
			stream << cosMessage.jsonData();
			stream << (quint32) cosMessage.m_binaryData.size();
			stream << cosMessage.m_binaryData;
			break;
		case CosMessage::MessageOther:
			stream << cosMessage.m_messageError;
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

	stream << cosMessage.m_messageType;
	stream << "#" << cosMessage.m_msgId;
	stream << "<" << cosMessage.m_peerMsgId;
	stream << ">" << cosMessage.m_responsedMsgId;
	stream << cosMessage.m_messageError;
	stream << cosMessage.m_serverError << cosMessage.m_serverErrorDetails;
	stream << cosMessage.m_clientRole << cosMessage.m_jsonAuth;
	stream << cosMessage.m_cosClass << cosMessage.m_cosFunc << cosMessage.m_jsonData;
	stream << "BINARY DATA" << cosMessage.m_binaryData.size();
	stream << "BINARY DATA EXPECTED SIZE" << cosMessage.m_binaryDataExpectedSize;

	return stream;
}
