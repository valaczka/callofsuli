/*
 * ---- Call of Suli ----
 *
 * rpgengine.cpp
 *
 * Created on: 2025. 01. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgEngine
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

#include "rpgengine.h"
#include "Logger.h"
#include "udpserver.h"
#include <QCborArray>
#include <QCborMap>

int RpgEngine::m_nextId = 1;


class RpgEnginePrivate
{
private:
	RpgEnginePrivate(RpgEngine *engine)
		: q(engine)
	{}

	RpgEnginePlayer* getPlayer(UdpServerPeer *peer) const;

	void dataReceived(RpgEnginePlayer *player, const QByteArray &data);
	void dataReceivedChrSel(RpgEnginePlayer *player, const QByteArray &data);

	void dataSendChrSel();

	RpgGameData::GameConfig m_gameConfig;

	RpgEngine *q;

	// --- tmp

	bool m_playersCreated = false;

	friend class RpgEngine;
};



/**
 * @brief RpgEngine::RpgEngine
 * @param handler
 * @param parent
 */

RpgEngine::RpgEngine(EngineHandler *handler, QObject *parent)
	: UdpEngine(EngineRpg, handler, parent)
	, d(new RpgEnginePrivate(this))
{
	m_config.gameState = RpgConfig::StateCharacterSelect;
}


/**
 * @brief RpgEngine::~RpgEngine
 */

RpgEngine::~RpgEngine()
{
	delete d;
	d = nullptr;
}


/**
 * @brief RpgEngine::engineCreate
 * @param handler
 * @return
 */

std::shared_ptr<RpgEngine> RpgEngine::engineCreate(EngineHandler *handler, UdpServer *server)
{
	if (!handler)
		return {};

	LOG_CDEBUG("engine") << "Create RpgEngine" << m_nextId << server;


	auto ptr = std::make_shared<RpgEngine>(handler);
	ptr->setId(m_nextId);
	increaseNextId();

	ptr->setUdpServer(server);
	return ptr;
}


/**
 * @brief RpgEngine::timerTick
 */

void RpgEngine::timerTick()
{
	if (!m_udpServer) {
		LOG_CERROR("engine") << "Missing UdpServer";
		return;
	}


	if (m_config.gameState == RpgConfig::StatePlay && m_currentTick > 5000 && !d->m_playersCreated) {
		LOG_CINFO("engine") << "CREATE PLAYERS";

		for (RpgEnginePlayer &p : m_player) {
			if (!p.isHost())
				continue;

			p.s = 1;
			p.id = p.m_playerId;
		}

		//RpgPlayer *player = createPlayer(m_rpgGame->m_currentScene, RpgGame::characters().find(m_playerConfig.character).value());

		d->m_playersCreated = true;

	}

	if (m_config.gameState == RpgConfig::StateCharacterSelect)
		d->dataSendChrSel();
}



/**
 * @brief RpgEngine::binaryDataReceived
 * @param peer
 * @param data
 */

void RpgEngine::binaryDataReceived(UdpServerPeer *peer, const QByteArray &data)
{
	Q_ASSERT(peer);

	RpgEnginePlayer *player = d->getPlayer(peer);

	if (!player) {
		LOG_CERROR("engine") << "Player not found" << peer;
		return;
	}

	d->dataReceived(player, data);
}


/**
 * @brief RpgEngine::udpPeerAdd
 * @param peer
 */

void RpgEngine::udpPeerAdd(UdpServerPeer *peer)
{
	bool isHost = m_player.empty();

	RpgEnginePlayer &p = m_player.emplace_back(peer, isHost);
	p.setPlayerId(m_nextPlayerId++);

	LOG_CDEBUG("engine") << "Add player" << p.m_playerId << peer << isHost << p.isHost();

}



/**
 * @brief RpgEngine::udpPeerRemove
 * @param peer
 */

