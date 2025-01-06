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
#include "utils_.h"
#include <sodium.h>


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

QByteArray Credential::createJWT(const QByteArray &secret) const
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
	if (m_roles.testFlag(SNI))
		list.append(QStringLiteral("sni"));

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


	QJsonObject obj;

	obj.insert(QStringLiteral("iss"), JWT_ISSUER);
	obj.insert(QStringLiteral("sub"), m_username);
	obj.insert(QStringLiteral("iat"), QDateTime::currentSecsSinceEpoch());
	obj.insert(QStringLiteral("exp"), exp.toSecsSinceEpoch());
	obj.insert(QStringLiteral("roles"), list.join("|"));

	Token jwt;

	jwt.setSecret(secret);
	jwt.setPayload(obj);

	LOG_CTRACE("credential") << "Token created for user:" << m_username;

	return jwt.getToken();
}





/**
 * @brief Credential::fromJWT
 * @param jwt
 * @param secret
 * @return
 */

Credential Credential::fromJWT(const QByteArray &jwt)
{
	Token token(jwt);

	if (token.payload().isEmpty()) {
		LOG_CWARNING("credential") << "Invalid token:" << jwt;
		return Credential();
	}

	const QJsonObject &obj = token.payload();

	Credential c;

	c.setUsername(obj.value(QStringLiteral("sub")).toString());

	const QString &r = obj.value(QStringLiteral("roles")).toString();

	c.m_iat = obj.value(QStringLiteral("iat")).toInteger();

	Roles roles;

	foreach (const QString &s, r.split('|')) {
		if (s == QStringLiteral("student"))
			roles.setFlag(Student);
		else if (s == QStringLiteral("teacher"))
			roles.setFlag(Teacher);
		else if (s == QStringLiteral("panel"))
			roles.setFlag(Panel);
		else if (s == QStringLiteral("admin"))
			roles.setFlag(Admin);
		else if (s == QStringLiteral("sni"))
			roles.setFlag(SNI);
	}

	c.setRoles(roles);

	return c;
}




/**
 * @brief Credential::verify
 * @param token
 * @return
 */

bool Credential::verify(const QByteArray &token, const QByteArray &secret, const qint64 &firstIat)
{
	Token jwt(token);

	if (!jwt.verify(secret))
			return false;

	const QJsonObject &object = jwt.payload();

	if (firstIat > 0 && object.value(QStringLiteral("iat")).toInteger() < firstIat)
		return false;

	if (object.value(QStringLiteral("exp")).toInteger() <= QDateTime::currentSecsSinceEpoch())
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





/**
 * @brief Token::generateSignature
 * @param header
 * @param payload
 * @param secret
 * @return
 */

Token::Token(const QByteArray &token)
	: Token()
{
	QByteArrayList listJwtParts = token.split('.');

	if (listJwtParts.count() != 3) {
		return;
	}

	QJsonParseError error;

	QJsonDocument hDoc = QJsonDocument::fromJson(QByteArray::fromBase64(listJwtParts.at(0), QByteArray::Base64UrlEncoding), &error);

	if (error.error != QJsonParseError::NoError)
		return;

	if (hDoc.isEmpty() || hDoc.isNull() || !hDoc.isObject())
		return;

	if (hDoc.object().value(QStringLiteral("alg")).toString() != QStringLiteral("HS512"))
		return;


	QJsonDocument pDoc = QJsonDocument::fromJson(QByteArray::fromBase64(listJwtParts.at(1), QByteArray::Base64UrlEncoding), &error);

	if (error.error != QJsonParseError::NoError)
		return;

	if (pDoc.isEmpty() || pDoc.isNull() || !pDoc.isObject())
		return;

	m_header = hDoc.object();
	m_payload = pDoc.object();
	m_signature = QByteArray::fromBase64(listJwtParts.at(2), QByteArray::Base64UrlEncoding);
	m_origToken = token;
}





/**
 * @brief Token::defaultHeader
 * @return
 */

QByteArray Token::defaultHeader()
{
	return QByteArrayLiteral(R"({"typ": "JWT", "alg": "HS512"})");
}



/**
 * @brief Token::generateSignature
 * @param payload
 * @param secret
 * @param header
 * @return
 */

QByteArray Token::generateSignature(const QJsonObject &payload, const QByteArray &secret, const QJsonObject &header)
{
	QByteArray headerBase64 = header.isEmpty() ?
								  defaultHeader().toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals) :
								  QJsonDocument(header).toJson(QJsonDocument::Compact).toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

	QByteArray payloadBase64 = QJsonDocument(payload).toJson(QJsonDocument::Compact).toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

	return generateSignature(headerBase64, payloadBase64, secret);
}


