/*
 * ---- Call of Suli ----
 *
 * rpgmapplaytutorial.cpp
 *
 * Created on: 2026. 07. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgMapPlayTutorial
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

#include "rpgmapplaytutorial.h"
#include "client.h"
#include "rpggame.h"
#include "rpgtower.h"
#include "rpgplayer.h"
#include <QRandomGenerator>




/**
 * @brief RpgMapPlayTutorial::RpgMapPlayTutorial
 * @param client
 * @param parent
 */

RpgMapPlayTutorial::RpgMapPlayTutorial(Client *client, QObject *parent)
	: MapPlay(client, parent)
{

}


/**
 * @brief RpgMapPlayTutorial::~RpgMapPlayTutorial
 */

RpgMapPlayTutorial::~RpgMapPlayTutorial()
{

}


/**
 * @brief RpgMapPlayTutorial::load
 * @param map
 * @return
 */

QQuickItem* RpgMapPlayTutorial::load(const QUrl &url)
{
	if (!m_client)
		return nullptr;

	if (m_client->currentGame()) {
		m_client->messageError(tr("Még folyamatban van egy másik játék"), tr("Játék nem indítható"));
		return nullptr;
	}

	static const QString m = QStringLiteral(":/internal/game/demo.map");

	if (!loadFromFile(m))
		return nullptr;

	const QList<GameMapMissionLevel *> &levels = m_gameMap->missions().first()->levels();


	//////////////////////////////////////////

	std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> tutorial = std::make_unique<Rpg::RpgLogicClientTutorial::Tutorial>();
	tutorial->character = "character01a";
	tutorial->terrain = "test";

	tutorial->addTower(120);
	tutorial->addEmitter(127);


	tutorial->fnInit = [](Rpg::RpgLogicClientTutorial *logic) {
		Q_ASSERT(logic);
		RpgStream::NpcData d;
		d.setCharacterResolved("soldier04");
		d.setTeam(RpgStream::TeamNone);
		d.setType(RpgStream::NpcData::TowerAttacker);
		RpgNpcDefinition def = RpgGame::readNpcDefinition("soldier04").value_or(RpgNpcDefinition{});
		d.setEntity(def.toEntityConfig());

		LOG_CINFO("game") << "TUTORIAL ADD NPC";

		logic->addNpc(d, entt::null);
	};

	{
		Rpg::RpgLogicClientTutorial::Tutorial::Step step;

		step.message = "Jöhet mindjárt";

		step.addTargetControlEvent([](TiledObjectBody *obj) {
			LOG_CINFO("game") << "CHECK CONTROL" << obj;

			if (dynamic_cast<RpgTower*>(obj))
				return true;
			else
				return false;
		});

		step.fnNext = [](Rpg::RpgLogicClientTutorial *logic, const quint32 &tick) {
			Q_ASSERT(logic);

			cpVect pos = logic->player()->bodyPosition();


			for (int i=1; i<6; ++i) {
				Rpg::EventNpcCreate ev;
				ev.data.setCharacterResolved("soldier04");
				ev.data.setTeam(RpgStream::TeamNone);
				ev.data.setType(RpgStream::NpcData::TowerAttacker);
				RpgNpcDefinition def = RpgGame::readNpcDefinition("soldier04").value_or(RpgNpcDefinition{});
				ev.data.setEntity(def.toEntityConfig());

				ev.setTick(tick);

				ev.pos.x = pos.x - i*25;
				ev.pos.y = pos.y + i*20;

				logic->eventStore(std::move(ev));
			}

			Rpg::RpgLogicScope scope = logic->getScope();

			quint32 id = logic->getId(127);

			LOG_CINFO("game") << "*******MP" << id;

			Rpg::EventMpCreate ev;
			ev.emitter = scope.entityFromIdTag(logic->getId(127));
			ev.mpCount = 8;
			ev.setTick(tick+120);

			logic->eventStore(std::move(ev));
		};

		tutorial->steps.emplace_back(std::move(step));

	}

	{
		Rpg::RpgLogicClientTutorial::Tutorial::Step step;

		step.message = "Wait for start...";

		auto ev = std::make_unique<RpgStream::EventStageChanged>();
		ev->config().setStage(RpgStream::GameConfig::StageMain);

		step.inputEvents.emplace_back(std::move(ev));

		step.addTargetControlEvent([](TiledObjectBody *obj) {
			LOG_CINFO("game") << "CHECK CONTROL" << obj;

			if (dynamic_cast<RpgTower*>(obj))
				return true;
			else
				return false;
		});

		step.fnNext = [](Rpg::RpgLogicClientTutorial *logic, const quint32 &tick) {
			Q_ASSERT(logic);

			Rpg::EventNpcCreate ev;
			ev.data.setCharacterResolved("soldier04");
			ev.data.setTeam(RpgStream::TeamNone);
			ev.data.setType(RpgStream::NpcData::TowerAttacker);
			RpgNpcDefinition def = RpgGame::readNpcDefinition("soldier04").value_or(RpgNpcDefinition{});
			ev.data.setEntity(def.toEntityConfig());

			ev.setTick(tick);

			logic->eventStore(std::move(ev));
		};



		tutorial->steps.emplace_back(std::move(step));

	}


	/////////////////////////////

	m_game = new RpgGame(levels.at(QRandomGenerator::global()->bounded(levels.size())), m_client, false, std::move(tutorial));
	setGameState(StateLoading);

	m_game->setStorageSeed(m_storageSeed.get());

	m_client->setCurrentGame(m_game);


	if (m_client->currentGame()->load())
		setGameState(StatePlay);
	else {
		setGameState(StateInvalid);
		m_client->currentGame()->setReadyToDestroy(true);
	}

	connect(m_game, &RpgGame::gameFinished, this, &RpgMapPlayTutorial::onFinished);

	return m_game->pageItem();
}



/**
 * @brief RpgMapPlayTutorial::onFinished
 */

void RpgMapPlayTutorial::onFinished(AbstractGame::FinishState)
{
	if (m_game)
		m_game->setReadyToDestroy(true);

	setGameState(StateFinished);
}
