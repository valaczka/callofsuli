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
#include "abstractgame.h"

class RpgObject;
class RpgTower;
class RpgUdpEngine;


namespace Rpg {


/**
 * @brief The RpgEntityStatePull class
 */



typedef BaseStatePull<RpgStream::PlayerState, 10> RpgPlayerStatePull;




/**
 * @brief The RpgLogicClient class
 */

class RpgLogicClient : public RpgLogic
{
public:
	RpgLogicClient(const quint32 &lastAuthDiff, const quint32 &jitterDiff);

	void removeFromMapper(RpgObject *object);
	QPoint getChunkFromVector(const cpVect &point, cpVect *centerPtr = nullptr);
	QPoint getChunkFromVector(const cpVect &point, const float &angle, cpVect *centerPtr = nullptr);

	const quint32 &lastAuthDiff() const { return m_lastAuthTickDiff; }
	quint32 jitterTick() const {
		return m_serverTick > (m_jitterDiff+m_lastAuthTickDiff) ? (m_serverTick-m_jitterDiff-m_lastAuthTickDiff) : 0;
	}

	quint32 estimatedServerTick(const quint32 &lastAuthTick) {
		if (m_serverRtt <= 0)
			return lastAuthTick + m_lastAuthTickDiff;

		return lastAuthTick + m_lastAuthTickDiff + AbstractGame::TickTimer::msecToTick(m_serverRtt/2.);
	}

	qint64 serverRtt() const { return m_serverRtt; }
	void setServerRtt(qint64 newServerRtt) { m_serverRtt = newServerRtt; }

protected:
	virtual void eventRealized(entt::entity entity) override;

protected:
	qint64 m_serverRtt = 0;
	const quint32 m_jitterDiff = 0;
};




/**
 * @brief The RpgLogicClientSingle class
 */

class RpgLogicClientSingle : public RpgLogicClient
{
public:
	RpgLogicClientSingle();
};


/**
 * @brief The RpgLogicClientMulti class
 */

class RpgLogicClientMulti : public RpgLogicClient
{
public:
	RpgLogicClientMulti();

	void loadFull(const RpgStream::Full &full);

	RpgUdpEngine *engine() const;
	void setEngine(RpgUdpEngine *newEngine);

private:
	RpgUdpEngine *m_engine = nullptr;
};



struct RpgLogicControlledObjects
{
	quint32 player = 0;
	std::unordered_set<quint32> entities;
};

}		// end of namespace

#endif // RPGLOGICCLIENT_H
