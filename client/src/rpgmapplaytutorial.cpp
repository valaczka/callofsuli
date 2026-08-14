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



struct TutorialData {
	static std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> testTutorial1(const QUrl &url);
	static std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> testCharacter(const QUrl &url);

	static std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> defaultTutorial(const QUrl &url);


	static inline const QHash<QString, std::function<std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial>(const QUrl &)> >
	tutorials = {
	{ QStringLiteral("test_tutorial1"), &TutorialData::testTutorial1 },
	{ QStringLiteral("default"), &TutorialData::defaultTutorial },
	{ QStringLiteral("character"), &TutorialData::testCharacter },
				};
};




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

	auto ptr = TutorialData::tutorials.value(url.host());

	if (!ptr)
		return nullptr;

	std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> tutorial = ptr(url);

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






/**
 * @brief TutorialData::testTutorial1
 * @return
 */


std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> TutorialData::testTutorial1(const QUrl &url)
{

	std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> tutorial = std::make_unique<Rpg::RpgLogicClientTutorial::Tutorial>();
	tutorial->character = "character01a";
	tutorial->terrain = "test";
	tutorial->power = 1;
	tutorial->duration = CFG_GAME_DURATION;

	tutorial->addTower(120);
	tutorial->addEmitter(127);
	tutorial->addChest("entry3");
	tutorial->addChest("entry2");


	tutorial->fnFirst = [](Rpg::RpgLogicClientTutorial *logic) {
		Q_ASSERT(logic);

		logic->npcAddToPoint(QStringLiteral("soldier02"), "entry2");

		logic->towerSet(120, RpgStream::TeamB, 100);
		logic->defenderAddToTower(120, RpgStream::BaseDefenderObject::Pulse, RpgStream::TeamB);
		logic->defenderAddToTower(120, RpgStream::BaseDefenderObject::Electric, RpgStream::TeamB);
	};

	{
		Rpg::RpgLogicClientTutorial::Tutorial::Step step;

		step.message = "Jöhet mindjárt";

		step.infoIcon = 		"qrc:/Qaterial/Icons/abacus.svg";
		step.infoTitle = "Menj oda";
		step.infoText = "Menjél gyorsan oda, mielőtt meggondolom magam.";

		step.addTargetControlEvent([](TiledObjectBody *obj) {
			if (dynamic_cast<RpgTower*>(obj))
				return true;
			else
				return false;
		});

		step.fnNext = [](Rpg::RpgLogicClientTutorial *logic, const quint32 &tick) {
			Q_ASSERT(logic);

			logic->npcAddToPoint(QStringLiteral("soldier04"), "entry1", 6, 120);

			cpVect pos = logic->player()->bodyPosition();


			Rpg::RpgLogicScope scope = logic->getScope();

			quint32 id = logic->getId(127);

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

		step.infoIcon = 		"qrc:/Qaterial/Icons/abacus.svg";
		step.infoTitle = "Kövi utasítás";
		step.infoText = "Menjél gyorsan oda, mielőtt meggondolom magam. Ez már a másik";

		/*auto ev = std::make_unique<RpgStream::EventStageChanged>();
		ev->config().setStage(RpgStream::GameConfig::StageMain);

		step.inputEvents.emplace_back(std::move(ev));*/

		step.addTargetControlEvent([](TiledObjectBody *obj) {
			if (dynamic_cast<RpgTower*>(obj))
				return true;
			else
				return false;
		});

		step.fnNext = [](Rpg::RpgLogicClientTutorial *logic, const quint32 &tick) {
			Q_ASSERT(logic);

			logic->npcAddToPoint(QStringLiteral("skeleton01"), {"entry1", "entry2", "entry3"}, 6, 120);
		};



		tutorial->steps.emplace_back(std::move(step));


	}

	return tutorial;
}



/**
 * @brief TutorialData::testCharacter
 * @param url
 * @return
 */

std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> TutorialData::testCharacter(const QUrl &url)
{
	std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> tutorial = std::make_unique<Rpg::RpgLogicClientTutorial::Tutorial>();
	tutorial->character = url.path().mid(1);
	tutorial->terrain = "test";
	tutorial->power = 8;
	tutorial->duration = 2*60*60;

	tutorial->addTower(120);
	tutorial->addEmitter(127);


	tutorial->fnFirst = [character = tutorial->character](Rpg::RpgLogicClientTutorial *logic) {
		Q_ASSERT(logic);

		logic->npcAddToPoint(QStringLiteral("soldier02"), "entry2", RpgStream::TeamB);

		Rpg::RpgLogicScope scope = logic->getScope();

		Rpg::EventMpCreate ev2;
		ev2.emitter = scope.entityFromIdTag(logic->getId(127));
		ev2.mpCount = 8;
		ev2.setTick(120);

		logic->eventStore(std::move(ev2));


		logic->towerSet(120, RpgStream::TeamB, 100);

		RpgPlayerDefinition def = RpgGame::characters().value(character);

		if (def.name.isEmpty()) {
			LOG_CERROR("game") << "Invalid character" << character;
			return;
		}

		logic->defenderAddToPoint("entry3", RpgStream::BaseDefenderObject::Questionnaire, RpgStream::TeamB);

		for (const RpgStream::BaseDefenderObject::Type &d : def.defender) {
			logic->defenderAddToTower(120, d, RpgStream::TeamB);
		}

		CfgPowerLevel pwr = CfgPowerLevel::fromPlayerConfig(def.toPlayerConfig()).atLevel(8);

		Rpg::EventPatchPlayer ev;
		ev.tagId = Rpg::RpgLogic::packId(0, 1, 0);
		ev.deltaState.setBulletDelta(pwr.bullet, true);
		ev.deltaState.setMpDelta(pwr.mp, true);
		ev.setTick(1);

		logic->eventStore(std::move(ev));
	};

	return tutorial;
}




/**
 * @brief TutorialData::defaultTutorial
 * @param url
 * @return
 */

std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> TutorialData::defaultTutorial(const QUrl &)
{
	std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> tutorial = std::make_unique<Rpg::RpgLogicClientTutorial::Tutorial>();
	tutorial->character = "character01a";
	tutorial->terrain = "test";
	tutorial->power = 1;
	tutorial->duration = CFG_GAME_DURATION;

	tutorial->addTower(120);
	tutorial->addEmitter(127);
	tutorial->addChest("entry3");
	tutorial->addChest("entry2");


	tutorial->fnFirst = [](Rpg::RpgLogicClientTutorial *logic) {
		Q_ASSERT(logic);

		logic->npcAddToPoint(QStringLiteral("soldier02"), "entry2");

		logic->towerSet(120, RpgStream::TeamB, 100);
		logic->defenderAddToTower(120, RpgStream::BaseDefenderObject::Pulse, RpgStream::TeamB);
		logic->defenderAddToTower(120, RpgStream::BaseDefenderObject::Electric, RpgStream::TeamB);
	};

	{
		Rpg::RpgLogicClientTutorial::Tutorial::Step step;

		step.message = "Jöhet mindjárt";

		step.infoIcon = 		"qrc:/Qaterial/Icons/abacus.svg";
		step.infoTitle = "Menj oda";
		step.infoText = "Menjél gyorsan oda, mielőtt meggondolom magam.";

		step.addTargetControlEvent([](TiledObjectBody *obj) {
			if (dynamic_cast<RpgTower*>(obj))
				return true;
			else
				return false;
		});

		step.fnNext = [](Rpg::RpgLogicClientTutorial *logic, const quint32 &tick) {
			Q_ASSERT(logic);

			logic->npcAddToPoint(QStringLiteral("soldier04"), "entry1", 6, 120);

			cpVect pos = logic->player()->bodyPosition();


			Rpg::RpgLogicScope scope = logic->getScope();

			quint32 id = logic->getId(127);

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

		step.infoIcon = 		"qrc:/Qaterial/Icons/abacus.svg";
		step.infoTitle = "Kövi utasítás";
		step.infoText = "Menjél gyorsan oda, mielőtt meggondolom magam. Ez már a másik";

		/*auto ev = std::make_unique<RpgStream::EventStageChanged>();
		ev->config().setStage(RpgStream::GameConfig::StageMain);

		step.inputEvents.emplace_back(std::move(ev));*/

		step.addTargetControlEvent([](TiledObjectBody *obj) {
			if (dynamic_cast<RpgTower*>(obj))
				return true;
			else
				return false;
		});

		step.fnNext = [](Rpg::RpgLogicClientTutorial *logic, const quint32 &tick) {
			Q_ASSERT(logic);

			logic->npcAddToPoint(QStringLiteral("skeleton01"), {"entry1", "entry2", "entry3"}, 6, 120);
		};



		tutorial->steps.emplace_back(std::move(step));


	}

	return tutorial;
}
