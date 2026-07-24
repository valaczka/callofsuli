/*
 * ---- Call of Suli ----
 *
 * rpgudpengine.h
 *
 * Created on: 2026. 06. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgUdpEngine
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

#ifndef RPGUDPENGINE_H
#define RPGUDPENGINE_H

#include "abstractudpengine.h"
#include "rpglogicclient.h"
#include "rpgstream.h"

class RpgGame;

class RpgUdpEngine : public AbstractUdpEngine
{
	Q_OBJECT

	Q_PROPERTY(bool isHost READ isHost WRITE setIsHost NOTIFY isHostChanged FINAL)

public:
	explicit RpgUdpEngine(class RpgGamePrivate *game, const PublicKeySigner &signer, QObject *parent = nullptr);
	virtual ~RpgUdpEngine();

	PublicKeySigner signer() const { return m_signer; }

	RpgStream::EngineStream getStream(const RpgStream::EngineStream::Operation &operation) const {
		return RpgStream::EngineStream(m_signer, peerIndex(), operation);
	}

	RpgStream::EngineDataStream getDataStream(const RpgStream::EngineDataStream::DataOperation &dataOperation) const {
		return RpgStream::EngineDataStream(m_signer, peerIndex(), dataOperation);
	}

	const std::optional<RpgStream::Room> &room() const;

	bool isHost() const;
	void setIsHost(bool newIsHost);

	void updateStage(const RpgStream::GameConfig &config, const quint32 &tick);

signals:
	void isHostChanged();

protected:
	virtual void binaryDataReceived(std::vector<UdpPacketRcv> &&list, const int &currentRtt) override;

private:
	Rpg::RpgLogicClientMulti *logic() const;
	void onDataReceived(std::unique_ptr<UdpBitStream> data);

	void updateRoom();

	void onBeforeWorldStep(const qint64 &tick);


	// Lobby

	void lobbyReload();
	void lobbyCreate();
	void lobbyConnect(const int &id);
	void lobbyLoad(const RpgStream::RoomList &list);


	// Character select

	void sendCharacterSelect(const RpgStream::CharacterSelectClient &data);
	void sendQuestSelect(const RpgStream::QuestSelect &data);

	// Prepare

	void updateMapData(RpgStream::EngineDataStream &&stream);
	void updateFull(RpgStream::EngineDataStream &&stream);

	// Play

	void updateState(RpgStream::EngineDataStream &&stream);
	void sendState(const RpgStream::FullState &data);

	void updateResult(RpgStream::EngineDataStream &&stream);

protected:
	const PublicKeySigner m_signer;

private:
	RpgGamePrivate *const m_gamePrivate;
	RpgGame *const m_game;

	std::optional<RpgStream::Room> m_room;
	bool m_isHost = false;

	bool m_isMapReady = false;
	bool m_isMapReloaded = false;
	bool m_fullReceived = false;

	RpgStream::PlayerData::Flags m_gameFlags = RpgStream::PlayerData::FlagNull;


	friend class RpgGame;
	friend class RpgGamePrivate;
};

#endif // RPGUDPENGINE_H
