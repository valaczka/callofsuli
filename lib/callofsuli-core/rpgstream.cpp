/*
 * ---- Call of Suli ----
 *
 * rpgstream.cpp
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


#include "rpgstream.h"


namespace RpgStream
{



/**
 * @brief EngineList::toStream
 * @return
 */

EngineStream EngineList::toStream() const
{
	EngineStream stream(EngineStream::OperationList);

	*this >> stream;

	return stream;
}




/**
 * @brief EngineStream::data
 * @return
 */

std::vector<uint8_t> EngineStream::data() const
{
	finalize();

	return UdpBitStream::data();
}


/**
 * @brief EngineStream::EngineStream
 * @param other
 */

EngineStream::EngineStream(UdpBitStream &&other)
	: UdpBitStream(std::move(other))
{
	LOG_CINFO("engine") << "GET ENGINE STREAM" << this;

	readOperation(*this);
	setVersion(readVersion(*this));

	LOG_CINFO("engine") << "OPERATION" << m_operation << "v" << m_version;
}



EngineStream::EngineStream(std::unique_ptr<UdpBitStream> &stream)
	: UdpBitStream(std::move(*stream.release()))
{
	LOG_CINFO("engine") << "GET ENGINE STREAM" << this;

	readOperation(*this);
	setVersion(readVersion(*this));

	LOG_CINFO("engine") << "OPERATION" << m_operation << "v" << m_version;;
}


/**
 * @brief EngineStream::finalize
 */

void EngineStream::finalize() const
{
	if (!m_hasFinalized) {
		// Fill stream
		m_stream.writeBit(0, true);
		m_stream.write<uint8_t>(0, true);

		if (m_signer.has_value())
			this->authBuffer(m_signer.value());

		m_hasFinalized = true;
	}
}



/**
 * @brief EnginePlayer::operator <<
 * @param stream
 * @return
 */

EnginePlayer::EnginePlayer(const QByteArray &userName, const QByteArray &nickName)
	: RpgStream::EnginePlayer()
{
	setUserName(userName);
	setNickName(nickName);
}


EngineStream &EnginePlayer::operator<<(EngineStream &stream)
{
	readUserName(stream);
	readNickName(stream);

	return stream;
}



/**
 * @brief EnginePlayer::operator >>
 * @param stream
 * @return
 */

EngineStream &EnginePlayer::operator>>(EngineStream &stream) const
{
	writeUserName(stream);
	writeNickName(stream);

	return stream;
}



/**
 * @brief EngineList::operator <<
 * @param stream
 * @return
 */

EngineStream &EngineList::operator<<(EngineStream &stream)
{
	readCanCreate(stream);
	readEngines(stream);
	return stream;
}


/**
 * @brief EngineList::operator >>
 * @param stream
 * @return
 */

EngineStream &EngineList::operator>>(EngineStream &stream) const
{
	writeCanCreate(stream);
	writeEngines(stream);
	return stream;
}



/**
 * @brief Engine::readPlayers
 * @param stream
 * @return
 */

EngineStream &Engine::operator<<(EngineStream &stream)
{
	readId(stream);
	readReadableId(stream);
	readMaxPlayer(stream);
	m_owner << stream;
	readPlayers(stream);

	return stream;
}



/**
 * @brief Engine::operator >>
 * @param stream
 * @return
 */

EngineStream &Engine::operator>>(EngineStream &stream) const
{
	writeId(stream);
	writeReadableId(stream);
	writeMaxPlayer(stream);
	m_owner >> stream;
	writePlayers(stream);

	return stream;
}



/**
 * @brief GameConfig::operator <<
 * @param stream
 * @return
 */

EngineStream &GameConfig::operator<<(EngineStream &stream)
{
	readTerrain(stream);
	readFlags(stream);
	readDuration(stream);

	return stream;
}


/**
 * @brief GameConfig::operator >>
 * @param stream
 * @return
 */

EngineStream &GameConfig::operator>>(EngineStream &stream) const
{
	writeTerrain(stream);
	writeFlags(stream);
	writeDuration(stream);

	return stream;
}



/**
 * @brief CharacterSelectServer::operator <<
 * @param stream
 * @return
 */

