/*
 * ---- Call of Suli ----
 *
 * rpglogicclient.h
 *
 * Created on: 2026. 05. 12.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgLogicClient
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

#ifndef RPGLOGICCLIENT_H
#define RPGLOGICCLIENT_H

#include "chipmunk/chipmunk_types.h"
#include <rpglogic.h>
#include <QElapsedTimer>

class RpgObject;
class RpgTower;
class RpgUdpEngine;
class RpgGamePrivate;

namespace Rpg {


/**
 * @brief The RpgEntityStatePull class
 */



typedef BaseStatePull<RpgStream::PlayerState, 10> RpgPlayerStatePull;
typedef BaseStatePull<RpgStream::NpcState, 10> RpgNpcStatePull;




/**
 * @brief The RpgLogicClient class
 */

class RpgLogicClient : public RpgLogic
{
public:
	RpgLogicClient(const quint32 &lastAuthDiff, const quint32 &jitterDiff);

	void addLocalIdTag(entt::entity entity);

	void removeFromMapper(RpgObject *object);
	QPoint getChunkFromVector(const cpVect &point, cpVect *centerPtr = nullptr);
	QPoint getChunkFromVector(const cpVect &point, const float &angle, cpVect *centerPtr = nullptr);

	quint32 jitterTick(const quint32 &tick) const {
		return tick > (m_jitterDiff+m_lastAuthTickDiff) ? (tick-m_jitterDiff-m_lastAuthTickDiff) : 0;
	}

	const quint32 &lastAuthDiff() const { return m_lastAuthTickDiff; }
	const quint32 &jitterDiff() const { return m_jitterDiff; }

	quint32 estimatedServerTick() const;

	qint64 serverRtt() const { return m_serverRtt; }
	void setServerRtt(qint64 newServerRtt) { m_serverRtt = newServerRtt; }


	entt::entity addNpc(const RpgStream::NpcData &data, entt::entity owner);


	template <typename T, std::size_t PULL_SIZE = DEFAULT_PULL_SIZE,
			  typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T>::value>::type>
	std::map<quint32, T> getSimulatedStates(entt::entity ent, const BaseStatePull<T, PULL_SIZE> &local,
											const T** latestPtr = nullptr) const
	{
		QMutexLocker locker(&m_mutex);

		std::map<quint32, T> ret;

		const BaseStatePull<T> *pull = m_registry.try_get<BaseStatePull<T> >(ent);

		if (!pull)
			return ret;

		const T* latest = pull->latest();

		if (!latest)
			return ret;

		if (latestPtr)
			*latestPtr = latest;

		return local.extractToMap(latest->tick());
	}


protected:
	qint64 m_serverRtt = 0;
	const quint32 m_jitterDiff = 0;

	QElapsedTimer m_lastInputTimer;
};







/**
 * @brief The RpgLogicClientSingle class
 */

class RpgLogicClientSingle : public RpgLogicClient
{
public:
	RpgLogicClientSingle();

	virtual RpgStream::GameConfig start();
	RpgStream::GameConfig startGame();

protected:
	virtual void eventRealized(entt::entity entity) override;
	virtual void rewindStage(const RpgStream::GameConfig::Stage &oldStage) override;
};





/**
 * @brief The RpgLogicClientMulti class
 */

class RpgLogicClientMulti : public RpgLogicClient
{
public:
	RpgLogicClientMulti();

	void loadFull(const RpgStream::Full &full);
	void loadFullState(const RpgStream::FullState &full);

	RpgUdpEngine *engine() const;
	void setEngine(RpgUdpEngine *newEngine);

private:
	void loadPlayers(const std::vector<RpgStream::PlayerStateList> &list);
	void loadEvents(const std::vector<RpgStream::Events> &list);
	void loadTowers(const std::vector<RpgStream::TowerState> &list);
	void loadMp(const std::vector<RpgStream::MpData> &list);
	void loadDefenders(const std::vector<RpgStream::DefenderState> &list);
	void loadNpc(const std::vector<RpgStream::NpcStateList> &list);

	RpgUdpEngine *m_engine = nullptr;
};





/**
 * @brief The LocalIdTag class
 */

struct LocalIdTag {};

/**
 * @brief The RpgLogicControlledObjects class
 */

struct RpgLogicControlledObjects
{
	quint32 player = 0;
	std::unordered_set<quint32> entities;
};

}		// end of namespace

#endif // RPGLOGICCLIENT_H
