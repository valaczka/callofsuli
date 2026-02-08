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

#include <QJsonObject>
#include <QSerializer>
#include <QCborArray>
#include <sodium.h>

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

	static QByteArray generateSecret();


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






/**
 * @brief The Credential class
 */

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

	QByteArray createJWT(const QByteArray &secret, const QByteArray &session, const QByteArray &publicKey) const;
	static Credential fromJWT(const QByteArray &jwt);

	static bool verify(const QByteArray &token, const QByteArray &secret, const qint64 &firstIat);

	virtual bool isValid() const;

	const QString &username() const;
	void setUsername(const QString &newUsername);

	const Roles &roles() const;
	void setRoles(const Roles &newRoles);
	void setRole(const Role &role, const bool &on = true);

	qint64 iat() const;

	QByteArray session() const;
	QByteArray devicePub() const;

	inline bool operator==(const Credential &other) {
		return other.m_username == m_username &&
				other.m_roles == m_roles;
	}


private:
	QString m_username;
	Roles m_roles = None;
	qint64 m_iat = 0;
	QByteArray m_session;
	QByteArray m_devicePub;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Credential::Roles);








/**
 * @brief The AbstractSigner class
 */

template <std::size_t N>
class AbstractSigner
{
public:
	AbstractSigner() {
		sodium_memzero(m_secret.data(), m_secret.size());
	}

	AbstractSigner(const std::array<unsigned char, N> &secret)
		: AbstractSigner() {
		setSecret(secret);
	}

	AbstractSigner(const QByteArray &secret)
		: AbstractSigner() {
		setSecret(secret);
	}

	virtual ~AbstractSigner() {
		sodium_memzero(m_secret.data(), m_secret.size());
	}

	static bool isValidSecret(const QByteArray &secret) { return secret.size() == N; }

	std::array<unsigned char, N> secret() const { return m_secret; }
	void setSecret(const std::array<unsigned char, N> &secret) { m_secret = secret; }
	void setSecret(const QByteArray &secret);

	QCborMap signToMap(const QByteArray &content) const;
	QByteArray signToRaw(const QByteArray &content) const;

	template <typename T,
			  typename = std::enable_if<std::is_base_of<QSerializer, T>::value>::type>
	QByteArray signToRaw(const T &content) const {
		return signToRaw(content.toCborMap().toCborValue().toCbor());
	}

	template <typename T,
			  typename = std::enable_if<std::is_base_of<QSerializer, T>::value>::type>
	QByteArray signToRaw(const std::vector<T> &content) const {
		QCborArray a;

		for (const T &c : content)
			a.append(c.toCborMap());

		return signToRaw(a.toCborValue().toCbor());
	}


	std::optional<QByteArray> verify(const QByteArray &data) const;
	std::optional<QByteArray> verify(const QCborMap &data) const;


	template <typename T,
			  typename = std::enable_if<std::is_base_of<QSerializer, T>::value>::type>
	std::optional<T> verifyTo(const QByteArray &content) const {
		if (const std::optional<QByteArray> &ptr = verify(content)) {
			const QCborValue v = QCborValue::fromCbor(*ptr);
			T r;
			r.fromCbor(v);
			return r;
		} else {
			return std::nullopt;
		}
	}


	template <typename T,
			  typename = std::enable_if<std::is_base_of<QSerializer, T>::value>::type>
	std::optional<std::vector<T>> verifyToList(const QByteArray &content) const {
		if (std::optional<QByteArray> &ptr = verify(content)) {
			const QCborArray a = QCborValue::fromCbor(*ptr).toArray();
			std::vector<T> list;
			list.reserve(a.size());

			for (const QCborValue &v : a) {
				T r;
				r.fromCbor(v);
				list.push_back(std::move(r));
			}
			return list;
		} else {
			return std::nullopt;
		}
	}

	virtual QByteArray sign(const QByteArray &content) const = 0;
	virtual bool verifySign(const QByteArray &content, const QByteArray &signature) const = 0;
	virtual bool isValidSignature(const QByteArray &secret) const = 0;

protected:
	std::array<unsigned char, N> m_secret;
};




/**
 * @brief AbstractSigner::setSecret
 * @param secret
 */

template<std::size_t N>
inline void AbstractSigner<N>::setSecret(const QByteArray &secret)
{
	if (secret.size() != N) {
		qWarning() << "Invalid secret size" << secret.size();
		return;
	}

	std::memcpy(m_secret.data(), reinterpret_cast<const unsigned char*>(secret.constData()), N);
}


/**
 * @brief AbstractSigner::signToMap
 * @param content
 * @return
 */

template<std::size_t N>
inline QCborMap AbstractSigner<N>::signToMap(const QByteArray &content) const
{
	QCborMap m;
	m[QStringLiteral("content")] = content;
	m[QStringLiteral("sig")] = sign(content);
	return m;
}


/**
 * @brief AbstractSigner::signToRaw
 * @param content
 * @return
 */

template<std::size_t N>
inline QByteArray AbstractSigner<N>::signToRaw(const QByteArray &content) const
{
	return signToMap(content).toCborValue().toCbor();
}


/**
 * @brief AbstractSigner::verify
 * @param data
 * @return
 */

template<std::size_t N>
inline std::optional<QByteArray> AbstractSigner<N>::verify(const QByteArray &data) const
{
	const QCborMap m = QCborValue::fromCbor(data).toMap();

	if (m.isEmpty())
		return std::nullopt;

	return verify(m);
}


