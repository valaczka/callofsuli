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
#include <QRectF>


#define ENGINE_ID_TYPE				quint32
#define ENGINE_ID_BITS				8

#define ENGINE_READABLE_ID_TYPE		quint32
#define ENGINE_READABLE_ID_BITS		20

#define PLAYER_ID_TYPE				quint8
#define PLAYER_ID_BITS				3


#define TAG_ID_TYPE					quint32
#define TAG_ID_BITS					32

#define CHUNK_SIZE_TYPE				quint32
#define CHUNK_SIZE_BITS				12						// Max: 4096

#define QUANTIZED_TYPE				quint32
#define QUANTIZED_BITS				24

#define QUANTIZED_SIGNED_TYPE		qint32
#define QUANTIZED_SIGNED_BITS		24

#define STATE_LIST_TYPE				quint8
#define STATE_LIST_BITS				8						// Max: 256 frame = ~4 sec

#define ENTITY_LIST_TYPE			quint32
#define ENTITY_LIST_BITS			12						// Max: 4096



#define ENTITY_HP_TYPE				quint32
#define ENTITY_HP_BITS				8						// Max: 256

#define ENTITY_MP_TYPE				quint32
#define ENTITY_MP_BITS				16						// Max: 65535



namespace RpgStream
{

// Quantization

static constexpr float QUANTIZE_MIN = 0.f;
static constexpr float QUANTIZE_MAX = 16500.f;

static QUANTIZED_TYPE quantize(const float &value) {
	return static_cast<QUANTIZED_TYPE>(std::round(std::clamp(value, QUANTIZE_MIN, QUANTIZE_MAX) * 1000.f));
}

static float dequantize(const QUANTIZED_TYPE &value) {
	return static_cast<float>(value / 1000.f);
}

static constexpr float QUANTIZE_SIGNED_MIN = -8300.f;
static constexpr float QUANTIZE_SIGNED_MAX = 8300.f;

static QUANTIZED_SIGNED_TYPE quantizeSigned(const float &value) {
	return static_cast<QUANTIZED_SIGNED_TYPE>(std::round(std::clamp(value, QUANTIZE_SIGNED_MIN, QUANTIZE_SIGNED_MAX) * 1000.f));
}

static float dequantizeSigned(const QUANTIZED_SIGNED_TYPE &value) {
	return static_cast<float>(value / 1000.f);
}




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
 * @brief The Team enum
 */

enum Team {
	TeamNone = 0,
	TeamA = 1,
	TeamB = 2
};




/**
 * @brief The RpgConnectionToken class
 */

class ConnectionToken : public UdpConnectionToken
{
	Q_GADGET

public:
	ConnectionToken(const QString &_user = {}, const quint32 &_peer = 0, const qint64 &_exp = 0)
		: UdpConnectionToken(1, _user, _peer, _exp)
		, campaign(-1)
		, duration(0)
	{}

