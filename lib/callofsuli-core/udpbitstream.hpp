/*
 * ---- Call of Suli ----
 *
 * UdpBitStream.hpp
 *
 * Created on: 2025. 12. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#ifndef UDPBITSTREAM_H
#define UDPBITSTREAM_H

#include "Logger.h"
#include "qassert.h"
#include "sodium/crypto_auth.h"
#include "sodium/crypto_box.h"
#include <BMLib/BinaryStream.hpp>
#include <QSerializer>

#ifndef Q_OS_WASM
#include <enet/enet.h>
#endif


#define PEER_INDEX_BITS			8
#define CHALLENGE_BYTES			32



/* ---------------------------------------------------------

Payload (bit-packed):

	repeat n = 0..m_magic.size():
		[8 bit] m_magic[n]

	[8 bit] messageType (0x00..0xFF)

	... (message)

	[PEER_INDEX_BITS bit] peerIndex (0x00..0xFF)

------------------------------------------------------------ */


static constexpr quint32 _pow(quint32 x, size_t y) {
	return y ? x * _pow(x, y-1) : 1;
}


/**
 * @brief The UdpBitStream class
 */

class UdpBitStream
{
public:
	enum MessageType {
		MessageInvalid		= 0x00,
		MessageConnect		= 0x01,
		MessageChallenge	= 0x02,
		MessageServerFull	= 0x03,
		MessageConnected	= 0x04,
		MessageRejected		= 0x05,

		MessageUser = 0x10,					// MessageUser + 0, MessageUser + 1,...

		MessageMax = 0xFF
	};

	UdpBitStream(const UdpBitStream& temp_obj) = delete;
	UdpBitStream& operator=(const UdpBitStream& temp_obj) = delete;
	UdpBitStream(UdpBitStream&&) noexcept = default;
	UdpBitStream& operator=(UdpBitStream&& temp_obj) noexcept = default;

	UdpBitStream(const std::uint8_t &type = MessageInvalid);
	UdpBitStream(const std::vector<unsigned char> &buffer);
	UdpBitStream(std::uint8_t *buffer, const std::size_t &size, const bool &auto_realloc_enabled = false);

	virtual ~UdpBitStream();

#ifndef Q_OS_WASM
	UdpBitStream(const ENetEvent &event);
#endif


	// MessageConnect

	UdpBitStream(const QByteArray &connectToken);

	// MessageChallenge

	UdpBitStream(const std::array<std::uint8_t, CHALLENGE_BYTES> &challenge,
				 const std::array<std::uint8_t, crypto_box_PUBLICKEYBYTES> &publicKey);

	// MessageChallenge (response)

	UdpBitStream(const QByteArray &connectToken, const QByteArray &encryptedData);
	UdpBitStream(const QByteArray &connectToken, const std::vector<std::uint8_t> &encryptedData);

	// MessageConnected

	UdpBitStream(const quint32 &peerId, const quint32 &peerIndex);



	/**
	 * @brief The BinaryStream class
	 */

	class UdpBinaryStream : public BMLib::BinaryStream
	{
	public:
		explicit UdpBinaryStream(BMLib::Buffer *buffer, std::size_t position) :
			BMLib::BinaryStream(buffer, position)
		{}


		bool readBit(bool skip)
		{
			if (this->curr_bit_read_pos == 0 || this->curr_bit_read_pos == 8 || skip) {
				this->curr_read_octet = this->readSingle();
				this->curr_bit_read_pos = 0;
			}
			std::uint8_t bit_value = this->curr_read_octet;

			bit_value >>= (7 - this->curr_bit_read_pos);

			++this->curr_bit_read_pos;
			return (bit_value & 0b1) == 1;
		}

		template <typename T>
		T readBits(std::size_t size)
		{
			T result = 0;
			for (std::size_t i = 0; i < size; ++i)
				result |= static_cast<std::uint8_t>(this->readBit(false)) << (true ? (size - i - 1) : i);
			return result;
		}

	private:
		template <typename T>
		T readBits(std::size_t size, bool msb_o) = delete;
		bool readBit(bool skip, bool msb_o) = delete;


		friend class UdpBitStream;
	};



	/// Member functions

