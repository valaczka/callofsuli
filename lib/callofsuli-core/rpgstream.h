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


#define ENGINE_ID_TYPE				quint32
#define ENGINE_ID_BITS				8

#define ENGINE_READABLE_ID_TYPE		quint32
#define ENGINE_READABLE_ID_BITS		20


#define PLAYER_ID_TYPE				quint8
#define PLAYER_ID_BITS				3



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







/**
 * @brief uint64_t
 */

class HashFnv1A64 : public QHash<quint64, QString>
{
public:
	HashFnv1A64() = default;
	virtual ~HashFnv1A64() {}

	static constexpr quint64 hashFnv1a64(std::string_view s) {
		if (s.empty())
			return 0;

		quint64 h = 1469598103934665603ull;
		for (unsigned char c : s) {
			h ^= quint64(c);
			h *= 1099511628211ull;
		}
		return h;
	}

	std::optional<quint64> insert(const QString &value) {
		const quint64 h = hashFnv1a64(value.toStdString());

		auto it = this->tryEmplace(h, value);

		if (!it.inserted) {
			LOG_CWARNING("engine") << "Hash already exists" << h << "for value" << value;
			return std::nullopt;
		}

		return h;
	}
};





#define STREAM_FIELD(type, field, name) \
	private: \
	type m_##field; \
	public: \
	type& field() { return m_##field; } \
	const type& field() const { return m_##field; } \
	void set##name(const type &value) { m_##field = value; }


#define STREAM_STATIC(name, type, bits, error) \
	public: \
	type read##name() { \
	return readBits<type>(*this, bits, error); } \
	void write##name(const type &value) { \
	writeBits<type>(*this, value, bits); } \


#define STREAM_STATIC_CAST(name, cast, type, bits, error) \
	public: \
	cast read##name() { \
	return readBitsAs<cast, type>(*this, bits, error); } \
	void write##name(const cast &value) { \
	writeBitsAs<cast, type>(*this, value, bits); }


