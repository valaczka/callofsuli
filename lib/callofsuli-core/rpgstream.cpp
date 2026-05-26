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
	readStage(stream);

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
	writeStage(stream);

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
	readMpDelta(stream);

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
	writeMpDelta(stream);

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
 * @brief FullState::operator <<
 * @param stream
 * @return
 */

EngineStream &FullState::operator<<(EngineStream &stream)
{
	readServerAuthTick(stream);
	readFlags(stream);

	m_state << stream;

	if (m_flags & Player)
		readPlayers(stream);

	if (m_flags & Event)
		readEvents(stream);

	if (m_flags & Mp)
		readMps(stream);

	if (m_flags & Tower)
		readTowers(stream);

	return stream;
}


/**
 * @brief FullState::operator >>
 * @param stream
 * @return
 */

EngineStream &FullState::operator>>(EngineStream &stream) const
{
	writeServerAuthTick(stream);
	writeFlags(stream);

	m_state >> stream;

	if (m_flags & Player)
		writePlayers(stream);

	if (m_flags & Event)
		writeEvents(stream);

	if (m_flags & Mp)
		writeMps(stream);

	if (m_flags & Tower)
		writeTowers(stream);

	return stream;
}


/**
 * @brief EventList::operator <<
 * @param stream
 * @return
 */

EngineStream &Events::operator<<(EngineStream &stream)
{
	readTick(stream);
	readPlayer(stream);
	readEmitter(stream);

	return stream;
}


/**
 * @brief EventList::operator >>
 * @param stream
 * @return
 */

EngineStream &Events::operator>>(EngineStream &stream) const
{
	writeTick(stream);
	writePlayer(stream);
	writeEmitter(stream);

	return stream;
}


/**
 * @brief EventPlayer::operator <<
 * @param stream
 * @return
 */

EngineStream &EventPlayer::operator<<(EngineStream &stream)
{
	readTick(stream);
	readTagId(stream);
	readType(stream);

	if (m_type == EventMpPick ||
			m_type == EventTower ||
			m_type == EventDefender ||
			m_type == EventAttackPlayer ||
			m_type == EventAttackDefender
			) {
		readTarget(stream);
	}

	if (m_type == EventDefender && m_tagId == 0) {
		m_chunk << stream;
	}

	if (m_type == EventTower) {
		readSuccess(stream);
	}

	return stream;
}


/**
 * @brief EventPlayer::operator >>
 * @param stream
 * @return
 */

EngineStream &EventPlayer::operator>>(EngineStream &stream) const
{
	writeTick(stream);
	writeTagId(stream);
	writeType(stream);

	if (m_type == EventMpPick ||
			m_type == EventTower ||
			m_type == EventDefender ||
			m_type == EventAttackPlayer ||
			m_type == EventAttackDefender
			) {
		writeTarget(stream);
	}

	if (m_type == EventDefender && m_tagId == 0) {
		m_chunk >> stream;
	}

	if (m_type == EventTower) {
		writeSuccess(stream);
	}

	return stream;
}



/**
 * @brief MpEmitter::operator <<
 * @param stream
 * @return
 */

EngineStream &MpEmitter::operator<<(EngineStream &stream)
{
	readTagId(stream);
	readPosX(stream);
	readPosY(stream);
	readRadius(stream);
	readCapacity(stream);

	return stream;
}


/**
 * @brief MpEmitter::operator >>
 * @param stream
 * @return
 */

EngineStream &MpEmitter::operator>>(EngineStream &stream) const
{
	writeTagId(stream);
	writePosX(stream);
	writePosY(stream);
	writeRadius(stream);
	writeCapacity(stream);

	return stream;
}




/**
 * @brief MpData::operator <<
 * @param stream
 * @return
 */

EngineStream &MpData::operator<<(EngineStream &stream)
{
	readTagId(stream);
	readPosX(stream);
	readPosY(stream);

	return stream;
}


/**
 * @brief MpData::operator >>
 * @param stream
 * @return
 */

EngineStream &MpData::operator>>(EngineStream &stream) const
{
	writeTagId(stream);
	writePosX(stream);
	writePosY(stream);

	return stream;
}



/**
 * @brief Tower::operator <<
 * @param stream
 * @return
 */

EngineStream &Tower::operator<<(EngineStream &stream)
{
	readTagId(stream);
	readDefenders(stream);

	return stream;
}