	virtual std::vector<std::uint8_t> data() const;
	std::vector<std::uint8_t> operator*() const { return data(); }

	bool validate();
	const std::uint8_t &type() const { return m_type; }

	const UdpBinaryStream &stream() const { return m_stream; }
	UdpBinaryStream &stream() { return m_stream; }

	static constexpr quint32 peerCapacity() { return m_peerCapacity; }

	std::optional<UdpBitStream> getRemainingData() const;


	// Crypto Auth

	void setAuthLastPosition(const std::size_t &pos);
	bool authBuffer(const std::array<std::uint8_t, crypto_auth_KEYBYTES> &secret) const;
	std::optional<std::size_t> verifyBuffer(const std::array<std::uint8_t, crypto_auth_KEYBYTES> &secret) const;


	// MessageConnect

	std::optional<QByteArray> readByteArray(const bool &bitAligned = false) const;
	void writeByteArray(const QByteArray &data, const bool &bitAligned = false);

	std::optional<std::vector<std::uint8_t>> readVector(const bool &bitAligned = false) const;
	void writeVector(const std::vector<std::uint8_t> &data, const bool &bitAligned = false);


	// MessageChallenge

	bool getChallenge(std::array<std::uint8_t, CHALLENGE_BYTES> *challengePtr,
					  std::array<std::uint8_t, crypto_box_PUBLICKEYBYTES> *publicKeyPtr);

	std::optional<std::array<std::uint8_t, CHALLENGE_BYTES> > readChallenge() const;
	void writeChallenge(const std::array<std::uint8_t, CHALLENGE_BYTES> &challenge);

	std::optional<std::array<std::uint8_t, crypto_box_PUBLICKEYBYTES> > readPublicKey() const;
	void writePublicKey(const std::array<std::uint8_t, crypto_box_PUBLICKEYBYTES> &key);

	std::optional<std::array<std::uint8_t, crypto_auth_KEYBYTES> > readAuthKey() const;
	void writeAuthKey(const std::array<std::uint8_t, crypto_auth_KEYBYTES> &key);

	// MessageChallenge (response)

	bool getChallengeResponse(QByteArray *tokenPtr, QByteArray *encryptedDataPtr);
	bool getChallengeResponse(QByteArray *tokenPtr, std::vector<std::uint8_t> *encryptedDataPtr);

	// MessageConnected

	bool getConnected(quint32 *peerIdPtr, quint32 *peerIndexPtr);

	// MessageUser

	std::optional<quint32> readPeerIndex() const;
	void writePeerIndex(const quint32 &idx);


	friend QDebug operator<<(QDebug debug, const UdpBitStream &c) {
		QDebugStateSaver saver(debug);

		if (!c.m_stream.getBuffer()) {
			debug.nospace().noquote() << QStringLiteral("UdpBitStream()");
			return debug;
		}

		debug.nospace().noquote() << QStringLiteral("\nUdpBitStream(") << c.m_stream.getBuffer()->size
								  << ',' << c.m_stream.position
								  << QStringLiteral(" | r")
								  << c.m_stream.curr_read_octet << '/' << c.m_stream.curr_bit_read_pos
								  << QStringLiteral(" | w")
								  << c.m_stream.curr_write_octet << '/' << c.m_stream.curr_bit_write_pos
								  << QStringLiteral(")\n");

		std::uint8_t *ptr = c.m_stream.getBuffer()->binary;

		for (size_t i=0; i<c.m_stream.getBuffer()->size; ++i) {
			debug.nospace().noquote() << QStringLiteral("%1 ").arg(*ptr, 2, 16, QChar('0'));

			if (i % 16 == 15)
				debug.nospace() << '\n';
			else if (i % 4 == 3)
				debug.nospace() << ' ';

			++ptr;
		}

		debug.nospace() << '\n';

		return debug;
	};

protected:
	mutable UdpBinaryStream m_stream;

	std::uint8_t m_type = MessageInvalid;

	static constexpr std::array<uint8_t, 3> m_magic = { 0x43, 0x4F, 0x53 };		// "COS"
	static constexpr quint32 m_peerCapacity = _pow(2, PEER_INDEX_BITS);

	// Crypt Auth

	std::size_t m_authLastPos = 0;
};




