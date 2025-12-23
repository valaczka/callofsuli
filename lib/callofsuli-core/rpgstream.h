/*
 * ---- Call of Suli ----
 *
 * rpgstream.h
 *
 * Created on: 2025. 12. 23.
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

#ifndef RPGSTREAM_H
#define RPGSTREAM_H

#include "udpbitstream.hpp"
#include "udphelper.h"


namespace RpgStream
{

template <typename T>
T readBits(UdpBitStream &stream, const size_t &bits, const T &errorValue) {
	try {
		return stream.stream().readBits<T>(bits);

	} catch (const std::out_of_range &err) {
		LOG_CWARNING("engine") << "Out of range" << err.what();
		return errorValue;
	} catch (const std::exception &err) {
		LOG_CWARNING("engine") << "Exception" << err.what();
		return errorValue;
	}
}


template <typename T>
void writeBits(UdpBitStream &stream, const T &value, const size_t &bits) {
	if (value >= std::pow(2, bits))
		LOG_CWARNING("engine") << "Out of range" << value << "on" << bits << "bits";
	stream.stream().writeBits<T>(value, bits, true);
}



template <typename C, typename T>
C readBitsAs(UdpBitStream &stream, const size_t &bits, const C &errorValue) {
	try {
		return static_cast<C>(stream.stream().readBits<T>(bits));

	} catch (const std::out_of_range &err) {
		LOG_CWARNING("engine") << "Out of range" << err.what();
		return errorValue;
	} catch (const std::exception &err) {
		LOG_CWARNING("engine") << "Exception" << err.what();
		return errorValue;
	}
}


template <typename C, typename T>
void writeBitsAs(UdpBitStream &stream, const C &value, const size_t &bits) {
	if (static_cast<T>(value) >= std::pow(2, bits))
		LOG_CWARNING("engine") << "Out of range" << value << "on" << bits << "bits";
	stream.stream().writeBits<T>(value, bits, true);
}




#define STREAM_STATIC(name, type, bits, error) \
	public: \
	static type read##name(UdpBitStream &stream) { \
		return readBits<type>(stream, bits, error); } \
	static void write##name(UdpBitStream &stream, const type &value) { \
		writeBits<type>(stream, value, bits); }


#define STREAM_STATIC_CAST(name, cast, type, bits, error) \
	public: \
	static cast read##name(UdpBitStream &stream) { \
		return readBitsAs<cast, type>(stream, bits, error); } \
	static void write##name(UdpBitStream &stream, const cast &value) { \
		writeBitsAs<cast, type>(stream, value, bits); }


#define STREAM_MEMBER(type, field, name, bits, error) \
	private: \
		type m_##field = error; \
	public: \
	type& field() { return m_##field; } \
	const type& field() const { return m_##field; } \
	void set##name(const type &value) { m_##field = value; } \
	const type& read##name(UdpBitStream &stream) { \
		m_##field = readBits<type>(stream, bits, error); \
		return m_##field; } \
	void write##name(UdpBitStream &stream) const { \
		writeBits<type>(stream, m_##field, bits); }


#define STREAM_MEMBER_CAST(cast, field, name, type, bits, error) \
	private: \
		cast m_##field = error; \
	public: \
	cast& field() { return m_##field; } \
	const cast& field() const { return m_##field; } \
	void set##name(const cast &value) { m_##field = value; } \
	const cast& read##name(UdpBitStream &stream) { \
		m_##field = readBitsAs<cast, type>(stream, bits, error); \
		return m_##field; } \
	void write##name(UdpBitStream &stream) const { \
		writeBitsAs<cast, type>(stream, m_##field, bits); }




#define STREAM_MEMBER_BYTEARRAY(field, name) \
	private: \
		QByteArray m_##field; \
	public: \
	QByteArray& field() { return m_##field; } \
	const QByteArray& field() const { return m_##field; } \
	void set##name(const QByteArray &value) { m_##field = value; } \
	const QByteArray& read##name(UdpBitStream &stream) { \
		m_##field = stream.readByteArray(true).value_or(QByteArray()); \
		return m_##field; } \
	void write##name(UdpBitStream &stream) const { \
		stream.writeByteArray(m_##field, true); }




#define STREAM_FIELD(type, field, name) \
	private: \
		type m_##field; \
	public: \
	type& field() { return m_##field; } \
	const type& field() const { return m_##field; } \
	void set##name(const type &value) { m_##field = value; }



/**
 * @brief The EnginePlayer class
 */

class EnginePlayer
{
public:
	EnginePlayer() = default;
	EnginePlayer(const QByteArray &userName, const QByteArray &nickName)
		: m_userName(userName)
		, m_nickName(nickName)
	{}

	STREAM_MEMBER_BYTEARRAY(userName, UserName)
	STREAM_MEMBER_BYTEARRAY(nickName, NickName)
};






/**
 * @brief The Engine class
 */

class Engine
{
public:
	enum Operation {
		OperationInvalid	= 0x0,
		OperationList,
		OperationConnect,
		OperationCreate,
		OperationDisconnect,


		OperationData		= 0x7
	};

	Engine() = default;

	void readPlayers(UdpBitStream &stream);
	void writePlayers(UdpBitStream &stream) const;


	STREAM_STATIC_CAST(Operation, Operation, quint32, 3, OperationInvalid)

	STREAM_MEMBER(quint32, id, Id, 8, 0);
	STREAM_MEMBER(quint32, readableId, ReadableId, 20, 0);

	STREAM_FIELD(std::vector<EnginePlayer>, players, Players)

};



/**
 * @brief The EngineStream class
 */

class EngineStream : public UdpBitStream
{
public:
	EngineStream(const UdpAuthKey &authKey, const quint32 &peerIndex, const Engine::Operation &operation = Engine::OperationData)
		: UdpBitStream(MessageUser)
		, m_authKey(authKey)
		, m_hasAuthKey(true)
	{
		writePeerIndex(peerIndex);
		Engine::writeOperation(*this, operation);
	}

	EngineStream(const quint32 &peerIndex, const Engine::Operation &operation = Engine::OperationData)
		: UdpBitStream(MessageUser)
		, m_authKey({})
		, m_hasAuthKey(false)
	{
		writePeerIndex(peerIndex);
		Engine::writeOperation(*this, operation);
	}

	virtual std::vector<std::uint8_t> data() const override;

	const bool &hasAuthKey() const { return m_hasAuthKey; }
	const UdpAuthKey &authKey() const { return m_authKey; }
	void setAuthKey(const UdpAuthKey &key) {
		m_authKey = key;
		m_hasAuthKey = true;
	}

protected:
	UdpAuthKey m_authKey;
	bool m_hasAuthKey;
};



/**
 * @brief The EngineList class
 */

class EngineList
{
public:
	EngineList() = default;

	void readEngines(UdpBitStream &stream);
	void writeEngines(UdpBitStream &stream) const;

	EngineStream toStream() const;

	STREAM_FIELD(std::vector<Engine>, engines, Engines)
};


};

#endif // RPGSTREAM_H
