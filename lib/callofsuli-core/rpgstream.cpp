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
		m_stream.write<uint8_t>(5, true);
		m_stream.write<uint8_t>(4, true);
		m_stream.write<uint8_t>(3, true);
		m_stream.write<uint8_t>(0, true);

		if (m_hasAuthKey) {
			LOG_CDEBUG("engine") << "#################### auth buffer";
			this->authBuffer(m_authKey);
		}

		m_hasFinalized = true;
	}
}



/**
 * @brief EnginePlayer::operator <<
 * @param stream
 * @return
 */

EngineStream &EnginePlayer::operator<<(EngineStream &stream)
{
	readDeltaMask(stream);

	readUserNameDelta(stream);
	readNickNameDelta(stream);

	return stream;
}



/**
 * @brief EnginePlayer::operator >>
 * @param stream
 * @return
 */

EngineStream &EnginePlayer::operator>>(EngineStream &stream) const
{
	writeDeltaMask(stream);

	writeUserNameDelta(stream);
	writeNickNameDelta(stream);

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
	readMaxPlayer(stream);
	m_owner << stream;
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
	writeMaxPlayer(stream);
	m_owner >> stream;
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
	readDeltaMask(stream);

	readIdDelta(stream);
	readReadableId(stream);
	readPlayersVectorDelta(stream, m_isDeltaMode);

	return stream;
}



/**
 * @brief Engine::operator >>
 * @param stream
 * @return
 */

EngineStream &Engine::operator>>(EngineStream &stream) const
{
	writeDeltaMask(stream);

	writeIdDelta(stream);

	writeReadableId(stream);
	writePlayersDelta(stream);

	return stream;
}




};