/**
 * @brief The UdpChallengeResponseStream class
 */

class UdpChallengeResponseStream : public UdpBitStream
{
public:
	UdpChallengeResponseStream(const std::vector<unsigned char> &buffer);
	UdpChallengeResponseStream(const std::array<std::uint8_t, CHALLENGE_BYTES> &challenge,
							   const std::array<std::uint8_t, crypto_auth_KEYBYTES> &authKey);

	bool getResponse(std::array<std::uint8_t, CHALLENGE_BYTES> *challengePtr,
					 std::array<std::uint8_t, crypto_auth_KEYBYTES> *authKeyPtr) const;
};



/**
 * @brief UdpChallengeResponseStream::UdpChallengeResponseStream
 * @param buffer
 */

inline UdpChallengeResponseStream::UdpChallengeResponseStream(const std::vector<unsigned char> &buffer)
	: UdpBitStream(buffer)
{

}


/**
 * @brief UdpChallengeResponseStream::UdpChallengeResponseStream
 * @param challenge
 * @param publicKey
 */

inline UdpChallengeResponseStream::UdpChallengeResponseStream(const std::array<uint8_t, CHALLENGE_BYTES> &challenge,
															  const std::array<uint8_t, crypto_auth_KEYBYTES> &authKey)
	: UdpBitStream()
{
	this->writeChallenge(challenge);
	this->writeAuthKey(authKey);
}


/**
 * @brief UdpChallengeResponseStream::getResponse
 * @param challengePtr
 * @param authKeyPtr
 * @return
 */

inline bool UdpChallengeResponseStream::getResponse(std::array<uint8_t, CHALLENGE_BYTES> *challengePtr,
													std::array<uint8_t, crypto_auth_KEYBYTES> *authKeyPtr) const
{
	auto ptrCh = readChallenge();

	if (!ptrCh)
		return false;

	auto ptrKey = readAuthKey();

	if (!ptrKey)
		return false;

	if (challengePtr)
		*challengePtr = std::move(*ptrCh);

	if (authKeyPtr)
		*authKeyPtr = std::move(*ptrKey);

	return true;
}





/**
 * @brief The UdpConnectionToken class
 */

class UdpConnectionToken : public QSerializer
{
	Q_GADGET

public:
	UdpConnectionToken(const quint32 &_type = 0, const QString &_user = QString(), const quint32 &_peer = 0, const qint64 &_exp = 0)
		: QSerializer()
		, type(_type)
		, user(_user)
		, peer(_peer)
		, exp(_exp)
	{}

	QS_SERIALIZABLE

	QS_FIELD(quint32, type)
	QS_FIELD(QString, user)
	QS_FIELD(quint32, peer)
	QS_FIELD(qint64, exp)
};




/// ------------------------------------------------------------------------------------------- ///



inline UdpBitStream::UdpBitStream(const std::uint8_t &type)
	: m_stream(BMLib::Buffer::allocate(true, 1), 0)
{
	Q_ASSERT(type >= MessageInvalid && type <= MessageMax);

	m_stream.resetBitWriter();

	/// Workaround for BMLib::Buffer

	if (type != MessageInvalid) {
		m_stream.getBuffer()->binary[0] = m_magic[0];
		m_stream.getBuffer()->position = 1;

		for (size_t i=1; i<m_magic.size(); ++i)
			m_stream.write<std::uint8_t>(m_magic[i], true);

		m_stream.write<std::uint8_t>(type, true);
	} else {
		m_stream.getBuffer()->binary[0] = 0;
	}
}



/**
 * @brief UdpBitStream::UdpBitStream
 * @param event
 */

#ifndef Q_OS_WASM

inline UdpBitStream::UdpBitStream(const ENetEvent &event)
	: m_stream(nullptr, 0)
{
	if (event.packet && event.packet->dataLength > 0) {
		BMLib::Buffer *buf = BMLib::Buffer::allocate(false, event.packet->dataLength);

		std::memcpy(buf->binary, event.packet->data, event.packet->dataLength);
		buf->position = 0;

		m_stream.setBuffer(buf);
		m_stream.resetBitReader();
		m_authLastPos = buf->size;
	} else {
		LOG_CERROR("engine") << "Empty ENetEvent packet";
	}
}

