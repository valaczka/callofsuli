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
 * @brief Engine::readPlayers
 * @param stream
 * @return
 */

void Engine::readPlayers(UdpBitStream &stream)
{
	m_players.clear();

	quint8 size = readBits(stream, 3, 0);

	m_players.reserve(size);

	for (quint8 i=0; i<size; ++i) {
		EnginePlayer p;
		p.readUserName(stream);
		p.readNickName(stream);
		m_players.emplace_back(std::move(p));
	}
}


/**
 * @brief Engine::writePlayers
 * @param stream
 * @return
 */

void Engine::writePlayers(UdpBitStream &stream) const
{
	writeBits<quint8>(stream, m_players.size(), 3);
	for (const EnginePlayer &p : m_players) {
		p.writeUserName(stream);
		p.writeNickName(stream);
	}
}



/**
 * @brief EngineList::readEngines
 * @param stream
 */

void EngineList::readEngines(UdpBitStream &stream)
{
	m_engines.clear();

	quint8 size = readBits(stream, 3, 0);

	m_engines.reserve(size);

	for (quint8 i=0; i<size; ++i) {
		Engine p;
		p.readId(stream);
		p.readReadableId(stream);
		p.readPlayers(stream);
		m_engines.emplace_back(std::move(p));
	}
}


/**
 * @brief EngineList::writeEngines
 * @param stream
 */

void EngineList::writeEngines(UdpBitStream &stream) const
{
	writeBits<quint8>(stream, m_engines.size(), 3);
	for (const Engine &p : m_engines) {
		p.writeId(stream);
		p.writeReadableId(stream);
		p.writePlayers(stream);
	}
}


/**
 * @brief EngineList::toStream
 * @return
 */

EngineStream EngineList::toStream() const
{
	EngineStream stream(Engine::OperationList);

	writeEngines(stream);

	return stream;
}


/**
 * @brief EngineStream::data
 * @return
 */

std::vector<uint8_t> EngineStream::data() const
{
	// Fill stream
	m_stream.writeBit(0, true);
	m_stream.write<uint8_t>(0, true);
	m_stream.write<uint8_t>(5, true);
	m_stream.write<uint8_t>(4, true);
	m_stream.write<uint8_t>(3, true);
	m_stream.write<uint8_t>(0, true);

	if (m_hasAuthKey) {
		LOG_CDEBUG("engine") << "#################### auth buffer";
		this->authBuffer(m_authKey);
	}

	return UdpBitStream::data();
}







};


