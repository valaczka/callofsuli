/*
 * ---- Call of Suli ----
 *
 * rpgevent.cpp
 *
 * Created on: 2025. 07. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgEvent
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

#include "rpgevent.h"
#include "rpgengine.h"
#include "rpgengine_p.h"


/**
 * @brief RpgEventEnemyDied::process
 * @return
 */

bool RpgEventEnemyDied::process(const qint64 &/*tick*/, RpgGameData::CurrentSnapshot *dst)
{
	Q_UNUSED(dst);

	ELOG_DEBUG << "Enemy died processed" << m_data << m_pickables;

	/*bool allDead = true;

	for (const auto &ptr : m_engine->enemies()) {
		if (ptr.data == m_data)
			continue;

		if (ptr.list.empty()) {
			allDead = false;
			break;
		}

		if (ptr.get(m_tick)->hp > 0) {
			allDead = false;
			break;
		}
	}

	if (allDead) {
		ELOG_INFO << "All enemies died";
		m_engine->eventAdd<RpgEventEnemyResurrect>(m_tick+180);
	}*/

	for (const RpgGameData::PickableBaseData &base : m_pickables) {
		const auto &pl = m_engine->pickables();

		const auto it = std::find_if(pl.cbegin(),
									 pl.cend(),
									 [&base](const auto &p){
			return p.data == base;
		});

		if (it == pl.cend()) {
			ELOG_ERROR << "Invalid pickable" << m_data;
			continue;
		}

		RpgGameData::Pickable d;

		if (!it->list.empty())
			d = it->get(m_tick).value();

		d.f = m_tick;
		d.u = {};
		d.a = true;
		d.own = {};
		d.st = RpgGameData::LifeCycle::StageLive;

		dst->assign(dst->controls.pickables, base, d);
	}

	return true;
}




/**
 * @brief RpgEventEnemyResurrect::process
 * @return
 */

bool RpgEventEnemyResurrect::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_ASSERT(dst);

	if (tick < m_tick)
		return false;

	for (const auto &ptr : m_engine->enemies()) {
		if (ptr.list.empty())
			continue;

		RpgGameData::Enemy edata = ptr.get(m_tick).value();
		edata.f = m_tick;
		edata.hp = ptr.data.mhp;

		dst->assign(dst->enemies, ptr.data, edata);
	}

	return true;
}





/**
 * @brief RpgEventPlayerDied::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventPlayerDied::process(const qint64 &/*tick*/, RpgGameData::CurrentSnapshot *dst)
{
	Q_UNUSED(dst);

	ELOG_DEBUG << "Player died processed" << m_data << "@" << m_tick;

	RpgGameData::Message msg;

	if (RpgEnginePlayer *player = m_engine->player(m_data)) {
		msg.m = QStringLiteral("%1 died").arg(player->config().nickname.isEmpty() ?
												  player->config().username :
												  player->config().nickname);
	} else {
		msg.m = QStringLiteral("Player %1 died").arg(m_data.o);
	}


	m_engine->eventAdd<RpgEventPlayerResurrect>(m_tick+5*60, m_data);
	m_engine->messageAdd(msg, QList<int>{m_data.o}, true);

	return true;
}





/**
 * @brief RpgEventPlayerResurrect::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventPlayerResurrect::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_ASSERT(dst);

	if (tick < m_tick)
		return false;

	const auto &pl = m_engine->players();

	const auto it = std::find_if(pl.cbegin(),
								 pl.cend(),
								 [this](const auto &p){
		return p.data == m_data;
	});

	RpgEnginePlayer *realPlayer = m_engine->player(m_data);

	if (it == pl.cend() || !realPlayer) {
		ELOG_ERROR << "Invalid player" << m_data;
		return true;
	}

	if (!realPlayer->udpPeer()) {
		ELOG_WARNING << "Player connection lost" << m_data << "skip resurrection";
		return true;
	}

	RpgGameData::Player player;

	if (!it->list.empty())
		player = it->get(m_tick).value();

	if (player.hp > 0) {
		ELOG_WARNING << "Player alive" << m_data << player.hp << "HP, skip resurrection";
		return true;
	}

	player.f = m_tick;
	player.hp = realPlayer->config().maxHp;

	player.pck = {};
	player.st = RpgGameData::Player::PlayerExit;

	if (realPlayer->startPosition().x > 0 && realPlayer->startPosition().y > 0) {
		player.p = QList<float>{realPlayer->startPosition().x, realPlayer->startPosition().y};

		ELOG_DEBUG << "Emplace player" << m_data << "at" << realPlayer->startPosition().x << realPlayer->startPosition().y;
	}

	dst->assign(dst->players, m_data, player);

	return true;
}




/**
 * @brief RpgEventPlayerResurrect::isEqual
 * @param other
 * @return
 */

