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
#include "qdatetime.h"
#include <QJsonObject>
#include <qjsonwebtoken.h>

#define JWT_ISSUER QStringLiteral("Call of Suli")

class Credential
{
	Q_GADGET

public:
	Credential() {}

	enum Role {
		None = 0,
		Student = 1,
		Teacher = 1 << 1,
		Panel = 1 << 2,
		Admin = 1 << 8
	};

	Q_ENUM(Role)
	Q_DECLARE_FLAGS(Roles, Role)
	Q_FLAG(Roles)

	Credential(const QString &username, const Roles &roles = None);

	QString createJWT(const QString &secret) const;
	static Credential fromJWT(const QString &jwt);

	static bool verify(const QString &token, const QString &secret, const qint64 &firstIat);

	static QString hashString(const QString &str, const QString &salt,
							  const QCryptographicHash::Algorithm &method = QCryptographicHash::Sha1);

	static QString hashString(const QString &str, QString *salt = nullptr,
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