#endif


/**
 * @brief UdpBitStream::UdpBitStream
 * @param buffer
 */

inline UdpBitStream::UdpBitStream(const std::vector<unsigned char> &buffer)
	: m_stream(nullptr, 0)
{
	BMLib::Buffer *buf = BMLib::Buffer::allocate(false, buffer.size());

	std::memcpy(buf->binary, buffer.data(), buffer.size());
	buf->position = 0;

	m_stream.setBuffer(buf);
	m_stream.resetBitReader();
	m_authLastPos = buf->size;
}




/**
 * @brief UdpBitStream::UdpBitStream
 * @param buffer
 * @param size
 */

inline UdpBitStream::UdpBitStream(uint8_t *buffer, const std::size_t &size, const bool &auto_realloc_enabled)
	: m_stream(BMLib::Buffer::allocate(auto_realloc_enabled, size), 0)
{
	BMLib::Buffer *buf = m_stream.getBuffer();

	std::memcpy(buf->binary, buffer, size);
	buf->position = 0;

	m_stream.resetBitReader();
	m_authLastPos = buf->size;
}


/**
 * @brief UdpBitStream::~UdpBitStream
 */

inline UdpBitStream::~UdpBitStream()
{
	LOG_CDEBUG("engine") << "DESTROY" << this;
}




/**
 * @brief UdpBitStream::UdpBitStream
 * @param connectToken
 */

inline UdpBitStream::UdpBitStream(const QByteArray &connectToken)
	: UdpBitStream(MessageConnect)
{
	this->writeByteArray(connectToken);
}


/**
 * @brief UdpBitStream::UdpBitStream
 * @param connectToken
 * @param encryptedData
 */

inline UdpBitStream::UdpBitStream(const QByteArray &connectToken, const QByteArray &encryptedData)
	: UdpBitStream(MessageChallenge)
{
	this->writeByteArray(connectToken);
	this->writeByteArray(encryptedData);
}


/**
 * @brief UdpBitStream::UdpBitStream
 * @param connectToken
 * @param encryptedData
 */

inline UdpBitStream::UdpBitStream(const QByteArray &connectToken, const std::vector<uint8_t> &encryptedData)
	: UdpBitStream(MessageChallenge)
{
	this->writeByteArray(connectToken);
	this->writeVector(encryptedData);
}

/**
 * @brief UdpBitStream::UdpBitStream
 * @param peerId
 * @param peerIndex
 */

inline UdpBitStream::UdpBitStream(const quint32 &peerId, const quint32 &peerIndex)
	: UdpBitStream(MessageConnected)
{
	m_stream.write<quint32>(peerId, true);
	m_stream.write<quint32>(peerIndex, true);
}


/**
 * @brief UdpBitStream::UdpBitStream
 * @param challenge
 * @param publicKey
 */

inline UdpBitStream::UdpBitStream(const std::array<uint8_t, CHALLENGE_BYTES> &challenge,
								  const std::array<uint8_t, crypto_box_PUBLICKEYBYTES> &publicKey)
	: UdpBitStream(MessageChallenge)
{
	this->writeChallenge(challenge);
	this->writePublicKey(publicKey);
}


/**
 * @brief UdpBitStream::data
 * @return
 */

inline std::vector<std::uint8_t> UdpBitStream::data() const
{
	std::vector<std::uint8_t> data;

	if (BMLib::Buffer *buffer = m_stream.getBuffer()) {
		data.assign(buffer->binary, buffer->binary + buffer->size);
	}

	return data;
}




/**
 * @brief UdpBitStream::validate
 * @return
 */

inline bool UdpBitStream::validate()
{
	if (!m_stream.getBuffer()) {
		m_type = MessageInvalid;
		return false;
	}

	if (m_stream.getNumOfBytesRead() > 0)
		return m_type != MessageInvalid;

	try {
		for (size_t i=0; i<m_magic.size(); ++i) {
			if (m_stream.read<std::uint8_t>(true) != m_magic[i]) {
				LOG_CWARNING("engine") << "Magic number failed";
				m_type = MessageInvalid;
				return false;
			}
		}

		m_type = m_stream.read<std::uint8_t>(true);

	} catch (const std::out_of_range &err) {
		LOG_CWARNING("engine") << "Out of range" << err.what();
	} catch (const std::exception &err) {
		LOG_CWARNING("engine") << "Exception" << err.what();
		return false;
	}

	return true;
}