#define STREAM_MEMBER(type, field, name, bits, error) \
	STREAM_FIELD(type, field, name) \
	public: \
	const type& read##name(EngineStream &stream) { \
	m_##field = readBits<type>(stream, bits, error); \
	return m_##field; } \
	void write##name(EngineStream &stream) const { \
	writeBits<type>(stream, m_##field, bits); }


#define STREAM_MEMBER_CAST(cast, field, name, type, bits, error) \
	STREAM_FIELD(cast, field, name) \
	public: \
	const cast& read##name(EngineStream &stream) { \
	m_##field = readBitsAs<cast, type>(stream, bits, error); \
	return m_##field; } \
	void write##name(EngineStream &stream) const { \
	writeBitsAs<cast, type>(stream, m_##field, bits); }





#define STREAM_MEMBER_BYTEARRAY(field, name) \
	STREAM_FIELD(QByteArray, field, name) \
	public: \
	const QByteArray& read##name(EngineStream &stream) { \
	m_##field = stream.readByteArray(true).value_or(QByteArray()); \
	return m_##field; } \
	void write##name(EngineStream &stream) const { \
	stream.writeByteArray(m_##field, true); }





#define STREAM_MEMBER_VECTOR(type, field, name, sizetype, bits) \
	STREAM_FIELD(std::vector<type>, field, name) \
	public: \
	const std::vector<type>& read##name(EngineStream &stream) { \
	m_##field.clear(); \
	sizetype size = readBits<sizetype>(stream, bits, 0); \
	m_##field.reserve(size); \
	for (sizetype i=0; i<size; ++i) { \
	type p; \
	p << stream; \
	m_##field.emplace_back(std::move(p)); \
} \
	return m_##field; } \
	void write##name(EngineStream &stream) const { \
	writeBits<sizetype>(stream, m_##field.size(), bits); \
	for (const type &p : m_##field) { \
	p >> stream; \
} \
}



#define ADD_STRING_RESOLVER(field, name) \
	public: \
	QString field##Resolved(const HashFnv1A64 &hash) const { return hash.value(m_##field, QString()); } \
	void set##name##Resolved(const QString &value) { set##name(HashFnv1A64::hashFnv1a64(value.toStdString())); }



#define STREAM_MEMBER_RESOLVED(field, name) \
	STREAM_MEMBER(quint64, field, name, 64, 0) \
	ADD_STRING_RESOLVER(field, name)





#define STREAM_DELTA_MASK(type, bits, ...) \
	private:\
	type m_deltaMask = 0; \
	enum DeltaMaskBit { __VA_ARGS__ }; \
	bool m_isDeltaMode = false; \
	public:\
	const bool &isDeltaMode() const { return m_isDeltaMode; } \
	void setIsDeltaMode(const bool &mode) { m_isDeltaMode = mode; } \
	type readDeltaMask(EngineStream &stream) {\
	if (m_isDeltaMode) \
	m_deltaMask = readBits<type>(stream, bits, 0); \
	return m_deltaMask; \
} \
	void writeDeltaMask(EngineStream &stream) const { \
	if (m_isDeltaMode) \
	writeBits<type>(stream, m_deltaMask, bits); \
} \
	const type &deltaMask() const { return m_deltaMask; } \
	bool deltaMask(const DeltaMaskBit &bit) const { \
	return (m_deltaMask & (1<<bit)); \
} \
	private: \
	void assignDeltaMask(const DeltaMaskBit &bit) { \
	m_deltaMask |= 1<<bit; \
}



#define STREAM_MEMBER_ADD_DELTA(type, field, name, msk) \
	public: \
	bool set##name##Delta(const type &value, const bool &condition) { \
	if (condition) { set##name(value); assignDeltaMask(msk); return true;} return false; } \
	bool set##name##Delta(const type &value, const std::function<bool(const type &t)> &func) { \
	if (func && func(value)) { set##name(value); assignDeltaMask(msk); return true;} return false; } \
	type read##name##Delta(EngineStream &stream) { \
	if (!m_isDeltaMode || deltaMask(msk)) return read##name(stream); \
	else return m_##field;} \
	void write##name##Delta(EngineStream &stream) const { \
	if (!m_isDeltaMode || deltaMask(msk)) write##name(stream); }


#define STREAM_MEMBER_VECTOR_ADD_READ_DELTA(type, field, name, sizetype, bits) \
	const std::vector<type>& read##name##VectorDelta(EngineStream &stream, const bool &isDeltaMode = true) { \
	if (!isDeltaMode) { return read##name(stream); } \
	m_##field.clear(); \
	sizetype size = readBits<sizetype>(stream, bits, 0); \
	m_##field.reserve(size); \
	for (sizetype i=0; i<size; ++i) { \
	type p; \
	p.setIsDeltaMode(isDeltaMode); \
	p << stream; \
	m_##field.emplace_back(std::move(p)); \
} \
	return m_##field; }


#define STREAM_DELTA_MEMBER(type, field, name, bits, error, msk) \
	STREAM_MEMBER(type, field, name, bits, error) \
	STREAM_MEMBER_ADD_DELTA(type, field, name, msk)


#define STREAM_DELTA_MEMBER_CAST(cast, field, name, type, bits, error, msk) \
	STREAM_MEMBER_CAST(cast, field, name, type, bits, error) \
	STREAM_MEMBER_ADD_DELTA(type, field, name, msk)


#define STREAM_DELTA_MEMBER_BYTEARRAY(field, name, msk) \
	STREAM_MEMBER_BYTEARRAY(field, name) \
	STREAM_MEMBER_ADD_DELTA(QByteArray, field, name, msk)


#define STREAM_DELTA_MEMBER_VECTOR(type, field, name, sizetype, bits, msk) \
	STREAM_MEMBER_VECTOR(type, field, name, sizetype, bits) \
	STREAM_MEMBER_ADD_DELTA(std::vector<type>, field, name, msk) \
	STREAM_MEMBER_VECTOR_ADD_READ_DELTA(type, field, name, sizetype, bits) \
	void set##name##Delta(const bool &condition = true) { if (condition) assignDeltaMask(msk); }


#define STREAM_MEMBER_VECTOR_READ_DELTA(type, field, name, sizetype, bits) \
	STREAM_MEMBER_VECTOR(type, field, name, sizetype, bits) \
	STREAM_MEMBER_VECTOR_ADD_READ_DELTA(type, field, name, sizetype, bits)


#define STREAM_DELTA_MEMBER_RESOLVED(field, name, msk) \
	STREAM_MEMBER_RESOLVED(field, name) \
	STREAM_MEMBER_ADD_DELTA(quint64, field, name, msk)





#define TO_DATA_STREAM(dataOp) \
	EngineDataStream toDataStream() const { \
	EngineDataStream stream(dataOp); \
	*this >> stream; \
	return stream; \
} \
	EngineDataStream toDataStream(const quint32 &peerIndex) const { \
	EngineDataStream stream(peerIndex, dataOp); \
	*this >> stream; \
	return stream; \
}

/*STREAM_DELTA_MASK (
		quint32, 5,

		Egy,
		Ketto,
		Negy,
		Nyolc,
		Tizenhat,
		)

STREAM_DELTA_MEMBER(quint32, id, Id, 8, 0, Egy);
STREAM_DELTA_MEMBER_BYTEARRAY(description, Description, Ketto)
STREAM_MEMBER(quint32, readableId, ReadableId, 20, 0);
STREAM_DELTA_MEMBER_VECTOR(EnginePlayer, players, Players, quint8, 3, Negy)*/




/**
 * @brief The EngineStream class
 */

class EngineStream : public UdpBitStream
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

	static constexpr quint8 CurrentVersion = 1;

	EngineStream(const UdpAuthKey &authKey, const quint32 &peerIndex, const Operation &operation)
		: UdpBitStream(MessageUser)
		, m_operation(operation)
		, m_version(CurrentVersion)
		, m_authKey(authKey)
		, m_hasAuthKey(true)
	{
		writePeerIndex(peerIndex);
		writeOperation(*this);
		writeVersion(*this);
	}

	EngineStream(const quint32 &peerIndex, const Operation &operation)
		: UdpBitStream(MessageUser)
		, m_operation(operation)
		, m_version(CurrentVersion)
		, m_authKey({})
		, m_hasAuthKey(false)
	{
		writePeerIndex(peerIndex);
		writeOperation(*this);
		writeVersion(*this);
	}

	EngineStream(const Operation &operation)
		: UdpBitStream(MessageUser)
		, m_operation(operation)
		, m_version(CurrentVersion)
		, m_authKey({})
		, m_hasAuthKey(false)
	{
		writeOperation(*this);
		writeVersion(*this);
	}


	EngineStream(UdpBitStream &&other);
	EngineStream(std::unique_ptr<UdpBitStream> &stream);

	STREAM_MEMBER_CAST(Operation, operation, Operation, quint32, 3, OperationInvalid)
	STREAM_MEMBER(quint8, version, Version, 4, 0);
	STREAM_STATIC(EngineId, ENGINE_ID_TYPE, ENGINE_ID_BITS, 0);

	virtual std::vector<std::uint8_t> data() const override;

	const bool &hasAuthKey() const { return m_hasAuthKey; }
	const UdpAuthKey &authKey() const { return m_authKey; }
	void setAuthKey(const UdpAuthKey &key) {
		m_authKey = key;
		m_hasAuthKey = true;
	}

	void finalize() const;

protected:
	UdpAuthKey m_authKey;
	bool m_hasAuthKey;
	mutable bool m_hasFinalized = false;
};





/**
 * @brief The EngineDataStream class
 */

class EngineDataStream : public EngineStream
{
public:
	enum DataOperation {
		DataOperationInvalid = 0x0,
		DataOperationCharacterSelect
	};

	EngineDataStream(const DataOperation &dataOperation)
		: EngineStream(OperationData)
		, m_dataOperation(dataOperation)
	{
		writeDataOperation(*this);
	}

	EngineDataStream(const quint32 &peerIndex, const DataOperation &dataOperation)
		: EngineStream(peerIndex, OperationData)
		, m_dataOperation(dataOperation)
	{
		writeDataOperation(*this);
	}

	EngineDataStream(UdpBitStream &&other)
		: EngineStream(std::move(other))
		, m_dataOperation(DataOperationInvalid)
	{
		if (this->operation() == OperationData)
			readDataOperation(*this);
	}

	EngineDataStream(std::unique_ptr<UdpBitStream> &stream)
		: EngineStream(stream)
		, m_dataOperation(DataOperationInvalid)
	{
		if (this->operation() == OperationData)
			readDataOperation(*this);
	}


	STREAM_MEMBER_CAST(DataOperation, dataOperation, DataOperation, quint32, 4, DataOperationInvalid)
};




/**
 * @brief The EnginePlayer class
 */

class EnginePlayer
{
public:
	EnginePlayer() = default;
	EnginePlayer(const QByteArray &userName, const QByteArray &nickName);

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;

	STREAM_MEMBER_BYTEARRAY(userName, UserName)
	STREAM_MEMBER_BYTEARRAY(nickName, NickName)
};







/**
 * @brief The Engine class
 */

class Engine
{
public:

	Engine() = default;

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;


	STREAM_MEMBER(ENGINE_ID_TYPE, id, Id, ENGINE_ID_BITS, 0);
	STREAM_MEMBER(ENGINE_READABLE_ID_TYPE, readableId, ReadableId, ENGINE_READABLE_ID_BITS, 0);
	STREAM_MEMBER_VECTOR(EnginePlayer, players, Players, PLAYER_ID_TYPE, PLAYER_ID_BITS)
	STREAM_FIELD(EnginePlayer, owner, Owner)
	STREAM_MEMBER(PLAYER_ID_TYPE, maxPlayer, MaxPlayer, PLAYER_ID_BITS, 0)

};




/**
 * @brief The EngineList class
 */

class EngineList
{
public:
	EngineList() = default;

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;

	EngineStream toStream() const;

	STREAM_MEMBER_VECTOR(Engine, engines, Engines, quint8, 8)
	STREAM_MEMBER_CAST(bool, canCreate, CanCreate, char, 1, false)
};




/**
 * @brief The GameConfig class
 */

class GameConfig
{
public:
	GameConfig() = default;

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;

	STREAM_MEMBER_RESOLVED(terrain, Terrain)
	STREAM_MEMBER(quint32, duration, Duration, 32, 0)

	/*QS_COLLECTION_OBJECTS(QList, PlayerPosition, positionList)
	QS_FIELD(QString, terrain)
	QS_OBJECT(Collection, collection)
	QS_OBJECT(Randomizer, randomizer)
	QS_FIELD(int, duration)*/
};



class PlayerData
{
public:
	PlayerData() = default;

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;


	STREAM_MEMBER(PLAYER_ID_TYPE, playerId, PlayerId, PLAYER_ID_BITS, 0)
	STREAM_MEMBER_BYTEARRAY(userName, UserName)
	STREAM_MEMBER_BYTEARRAY(nickName, NickName)

	STREAM_MEMBER_RESOLVED(character, Character)

	STREAM_MEMBER_CAST(bool, completed, Completed, char, 1, false)


	/*QS_FIELD(int, playerId)
	QS_FIELD(QString, username)
	QS_FIELD(QString, nickname)

	QS_FIELD(QString, character)
	QS_FIELD(bool, completed)
	QS_FIELD(bool, locked)											// lock the engine

	// Character specification

	QS_OBJECT(Armory, armory)
	QS_FIELD(int, maxHp)
	QS_FIELD(int, maxMp)
	QS_FIELD(int, mp)

	QS_OBJECT(GameConfig, gameConfig)

	QS_FIELD(int, lastObjectId)

	QS_FIELD(bool, finished)										// finished
	QS_FIELD(int, xp)												// XP
	QS_FIELD(int, cur)												// currency
	QS_FIELD(int, kill)												// killed enemies*/
};





class CharacterSelectServer
{
public:
	CharacterSelectServer() = default;

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;

	TO_DATA_STREAM(EngineDataStream::DataOperationCharacterSelect)

	STREAM_FIELD(GameConfig, gameConfig, GameConfig)
	STREAM_MEMBER_VECTOR(PlayerData, players, Players, PLAYER_ID_TYPE, PLAYER_ID_BITS)
	STREAM_MEMBER(PLAYER_ID_TYPE, maxPlayers, MaxPlayers, PLAYER_ID_BITS, 0)
	STREAM_MEMBER(ENGINE_READABLE_ID_TYPE, engineReadableId, EngineReadableId, ENGINE_READABLE_ID_BITS, 0);

	/*QS_OBJECT(GameConfig, gameConfig)
	QS_COLLECTION_OBJECTS(QList, CharacterSelect, players)
	QS_FIELD(bool, locked)
	QS_FIELD(int, max)
	QS_FIELD(int, engineReadableId)*/
};


};

#endif // RPGSTREAM_H
