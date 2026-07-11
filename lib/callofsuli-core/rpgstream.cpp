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
 * @brief BaseDefenderObject::requiredMp
 * @param type
 * @return
 */

quint32 BaseDefenderObject::requiredMp(const Type &type) {
	switch (type) {
		case Multiplier1:				return 1;
		case Fog:						return 1;
		case Pulse:						return 1;
		case None:						return 0;
	}

	return 0;
}



/**
 * @brief BaseDefenderObject::placementFlags
 * @param type
 * @return
 */

BaseDefenderObject::PlacementFlags BaseDefenderObject::placementFlags(const Type &type)
{
	switch (type) {
		case Fog:
		case Pulse:
			return PlacementTower | PlacementChunk;

		case Multiplier1:
			return PlacementTower;

		case None:
			return PlacementNone;

	}

	return PlacementNone;
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
	readOperation(*this);
	setVersion(readVersion(*this));
}



EngineStream::EngineStream(std::unique_ptr<UdpBitStream> &stream)
	: UdpBitStream(std::move(*stream.release()))
{
	readOperation(*this);
	setVersion(readVersion(*this));
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
	readTeam(stream);

	m_config << stream;

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
	writeTeam(stream);

	m_config >> stream;

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
	readVelSqDelta(stream);
	readAngleDelta(stream);
	readFacingDelta(stream);
	readSlideXDelta(stream);
	readSlideYDelta(stream);

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
	writeVelSqDelta(stream);
	writeAngleDelta(stream);
	writeFacingDelta(stream);
	writeSlideXDelta(stream);
	writeSlideYDelta(stream);

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
	readBulletDelta(stream);
	readLockDelta(stream);
	readPenaltyDelta(stream);

	readDefenderDelta(stream);
	readHasDefenderDelta(stream);

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
	writeBulletDelta(stream);
	writeLockDelta(stream);
	writePenaltyDelta(stream);

	writeDefenderDelta(stream);
	writeHasDefenderDelta(stream);

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
	readTowerList(stream);
	readChestPositionList(stream);

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
	writeTowerList(stream);
	writeChestPositionList(stream);

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
	readServerTick(stream);
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

	if (m_flags & Defender)
		readDefenders(stream);

	if (m_flags & Npc)
		readNpcs(stream);

	if (m_flags & Control)
		readControls(stream);

	return stream;
}


/**
 * @brief FullState::operator >>
 * @param stream
 * @return
 */