/**
 * @brief UdpBitStream::writePeerIndex
 * @param idx
 */

inline void UdpBitStream::writePeerIndex(const quint32 &idx)
{
	m_stream.writeBits<quint32>(idx, PEER_INDEX_BITS, true);
}



/**
 * @brief UdpBitStream::readPeerIndex
 * @return
 */

inline std::optional<quint32> UdpBitStream::readPeerIndex() const
{
	try {
		quint32 idx = m_stream.readBits<quint32>(PEER_INDEX_BITS);

		return idx;
	} catch (const std::out_of_range &err) {
		LOG_CWARNING("engine") << "Out of range" << err.what();
		return std::nullopt;
	} catch (const std::exception &err) {
		LOG_CWARNING("engine") << "Exception" << err.what();
		return std::nullopt;
	}
}




/**
 * @brief UdpBitStream::getRemainingData
 * @param lenPtr
 * @return
 */

inline std::optional<UdpBitStream> UdpBitStream::getRemainingData() const
{
	BMLib::Buffer *buffer = m_stream.getBuffer();

	if (!buffer) {
		LOG_CERROR("engine") << "Missing buffer";
		return std::nullopt;
	}

	std::size_t len = buffer->size - buffer->position;

	if (m_authLastPos > 0) {
		if (buffer->position >= m_authLastPos) {
			LOG_CERROR("engine") << "Buffer overrun";
			return std::nullopt;
		}

		len = m_authLastPos - buffer->position;
	}

	return std::optional<UdpBitStream>(std::in_place, buffer->binary + buffer->position, len);
}




/**
 * @brief UdpBitStream::setAuthLastPosition
 * @param pos
 */

inline void UdpBitStream::setAuthLastPosition(const std::size_t &pos)
{
	m_authLastPos = pos;
}



/**
 * @brief UdpBitStream::verifyBuffer
 * @param secret
 * @return
 */

inline std::optional<std::size_t> UdpBitStream::verifyBuffer(const std::array<uint8_t, crypto_auth_KEYBYTES> &secret) const
{
	BMLib::Buffer *buffer = m_stream.getBuffer();

	if (!buffer) {
		LOG_CERROR("engine") << "Missing buffer";
		return std::nullopt;
	}


	if (crypto_auth_BYTES >= buffer->size) {
		LOG_CERROR("engine") << "Crypto auth buffer size error" << buffer->size;
		return std::nullopt;
	}

	std::size_t macStart = buffer->size - crypto_auth_BYTES;

	if (crypto_auth_verify(buffer->binary + macStart,
						   buffer->binary,
						   macStart,
						   secret.data()) == 0) {
		return macStart;
	}

	return std::nullopt;
}


/**
 * @brief UdpBitStream::encryptBuffer
 */

inline bool UdpBitStream::authBuffer(const std::array<uint8_t, crypto_auth_KEYBYTES> &secret) const
{
	BMLib::Buffer *buffer = m_stream.getBuffer();

	if (!buffer) {
		LOG_CERROR("engine") << "Missing buffer";
		return false;
	}

	std::array<std::uint8_t, crypto_auth_BYTES> mac;

	if (crypto_auth(mac.data(),
					buffer->binary,
					buffer->size,
					secret.data()
					) != 0) {
		LOG_CERROR("utils") << "crypto_auth error";
		return false;
	}

	for (const std::uint8_t &b : std::as_const(mac))
		m_stream.write<std::uint8_t>(b, true);

	return true;
}




/**
 * @brief UdpBitStream::readByteArray
 * @return
 */

inline std::optional<QByteArray> UdpBitStream::readByteArray(const bool &bitAligned) const
{
	try {
		QByteArray data;

		quint32 size = bitAligned ?
						   m_stream.readBits<quint32>(32) :
						   m_stream.read<quint32>(true);

		data.reserve(size);

		for (size_t i=0; i<size; ++i)
			data.append(bitAligned ?
							m_stream.readBits<char>(8) :
							m_stream.read<char>(true)
							);

		return data;

	} catch (const std::out_of_range &err) {
		LOG_CWARNING("engine") << "Out of range" << err.what();
		return std::nullopt;
	} catch (const std::exception &err) {
		LOG_CWARNING("engine") << "Exception" << err.what();
		return std::nullopt;
	}
}