EngineStream &CharacterSelectServer::operator<<(EngineStream &stream)
{
	m_gameConfig << stream;
	readEngineReadableId(stream);
	readMaxPlayers(stream);
	readPlayers(stream);

	return stream;
}


/**
 * @brief CharacterSelectServer::operator >>
 * @param stream
 * @return
 */

EngineStream &CharacterSelectServer::operator>>(EngineStream &stream) const
{
	m_gameConfig >> stream;
	writeEngineReadableId(stream);
	writeMaxPlayers(stream);
	writePlayers(stream);

	return stream;
}


/**
 * @brief PlayerData::operator <<
 * @param stream
 * @return
 */

EngineStream &PlayerData::operator<<(EngineStream &stream)
{
	readPlayerId(stream);
	readUserName(stream);
	readNickName(stream);
	readCharacter(stream);
	readFlags(stream);

	return stream;
}


/**
 * @brief PlayerData::operator >>
 * @param stream
 * @return
 */

EngineStream &PlayerData::operator>>(EngineStream &stream) const
{
	writePlayerId(stream);
	writeUserName(stream);
	writeNickName(stream);
	writeCharacter(stream);
	writeFlags(stream);

	return stream;
}


/**
 * @brief EntityState::operator <<
 * @param stream
 * @return
 */

EngineStream &EntityState::operator<<(EngineStream &stream)
{
	readDeltaMask(stream);
	readPosXDelta(stream);
	readPosYDelta(stream);
	readVelXDelta(stream);
	readVelYDelta(stream);
	readAngleDelta(stream);
	readFacingDelta(stream);

	return stream;
}


/**
 * @brief EntityState::operator >>
 * @param stream
 * @return
 */

EngineStream &EntityState::operator>>(EngineStream &stream) const
{
	writeDeltaMask(stream);
	writePosXDelta(stream);
	writePosYDelta(stream);
	writeVelXDelta(stream);
	writeVelYDelta(stream);
	writeAngleDelta(stream);
	writeFacingDelta(stream);

	return stream;
}


/**
 * @brief PlayerState::operator <<
 * @param stream
 * @return
 */

EngineStream &PlayerState::operator<<(EngineStream &stream)
{
	readTick(stream);
	readDeltaMask(stream);
	m_entityState.setIsDeltaMode(m_isDeltaMode);
	m_entityState << stream;

	readHpDelta(stream);

	return stream;
}


/**
 * @brief PlayerState::operator >>
 * @param stream
 * @return
 */

EngineStream &PlayerState::operator>>(EngineStream &stream) const
{
	writeTick(stream);
	writeDeltaMask(stream);
	m_entityState.setIsDeltaMode(m_isDeltaMode);
	m_entityState >> stream;

	writeHpDelta(stream);

	return stream;
}



/**
 * @brief Chunk::operator <<
 * @param stream
 * @return
 */

EngineStream &Chunk::operator<<(EngineStream &stream)
{
	readX(stream);
	readY(stream);

	return stream;
}


/**
 * @brief Chunk::operator >>
 * @param stream
 * @return
 */

EngineStream &Chunk::operator>>(EngineStream &stream) const
{
	writeX(stream);
	writeY(stream);

	return stream;
}


/**
 * @brief ChunkGrid::operator <<
 * @param stream
 * @return
 */

EngineStream &ChunkGrid::operator<<(EngineStream &stream)
{
	readViewportX(stream);
	readViewportY(stream);
	readViewportWidth(stream);
	readViewportHeight(stream);

	readChunkWidth(stream);
	readChunkHeight(stream);

	readExcludeList(stream);

	return stream;
}


/**
 * @brief ChunkGrid::operator >>
 * @param stream
 * @return
 */

EngineStream &ChunkGrid::operator>>(EngineStream &stream) const
{
	writeViewportX(stream);
	writeViewportY(stream);
	writeViewportWidth(stream);
	writeViewportHeight(stream);

	writeChunkWidth(stream);
	writeChunkHeight(stream);

	writeExcludeList(stream);

	return stream;
}


/**
 * @brief PlayerPosition::operator <<
 * @param stream
 * @return
 */

EngineStream &PlayerPosition::operator<<(EngineStream &stream)
{
	readPosX(stream);
	readPosY(stream);
	readTeam(stream);

	return stream;
}



/**
 * @brief PlayerPosition::operator >>
 * @param stream
 * @return
 */