EngineStream &FullState::operator>>(EngineStream &stream) const
{
	writeServerTick(stream);
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

	if (m_flags & Defender)
		writeDefenders(stream);

	if (m_flags & Npc)
		writeNpcs(stream);

	if (m_flags & Control)
		writeControls(stream);

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
	readFlags(stream);

	if (m_flags & Player)
		readPlayer(stream);

	if (m_flags & Emitter)
		readEmitter(stream);

	if (m_flags & Stage)
		readStage(stream);

	if (m_flags & Npc)
		readNpc(stream);

	if (m_flags & Defender)
		readDefender(stream);

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
	writeFlags(stream);

	if (m_flags & Player)
		writePlayer(stream);

	if (m_flags & Emitter)
		writeEmitter(stream);

	if (m_flags & Stage)
		writeStage(stream);

	if (m_flags & Npc)
		writeNpc(stream);

	if (m_flags & Defender)
		writeDefender(stream);

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
	readSeq(stream);
	readType(stream);
	readLockId(stream);

	if (m_type == EventMpPick ||
			m_type == EventTower ||
			m_type == EventDefender ||
			m_type == EventAttackPlayer ||
			m_type == EventAttackDefender ||
			m_type == EventRespawn
			) {
		readTarget(stream);
	}

	if (m_type == EventDefender && m_target == 0) {
		m_chunk << stream;
	}

	if (m_type == EventRespawn ||
			m_type == EventStreak)				// itt a streak-et tároljuk nem az msec-t!
		readAt(stream);

	if (m_type == EventAttackPlayer)
		readSuccess(stream);

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
	writeSeq(stream);
	writeType(stream);
	writeLockId(stream);

	if (m_type == EventMpPick ||
			m_type == EventTower ||
			m_type == EventDefender ||
			m_type == EventAttackPlayer ||
			m_type == EventAttackDefender ||
			m_type == EventRespawn
			) {
		writeTarget(stream);
	}

	if (m_type == EventDefender && m_target == 0) {
		m_chunk >> stream;
	}

	if (m_type == EventRespawn ||
			m_type == EventStreak)
		writeAt(stream);

	if (m_type == EventAttackPlayer)
		writeSuccess(stream);

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
	readActive(stream);

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
	writeActive(stream);

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
	readOrigX(stream);
	readOrigY(stream);

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
	writeOrigX(stream);
	writeOrigY(stream);

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
	readPosX(stream);
	readPosY(stream);
	readActive(stream);

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
	writePosX(stream);
	writePosY(stream);
	writeActive(stream);

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
	readDefenders(stream);
	readMultiply(stream);

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
	writeDefenders(stream);
	writeMultiply(stream);

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

	readPosX(stream);
	readPosY(stream);

	/*switch (m_type) {
		case BaseDefenderObject::Dummy:
			readDummy(stream);
			break;

		case BaseDefenderObject::Multiplier1:
		case BaseDefenderObject::Fog:
		case BaseDefenderObject::Pulse:
		case BaseDefenderObject::None:
			break;
	}*/

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

	writePosX(stream);
	writePosY(stream);

	/*switch (m_type) {
		case BaseDefenderObject::Dummy:
			writeDummy(stream);
			break;

		case BaseDefenderObject::Multiplier1:
		case BaseDefenderObject::Fog:
		case BaseDefenderObject::Pulse:
		case BaseDefenderObject::None:
			break;
	}*/


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
	readVisible(stream);
	readTargetId(stream);

/*
	switch (m_type) {
		case BaseDefenderObject::Dummy:
			readDummy(stream);
			break;

		case BaseDefenderObject::Multiplier1:
		case BaseDefenderObject::Fog:
		case BaseDefenderObject::Pulse:
		case BaseDefenderObject::None:
			break;
	}
*/

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
	writeVisible(stream);
	writeTargetId(stream);

	/*
	switch (m_type) {
		case BaseDefenderObject::Dummy:
			writeDummy(stream);
			break;

		case BaseDefenderObject::Multiplier1:
		case BaseDefenderObject::Fog:
		case BaseDefenderObject::Pulse:
		case BaseDefenderObject::None:
			break;
	}
	*/

	return stream;
}



/**
 * @brief PlayerConfig::operator <<
 * @param stream
 * @return
 */

EngineStream &PlayerConfig::operator<<(EngineStream &stream)
{
	readPower(stream);
	readMaxMp(stream);
	readMaxBullet(stream);

	m_entity << stream;

	readDefenders(stream);

	return stream;
}


/**
 * @brief PlayerConfig::operator >>
 * @param stream
 * @return
 */

EngineStream &PlayerConfig::operator>>(EngineStream &stream) const
{
	writePower(stream);
	writeMaxMp(stream);
	writeMaxBullet(stream);

	m_entity >> stream;

	writeDefenders(stream);

	return stream;
}




/**
 * @brief EntityConfig::operator <<
 * @param stream
 * @return
 */

EngineStream &EntityConfig::operator<<(EngineStream &stream)
{
	readMaxHp(stream);
	readPush(stream);
	readResist(stream);
	readPushDist(stream);

	return stream;
}




/**
 * @brief EntityConfig::operator >>
 * @param stream
 * @return
 */

EngineStream &EntityConfig::operator>>(EngineStream &stream) const
{
	writeMaxHp(stream);
	writePush(stream);
	writeResist(stream);
	writePushDist(stream);

	return stream;
}



/**
 * @brief RoomList::operator <<
 * @param stream
 * @return
 */

EngineStream &RoomList::operator<<(EngineStream &stream)
{
	readCanCreate(stream);
	readRooms(stream);

	return stream;
}


/**
 * @brief RoomList::operator >>
 * @param stream
 * @return
 */

EngineStream &RoomList::operator>>(EngineStream &stream) const
{
	writeCanCreate(stream);
	writeRooms(stream);

	return stream;
}



/**
 * @brief RoomList::toStream
 * @return
 */

EngineStream RoomList::toStream() const
{
	EngineStream stream(EngineStream::OperationList);

	*this >> stream;

	return stream;
}



/**
 * @brief Room::operator <<
 * @param stream
 * @return
 */

EngineStream &Room::operator<<(EngineStream &stream)
{
	readId(stream);
	readReadableId(stream);
	readHostId(stream);
	readPlayers(stream);

	return stream;
}

EngineStream &Room::operator>>(EngineStream &stream) const
{
	writeId(stream);
	writeReadableId(stream);
	writeHostId(stream);
	writePlayers(stream);

	return stream;
}


