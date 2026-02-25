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

QByteArray Credential::createJWT(const QByteArray &secret, const QByteArray &session, const QByteArray &publicKey) const
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

	if (!session.isEmpty())
		obj.insert(QStringLiteral("sid"), QString::fromLatin1(session));

	if (!publicKey.isEmpty())
		obj.insert(QStringLiteral("pub"), QString::fromLatin1(publicKey.toBase64()));

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


	if (const QString &sid = obj.value(QStringLiteral("sid")).toString(); !sid.isEmpty())
		c.m_session = sid.toLatin1();


	if (const QString &pub = obj.value(QStringLiteral("pub")).toString(); !pub.isEmpty())
		c.m_devicePub = QByteArray::fromBase64(pub.toLatin1());

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

QByteArray Credential::session() const
{
	return m_session;
}

QByteArray Credential::devicePub() const
{
	return m_devicePub;
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
	if (!AuthKeySigner::isValidSecret(secret)) {
		LOG_CERROR("utils") << "Invalid secret length" << secret.size();
		return {};
	}

	return AuthKeySigner(secret).sign(content);
}


/**
 * @brief Token::verify
 * @param content
 * @param secret
 * @return
 */

bool Token::verify(const QByteArray &content, const QByteArray &mac, const QByteArray &secret)
{
	if (!AuthKeySigner::isValidSecret(secret)) {
		LOG_CERROR("utils") << "Invalid secret length" << secret.size();
		return false;
	}

	return AuthKeySigner(secret).verifySign(content, mac);
}



/**
 * @brief Token::generateSecret
 * @return
 */

QByteArray Token::generateSecret()
{
	unsigned char k[crypto_auth_KEYBYTES];
	crypto_auth_keygen(k);

	return QByteArray(reinterpret_cast<const char*>(k), crypto_auth_KEYBYTES);
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



/**
 * @brief AuthKeySigner::sign
 * @param content
 * @return
 */

QByteArray AuthKeySigner::sign(const QByteArray &content) const
{
	QByteArray sig(crypto_auth_BYTES, Qt::Uninitialized);

	if (crypto_auth(reinterpret_cast<unsigned char*>(sig.data()),
					reinterpret_cast<const unsigned char*>(content.constData()), content.size(),
					m_secret.data()) != 0) {
		LOG_CERROR("utils") << "crypto_auth error";
		return {};
	}

	return sig;
}



/**
 * @brief AuthKeySigner::verifySign
 * @param content
 * @param signature
 * @return
 */

bool AuthKeySigner::verifySign(const QByteArray &content, const QByteArray &signature) const
{
	if (!isValidSignature(signature)) {
		LOG_CERROR("utils") << "Invalid mac length" << signature.size();
		return false;
	}

	return (crypto_auth_verify(reinterpret_cast<const unsigned char*>(signature.constData()),
							   reinterpret_cast<const unsigned char*>(content.constData()),
							   content.size(),
							   m_secret.data()) == 0);
}


/**
 * @brief PublicKeySigner::sign
 * @param content
 * @return
 */

void PublicKeySigner::setPublicKey(const QByteArray &publicKey)
{
	if (!isValidPublicKey(publicKey)) {
		LOG_CERROR("utils") << "Invalid public key length" << publicKey.size();
		return;
	}

	std::memcpy(m_publicKey.data(), reinterpret_cast<const unsigned char*>(publicKey.constData()), publicKey.size());
}



/**
 * @brief PublicKeySigner::sign
 * @param content
 * @return
 */


QByteArray PublicKeySigner::sign(const QByteArray &content) const
{
	QByteArray sig(crypto_sign_BYTES, Qt::Uninitialized);

	if (crypto_sign_detached(reinterpret_cast<unsigned char*>(sig.data()),
							 nullptr,
							 reinterpret_cast<const unsigned char*>(content.constData()),
							 (unsigned long long) content.size(),
							 m_secret.data()) != 0) {
		LOG_CERROR("utils") << "crypto_sign_detached error";
		return {};
	}

	return sig;
}



/**
 * @brief PublicKeySigner::verifySign
 * @param content
 * @param signature
 * @return
 */

bool PublicKeySigner::verifySign(const QByteArray &content, const QByteArray &signature) const
{
	if (!isValidSignature(signature)) {
		LOG_CERROR("utils") << "Invalid mac length" << signature.size();
		return false;
	}

	return (crypto_sign_verify_detached(reinterpret_cast<const unsigned char*>(signature.constData()),
									reinterpret_cast<const unsigned char*>(content.constData()),
									content.size(),
									m_publicKey.data()) == 0);
}



/**
 * @brief PublicKeySigner::verifySign
 * @param content
 * @param signature
 * @return
 */

bool PublicKeySigner::verifySign(const std::vector<unsigned char> &content, const QByteArray &signature) const
{
	if (!isValidSignature(signature)) {
		LOG_CERROR("utils") << "Invalid mac length" << signature.size();
		return false;
	}

	return (crypto_sign_verify_detached(reinterpret_cast<const unsigned char*>(signature.constData()),
									content.data(),
									content.size(),
									m_publicKey.data()) == 0);
}
