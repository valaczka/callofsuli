/*
 * ---- Call of Suli ----
 *
 * websocketmessage.h
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

#ifndef WEBSOCKETMESSAGE_H
#define WEBSOCKETMESSAGE_H

#include "qjsonobject.h"
#include "qobjectdefs.h"

/**
 * @brief The WebSocketMessage class
 */

class WebSocketMessage
{
	Q_GADGET

public:
	WebSocketMessage();

	/**
	 * @brief The WebSocketOpCode enum
	 */

	enum WebSocketOpCode {
		Invalid = 0,
		Hello,
		Request,
		RequestResponse,
		Event
	};

	Q_ENUM(WebSocketOpCode);


	/**
	 * @brief The WebSocketError enum
	 */

	enum WebSocketError {
		NoError = 0,
		InvalidMessage,
		InvalidHeaderObject,
		InvalidOpCode,
		MissingBinaryData,
		MissingRequestNumber,
		UncompletedTransaction
	};

	Q_ENUM(WebSocketError);


	/**
	 * @brief The Handler enum
	 */

	enum ClassHandler {
		ClassInvalid,
		ClassServer,
		ClassClient,
		ClassAuth,
		ClassGeneral,
		ClassUser,
		ClassTeacher,
		ClassPanel,
		ClassAdmin
	};

	Q_ENUM(ClassHandler);


	static WebSocketMessage createHello();
	static WebSocketMessage createEvent(const QJsonObject &data, const QByteArray &binaryData = QByteArray());
	static WebSocketMessage createEvent(const ClassHandler &classHandler, const QJsonObject &data, const QByteArray &binaryData = QByteArray());
	static WebSocketMessage createRequest(const QJsonObject &data, const QByteArray &binaryData = QByteArray());
	static WebSocketMessage createRequest(const ClassHandler &classHandler, const QJsonObject &data, const QByteArray &binaryData = QByteArray());
	WebSocketMessage createResponse(QJsonObject data, const QByteArray &binaryData = QByteArray()) const;
	WebSocketMessage createErrorResponse(const QString &errorString) const;
	WebSocketMessage createStatusResponse(const QString &statusString = QStringLiteral("ok")) const;
	static WebSocketMessage createErrorEvent(const QString &errorString, const ClassHandler &classHandler = ClassInvalid);

	static WebSocketMessage fromByteArray(const QByteArray &binaryData, const bool &isFrame = false);

	static quint32 versionMajor();
	static quint32 versionMinor();
	static quint32 versionCode();
	static quint32 versionCode(const int &major, const int &minor);

	const WebSocketOpCode &opCode() const;
	const ClassHandler &classHandler() const;

	bool isValid() const { return m_opCode != Invalid && m_error == NoError; }

	const QJsonObject &data() const;
	void setData(const QJsonObject &newData);

	const QByteArray &binaryData() const;
	void setBinaryData(const QByteArray &newBinaryData);

	int expectedDataSize() const;

	int msgNumber() const;
	int requestMsgNumber() const;

	const WebSocketError &error() const;

	bool hasResponseError() const;
	QString responseError() const;

	QJsonObject headerObject() const;
	QByteArray toByteArray() const;

	friend QDataStream &operator<<(QDataStream &stream, const WebSocketMessage &message);
	friend QDataStream &operator>>(QDataStream &stream, const WebSocketMessage &message);
	friend QDebug operator<<(QDebug stream, const WebSocketMessage &message);



private:
	static int nextMsgNumber();

	static const quint32 m_versionMajor;
	static const quint32 m_versionMinor;
	static int m_msgNumberSequence;

	WebSocketOpCode m_opCode = Invalid;
	WebSocketError m_error = NoError;
	ClassHandler m_classHandler = ClassInvalid;
	QJsonObject m_data;
	QByteArray m_binaryData;
	int m_msgNumber = -1;
	int m_requestMsgNumber = -1;
	int m_expectedDataSize = -1;
};

Q_DECLARE_METATYPE(WebSocketMessage::ClassHandler)

#endif // WEBSOCKETMESSAGE_H