/**
 * @brief CharacterSelectServer::operator <<
 * @param stream
 * @return
 */

EngineStream &CharacterSelectServer::operator<<(EngineStream &stream)
{
	m_room << stream;
	m_gameConfig << stream;

	return stream;
}



EngineStream &CharacterSelectServer::operator>>(EngineStream &stream) const
{
	m_room >> stream;
	m_gameConfig >> stream;

	return stream;
}



/**
 * @brief CharacterSelectClient::operator <<
 * @param stream
 * @return
 */

EngineStream &CharacterSelectClient::operator<<(EngineStream &stream)
{
	m_gameConfig << stream;
	m_data << stream;

	return stream;
}


/**
 * @brief CharacterSelectClient::operator >>
 * @param stream
 * @return
 */

EngineStream &CharacterSelectClient::operator>>(EngineStream &stream) const
{
	m_gameConfig >> stream;
	m_data >> stream;

	return stream;
}



/**
 * @brief Full::operator <<
 * @param stream
 * @return
 */

EngineStream &Full::operator<<(EngineStream &stream)
{
	readServerTick(stream);
	m_config << stream;

	readPlayers(stream);
	readMpEmitters(stream);
	readTowers(stream);
	readDefenders(stream);
	readNpcs(stream);
	readControls(stream);

	readMap(stream);
	m_fullState << stream;

	return stream;
}



/**
 * @brief Full::operator >>
 * @param stream
 * @return
 */

EngineStream &Full::operator>>(EngineStream &stream) const
{
	writeServerTick(stream);
	m_config >> stream;

	writePlayers(stream);
	writeMpEmitters(stream);
	writeTowers(stream);
	writeDefenders(stream);
	writeNpcs(stream);
	writeControls(stream);

	writeMap(stream);
	m_fullState >> stream;

	return stream;
}


/**
 * @brief FullPlayerMap::operator <<
 * @param stream
 * @return
 */

EngineStream &FullPlayerMap::operator<<(EngineStream &stream)
{
	readPeerId(stream);
	readPlayer(stream);
	readEntities(stream);

	return stream;
}


/**
 * @brief FullPlayerMap::operator >>
 * @param stream
 * @return
 */

EngineStream &FullPlayerMap::operator>>(EngineStream &stream) const
{
	writePeerId(stream);
	writePlayer(stream);
	writeEntities(stream);

	return stream;
}


/**
 * @brief FullMapTag::operator <<
 * @param stream
 * @return
 */

EngineStream &FullMapTag::operator<<(EngineStream &stream)
{
	readTagId(stream);

	return stream;
}


/**
 * @brief FullMapTag::operator >>
 * @param stream
 * @return
 */

EngineStream &FullMapTag::operator>>(EngineStream &stream) const
{
	writeTagId(stream);

	return stream;
}


/**
 * @brief EventStageChanged::operator <<
 * @param stream
 * @return
 */

EngineStream &EventStageChanged::operator<<(EngineStream &stream)
{
	m_config << stream;

	return stream;
}


/**
 * @brief EventStageChanged::operator >>
 * @param stream
 * @return
 */

EngineStream &EventStageChanged::operator>>(EngineStream &stream) const
{
	m_config >> stream;

	return stream;
}


/**
 * @brief NpcData::operator <<
 * @param stream
 * @return
 */

EngineStream &NpcData::operator<<(EngineStream &stream)
{
	readTagId(stream);
	readType(stream);
	m_entity << stream;
	readCharacter(stream);
	readTeam(stream);

	if (m_type == TowerAttacker)
		readForce(stream);

	return stream;
}


/**
 * @brief NpcData::operator >>
 * @param stream
 * @return
 */

EngineStream &NpcData::operator>>(EngineStream &stream) const
{
	writeTagId(stream);
	writeType(stream);
	m_entity >> stream;
	writeCharacter(stream);
	writeTeam(stream);

	if (m_type == TowerAttacker)
		writeForce(stream);

	return stream;
}



/**
 * @brief NpcState::operator <<
 * @param stream
 * @return
 */

EngineStream &NpcState::operator<<(EngineStream &stream)
{
	readTick(stream);
	readType(stream);
	readDeltaMask(stream);

	readHpDelta(stream);
	readTargetDelta(stream);

	m_entityState.setIsDeltaMode(m_isDeltaMode);
	m_entityState << stream;

	if (m_type == NpcData::TowerAttacker) {
		readDestinationTowerDelta(stream);
		readDestinationXDelta(stream);
		readDestinationYDelta(stream);
	}

	return stream;
}



