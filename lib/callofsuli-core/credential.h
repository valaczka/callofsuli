/*
 * ---- Call of Suli ----
 *
 * credential.h
 *
 * Created on: 2023. 01. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Credential
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

#ifndef CREDENTIAL_H
#define CREDENTIAL_H

#include "qcryptographichash.h"
#include <QJsonObject>

#define JWT_ISSUER QStringLiteral("Call of Suli")


class Token
{
public:
	Token() {}
	Token(const QByteArray &token);
	Token(const QJsonObject &payload, const QJsonObject &header = {})
		: m_header(header), m_payload(payload)
	{  }

	const QJsonObject &header() const { return m_header; }
	void setHeader(const QJsonObject &newHeader) { m_header = newHeader; m_origToken.clear(); }
	QJsonObject &header() { return m_header; }

	const QJsonObject &payload() const { return m_payload; }
	void setPayload(const QJsonObject &newPayload) { m_payload = newPayload; m_origToken.clear(); }
	QJsonObject &payload() { return m_payload; }

	const QByteArray &signature() const { return m_signature;}
	void setSignature(const QByteArray &newSignature) { m_signature = newSignature; m_origToken.clear(); }

	const QByteArray &secret() const { return m_secret; }
	void setSecret(const QByteArray &newSecret) { m_secret = newSecret; }

	static QByteArray defaultHeader();
	static QByteArray generateSignature(const QJsonObject &payload, const QByteArray &secret, const QJsonObject &header = {});
	static QByteArray generateSignature(const QByteArray &header, const QByteArray &payload, const QByteArray &secret);
	static QByteArray getToken(const QByteArray &header, const QByteArray &payload, const QByteArray &signature);
	static QByteArray getToken(const QJsonObject &payload, const QByteArray &secret, const QJsonObject &header = {});

	static QByteArray sign(const QByteArray &content, const QByteArray &secret);
	static bool verify(const QByteArray &content, const QByteArray &mac, const QByteArray &secret);

	bool verify() const;
	bool verify(const QByteArray &secret) const;

	const QByteArray &generateSignature() {
		m_signature = generateSignature(m_payload, m_secret, m_header);
		return m_signature;
	}

	QByteArray getToken() const;
	QByteArray getToken(const QByteArray &secret) {
		setSecret(secret);
		return getToken();
	}

private:
	QJsonObject m_header;
	QJsonObject m_payload;
	QByteArray m_signature;
	QByteArray m_secret;
	QByteArray m_origToken;
};



class Credential
{
	Q_GADGET

public:
	Credential() {}
	virtual ~Credential() {}

	enum Role {
		None = 0,
		Student = 1,
		Teacher = 1 << 1,
		Panel = 1 << 2,
		SNI = 1 << 3,
		Admin = 1 << 8
	};

	Q_ENUM(Role)
	Q_DECLARE_FLAGS(Roles, Role)
	Q_FLAG(Roles)

	Credential(const QString &username, const Roles &roles = None);

	QByteArray createJWT(const QByteArray &secret) const;
	static Credential fromJWT(const QByteArray &jwt);

	static bool verify(const QByteArray &token, const QByteArray &secret, const qint64 &firstIat);

	[[deprecated]] static QString hashString(const QString &str, const QString &salt,
							  const QCryptographicHash::Algorithm &method = QCryptographicHash::Sha1);

	[[deprecated]] static QString hashString(const QString &str, QString *salt = nullptr,
							  const QCryptographicHash::Algorithm &method = QCryptographicHash::Sha1);

	virtual bool isValid() const;

	const QString &username() const;
	void setUsername(const QString &newUsername);

	const Roles &roles() const;
	void setRoles(const Roles &newRoles);
	void setRole(const Role &role, const bool &on = true);

	qint64 iat() const;

	inline bool operator==(const Credential &other) {
		return other.m_username == m_username &&
				other.m_roles == m_roles;
	}


private:
	QString m_username;
	Roles m_roles = None;
	qint64 m_iat = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Credential::Roles);

#endif // CREDENTIAL_H
