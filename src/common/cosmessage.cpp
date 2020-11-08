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

CosMessage::CosMessage()
	: m_messageType(MessageInvalid)
	, m_messageError(NoError)
	, m_binaryData()
	, m_binaryDataExpectedSize(0)
	, m_isLoading(false)
{

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

	stream << cosMessage.m_messageType;

	switch (cosMessage.m_messageType) {
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
 * @brief operator >>
 * @param stream
 * @param cosMessage
 * @return
 */

QDataStream &operator>>(QDataStream &stream, CosMessage &cosMessage)
{
	if (cosMessage.m_messageType == CosMessage::MessageBinaryData && cosMessage.m_isLoading) {
		stream >> cosMessage.m_binaryData;

		if ((quint32) cosMessage.m_binaryData.size() == cosMessage.m_binaryDataExpectedSize)
			cosMessage.m_isLoading = false;

		return stream;
	}

	quint32 magic;

	stream >> magic;

	if (magic != 0x434F53) {			// COS
		cosMessage.m_messageType = CosMessage::MessageInvalid;
		cosMessage.m_messageError = CosMessage::BadMessageFormat;
		return stream;
	}

	quint32 version;

	stream >> version;

	if (version < CosMessage::versionNumber()) {
		cosMessage.m_messageType = CosMessage::MessageInvalid;
		cosMessage.m_messageError = CosMessage::MessageTooOld;
		return stream;
	}

	if (version > CosMessage::versionNumber()) {
		cosMessage.m_messageType = CosMessage::MessageInvalid;
		cosMessage.m_messageError = CosMessage::MessageTooNew;
		return stream;
	}

	stream.setVersion(QDataStream::Qt_5_11);

	CosMessage::CosMessageType messageType = CosMessage::MessageInvalid;

	stream >> messageType;

	switch (messageType) {
		case CosMessage::MessageBinaryData:
			cosMessage.m_messageType = CosMessage::MessageBinaryData;
			stream >> cosMessage.m_binaryDataExpectedSize;
			stream >> cosMessage.m_binaryData;

			if ((quint32) cosMessage.m_binaryData.size() < cosMessage.m_binaryDataExpectedSize)
				cosMessage.m_isLoading = true;
			break;
		default:
			cosMessage.m_messageType = CosMessage::MessageInvalid;
			cosMessage.m_messageError = CosMessage::InvalidMessageType;
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

	stream << "VERSION" << CosMessage::versionNumber();
	stream << "MESSAGE TYPE" << cosMessage.m_messageType;
	stream << "MESSAGE ERROR" << cosMessage.m_messageError;
	stream << "BINARY DATA" << cosMessage.m_binaryData.size();
	stream << "BINARY DATA EXPECTED SIZE" << cosMessage.m_binaryDataExpectedSize;
	stream << "BINARY DATA LOADING" << cosMessage.m_isLoading;

	return stream;
}