/**
 * @brief NpcState::operator >>
 * @param stream
 * @return
 */

EngineStream &NpcState::operator>>(EngineStream &stream) const
{
	writeTick(stream);
	writeType(stream);

	writeDeltaMask(stream);

	writeHpDelta(stream);
	writeTargetDelta(stream);

	m_entityState.setIsDeltaMode(m_isDeltaMode);
	m_entityState >> stream;

	if (m_type == NpcData::TowerAttacker) {
		writeDestinationTowerDelta(stream);
		writeDestinationXDelta(stream);
		writeDestinationYDelta(stream);
	}

	return stream;
}


/**
 * @brief NpcStateList::operator <<
 * @param stream
 * @return
 */

EngineStream &NpcStateList::operator<<(EngineStream &stream)
{
	readTagId(stream);
	readStateVectorDelta(stream, m_isDeltaMode);

	return stream;
}


/**
 * @brief NpcStateList::operator >>
 * @param stream
 * @return
 */

EngineStream &NpcStateList::operator>>(EngineStream &stream) const
{
	writeTagId(stream);
	writeStateVectorDelta(stream, m_isDeltaMode);

	return stream;
}



/**
 * @brief EventNpc::operator <<
 * @param stream
 * @return
 */

EngineStream &EventNpc::operator<<(EngineStream &stream)
{
	readTick(stream);
	readSeq(stream);
	readTagId(stream);
	readType(stream);

	if (m_type == EventAttack)
		readTargetId(stream);

	return stream;
}


/**
 * @brief EventNpc::operator >>
 * @param stream
 * @return
 */

EngineStream &EventNpc::operator>>(EngineStream &stream) const
{
	writeTick(stream);
	writeSeq(stream);
	writeTagId(stream);
	writeType(stream);

	if (m_type == EventAttack)
		writeTargetId(stream);

	return stream;
}




/**
 * @brief ControlData::operator <<
 * @param stream
 * @return
 */

EngineStream &ControlData::operator<<(EngineStream &stream)
{
	readTagId(stream);
	readType(stream);
	readPosX(stream);
	readPosY(stream);

	return stream;
}



/**
 * @brief ControlData::operator >>
 * @param stream
 * @return
 */

EngineStream &ControlData::operator>>(EngineStream &stream) const
{
	writeTagId(stream);
	writeType(stream);
	writePosX(stream);
	writePosY(stream);

	return stream;
}



/**
 * @brief ControlState::operator <<
 * @param stream
 * @return
 */

EngineStream &ControlState::operator<<(EngineStream &stream)
{
	readTick(stream);
	readType(stream);
	readActive(stream);

	return stream;
}


/**
 * @brief ControlState::operator >>
 * @param stream
 * @return
 */

EngineStream &ControlState::operator>>(EngineStream &stream) const
{
	writeTick(stream);
	writeType(stream);
	writeActive(stream);

	return stream;
}



/**
 * @brief ControlEvent::operator <<
 * @param stream
 * @return
 */

EngineStream &EventControl::operator<<(EngineStream &stream)
{
	readTick(stream);
	readTagId(stream);
	readType(stream);

	return stream;
}



/**
 * @brief ControlEvent::operator >>
 * @param stream
 * @return
 */

EngineStream &EventControl::operator>>(EngineStream &stream) const
{
	writeTick(stream);
	writeTagId(stream);
	writeType(stream);

	return stream;
}




/**
 * @brief ControlStateList::operator <<
 * @param stream
 * @return
 */

EngineStream &ControlStateList::operator<<(EngineStream &stream)
{
	readTagId(stream);
	readState(stream);

	return stream;
}




/**
 * @brief ControlStateList::operator >>
 * @param stream
 * @return
 */

EngineStream &ControlStateList::operator>>(EngineStream &stream) const
{
	writeTagId(stream);
	writeState(stream);

	return stream;
}


/**
 * @brief EventDefender::operator <<
 * @param stream
 * @return
 */

EngineStream &EventDefender::operator<<(EngineStream &stream)
{
	readTick(stream);
	readTagId(stream);
	readType(stream);
	readTargetId(stream);

	return stream;
}


/**
 * @brief EventDefender::operator >>
 * @param stream
 * @return
 */

EngineStream &EventDefender::operator>>(EngineStream &stream) const
{
	writeTick(stream);
	writeTagId(stream);
	writeType(stream);
	writeTargetId(stream);

	return stream;
}




};


