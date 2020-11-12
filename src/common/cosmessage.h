/*
 * ---- Call of Suli ----
 *
 * cosmessage.h
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

#ifndef COSMESSAGE_H
#define COSMESSAGE_H

#include <QDataStream>
#include <QJsonObject>
#include <QWebSocket>


class CosMessage
{
public:
	enum CosMessageType {
		MessageInvalid,
		MessageServerError,
		MessageJson,
		MessageBinaryData,
		MessageOther
	};

	enum CosMessageError {
		NoError,
		BadMessageFormat,
		MessageTooOld,
		MessageTooNew,
		InvalidMessageType,
		NoBinaryData,
		PasswordRequestMissingEmail,
		PasswordRequestInvalidEmail,
		PasswordRequestInvalidCode,
		PasswordRequestCodeSent,
		PasswordRequestSuccess,
		InvalidSession,
		InvalidUser,
		PasswordResetRequired,
		InvalidClass,
		InvalidFunction,
		ClassPermissionDenied,
		OtherError
	};

	enum CosMessageServerError {
		ServerNoError,
		ServerInternalError,
		ServerSmtpError
	};


	enum ClientRole {
		RoleGuest = 0x01,
		RoleStudent = 0x02,
		RoleTeacher = 0x04,
		RoleAdmin = 0x08
	};

	Q_DECLARE_FLAGS(ClientRoles, ClientRole)


	enum CosClass {
		ClassInvalid,
		ClassLogout,
		ClassAdmin,
		ClassUserInfo
	};


	CosMessage(const QByteArray &message);

	CosMessage();
	CosMessage(const CosMessageType &messageType, const CosMessage &orig = CosMessage());
	CosMessage(const CosMessageError &messageError, const CosMessage &orig = CosMessage());
	CosMessage(const CosMessageServerError &serverError, const QString &details = QString(), const CosMessage &orig = CosMessage());
	CosMessage(const QJsonObject &jsonData, const CosMessage &orig = CosMessage());

	static int versionMajor();
	static int versionMinor();
	static quint32 versionNumber();

	void send(QWebSocket *socket);

	int msgId() const { return m_msgId; }

	CosMessageType messageType() const { return m_messageType; }
	inline bool hasError() const { return m_messageError != NoError; }
	inline bool valid() const { return m_messageType != MessageInvalid && m_messageError == NoError;}
	quint32 binaryDataExpectedSize() const { return m_binaryDataExpectedSize; }
	quint32 receivedFrameSize() const { return m_receivedFrameSize; }
	inline qreal receivedDataRatio() const { return m_binaryDataExpectedSize ? (qreal) m_receivedFrameSize/ (qreal) m_binaryDataExpectedSize : 0;}

	CosMessage &appendFrame(const QByteArray &frame);

	friend QDataStream &operator<<(QDataStream &stream, const CosMessage &cosMessage);
	friend QDebug operator<<(QDebug stream, const CosMessage &cosMessage);

	QByteArray binaryData() const;
	void setBinaryData(const QByteArray &binaryData);

	CosMessageServerError serverError() const { return m_serverError; }
	void setServerError(const CosMessageServerError &serverError);

	QJsonObject jsonAuth() const { return m_jsonAuth; }
	void setJsonAuth(const QJsonObject &jsonAuth) { m_jsonAuth = jsonAuth; }

	ClientRoles clientRole() const { return m_clientRole; }
	void setClientRole(const ClientRoles &clientRole) { m_clientRole = clientRole; }

	CosClass cosClass() const { return m_cosClass; }
	void setCosClass(const CosClass &cosClass) { m_cosClass = cosClass; }

	QString cosFunc() const { return m_cosFunc; }
	void setCosFunc(const QString &cosFunc) { m_cosFunc = cosFunc; }

	QJsonObject jsonData() const { return m_jsonData; }
	void setJsonData(const QJsonObject &jsonData) { m_jsonData = jsonData; }

	CosMessageError messageError() const { return m_messageError; }
	void setMessageError(const CosMessageError &messageError) { m_messageError = messageError; }

private:
	void increaseMsgId();

	CosMessageType m_messageType;
	CosMessageError m_messageError;
	CosMessageServerError m_serverError;
	QString m_serverErrorDetails;
	QByteArray m_binaryData;
	quint32 m_binaryDataExpectedSize;
	quint32 m_receivedFrameSize;

	QJsonObject m_jsonAuth;
	ClientRoles m_clientRole;

	CosClass m_cosClass;
	QString m_cosFunc;
	QJsonObject m_jsonData;

	int m_peerMsgId;
	static int m_msgId;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(CosMessage::ClientRoles)

#endif // COSMESSAGE_H