/**
 * @brief UdpBitStream::writeByteArray
 * @param token
 */

inline void UdpBitStream::writeByteArray(const QByteArray &data, const bool &bitAligned)
{
	if (bitAligned)
		m_stream.writeBits<quint32>(data.size(), 32, true);
	else
		m_stream.write<quint32>(data.size(), true);

	for (const char &b : data) {
		if (bitAligned)
			m_stream.writeBits<char>(b, 8, true);
		else
			m_stream.write<char>(b, true);
	}
}


/**
 * @brief UdpBitStream::writeVector
 * @param data
 */

inline void UdpBitStream::writeVector(const std::vector<uint8_t> &data, const bool &bitAligned)
{
	if (bitAligned)
		m_stream.writeBits<quint32>(data.size(), 32, true);
	else
		m_stream.write<quint32>(data.size(), true);

	for (const std::uint8_t &b : data) {
		if (bitAligned)
			m_stream.writeBits<std::uint8_t>(b, 8, true);
		else
			m_stream.write<std::uint8_t>(b, true);
	}
}


/**
 * @brief UdpBitStream::readVector
 * @return
 */

inline std::optional<std::vector<uint8_t> > UdpBitStream::readVector(const bool &bitAligned) const
{
	try {
		std::vector<uint8_t> data;

		quint32 size = bitAligned ?
						   m_stream.readBits<quint32>(32) :
						   m_stream.read<quint32>(true);

		data.reserve(size);

		for (size_t i=0; i<size; ++i)
			data.emplace_back(bitAligned ?
								  std::move(m_stream.readBits<std::uint8_t>(8)) :
								  std::move(m_stream.read<std::uint8_t>(true))
								  );



		return data;

	} catch (const std::out_of_range &err) {
		LOG_CWARNING("engine") << "Out of range" << err.what();
		return std::nullopt;
	} catch (const std::exception &err) {
		LOG_CWARNING("engine") << "Exception" << err.what();
		return std::nullopt;
	}
}



/**
 * @brief UdpBitStream::writeAuthKey
 * @param key
 */

inline void UdpBitStream::writeAuthKey(const std::array<uint8_t, crypto_auth_KEYBYTES> &key)
{
	for (const std::uint8_t &b : key)
		m_stream.write<std::uint8_t>(b, true);
}


/**
 * @brief UdpBitStream::readAuthKey
 * @return
 */

inline std::optional<std::array<uint8_t, crypto_auth_KEYBYTES> > UdpBitStream::readAuthKey() const
{
	try {
		std::array<std::uint8_t, crypto_auth_KEYBYTES> r;

		for (size_t i=0; i<r.size(); ++i)
			r[i] = m_stream.read<std::uint8_t>(true);

		return r;

	} catch (const std::out_of_range &err) {
		LOG_CWARNING("engine") << "Out of range" << err.what();
		return std::nullopt;
	} catch (const std::exception &err) {
		LOG_CWARNING("engine") << "Exception" << err.what();
		return std::nullopt;
	}
}



/**
 * @brief UdpBitStream::getChallengeResponse
 * @param tokenPtr
 * @param encryptedDataPtr
 * @return
 */

inline bool UdpBitStream::getChallengeResponse(QByteArray *tokenPtr, QByteArray *encryptedDataPtr)
{
	if (this->type() != MessageChallenge)
		return false;

	auto ptrToken = readByteArray();

	if (!ptrToken)
		return false;

	auto ptrEncrypted = readByteArray();

	if (!ptrEncrypted)
		return false;

	if (tokenPtr)
		*tokenPtr = std::move(*ptrToken);

	if (encryptedDataPtr)
		*encryptedDataPtr = std::move(*ptrEncrypted);

	return true;
}


/**
 * @brief UdpBitStream::getChallengeResponse
 * @param tokenPtr
 * @param encryptedDataPtr
 * @return
 */

