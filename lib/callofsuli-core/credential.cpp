/*
 * ---- Call of Suli ----
 *
 * credential.cpp
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

#include "credential.h"
#include "Logger.h"
#include "utils.h"


/**
 * @brief Credential::Credential
 * @param username
 * @param roles
 * @param expiration
 */

Credential::Credential(const QString &username, const Roles &roles)
	: m_username(username)
	, m_roles(roles)
	, m_iat(QDateTime::currentSecsSinceEpoch())
{

}




/**
 * @brief Credential::isValid
 * @return
 */

bool Credential::isValid() const
{
	return (!m_username.isEmpty());
}



/**
 * @brief Credential::toJWT
 * @param secret
 * @return
 */

QString Credential::createJWT(const QString &secret) const
{
	QStringList list;

	if (m_roles.testFlag(Student))
		list.append(QStringLiteral("student"));
	if (m_roles.testFlag(Teacher))
		list.append(QStringLiteral("teacher"));
	if (m_roles.testFlag(Panel))
		list.append(QStringLiteral("panel"));
	if (m_roles.testFlag(Admin))
		list.append(QStringLiteral("admin"));

	QDateTime exp = QDateTime::currentDateTimeUtc();

	if (m_roles.testFlag(Admin))
		exp = exp.addDays(1);
	else if (m_roles.testFlag(Teacher))
		exp = exp.addDays(5);
	else if (m_roles.testFlag(Student))
		exp = exp.addDays(10);
	else if (m_roles.testFlag(Panel))
		exp = exp.addDays(30);
	else
		exp = exp.addDays(50);

	QJsonWebToken jwt;

	jwt.setSecret(secret);


	QJsonObject obj;

	obj.insert(QStringLiteral("iss"), JWT_ISSUER);
	obj.insert(QStringLiteral("sub"), m_username);
	obj.insert(QStringLiteral("iat"), QDateTime::currentSecsSinceEpoch());
	obj.insert(QStringLiteral("exp"), exp.toSecsSinceEpoch());
	obj.insert(QStringLiteral("roles"), list.join("|"));

	jwt.setPayloadJDoc(QJsonDocument(obj));

	LOG_CTRACE("credential") << "Token created for user:" << m_username;

	return jwt.getToken();
}





/**
 * @brief Credential::fromJWT
 * @param jwt
 * @param secret
 * @return
 */

Credential Credential::fromJWT(const QString &jwt)
{
	QJsonWebToken token;

	if (!token.setToken(jwt)) {
		LOG_CWARNING("credential") << "Invalid token:" << jwt;
		return Credential();
	}

	const QJsonObject &obj = token.getPayloadJDoc().object();

	Credential c;

	c.setUsername(obj.value(QStringLiteral("sub")).toString());

	const QString &r = obj.value(QStringLiteral("roles")).toString();

	c.m_iat = obj.value(QStringLiteral("iat")).toInt();

	Roles roles;

	foreach (const QString &s, r.split("|")) {
		if (s == QLatin1String("student"))
			roles.setFlag(Student);
		else if (s == QLatin1String("teacher"))
			roles.setFlag(Teacher);
		else if (s == QLatin1String("panel"))
			roles.setFlag(Panel);
		else if (s == QLatin1String("admin"))
			roles.setFlag(Admin);
	}

	c.setRoles(roles);

	return c;
}




/**
 * @brief Credential::verify
 * @param token
 * @return
 */

bool Credential::verify(const QString &token, const QString &secret, const qint64 &firstIat)
{
	const QJsonWebToken &jwt = QJsonWebToken::fromTokenAndSecret(token, secret);

	if (!jwt.isValid())
		return false;

	const QJsonObject &object = jwt.getPayloadJDoc().object();

	if (firstIat > 0 && object.value(QStringLiteral("iat")).toInt() < firstIat)
		return false;

	if (object.value(QStringLiteral("exp")).toInt() <= QDateTime::currentSecsSinceEpoch())
		return false;

	if (object.value(QStringLiteral("iss")).toString() != JWT_ISSUER)
		return false;

	return true;
}



/**
 * @brief Credential::hashString
 * @param str
 * @param salt
 * @param method
 * @return
 */

QString Credential::hashString(const QString &str, const QString &salt, const QCryptographicHash::Algorithm &method)
{
	QByteArray d;
	d.append(str.toUtf8()).append(salt.toUtf8());

	QByteArray hash = QCryptographicHash::hash(d, method);
	return QString::fromLatin1(hash.toHex());
}




/**
 * @brief Credential::hashString
 * @param str
 * @param salt
 * @param method
 * @return
 */

QString Credential::hashString(const QString &str, QString *salt, const QCryptographicHash::Algorithm &method)
{
	QByteArray _salt = Utils::generateRandomString(24);

	if (salt)
		*salt = QString::fromUtf8(_salt);

	return hashString(str, _salt, method);
}





/**
 * @brief Credential::username
 * @return
 */

const QString &Credential::username() const
{
	return m_username;
}

void Credential::setUsername(const QString &newUsername)
{
	m_username = newUsername;
}

const Credential::Roles &Credential::roles() const
{
	return m_roles;
}

void Credential::setRoles(const Roles &newRoles)
{
	m_roles = newRoles;
}


/**
 * @brief Credential::setRole
 * @param role
 * @param on
 */

void Credential::setRole(const Role &role, const bool &on)
{
	m_roles.setFlag(role, on);
}


/**
 * @brief Credential::iat
 * @return
 */

qint64 Credential::iat() const
{
	return m_iat;
}