/**
 * @brief AbstractSigner::verify
 * @param data
 * @return
 */

template<std::size_t N>
inline std::optional<QByteArray> AbstractSigner<N>::verify(const QCborMap &data) const
{
	const QByteArray content = data.value(QStringLiteral("content")).toByteArray();

	if (!verifySign(content, data.value(QStringLiteral("sig")).toByteArray()))
		return std::nullopt;

	return content;
}




/**
 * @brief The AuthKeySigner class
 */

class AuthKeySigner : public AbstractSigner<crypto_auth_KEYBYTES>
{
public:
	AuthKeySigner() : AbstractSigner() {}
	AuthKeySigner(const std::array<unsigned char, crypto_auth_KEYBYTES> &secret) : AbstractSigner(secret) {}
	AuthKeySigner(const QByteArray &secret) : AbstractSigner(secret) {}

	virtual QByteArray sign(const QByteArray &content) const override;
	virtual bool verifySign(const QByteArray &content, const QByteArray &signature) const override;
	virtual bool isValidSignature(const QByteArray &secret) const override { return secret.size() == crypto_auth_BYTES; }

};






/**
 * @brief The PublicKeySigner class
 */

class PublicKeySigner : public AbstractSigner<crypto_sign_SECRETKEYBYTES>
{
public:
	PublicKeySigner() : AbstractSigner() {}
	PublicKeySigner(const std::array<unsigned char, crypto_sign_SECRETKEYBYTES> &secret) : AbstractSigner(secret) {}
	PublicKeySigner(const QByteArray &secret) : AbstractSigner(secret) {}

	const std::array<unsigned char, crypto_sign_PUBLICKEYBYTES> &publicKey() const { return m_publicKey; };
	void setPublicKey(const std::array<unsigned char, crypto_sign_PUBLICKEYBYTES> &newPublicKey) { m_publicKey = newPublicKey; }
	void setPublicKey(const QByteArray &publicKey);

	virtual QByteArray sign(const QByteArray &content) const override;
	virtual bool verifySign(const QByteArray &content, const QByteArray &signature) const override;
	bool verifySign(const std::vector<unsigned char> &content, const QByteArray &signature) const;
	virtual bool isValidSignature(const QByteArray &secret) const override { return secret.size() == crypto_sign_BYTES; }

	static bool isValidPublicKey(const QByteArray &publicKey) { return publicKey.size() == crypto_sign_PUBLICKEYBYTES; }

private:
	std::array<unsigned char, crypto_sign_PUBLICKEYBYTES> m_publicKey;
};




/**
 * @brief The UdpToken class
 */

class UdpToken : public QSerializer
{
	Q_GADGET

public:
	enum Type {
		Invalid,
		Rpg
	};

	UdpToken(const Type &_type = Invalid, const QString &_user = {}, const quint32 &_peer = 0, const qint64 &_exp = 0)
		: QSerializer()
		, type(_type)
		, username(_user)
		, peerID(_peer)
		, exp(_exp)
	{}

	QS_SERIALIZABLE

	QS_FIELD(Type, type)
	QS_FIELD(QString, username)
	QS_FIELD(quint32, peerID)
	QS_FIELD(qint64, exp)
};





/**
 * @brief The UdpConnectRequest class
 */

class UdpConnectRequest : public QSerializer
{
	Q_GADGET

public:
	UdpConnectRequest(const QByteArray &_token = {})
		: QSerializer()
		, token(_token)
	{}

	QS_SERIALIZABLE

	QS_BYTEARRAY(token)				// connection token
};




/**
 * @brief The UdpConnectResponse class
 */

class UdpServerResponse : public QSerializer
{
	Q_GADGET

public:
	enum State {
		StateInvalid,
		StateConnected,
		StateRejected,
		StateChallenge,
		StateFull
	};

	Q_ENUM(State)

	UdpServerResponse(const State &_state = StateInvalid)
		: QSerializer()
		, state(_state)
	{}

	QS_SERIALIZABLE

	QS_FIELD(State, state)
};






/**
 * @brief The UdpConnectRequest class
 */

class UdpChallengeRequest : public UdpServerResponse
{
	Q_GADGET

public:
	UdpChallengeRequest(const QByteArray &_challenge = {}, const QByteArray &_key = {})
		: UdpServerResponse(StateChallenge)
		, challenge(_challenge)
		, key(_key)
	{}

	QS_SERIALIZABLE

	QS_BYTEARRAY(challenge)			// server challenge
	QS_BYTEARRAY(key)				// server public key
};









/**
 * @brief The UdpChallengeResponse class
 */

class UdpChallengeResponse : public QSerializer
{
	Q_GADGET

public:
	UdpChallengeResponse(const QByteArray &_response = {})
		: QSerializer()
		, response(_response)
	{}

	QS_SERIALIZABLE

	QS_BYTEARRAY(response)			// UpdChallengeResponseContent encrypted with server public key
	QS_BYTEARRAY(token)				// connection token
};





/**
 * @brief The UdpChallengeResponseContent class
 */

class UdpChallengeResponseContent : public QSerializer
{
	Q_GADGET

public:
	UdpChallengeResponseContent()
		: QSerializer()
	{}

	QS_SERIALIZABLE

	QS_BYTEARRAY(challenge)			// server challenge
	QS_BYTEARRAY(key)				// client secret key
};



#endif // CREDENTIAL_H