EngineStream &PlayerPosition::operator>>(EngineStream &stream) const
{
	writePosX(stream);
	writePosY(stream);
	writeTeam(stream);

	return stream;
}


/**
 * @brief PlayerPositionList::operator <<
 * @param stream
 * @return
 */

EngineStream &MapData::operator<<(EngineStream &stream)
{
	m_chunkGrid << stream;
	readPlayerPositionList(stream);
	readMpEmitterList(stream);

	return stream;
}


/**
 * @brief PlayerPositionList::operator >>
 * @param stream
 * @return
 */

EngineStream &MapData::operator>>(EngineStream &stream) const
{
	m_chunkGrid >> stream;
	writePlayerPositionList(stream);
	writeMpEmitterList(stream);

	return stream;
}


/**
 * @brief PlayerStateList::operator <<
 * @param stream
 * @return
 */

EngineStream &PlayerStateList::operator<<(EngineStream &stream)
{
	readTagId(stream);
	readStateVectorDelta(stream, m_isDeltaMode);

	return stream;
}


/**
 * @brief PlayerStateList::operator >>
 * @param stream
 * @return
 */

EngineStream &PlayerStateList::operator>>(EngineStream &stream) const
{
	writeTagId(stream);
	writeStateVectorDelta(stream, m_isDeltaMode);

	return stream;
}


/**
 * @brief PlayerStateEntityList::operator <<
 * @param stream
 * @return
 */

EngineStream &PlayerStateEntityList::operator<<(EngineStream &stream)
{
	readList(stream);

	return stream;
}



/**
 * @brief PlayerStateEntityList::operator >>
 * @param stream
 * @return
 */

EngineStream &PlayerStateEntityList::operator>>(EngineStream &stream) const
{
	writeList(stream);

	return stream;
}


/**
 * @brief FullState::operator <<
 * @param stream
 * @return
 */

EngineStream &FullState::operator<<(EngineStream &stream)
{
	readFlags(stream);

	if (m_flags & Player)
		m_players << stream;

	if (m_flags & Events)
		m_events << stream;

	return stream;
}


/**
 * @brief FullState::operator >>
 * @param stream
 * @return
 */

EngineStream &FullState::operator>>(EngineStream &stream) const
{
	writeFlags(stream);

	if (m_flags & Player)
		m_players >> stream;

	if (m_flags & Events)
		m_events >> stream;

	return stream;
}


/**
 * @brief EventList::operator <<
 * @param stream
 * @return
 */

EngineStream &EventList::operator<<(EngineStream &stream)
{
	readFlags(stream);

	if (m_flags & Player)
		readPlayerList(stream);

	return stream;
}


/**
 * @brief EventList::operator >>
 * @param stream
 * @return
 */

EngineStream &EventList::operator>>(EngineStream &stream) const
{
	writeFlags(stream);

	if (m_flags & Player)
		writePlayerList(stream);

	return stream;
}


/**
 * @brief EventPlayerList::operator <<
 * @param stream
 * @return
 */

EngineStream &EventPlayerList::operator<<(EngineStream &stream)
{
	readTagId(stream);
	readList(stream);

	return stream;
}


/**
 * @brief EventPlayerList::operator >>
 * @param stream
 * @return
 */

EngineStream &EventPlayerList::operator>>(EngineStream &stream) const
{
	writeTagId(stream);
	writeList(stream);

	return stream;
}


/**
 * @brief EventPlayer::operator <<
 * @param stream
 * @return
 */

EngineStream &EventPlayer::operator<<(EngineStream &stream)
{
	readType(stream);

	return stream;
}


/**
 * @brief EventPlayer::operator >>
 * @param stream
 * @return
 */

EngineStream &EventPlayer::operator>>(EngineStream &stream) const
{
	writeType(stream);

	return stream;
}



/**
 * @brief MpEmitter::operator <<
 * @param stream
 * @return
 */

EngineStream &MpEmitter::operator<<(EngineStream &stream)
{
	readPosX(stream);
	readPosY(stream);

	return stream;
}


/**
 * @brief MpEmitter::operator >>
 * @param stream
 * @return
 */

EngineStream &MpEmitter::operator>>(EngineStream &stream) const
{
	writePosX(stream);
	writePosY(stream);

	return stream;
}




};


