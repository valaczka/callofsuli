/*
 * ---- Call of Suli ----
 *
 * mapplaycampaign.cpp
 *
 * Created on: 2023. 04. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapPlayCampaign
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

#include "mapplaycampaign.h"


/**
 * @brief MapPlayCampaign::MapPlayCampaign
 * @param handler
 * @param parent
 */

MapPlayCampaign::MapPlayCampaign(StudentMapHandler *handler, QObject *parent)
	: MapPlay{handler ? handler->client() : nullptr, parent}
	, m_handler(handler)
{
	Q_ASSERT(m_handler);

	LOG_CTRACE("client") << "MapPlayCampaign created" << this;
}


/**
 * @brief MapPlayCampaign::~MapPlayCampaign
 */

MapPlayCampaign::~MapPlayCampaign()
{
	LOG_CTRACE("client") << "MapPlayCampaign destroyed" << this;
}



/**
 * @brief MapPlayCampaign::load
 * @param campaign
 * @param map
 * @return
 */

bool MapPlayCampaign::load(Campaign *campaign, StudentMap *map)
{
	Q_UNUSED(campaign)

	if (!m_client)
		return false;

	const QByteArray &data = m_handler->read(map);

	if (data.isEmpty()) {
		m_client->messageWarning(tr("A pálya még nincs letöltve!"), tr("Hiba"));
		return false;
	}

	if (!loadFromBinaryData(data)) {
		m_client->messageWarning(tr("Nem lehet betölteni a pályát!"), tr("Belső hiba"));
		return false;
	}

	MapPlaySolverAction *solver = new MapPlaySolverAction(this);
	setSolver(solver);

	return true;
}



/**
 * @brief MapPlayCampaign::onCurrentGamePrepared
 */

void MapPlayCampaign::onCurrentGamePrepared()
{
	if (!m_currentGame)
		return;

	m_currentGame->load();
}


/**
 * @brief MapPlayCampaign::onCurrentGameFinished
 */

void MapPlayCampaign::onCurrentGameFinished()
{
	if (!m_currentGame || !m_client)
		return;

	AbstractLevelGame *g = m_currentGame;

	setCurrentGame(nullptr);
	m_client->setCurrentGame(nullptr);
	g->setReadyToDestroy(true);
}
