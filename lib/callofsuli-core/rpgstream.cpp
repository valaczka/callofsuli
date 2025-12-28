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

		if (m_hasAuthKey)
			this->authBuffer(m_authKey);

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
	readCompleted(stream);

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
	writeCompleted(stream);

	return stream;
}





};