inline bool UdpBitStream::getChallengeResponse(QByteArray *tokenPtr, std::vector<uint8_t> *encryptedDataPtr)
{
	if (this->type() != MessageChallenge)
		return false;

	auto ptrToken = readByteArray();

	if (!ptrToken)
		return false;

	auto ptrEncrypted = readVector();

	if (!ptrEncrypted)
		return false;

	if (tokenPtr)
		*tokenPtr = std::move(*ptrToken);

	if (encryptedDataPtr)
		*encryptedDataPtr = std::move(*ptrEncrypted);

	return true;
}




/**
 * @brief UdpBitStream::getConnected
 * @param peerIdPtr
 * @param peerIndexPtr
 * @return
 */

inline bool UdpBitStream::getConnected(quint32 *peerIdPtr, quint32 *peerIndexPtr)
{
	if (this->type() != MessageConnected)
		return false;

	quint32 peer;
	quint32 index;

	try {
		peer = m_stream.read<quint32>(true);
		index = m_stream.read<quint32>(true);

	} catch (const std::exception &err) {
		LOG_CWARNING("engine") << "Exception error" << err.what();
		return false;
	}

	if (peerIdPtr)
		*peerIdPtr = std::move(peer);

	if (peerIndexPtr)
		*peerIndexPtr = std::move(index);

	return true;
}



/**
 * @brief UdpBitStream::getChallenge
 * @param challengePtr
 * @param publicKeyPtr
 * @return
 */

inline bool UdpBitStream::getChallenge(std::array<uint8_t, CHALLENGE_BYTES> *challengePtr,
									   std::array<uint8_t, crypto_box_PUBLICKEYBYTES> *publicKeyPtr)
{
	if (this->type() != MessageChallenge)
		return false;

	auto ptrCh = readChallenge();

	if (!ptrCh)
		return false;

	auto ptrKey = readPublicKey();

	if (!ptrKey)
		return false;

	if (challengePtr)
		*challengePtr = std::move(*ptrCh);

	if (publicKeyPtr)
		*publicKeyPtr = std::move(*ptrKey);

	return true;
}



/**
 * @brief UdpBitStream::writePublicKey
 * @param key
 */

inline void UdpBitStream::writePublicKey(const std::array<std::uint8_t, crypto_box_PUBLICKEYBYTES> &key)
{
	for (const std::uint8_t &b : key)
		m_stream.write<std::uint8_t>(b, true);
}



/**
 * @brief UdpBitStream::readPublicKey
 * @return
 */

inline std::optional<std::array<uint8_t, crypto_box_PUBLICKEYBYTES> > UdpBitStream::readPublicKey() const
{
	try {
		std::array<std::uint8_t, crypto_box_PUBLICKEYBYTES> r;

		for (size_t i=0; i<r.size(); ++i)
			r[i] = m_stream.read<std::uint8_t>(true);

		return r;

	} catch (const std::out_of_range &err) {
		LOG_CWARNING("engine") << "Out of range" << err.what();
		return std::nullopt;
	} catch (const std::exception &err) {
		LOG_CWARNING("engine") << "Exception" << err.what();
		return std::nullopt;
	}
}


/**
 * @brief UdpBitStream::writeChallenge
 * @param challenge
 */

inline void UdpBitStream::writeChallenge(const std::array<std::uint8_t, CHALLENGE_BYTES> &challenge)
{
	for (const std::uint8_t &b : challenge)
		m_stream.write<std::uint8_t>(b, true);
}


/**
 * @brief UdpBitStream::readChallenge
 * @return
 */

inline std::optional<std::array<uint8_t, CHALLENGE_BYTES> > UdpBitStream::readChallenge() const
{
	try {
		std::array<std::uint8_t, CHALLENGE_BYTES> r;

		for (size_t i=0; i<r.size(); ++i)
			r[i] = m_stream.read<std::uint8_t>(true);

		return r;

	} catch (const std::out_of_range &err) {
		LOG_CWARNING("engine") << "Out of range" << err.what();
		return std::nullopt;

	} catch (const std::exception &err) {
		LOG_CWARNING("engine") << "Exception" << err.what();
		return std::nullopt;
	}
}



#endif // UDPBITSTREAM_H