/**
 * @brief Tower::operator >>
 * @param stream
 * @return
 */

EngineStream &Tower::operator>>(EngineStream &stream) const
{
	writeTagId(stream);
	writeDefenders(stream);

	return stream;
}



/**
 * @brief TowerState::operator <<
 * @param stream
 * @return
 */

EngineStream &TowerState::operator<<(EngineStream &stream)
{
	readTick(stream);
	readTagId(stream);
	readTeam(stream);
	readLoad(stream);
	readLockedUntil(stream);
	readActive(stream);
	readHasDefender(stream);

	return stream;
}



/**
 * @brief TowerState::operator >>
 * @param stream
 * @return
 */

EngineStream &TowerState::operator>>(EngineStream &stream) const
{
	writeTick(stream);
	writeTagId(stream);
	writeTeam(stream);
	writeLoad(stream);
	writeLockedUntil(stream);
	writeActive(stream);
	writeHasDefender(stream);

	return stream;
}






/**
 * @brief EventMpEmitter::operator <<
 * @param stream
 * @return
 */

EngineStream &EventMpEmitter::operator<<(EngineStream &stream)
{
	readTick(stream);
	readTagId(stream);

	return stream;
}


/**
 * @brief EventMpEmitter::operator >>
 * @param stream
 * @return
 */

EngineStream &EventMpEmitter::operator>>(EngineStream &stream) const
{
	writeTick(stream);
	writeTagId(stream);

	return stream;
}



/**
 * @brief GameState::operator <<
 * @param stream
 * @return
 */

EngineStream &GameState::operator<<(EngineStream &stream)
{
	readTick(stream);
	readPtsA(stream);
	readPtsB(stream);

	return stream;
}



/**
 * @brief GameState::operator >>
 * @param stream
 * @return
 */

EngineStream &GameState::operator>>(EngineStream &stream) const
{
	writeTick(stream);
	writePtsA(stream);
	writePtsB(stream);

	return stream;
}


/**
 * @brief Defender::operator <<
 * @param stream
 * @return
 */

EngineStream &Defender::operator<<(EngineStream &stream)
{
	readTagId(stream);
	readPosX(stream);
	readPosY(stream);

	return stream;
}




/**
 * @brief Defender::operator >>
 * @param stream
 * @return
 */

EngineStream &Defender::operator>>(EngineStream &stream) const
{
	writeTagId(stream);
	writePosX(stream);
	writePosY(stream);

	return stream;
}


/**
 * @brief BaseDefenderObject::operator <<
 * @param stream
 * @return
 */

EngineStream &BaseDefenderObject::operator<<(EngineStream &stream)
{
	readTagId(stream);
	readType(stream);

	readTeam(stream);
	readMaxHp(stream);
	readDefenderId(stream);

	if (m_defenderId == 0u)
		m_chunk << stream;


	switch (m_type) {
		case BaseDefenderObject::Dummy:
			readDummy(stream);
			break;
	}

	return stream;
}



/**
 * @brief BaseDefenderObject::operator >>
 * @param stream
 * @return
 */

EngineStream &BaseDefenderObject::operator>>(EngineStream &stream) const
{
	writeTagId(stream);
	writeType(stream);

	writeTeam(stream);
	writeMaxHp(stream);
	writeDefenderId(stream);

	if (m_defenderId == 0u)
		m_chunk >> stream;

	switch (m_type) {
		case BaseDefenderObject::Dummy:
			writeDummy(stream);
			break;
	}


	return stream;
}







/**
 * @brief DefenderState::operator <<
 * @param stream
 * @return
 */

EngineStream &DefenderState::operator<<(EngineStream &stream)
{
	readTick(stream);
	readTagId(stream);
	readType(stream);
	readHp(stream);


	switch (m_type) {
		case BaseDefenderObject::Dummy:
			readDummy(stream);
			break;
	}


	return stream;
}


/**
 * @brief DefenderObjectState::operator >>
 * @param stream
 * @return
 */

EngineStream &DefenderState::operator>>(EngineStream &stream) const
{
	writeTick(stream);
	writeTagId(stream);
	writeType(stream);
	writeHp(stream);

	switch (m_type) {
		case BaseDefenderObject::Dummy:
			writeDummy(stream);
			break;
	}

	return stream;
}




};


