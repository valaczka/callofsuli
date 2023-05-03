/*
 * ---- Call of Suli ----
 *
 * mapplaydemo.cpp
 *
 * Created on: 2023. 01. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapPlayDemo
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

#include "mapplaydemo.h"
#include "client.h"


MapPlayDemo::MapPlayDemo(Client *client, QObject *parent)
	: MapPlay(client, parent)
{
	m_online = false;
}


/**
 * @brief MapPlayDemo::~MapPlayDemo
 */

MapPlayDemo::~MapPlayDemo()
{

}


/**
 * @brief MapPlayDemo::load
 * @return
 */

bool MapPlayDemo::load()
{
	if (!m_client)
		return false;

	if (!loadFromFile(QStringLiteral(":/internal/game/demo.map"))) {
		m_client->messageWarning(tr("Nem lehet betölteni a demó pályát!"), tr("Belső hiba"));
		return false;
	}

	MapPlaySolverDefault *solver = new MapPlaySolverDefault(this);
	setSolver(solver);

#ifndef Q_OS_WASM
	solverLoad();
#endif

	return true;
}


/**
 * @brief MapPlayDemo::solverLoad
 */

void MapPlayDemo::solverLoad()
{
	if (!m_gameMap || !m_solver)
		return;

	const QJsonObject &object = Utils::fileToJsonObject(m_file);

	for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
		const QString &uuid = it.key();
		const QJsonObject &data = it.value().toObject();

		GameMapMission *m = m_gameMap->mission(uuid);

		if (m) {
			m_solver->loadSolverInfo(m, GameMap::SolverInfo(data));
		}
	}

	m_solver->updateLock();
	m_solver->updateXP();
}





/**
 * @brief MapPlayDemo::solverSave
 */

void MapPlayDemo::solverSave()
{
	if (!m_gameMap)
		return;

	QJsonObject object = Utils::fileToJsonObject(m_file);

	for (const MapPlayMission *mission: *m_missionList) {
		object.insert(mission->mission()->uuid(), mission->toSolverInfo().toJsonObject());
	}

	Utils::jsonObjectToFile(object, m_file);
}



/**
 * @brief MapPlayDemo::onCurrentGamePrepared
 */

void MapPlayDemo::onCurrentGamePrepared()
{
	if (!m_currentGame)
		return;

	m_currentGame->load();

	setGameState(StatePlay);
}



/**
 * @brief MapPlayDemo::onGameFinished
 */

void MapPlayDemo::onCurrentGameFinished()
{
	if (!m_currentGame || !m_client)
		return;

	AbstractLevelGame *g = m_currentGame;

	if (g->finishState() == AbstractGame::Success) {
		MapPlayMissionLevel *ml = getMissionLevel(g->missionLevel(), g->deathmatch());

		if (ml) {
			ml->solverDataIncrement();
#ifndef Q_OS_WASM
			solverSave();
#endif
			updateSolver();
		}
	} else if (g->finishState() == AbstractGame::Fail) {
		emit currentGameFailed();
	}



	setCurrentGame(nullptr);
	m_client->setCurrentGame(nullptr);
	g->setReadyToDestroy(true);

	setGameState(StateFinished);
}