bool RpgEventPlayerResurrect::isEqual(RpgEventBase *other) const
{
	if (RpgEventPlayerResurrect *d = dynamic_cast<RpgEventPlayerResurrect*>(other); d &&
			d->baseData().isBaseEqual(m_data)
			)
		return true;

	return false;
}





/**
 * @brief RpgEventCollectionRelocate::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventCollectionUsed::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_ASSERT(dst);

	if (tick < m_tick)
		return false;

	const auto &pl = m_engine->controlCollections();

	const auto it = std::find_if(pl.cbegin(),
								 pl.cend(),
								 [this](const auto &p){
		return p.data == m_data;
	});

	if (it == pl.cend()) {
		ELOG_ERROR << "Invalid collection" << m_data;
		return true;
	}

	RpgGameData::ControlCollection d;

	if (!it->list.empty())
		d = it->get(m_tick).value();

	d.f = m_tick;
	d.u = {};

	const auto &players = m_engine->players();

	const auto pit = std::find_if(players.cbegin(),
								  players.cend(),
								  [this](const auto &p){
		return p.data.isBaseEqual(m_player);
	});


	if (m_success) {
		d.a = false;
		d.own = m_player;

		if (pit == players.cend() || pit->list.empty()) {
			ELOG_ERROR << "Invalid player" << m_data;
		} else {
			RpgGameData::Player playerData = pit->get(m_tick).value();
			playerData.c++;
			dst->assign(dst->players, m_player, playerData);

			const int left = pit->data.rq - playerData.c;

			if (left > 1)
				m_engine->messageAdd(RpgGameData::Message(QStringLiteral("Collect %1 more items").arg(left), true),
									 QList<int>{m_player.o});
			else if (left > 0)
				m_engine->messageAdd(RpgGameData::Message(QStringLiteral("Collect 1 more item"), true),
									 QList<int>{m_player.o});
			else if (left == 0)
				m_engine->messageAdd(RpgGameData::Message(QStringLiteral("All required items collected"),
														  QColorConstants::Svg::limegreen, true),
									 QList<int>{m_player.o});
		}

		m_engine->eventAdd<RpgEventCollectionPost>(m_tick+1, m_data, true);

	} else {
		if (pit == players.cend() || pit->list.empty()) {
			ELOG_ERROR << "Invalid player" << m_data;
		} else {
			RpgGameData::Player playerData = pit->get(m_tick).value();
			playerData.controlFailed(RpgConfig::ControlCollection);

			dst->assign(dst->players, m_player, playerData);
		}

		m_engine->eventAdd<RpgEventCollectionPost>(m_tick+180, m_data, false);
	}

	dst->assign(dst->controls.collections, m_data, d);

	return true;
}






/**
 * @brief RpgEventControlUnlock::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventControlUnlock::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_ASSERT(dst);

	if (tick < m_tick)
		return false;


	const auto &pp = m_engine->players();

	if (const auto &it = std::find_if(pp.cbegin(),
									  pp.cend(),
									  [this](const auto &pd){
									  return pd.data == m_data;
}); it != pp.cend() && !it->list.empty()) {
		RpgGameData::Player d = it->get(m_tick).value();

		d.l = false;

		ELOG_TRACE << "Unlock player" << it->data;
		dst->assign(dst->players, m_data, d);
	}

	const auto &pl = m_engine->controlContainers();



	for (const auto &ptr : pl) {
		if (ptr.list.empty())
			continue;

		RpgGameData::ControlContainer d = ptr.get(m_tick).value();

		if (d.u.isBaseEqual(m_data) && d.st == RpgGameData::ControlContainer::ContainerClose) {
			d.u = {};
			d.a = true;
			ELOG_TRACE << "Unlock container" << ptr.data;
			dst->assign(dst->controls.containers, ptr.data, d);
		}
	}



	const auto &pc = m_engine->controlCollections();

	for (const auto &ptr : pc) {
		if (ptr.list.empty())
			continue;

		RpgGameData::ControlCollection d = ptr.get(m_tick).value();

		if (d.u.isBaseEqual(m_data) && !d.own.isValid()) {
			d.u = {};
			d.a = true;
			ELOG_TRACE << "Unlock collection" << ptr.data;
			dst->assign(dst->controls.collections, ptr.data, d);
		}
	}


	return true;
}



/**
 * @brief RpgEventPickablePicked::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventPickablePicked::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_ASSERT(dst);

	if (tick < m_tick)
		return false;

	const auto &pl = m_engine->pickables();

	const auto it = std::find_if(pl.cbegin(),
								 pl.cend(),
								 [this](const auto &p){
		return p.data == m_data;
	});

	if (it == pl.cend()) {
		ELOG_ERROR << "Invalid pickable" << m_data;
		return true;
	}


	RpgGameData::Player pData;

	const auto &pp = m_engine->players();

	if (const auto &it = std::find_if(pp.cbegin(),
									  pp.cend(),
									  [this](const auto &pd){
									  return pd.data == m_player;
}); it != pp.cend() && !it->list.empty()) {
		pData = it->get(m_tick).value();
	} else {
		ELOG_ERROR << "Invalid player" << m_player;
		return true;
	}

	pData.f = m_tick;

	ELOG_INFO << "Player" << m_player << "picked" << m_data.pt;

	RpgGameData::Pickable d;

	if (!it->list.empty())
		d = it->get(m_tick).value();

	d.f = m_tick;
	d.u = {};
	d.a = false;
	d.own = m_player;
	d.st = RpgGameData::LifeCycle::StageDead;


	const int pv = pData.pick(m_data.pt);

	if (pv < 0) {
		dst->assign(dst->players, m_player, pData);

		if (m_data.pt == RpgGameData::PickableBaseData::PickableBullet)
			m_engine->messageAdd(RpgGameData::Message(RpgGameData::Message::MessagePick,
													  QStringLiteral("%1 bullets gained").arg(-pv),
													  QColorConstants::Svg::limegreen, false),
								 {m_player.o});
		else
			m_engine->messageAdd(RpgGameData::Message(RpgGameData::Message::MessagePick),
								 {m_player.o});


	} else if (pv > 0 && m_data.pt == RpgGameData::PickableBaseData::PickableTime) {
		m_engine->addMsec(pv*1000);
		m_engine->messageAdd(RpgGameData::Message(RpgGameData::Message::MessagePick,
												  QStringLiteral("%1 seconds gained").arg(pv)));
	} else {
		ELOG_WARNING << "Pickable process failed" << m_data.pt;
		///LOG_CERROR("engine") << "Pickable process failed" << m_data.pt;

		d.own = {};
		d.a = true;
		d.st = RpgGameData::LifeCycle::StageLive;
	}

	dst->assign(dst->controls.pickables, m_data, d);

	return true;
}





/**
 * @brief RpgEventContainerOpened::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventContainerUsed::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_UNUSED(dst);

	if (tick < m_tick)
		return false;

	ELOG_DEBUG << "Container use processed" << m_data << m_success << "@" << m_tick;

	if (!m_success) {
		const auto &pl = m_engine->players();

		const auto it = std::find_if(pl.cbegin(),
									 pl.cend(),
									 [this](const auto &p){
			return p.data.isBaseEqual(m_player);
		});

		if (it == pl.cend() || it->list.empty()) {
			ELOG_ERROR << "Invalid player" << m_data;
			return true;
		}

		RpgGameData::Player playerData = it->get(m_tick).value();
		playerData.controlFailed(RpgConfig::ControlContainer);

		dst->assign(dst->players, m_player, playerData);


	} else {
		for (const RpgGameData::PickableBaseData &base : m_pickables) {
			const auto &pl = m_engine->pickables();

			const auto it = std::find_if(pl.cbegin(),
										 pl.cend(),
										 [&base](const auto &p){
				return p.data == base;
			});

			if (it == pl.cend()) {
				ELOG_ERROR << "Invalid pickable" << m_data;
				continue;
			}

			RpgGameData::Pickable d;

			if (!it->list.empty())
				d = it->get(m_tick).value();

			d.f = m_tick;
			d.u = {};
			d.a = true;
			d.own = {};
			d.st = RpgGameData::LifeCycle::StageLive;

			dst->assign(dst->controls.pickables, base, d);
		}
	}

	return true;
}




/**
 * @brief RpgEventCollectionPost::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventCollectionPost::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_ASSERT(dst);

	if (tick < m_tick)
		return false;

	const auto &pl = m_engine->controlCollections();

	const auto it = std::find_if(pl.cbegin(),
								 pl.cend(),
								 [this](const auto &p){
		return p.data == m_data;
	});

	if (it == pl.cend()) {
		ELOG_ERROR << "Invalid collection" << m_data;
		return true;
	}

	RpgGameData::ControlCollection d;

	if (!it->list.empty())
		d = it->get(m_tick).value();

	if (m_success) {
		if (!m_engine->finishCollection(it->data, d.idx)) {
			ELOG_ERROR << "Finish colection failed" << it->data;
		}

		int left = -1;
		bool hasMissing = false;		// van-e, akinek még hiányzik

		// Recalculate collections

		for (const auto &p : m_engine->players()) {
			if (p.list.empty()) {
				ELOG_ERROR << "Empty player snap list";
				continue;
			}

			RpgEnginePlayer *realPlayer = m_engine->player(p.data);

			RpgGameData::Player playerData = p.get(m_tick).value();

			int coll = m_engine->getCollected(m_tick, p.data, left < 0 ? &left : nullptr);

			playerData.c = coll;

			if (coll < p.data.rq && realPlayer && !realPlayer->isLost())
				hasMissing = true;

			dst->assign(dst->players, p.data, playerData);
		}

		if (left == 0) {
			ELOG_INFO << "All items collected, open teleports";

			bool hasTeleport = false;

			for (const auto &p : m_engine->controlTeleports()) {
				// Nem final teleportok és hideoutok kihagyása
				if (p.data.dst.isValid() || p.data.hd)
					continue;

				if (p.list.empty()) {
					ELOG_ERROR << "Empty player snap list";
					continue;
				}

				hasTeleport = true;

				RpgGameData::ControlTeleport pData = p.get(m_tick).value();

				pData.a = true;

				dst->assign(dst->controls.teleports, p.data, pData);

			}

			if (hasTeleport)
				m_engine->messageAdd(RpgGameData::Message(QStringLiteral("Escape through the teleporter"), true));

		} else if (!hasMissing && !m_engine->hasMessageSent(RpgEngine::MessageCollectAllRemaining)) {
			m_engine->messageAdd(RpgGameData::Message(QStringLiteral("Collect all the remaining items"), true));
			m_engine->setMessageSent(RpgEngine::MessageCollectAllRemaining);
		}


	} else {
		d.f = m_tick;
		d.u = {};
		d.a = true;
		d.own = {};

		QPointF p;
		const int idx = m_engine->relocateCollection(it->data, m_tick, &p);

		ELOG_DEBUG << "Relocate collection" << m_data << "->" << idx << p << "@" << m_tick;

		if (idx >= 0) {
			d.idx = idx;
			d.p = QList<float>{(float) p.x(), (float) p.y()};
		}

		dst->assign(dst->controls.collections, m_data, d);
	}

	return true;
}






/**
 * @brief RpgEventTeleportUsed::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventTeleportUsed::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_ASSERT(dst);

	if (tick < m_tick)
		return false;

	const auto &pl = m_engine->controlTeleports();

	const auto it = std::find_if(pl.cbegin(),
								 pl.cend(),
								 [this](const auto &p){
		return p.data == m_data;
	});

	if (it == pl.cend()) {
		ELOG_ERROR << "Invalid teleport" << m_data;
		return true;
	}


	RpgGameData::Player pData;

	const auto &pp = m_engine->players();

	if (const auto &it = std::find_if(pp.cbegin(),
									  pp.cend(),
									  [this](const auto &pd){
									  return pd.data == m_player;
}); it != pp.cend() && !it->list.empty()) {
		pData = it->get(m_tick).value();
	} else {
		ELOG_ERROR << "Invalid player" << m_player;
		return true;
	}

	if (pData.useTeleport(m_data, m_player)) {
		ELOG_INFO << "Teleport used" << m_data << "by player" << m_player << "@" << m_tick;
		dst->assign(dst->players, m_player, pData);

		// Hideout

		if (m_data.hd)
			return true;

		// Final teleport

		if (!m_data.dst.isValid()) {
			if (RpgEnginePlayer *enginePlayer = m_engine->playerSetGameCompleted(m_player)) {
				m_engine->messageAdd(RpgGameData::Message(QStringLiteral("%1 has teleported").arg(enginePlayer->config().nickname.isEmpty() ?
																									  enginePlayer->config().username :
																									  enginePlayer->config().nickname),
														  true),
									 QList<int>{m_data.o}, true);
			} else {
				ELOG_ERROR << "Invalid player" << m_player;
			}
		}



	} else {
		ELOG_ERROR << "Player teleport" << m_data << "usage failed for player" << m_player;

		/*const int left = std::max(1, m_player.rq - pData.c);

		if (left > 1)
			m_engine->messageAdd(RpgGameData::Message(QStringLiteral("%1 items missing").arg(left), false),
								 QList<int>{m_player.o});
		else
			m_engine->messageAdd(RpgGameData::Message(QStringLiteral("1 item missing"), false),
								 QList<int>{m_player.o});*/
	}

	return true;
}







