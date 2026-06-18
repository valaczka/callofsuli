/*
 * ---- Call of Suli ----
 *
 * rpglogicserver.cpp
 *
 * Created on: 2026. 06. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgLogicServer
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

#include "rpglogicserver.h"
#include "rpgengine.h"
#include "serverservice.h"

#define LAST_AUTH_DIFF				6
#define SEND_STATE_COUNT			3

RpgLogicServer::RpgLogicServer(RpgEngine *engine)
	: Rpg::RpgLogic(LAST_AUTH_DIFF)
	, m_engine(engine)
{

}


/**
 * @brief RpgLogicServer::getRenderedState
 * @return
 */

RpgStream::Full RpgLogicServer::getRenderedState(const bool &requireFull)
{
	RpgStream::Full f;

#ifdef WITH_FTXUI
	QString txt;

	if (requireFull) {
		f = getFull(&txt);
	} else {
		const RpgStream::FullState &st = getFullState(SEND_STATE_COUNT, &txt);
		f.setFullState(st);
	}
#else
	if (requireFull) {
		f = getFull();
	} else {
		const RpgStream::FullState &st = getFullState(SEND_STATE_COUNT);
		f.setFullState(st);
	}
#endif


#ifdef WITH_FTXUI
	QCborMap m;
	m.insert(QStringLiteral("mode"), QStringLiteral("SND"));
	m.insert(QStringLiteral("txt"), txt);
	m_engine->udpServer()->service()->writeToSocket(m.toCborValue());
#endif

	return f;
}



/**
 * @brief RpgLogicServer::eventRealized
 * @param entity
 */

void RpgLogicServer::eventRealized(entt::entity entity)
{
	QMutexLocker locker(&m_mutex);

	if (RpgStream::EventStageChanged *ev = m_registry.try_get<RpgStream::EventStageChanged>(entity)) {
		LOG_CINFO("engine") << "REALIZED STAGE" << ev->config().stage();
		ELOG_DEBUG << "Stage changed to" << ev->config().stage();

		if (!onStageChanged(ev->config().stage()))
			eventRealizedDefault(entity);
	}



}



/**
 * @brief RpgLogicServer::onStageChanged
 * @param stage
 */

bool RpgLogicServer::onStageChanged(const RpgStream::GameConfig::Stage &stage)
{
	if (stage == RpgStream::GameConfig::StageWarmingUp)	{
		LOG_CINFO("engine") << "WARMING UP";
		ELOG_DEBUG << "Stage: warming up";
	}

	// run default

	return false;
}


/**
 * @brief RpgLogicServer::_logger
 * @return
 */

Logger *RpgLogicServer::_logger() const
{
	Q_ASSERT (m_engine);
	return m_engine->_logger();
}



