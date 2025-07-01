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
#include "qsettings.h"


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

bool MapPlayDemo::load(const QString &map)
{
	if (!m_client)
		return false;

	if (!loadFromFile(map)) {
		m_client->messageWarning(tr("Nem lehet betölteni a demó pályát!"), tr("Belső hiba"));
		return false;
	}

	MapPlaySolverDefault *solver = new MapPlaySolverDefault(this);
	setSolver(solver);

	solverLoad();

	return true;
}


/**
 * @brief MapPlayDemo::solverLoad
 */

void MapPlayDemo::solverLoad()
{
	if (!m_gameMap || !m_solver)
		return;

#ifdef Q_OS_WASM
	QSettings settings;
	const QJsonObject &object = settings.value(QStringLiteral("demo")).toJsonObject();
#else
	const QJsonObject &object = Utils::fileToJsonObject(m_file).value_or(QJsonObject{});
#endif

	for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
		const QString &uuid = it.key();
		const QJsonArray &data = it.value().toArray();

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

	QJsonObject object = Utils::fileToJsonObject(m_file).value_or(QJsonObject{});

	for (const MapPlayMission *mission: *m_missionList) {
		object.insert(mission->mission()->uuid(), mission->toSolverInfo().toJsonArray());
	}

#ifdef Q_OS_WASM
	QSettings settings;
	settings.setValue(QStringLiteral("demo"), object);
#else
	Utils::jsonObjectToFile(object, m_file);
#endif
}



/**
 * @brief MapPlayDemo::onCurrentGamePrepared
 */

void MapPlayDemo::onCurrentGamePrepared()
{
	if (!m_client->currentGame())
		return;

	if (m_client->currentGame()->load())
		setGameState(StatePlay);
	else {
		setGameState(StateInvalid);
		m_client->currentGame()->setReadyToDestroy(true);
	}
}



/**
 * @brief MapPlayDemo::onGameFinished
 */

void MapPlayDemo::onCurrentGameFinished()
{
	if (!m_client || !m_client->currentGame())
		return;

	auto g = qobject_cast<AbstractLevelGame *>(m_client->currentGame());

	if (!g) {
		LOG_CERROR("game") << "AbstractLevelGame cast error" << m_client->currentGame();
		return;
	}

	if (g->finishState() == AbstractGame::Success) {
		MapPlayMissionLevel *ml = getMissionLevel(g->missionLevel());

		if (ml) {
			ml->solverDataIncrement();
			solverSave();
			updateSolver();
		}
	} else if (g->finishState() == AbstractGame::Fail) {
		emit currentGameFailed();
	}

	g->setReadyToDestroy(true);

	setGameState(StateFinished);
}