	QS_SERIALIZABLE
	QS_FIELD(QString, mapUuid)
	QS_FIELD(QString, missionUuid)
	QS_FIELD(int, missionLevel)
	QS_FIELD(int, campaign)
	QS_FIELD(int, duration)
};




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





#define STREAM_FIELD(type, field, name, error) \
	private: \
	mutable type m_##field = error; \
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
	STREAM_FIELD(type, field, name, error) \
	public: \
	const type& read##name(EngineStream &stream) { \
	m_##field = readBits<type>(stream, bits, error); \
	return m_##field; } \
	void write##name(EngineStream &stream) const { \
	writeBits<type>(stream, m_##field, bits); }


#define STREAM_MEMBER_CAST(cast, field, name, type, bits, error) \
	STREAM_FIELD(cast, field, name, error) \
	public: \
	const cast& read##name(EngineStream &stream) { \
	m_##field = readBitsAs<cast, type>(stream, bits, error); \
	return m_##field; } \
	void write##name(EngineStream &stream) const { \
	writeBitsAs<cast, type>(stream, m_##field, bits); }


#define STREAM_MEMBER_BYTEARRAY(field, name) \
	STREAM_FIELD(QByteArray, field, name, {}) \
	public: \
	const QByteArray& read##name(EngineStream &stream) { \
	m_##field = stream.readByteArray(true).value_or(QByteArray()); \
	return m_##field; } \
	void write##name(EngineStream &stream) const { \
	stream.writeByteArray(m_##field, true); }





#define STREAM_MEMBER_VECTOR(type, field, name, sizetype, bits) \
	STREAM_FIELD(std::vector<type>, field, name, {}) \
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



#define STREAM_MEMBER_QUANT(field, name, error) \
	private: \
	QUANTIZED_TYPE m_##field = error; \
	public: \
	QUANTIZED_TYPE& field() { return m_##field; } \
	const QUANTIZED_TYPE& field() const { return m_##field; } \
	void set##name(const QUANTIZED_TYPE &value) { m_##field = value; } \
	float field##AsFloat() const { return dequantize(m_##field); } \
	void set##name##AsFloat(const float &value) { m_##field = quantize(value); } \
	public: \
	const QUANTIZED_TYPE& read##name(EngineStream &stream) { \
	m_##field = readBits<QUANTIZED_TYPE>(stream, QUANTIZED_BITS, error); \
	return m_##field; } \
	void write##name(EngineStream &stream) const { \
	writeBits<QUANTIZED_TYPE>(stream, m_##field, QUANTIZED_BITS); }


#define STREAM_MEMBER_QUANT_SIGNED(field, name, error) \
	private: \
	QUANTIZED_SIGNED_TYPE m_##field = error; \
	public: \
	QUANTIZED_SIGNED_TYPE& field() { return m_##field; } \
	const QUANTIZED_SIGNED_TYPE& field() const { return m_##field; } \
	void set##name(const QUANTIZED_SIGNED_TYPE &value) { m_##field = value; } \
	float field##AsFloat() const { return dequantizeSigned(m_##field); } \
	void set##name##AsFloat(const float &value) { m_##field = quantizeSigned(value); } \
	public: \
	const QUANTIZED_SIGNED_TYPE& read##name(EngineStream &stream) { \
	m_##field = readBits<QUANTIZED_SIGNED_TYPE>(stream, QUANTIZED_SIGNED_BITS, error); \
	return m_##field; } \
	void write##name(EngineStream &stream) const { \
	writeBits<QUANTIZED_SIGNED_TYPE>(stream, m_##field, QUANTIZED_SIGNED_BITS); }



#define STREAM_ADD_DELTA_MODE \
	private: \
	mutable bool m_isDeltaMode = false; \
	public:\
	const bool &isDeltaMode() const { return m_isDeltaMode; } \
	void setIsDeltaMode(const bool &mode) { m_isDeltaMode = mode; } \

#define STREAM_DELTA_MASK(type, bits, ...) \
	STREAM_ADD_DELTA_MODE \
	private:\
	type m_deltaMask = 0; \
	enum DeltaMaskBit { __VA_ARGS__ }; \
	public:\
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
	if (!m_isDeltaMode || deltaMask(msk)) write##name(stream); } \
	bool has##name##DeltaMask() const { return deltaMask(msk); }


#define STREAM_MEMBER_VECTOR_ADD_DELTA(type, field, name, sizetype, bits) \
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
	return m_##field; \
} \
	void write##name##VectorDelta(EngineStream &stream, const bool &isDeltaMode = true) const { \
	if (!isDeltaMode) { return write##name(stream); } \
	writeBits<sizetype>(stream, m_##field.size(), bits); \
	for (type &p : m_##field) { \
	p.setIsDeltaMode(isDeltaMode); \
	p >> stream; \
} \
} \
	void compress##name##Vector(const std::vector<type> &list) { \
	m_##field.clear(); \
	m_##field.reserve(list.size()); \
	if (list.empty()) \
	return; \
	const auto first = list.cbegin(); \
	m_##field.emplace_back(*first); \
	for (auto it = std::next(list.cbegin()); it != list.cend(); ++it) { \
	type tmp = *first; \
	tmp.setIsDeltaMode(true); \
	tmp.loadFromDelta(*it, false); \
	m_##field.emplace_back(std::move(tmp)); \
} \
} \
	std::vector<type> extract##name##Vector() const { \
	std::vector<type> out; \
	if (m_##field.empty()) \
	return out; \
	out.reserve(m_##field.size()); \
	const auto first = m_##field.cbegin(); \
	out.emplace_back(*first); \
	for (const type &p : m_##field) { \
	type tmp = *first; \
	tmp.setIsDeltaMode(true); \
	tmp.loadFromDelta(p, true); \
	out.emplace_back(std::move(tmp)); \
} \
	return out; \
}


#define STREAM_DELTA_MEMBER(type, field, name, bits, error, msk) \
	STREAM_MEMBER(type, field, name, bits, error) \
	STREAM_MEMBER_ADD_DELTA(type, field, name, msk)


#define STREAM_DELTA_MEMBER_CAST(cast, field, name, type, bits, error, msk) \
	STREAM_MEMBER_CAST(cast, field, name, type, bits, error) \
	STREAM_MEMBER_ADD_DELTA(cast, field, name, msk)


#define STREAM_DELTA_MEMBER_BYTEARRAY(field, name, msk) \
	STREAM_MEMBER_BYTEARRAY(field, name) \
	STREAM_MEMBER_ADD_DELTA(QByteArray, field, name, msk)


#define STREAM_DELTA_MEMBER_VECTOR(type, field, name, sizetype, bits) \
	STREAM_MEMBER_VECTOR(type, field, name, sizetype, bits) \
	STREAM_MEMBER_VECTOR_ADD_DELTA(type, field, name, sizetype, bits)


#define STREAM_DELTA_MEMBER_RESOLVED(field, name, msk) \
	STREAM_MEMBER_RESOLVED(field, name) \
	STREAM_MEMBER_ADD_DELTA(quint64, field, name, msk)


#define STREAM_DELTA_MEMBER_QUANT(field, name, error, msk) \
	STREAM_MEMBER_QUANT(field, name, error) \
	STREAM_MEMBER_ADD_DELTA(QUANTIZED_TYPE, field, name, msk)


#define STREAM_DELTA_MEMBER_QUANT_SIGNED(field, name, error, msk) \
	STREAM_MEMBER_QUANT_SIGNED(field, name, error) \
	STREAM_MEMBER_ADD_DELTA(QUANTIZED_SIGNED_TYPE, field, name, msk)




#define LOAD_FROM_DELTA_START(type) \
	void loadFromDelta(const type &other, const bool fromMask) { \
	m_deltaMask = 0;

#define LOAD_FROM_DELTA(field, name) \
	if (fromMask) \
	set##name##Delta(other.field(), other.has##name##DeltaMask()); \
	else \
	set##name##Delta(other.field(), other.field() != field());

#define LOAD_FROM_DELTA_MEMBER(field) \
	m_##field.loadFromDelta(other.m_##field, fromMask);

#define LOAD_WITHOUT_DELTA(field, name) \
	set##name(other.field());

#define LOAD_FROM_DELTA_END \
	setIsDeltaMode(true); \
}


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

	EngineStream(const std::array<unsigned char, crypto_auth_KEYBYTES> &secret,
				 const quint32 &peerIndex, const Operation &operation)
		: UdpBitStream(MessageUser)
		, m_operation(operation)
		, m_version(CurrentVersion)
		, m_signer(AuthKeySigner(secret))
	{
		writePeerIndex(peerIndex);
		writeOperation(*this);
		writeVersion(*this);
	}

	EngineStream(const quint32 &peerIndex, const Operation &operation)
		: UdpBitStream(MessageUser)
		, m_operation(operation)
		, m_version(CurrentVersion)
	{
		writePeerIndex(peerIndex);
		writeOperation(*this);
		writeVersion(*this);
	}

	EngineStream(const Operation &operation)
		: UdpBitStream(MessageUser)
		, m_operation(operation)
		, m_version(CurrentVersion)
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

	void setSecret(const std::array<unsigned char, crypto_auth_KEYBYTES> &secret) { m_signer = AuthKeySigner(secret); }
	void setSecret(const QByteArray &secret) { m_signer = AuthKeySigner(secret); }
	void clearSecret() { m_signer = std::nullopt; }

	const std::optional<AuthKeySigner> &signer() const { return m_signer; }

	void finalize() const;

protected:
	std::optional<AuthKeySigner> m_signer = std::nullopt;
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
		DataOperationCharacterSelect,
		DataOperationMapData,
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
	STREAM_FIELD(EnginePlayer, owner, Owner, {})
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
	STREAM_MEMBER_CAST(bool, canCreate, CanCreate, quint8, 1, false)
};




/**
 * @brief The Chunk class
 */

class Chunk
{
public:
	Chunk() = default;
	Chunk(const CHUNK_SIZE_TYPE &x, const CHUNK_SIZE_TYPE &y)
		: m_x(x)
		, m_y(y)
	{}

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;

	STREAM_MEMBER(CHUNK_SIZE_TYPE, x, X, CHUNK_SIZE_BITS, 0);
	STREAM_MEMBER(CHUNK_SIZE_TYPE, y, Y, CHUNK_SIZE_BITS, 0);
};



/**
 * @brief The ChunkGrid class
 */

class ChunkGrid
{
public:
	ChunkGrid() = default;

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;

	STREAM_MEMBER_QUANT(viewportX, ViewportX, 0);
	STREAM_MEMBER_QUANT(viewportY, ViewportY, 0);
	STREAM_MEMBER_QUANT(viewportWidth, ViewportWidth, 0);
	STREAM_MEMBER_QUANT(viewportHeight, ViewportHeight, 0);
	STREAM_MEMBER_QUANT(chunkWidth, ChunkWidth, 0);
	STREAM_MEMBER_QUANT(chunkHeight, ChunkHeight, 0);

	STREAM_MEMBER_VECTOR(Chunk, excludeList, ExcludeList, quint64, 64);
};




/**
 * @brief The PlayerPosition class
 */

class PlayerPosition
{
public:
	PlayerPosition() = default;

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;

	STREAM_MEMBER_QUANT(posX, PosX, 0);
	STREAM_MEMBER_QUANT(posY, PosY, 0);

	STREAM_MEMBER(quint8, team, Team, 2, 0);
};










/**
 * @brief The BaseTickState class - azok az oszályok, amik tick-enként tartalmaznak valamilyen állapotot
 */

class BaseTickState
{
public:
	BaseTickState() = default;

	STREAM_MEMBER(quint32, tick, Tick, 32, 0)
};






/**
 * @brief The GameState class
 */


class GameState : public BaseTickState
{
public:
	GameState() : BaseTickState() {}

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;

	STREAM_MEMBER(quint32, ptsA, PtsA, 32, 0)
	STREAM_MEMBER(quint32, ptsB, PtsB, 32, 0)

	bool operator==(const GameState &other) const {
		return other.m_ptsA == m_ptsA &&
				other.m_ptsB == m_ptsB
				;
	}
};





/**
 * @brief The MpEmitter class
 */

class MpEmitter
{
public:
	MpEmitter() = default;

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;

	STREAM_MEMBER(TAG_ID_TYPE, tagId, TagId, TAG_ID_BITS, 0);
	STREAM_MEMBER_QUANT(posX, PosX, 0);
	STREAM_MEMBER_QUANT(posY, PosY, 0);
	STREAM_MEMBER_QUANT(radius, Radius, 0);
	STREAM_MEMBER(quint32, capacity, Capacity, 13, 0);				// Max. 8192
};






/**
 * @brief The Tower class
 */

class Tower
{
public:
	Tower() = default;

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;

	STREAM_MEMBER(TAG_ID_TYPE, tagId, TagId, TAG_ID_BITS, 0);
};






/**
 * @brief The MapData class
 */

class MapData
{
public:
	MapData() = default;

	TO_DATA_STREAM(EngineDataStream::DataOperationMapData)

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;

	STREAM_MEMBER_VECTOR(PlayerPosition, playerPositionList, PlayerPositionList, quint8, 8);
	STREAM_FIELD(ChunkGrid, chunkGrid, ChunkGrid, {})
	STREAM_MEMBER_VECTOR(MpEmitter, mpEmitterList, MpEmitterList, quint8, 8);
	STREAM_MEMBER_VECTOR(Tower, towerList, towerList, quint8, 8);
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

	enum Flag {
		FlagNull				= 0,
		FlagSelected			= 1 << 0,				// terep és karakterek kiválasztva
		FlagWaitingData			= 1 << 1,				// várjuk a host-tól a terepadatokat
		FlagDataCompleted		= 1 << 2,				// megkaptuk az adatokat (player position, chunk grid, randomizer,...)
		FlagDataPrepared		= 1 << 3,				// feldolgoztuk és elküldtük az adatokat mindenkinek
		FlagPlaying				= 1 << 4,				// játék elindult
		FlagFinished			= 1 << 5,				// játék véget ért
	};

	Q_DECLARE_FLAGS(Flags, Flag)


	enum Stage {
		StageInit,										// a játék előkészítése
		StageSelect,									// a "felszerelés" kiválasztása (max. 30 mp)
		StageWarmingUp,									// az 1. perc
		StageMain,										// fő játék 3.5 perc
		StageLast,										// az utolsó 30 mp
		StageFinished									// befejeződött
	};

	STREAM_MEMBER_RESOLVED(terrain, Terrain)
	STREAM_MEMBER_CAST(Flags, flags, Flags, quint32, 16, FlagNull)
	STREAM_MEMBER_CAST(Stage, stage, Stage, quint32, 4, StageInit)
	STREAM_MEMBER(quint32, duration, Duration, 18, 0)	// egy játék hozza, 2^18 frame = max. ~72 perc
};

Q_DECLARE_OPERATORS_FOR_FLAGS(GameConfig::Flags)



/**
 * @brief The PlayerData class
 */

class PlayerData
{
public:
	PlayerData() = default;

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;

	enum Flag {
		FlagNull				= 0,
		FlagCompleted			= 1 << 0,				// a karakterválasztás befejeződött, rányomott a play-re
		FlagDownloadStarted		= 1 << 1,				// a szükséges letöltés elkezdődött
		FlagDownloadCompleted	= 1 << 2,				// a szükséges letöltés sikerült
		FlagLoadStarted			= 1 << 3,				// a játék betöltése helyben elkezdődőtt
		FlagLoadCompleted		= 1 << 4,				// a játék betöltése helyben befejeződött
		FlagGamePrepared		= 1 << 5,				// a játék teljesen elkészült (a szervertől kapottak alapján)
		FlagGameStarted			= 1 << 6,				// a játék elkezdődött
		FlagGameFinished		= 1 << 7,				// a játék befejeződött
		FlagPlayerOnline		= 1 << 8,				// a játékos elérhető (van udp-kapcsolat)
	};

	Q_DECLARE_FLAGS(Flags, Flag)

	STREAM_MEMBER(PLAYER_ID_TYPE, playerId, PlayerId, PLAYER_ID_BITS, 0)
	STREAM_MEMBER_BYTEARRAY(userName, UserName)
	STREAM_MEMBER_BYTEARRAY(nickName, NickName)

	STREAM_MEMBER_RESOLVED(character, Character)

	STREAM_MEMBER_CAST(Flags, flags, Flags, quint32, 16, FlagNull)

	STREAM_MEMBER(ENTITY_HP_TYPE, maxHp, MaxHp, ENTITY_HP_BITS, 0)
	STREAM_MEMBER(ENTITY_MP_TYPE, maxMp, MaxMp, ENTITY_MP_BITS, 0)
};


Q_DECLARE_OPERATORS_FOR_FLAGS(PlayerData::Flags)




/**
 * @brief The CharacterSelectServer class
 */

class CharacterSelectServer
{
public:
	CharacterSelectServer() = default;

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;


	STREAM_FIELD(GameConfig, gameConfig, GameConfig, {})
	STREAM_MEMBER_VECTOR(PlayerData, players, Players, PLAYER_ID_TYPE, PLAYER_ID_BITS)
	STREAM_MEMBER(PLAYER_ID_TYPE, maxPlayers, MaxPlayers, PLAYER_ID_BITS, 0)
	STREAM_MEMBER(ENGINE_READABLE_ID_TYPE, engineReadableId, EngineReadableId, ENGINE_READABLE_ID_BITS, 0);
};









/// Base entity state


class EntityState
{
public:
	EntityState() = default;

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;


	STREAM_DELTA_MASK (
			quint32, 7,

			Tick,
			PosX,
			PosY,
			VelX,
			VelY,
			Angle,
			Facing
			)


	STREAM_DELTA_MEMBER_QUANT(posX, PosX, 0, PosX)
	STREAM_DELTA_MEMBER_QUANT(posY, PosY, 0, PosY)
	STREAM_DELTA_MEMBER_QUANT_SIGNED(velX, VelX, 0, VelX)
	STREAM_DELTA_MEMBER_QUANT_SIGNED(velY, VelY, 0, VelY)
	STREAM_DELTA_MEMBER_QUANT_SIGNED(angle, Angle, 0, Angle)
	STREAM_DELTA_MEMBER_QUANT_SIGNED(facing, Facing, 0, Facing)


	bool operator==(const EntityState &other) const {
		return other.m_posX == m_posX &&
				other.m_posY == m_posY &&
				other.m_velX == m_velX &&
				other.m_velY == m_velY &&
				other.m_angle == m_angle &&
				other.m_facing == m_facing;
	}


	LOAD_FROM_DELTA_START(EntityState)

	LOAD_FROM_DELTA(posX, PosX)
	LOAD_FROM_DELTA(posY, PosY)
	LOAD_FROM_DELTA(velX, VelX)
	LOAD_FROM_DELTA(velY, VelY)
	LOAD_FROM_DELTA(angle, Angle)
	LOAD_FROM_DELTA(facing, Facing)

	LOAD_FROM_DELTA_END
};





/**
 * @brief The PlayerState class
 */

class PlayerState : public BaseTickState
{
public:
	PlayerState() : BaseTickState() {}

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;


	STREAM_DELTA_MASK (
			quint32, 2,

			Hp,
			Mp,

			)

	STREAM_FIELD(EntityState, entityState, EntityState, {})

	STREAM_DELTA_MEMBER(ENTITY_HP_TYPE, hp, Hp, ENTITY_HP_BITS, 0, Hp)
	STREAM_DELTA_MEMBER(ENTITY_MP_TYPE, mp, Mp, ENTITY_MP_BITS, 0, Mp)

	bool operator==(const PlayerState &other) const {
		return other.m_entityState == m_entityState &&
				other.m_hp == m_hp &&
				other.m_mp == m_mp;
	}


	LOAD_FROM_DELTA_START(PlayerState)

	LOAD_WITHOUT_DELTA(tick, Tick)
	LOAD_FROM_DELTA(hp, Hp)
	LOAD_FROM_DELTA(mp, Mp)

	LOAD_FROM_DELTA_MEMBER(entityState)

	LOAD_FROM_DELTA_END
};




/**
 * @brief The PlayerStateList class
 */

class PlayerStateList
{
public:
	PlayerStateList() = default;

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;

	STREAM_ADD_DELTA_MODE

	STREAM_MEMBER(TAG_ID_TYPE, tagId, TagId, TAG_ID_BITS, 0);
	STREAM_DELTA_MEMBER_VECTOR(PlayerState, state, State, STATE_LIST_TYPE, STATE_LIST_BITS)
};








/**
 * @brief The TowerState class
 */

class TowerState : public BaseTickState
{
public:
	TowerState() : BaseTickState() {}

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;


	STREAM_MEMBER_CAST(Team, team, Team, quint8, 2, TeamNone)
	STREAM_MEMBER(quint8, load, Load, 8, 0)
	STREAM_MEMBER(quint32, lockedUntil, LockedUntil, 32, 0)
	STREAM_MEMBER_CAST(bool, active, Active, quint8, 1, false)

	bool operator==(const TowerState &other) const {
		return other.m_team == m_team &&
				other.m_load == m_load &&
				other.m_lockedUntil == m_lockedUntil &&
				other.m_active == m_active;
	}
};





/**
 * @brief The TowerStateList class
 */

class TowerStateList
{
public:
	TowerStateList() = default;

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;

	STREAM_ADD_DELTA_MODE

	STREAM_MEMBER(TAG_ID_TYPE, tagId, TagId, TAG_ID_BITS, 0);
	STREAM_MEMBER_VECTOR(TowerState, state, State, STATE_LIST_TYPE, STATE_LIST_BITS)
};



///
/// EVENTS -------------------------------------------------------------------------
///




/**
 * @brief The EventPlayer class
 */

class EventPlayer : public BaseTickState
{
public:
	enum Type {
		EventNone = 0,
		EventTest,
		EventMpPick,
		EventTower
	};

	EventPlayer() : BaseTickState() {}
	EventPlayer(const Type &type)
		: BaseTickState()
		, m_type(type)
	{}

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;


	STREAM_MEMBER(TAG_ID_TYPE, tagId, TagId, TAG_ID_BITS, 0);
	STREAM_MEMBER_CAST(Type, type, Type, quint32, 4, EventNone)


	STREAM_MEMBER(TAG_ID_TYPE, target, Target, TAG_ID_BITS, 0);
	STREAM_MEMBER_CAST(bool, success, Success, quint8, 1, false);
};





/**
 * @brief The EventPlayer class
 */

class EventMpEmitter : public BaseTickState
{
public:
	EventMpEmitter() : BaseTickState() {}

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;

	STREAM_MEMBER(TAG_ID_TYPE, tagId, TagId, TAG_ID_BITS, 0);
};




/**
 * @brief The EventList class
 */

class Events : public BaseTickState
{
public:
	Events() : BaseTickState() {}

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;

	bool operator==(const Events &) const { return false; }			// soha nem lehet egyenlő, a Pull miatt kell

	STREAM_MEMBER_VECTOR(EventPlayer, player, Player, ENTITY_LIST_TYPE, ENTITY_LIST_BITS)
	STREAM_MEMBER_VECTOR(EventMpEmitter, emitter, Emitter, ENTITY_LIST_TYPE, ENTITY_LIST_BITS)
};














/**
 * @brief The MpData class
 */

class MpData
{
public:
	MpData() = default;

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;


	STREAM_MEMBER(TAG_ID_TYPE, tagId, TagId, TAG_ID_BITS, 0);
	STREAM_MEMBER_QUANT(posX, PosX, 0);
	STREAM_MEMBER_QUANT(posY, PosY, 0);
};





/**
 * @brief The FullState class
 */

class FullState
{
public:
	FullState() = default;

	EngineStream& operator<<(EngineStream &stream);
	EngineStream& operator>>(EngineStream &stream) const;

	enum Flag {
		Null			= 0,
		Player			= 1 << 0,
		Event			= 1 << 1,
		Mp				= 1 << 2,
		Tower			= 1 << 3,
	};

	Q_DECLARE_FLAGS(Flags, Flag)

	STREAM_ADD_DELTA_MODE

	STREAM_MEMBER_CAST(Flags, flags, Flags, quint32, 4, Null)
	STREAM_MEMBER(quint32, serverAuthTick, ServerAuthTick, 32, 0)

	STREAM_FIELD(GameState, state, State, {})

	STREAM_MEMBER_VECTOR(Events, events, Events, ENTITY_LIST_TYPE, ENTITY_LIST_BITS)
	STREAM_MEMBER_VECTOR(PlayerStateList, players, Players, ENTITY_LIST_TYPE, ENTITY_LIST_BITS)
	STREAM_MEMBER_VECTOR(MpData, mps, Mps, ENTITY_LIST_TYPE, ENTITY_LIST_BITS)
	STREAM_MEMBER_VECTOR(TowerStateList, towers, Towers, ENTITY_LIST_TYPE, ENTITY_LIST_BITS)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FullState::Flags)

}		// end of namespace

#endif // RPGSTREAM_H