/**
 * @brief Token::generateSignature
 * @param header
 * @param payload
 * @param secret
 * @return
 */

QByteArray Token::generateSignature(const QByteArray &header, const QByteArray &payload, const QByteArray &secret)
{
	const QByteArray data = sign(header + QByteArrayLiteral(".") + payload, secret);

	if (data.isEmpty())
		return {};

	return data;
}


/**
 * @brief Token::getToken
 * @param header
 * @param payload
 * @param signature
 * @return
 */

QByteArray Token::getToken(const QByteArray &header, const QByteArray &payload, const QByteArray &signature)
{
	if (header.isEmpty() || payload.isEmpty() || signature.isEmpty()) {
		LOG_CERROR("utils") << "Invalid token content";
		return {};
	}

	return header + QByteArrayLiteral(".") + payload + QByteArrayLiteral(".") + signature;
}


/**
 * @brief Token::getToken
 * @param payload
 * @param secret
 * @param header
 * @return
 */

QByteArray Token::getToken(const QJsonObject &payload, const QByteArray &secret, const QJsonObject &header)
{
	QByteArray headerBase64 = header.isEmpty() ?
								  defaultHeader().toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals) :
								  QJsonDocument(header).toJson(QJsonDocument::Compact).toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

	QByteArray payloadBase64 = QJsonDocument(payload).toJson(QJsonDocument::Compact).toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

	const QByteArray signature = generateSignature(headerBase64, payloadBase64, secret);
	return getToken(headerBase64, payloadBase64, signature.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
}


/**
 * @brief Token::sign
 * @param content
 * @param secret
 * @return
 */

QByteArray Token::sign(const QByteArray &content, const QByteArray &secret)
{
	if (secret.size() != crypto_auth_KEYBYTES) {
		LOG_CERROR("utils") << "Invalid secret length" << secret.size();
		return {};
	}

	unsigned char mac[crypto_auth_BYTES];

	if (crypto_auth(mac, (unsigned char*) content.constData(), content.size(), (unsigned char*) secret.constData()) != 0) {
		LOG_CERROR("utils") << "crypto_auth error";
		return {};
	}

	return QByteArray((char*) mac, crypto_auth_BYTES);
}


/**
 * @brief Token::verify
 * @param content
 * @param secret
 * @return
 */

bool Token::verify(const QByteArray &content, const QByteArray &mac, const QByteArray &secret)
{
	if (secret.size() != crypto_auth_KEYBYTES) {
		LOG_CERROR("utils") << "Invalid secret length" << secret.size();
		return false;
	}

	if (mac.size() != crypto_auth_BYTES) {
		LOG_CERROR("utils") << "Invalid secret length" << secret.size();
		return false;
	}

	return (crypto_auth_verify((unsigned char*) mac.constData(),
							   (unsigned char*) content.constData(),
							   content.size(),
							   (unsigned char*) secret.constData()) == 0);
}


/**
 * @brief Token::verify
 * @return
 */

bool Token::verify() const
{
	if (m_secret.isEmpty()) {
		LOG_CERROR("utils") << "Missing secret to verify";
		return false;
	}

	return verify(m_secret);
}




/**
 * @brief Token::verify
 * @param secret
 * @return
 */

bool Token::verify(const QByteArray &secret) const
{
	if (m_header.isEmpty() || m_payload.isEmpty() || m_signature.isEmpty())
		return false;

	QByteArray sign;

	if (m_origToken.isEmpty()) {
	sign = generateSignature(m_payload, secret, m_header);
	} else {
		QByteArrayList parts = m_origToken.split('.');
		sign = generateSignature(parts.at(0), parts.at(1), secret);
	}

	if (sign.isEmpty())
		return false;

	return (sign == m_signature);
}



/**
 * @brief Token::getToken
 * @return
 */

QByteArray Token::getToken() const
{
	return getToken(m_payload, m_secret, m_header);
}
