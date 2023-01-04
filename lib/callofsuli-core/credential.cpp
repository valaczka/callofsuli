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
#include <jwt-cpp/jwt.h>
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
{

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

	QDateTime exp = QDateTime::currentDateTime();

	if (m_roles.testFlag(Admin))
		exp = exp.addDays(1);
	else if (m_roles.testFlag(Teacher))
		exp = exp.addDays(5);
	else if (m_roles.testFlag(Student))
		exp = exp.addDays(10);
	else if (m_roles.testFlag(Panel))
		exp = exp.addDays(20);
	else
		exp = exp.addDays(30);

	auto token = jwt::create()
			.set_issuer(JWT_ISSUER)
			.set_type("JWS")
			.set_subject(m_username.toStdString())
			.set_issued_at(jwt::date::clock::now())
			.set_payload_claim("exp", picojson::value(int64_t(exp.toSecsSinceEpoch())))
			.set_payload_claim("roles", picojson::value(list.join("|").toStdString()))
			.sign(jwt::algorithm::hs256{secret.toStdString()});

	LOG_CINFO("credential") << "Token created for user:" << m_username;

	return QString::fromStdString(token);
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
 * @brief Credential::fromJWT
 * @param jwt
 * @param secret
 * @return
 */

Credential Credential::fromJWT(const QString &jwt)
{
	Credential c;

	try {
		auto decoded = jwt::decode(jwt.toStdString());

		c.setUsername(QString::fromStdString(decoded.get_subject()));

		const QString &r = QString::fromStdString(decoded.get_payload_claim("roles").as_string());

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

	} catch (...) {
		LOG_CWARNING("credential") << "Invalid token:" << jwt;
	}

	return c;
}




/**
 * @brief Credential::verify
 * @param token
 * @return
 */

bool Credential::verify(const QString &token, JwtVerifier *verifier)
{
	bool ret = false;

	try {
		auto decoded = jwt::decode(token.toStdString());
		verifier->verify(decoded);
		ret = true;

	} catch (const std::exception& ex) {
		LOG_CWARNING("credential") << "Invalid token: " << ex.what();
		ret = false;
	}

	return ret;
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
	QByteArray _salt = Utils::generateRandomString(12);

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