/**
 * @brief RpgEventPlayerLost::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventPlayerLost::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_ASSERT(dst);

	if (tick < m_tick)
		return false;

	RpgEnginePlayer *realPlayer = m_engine->player(m_data);

	if (!realPlayer) {
		ELOG_ERROR << "Invalid player" << m_data;
		return true;
	}

	if (realPlayer->udpPeer()) {
		ELOG_ERROR << "Player connection alive" << m_data << "skip lost event";
		return true;
	}

	realPlayer->setIsLost(true);

	ELOG_INFO << "Player" << m_data << "lost";

	int left = -1;
	bool hasMissing = false;		// van-e, akinek még hiányzik

	// Recalculate collections

	for (const auto &p : m_engine->players()) {
		if (p.list.empty()) {
			ELOG_ERROR << "Empty player snap list";
			continue;
		}

		RpgEnginePlayer *realPlayer = m_engine->player(p.data);

		int coll = m_engine->getCollected(m_tick, p.data, left < 0 ? &left : nullptr);

		if (coll < p.data.rq && realPlayer && !realPlayer->isLost())
			hasMissing = true;
	}

	if (left > 0 && !hasMissing && !m_engine->hasMessageSent(RpgEngine::MessageCollectAllRemaining)) {
		m_engine->messageAdd(RpgGameData::Message(QStringLiteral("Collect all the remaining items"), true));
		m_engine->setMessageSent(RpgEngine::MessageCollectAllRemaining);
	}

	return true;
}






/**
 * @brief RpgEventPlayerLost::isEqual
 * @param other
 * @return
 */

bool RpgEventPlayerLost::isEqual(RpgEventBase *other) const
{
	if (RpgEventPlayerLost *d = dynamic_cast<RpgEventPlayerLost*>(other); d &&
			d->baseData().isBaseEqual(m_data)
			)
		return true;

	return false;
}