void RpgEngine::udpPeerRemove(UdpServerPeer *peer)
{
	LOG_CDEBUG("engine") << "Remove player" << peer;

	std::erase_if(m_player, [peer](const RpgEnginePlayer &p) { return p.udpPeer() == peer; });
}












/**
 * @brief RpgEnginePrivate::getPlayer
 * @param peer
 * @return
 */

RpgEnginePlayer *RpgEnginePrivate::getPlayer(UdpServerPeer *peer) const
{
	auto it = std::find_if(q->m_player.begin(), q->m_player.end(), [peer](const RpgEnginePlayer &p) { return p.udpPeer() == peer; });

	if (it == q->m_player.end())
		return nullptr;
	else
		return &(*it);
}



/**
 * @brief RpgEnginePrivate::dataReceived
 * @param player
 * @param data
 */

void RpgEnginePrivate::dataReceived(RpgEnginePlayer *player, const QByteArray &data)
{
	Q_ASSERT(player);

	if (q->m_config.gameState == RpgConfig::StateCharacterSelect)
		dataReceivedChrSel(player, data);
}


/**
 * @brief RpgEnginePrivate::dataReceivedChrSel
 * @param player
 * @param cdata
 */

void RpgEnginePrivate::dataReceivedChrSel(RpgEnginePlayer *player, const QByteArray &data)
{
	Q_ASSERT(player);

	QCborMap m = QCborValue::fromCbor(data).toMap();

	if (const auto &it = m.find(QStringLiteral("chr")); it != m.cend()) {
		RpgGameData::CharacterSelect c;
		c.fromCbor(it.value());
		c.playerId = player->playerId();			// Ő nem változtathat rajta
		player->setConfig(c);
	}

	if (!player->isHost())
		return;

	if (const auto &it = m.find(QStringLiteral("cfg")); it != m.cend()) {
		m_gameConfig.fromCbor(it.value());
	}

	if (const auto &it = m.find(QStringLiteral("t")); it != m.cend()) {
		q->m_currentTick = it->toInteger();
	}

	/*if (const auto &it = m.find(QStringLiteral("ee")); it != m.cend()) {
		const QCborArray enemies = it->toArray();

		m_enemies.clear();

		for (const QCborValue &v : enemies) {
			RpgGameData::Enemy enemy;
			enemy.fromCbor(v);
			m_enemies.push_back(enemy);
		}
	}*/

	/*if (const auto &it = m.find(QStringLiteral("pp")); it != m.cend()) {
		const QCborArray players = it->toArray();

		for (const QCborValue &v : players) {
			QCborMap m = v.toMap();

			const int sceneId = m.value(QStringLiteral("s")).toInteger();
			const int id = m.value(QStringLiteral("id")).toInteger();

			if (sceneId <= 0 || id <= 0) {
				LOG_CWARNING("engine") << "Invalid id" << sceneId << id;
				continue;
			}

			if (player->s != sceneId || player->id != id)
				continue;

			player->fromCbor(v);
		}
	}*/
}


/**
 * @brief RpgEnginePrivate::dataSendChrSel
 */

void RpgEnginePrivate::dataSendChrSel()
{
	QCborArray players;

	for (const RpgEnginePlayer &p : q->m_player) {
		players.append(p.config().toCborMap(true));
	}

	QCborMap gConfig = m_gameConfig.toCborMap(true);

	for (auto it = q->m_player.cbegin(); it != q->m_player.cend(); ++it) {
		QCborMap map;
		map.insert(QStringLiteral("g"), gConfig);
		map.insert(QStringLiteral("pp"), players);
		map.insert(QStringLiteral("hst"), it->isHost());
		map.insert(QStringLiteral("ply"), it->playerId());

		QCborMap configMap = it->config().toCborMap(true);
		map.insert(QStringLiteral("cfg"), configMap);

		if (it->udpPeer())
			it->udpPeer()->send(map.toCborValue().toCbor(), true);
	}
}








