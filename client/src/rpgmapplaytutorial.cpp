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
	static std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> testCharacter(const QUrl &url);

	static std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> defaultTutorial(const QUrl &url);


	static inline const QHash<QString, std::function<std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial>(const QUrl &)> >
	tutorials = {
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
 * @brief TutorialData::testCharacter
 * @param url
 * @return
 */

std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> TutorialData::testCharacter(const QUrl &url)
{
	std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> tutorial = std::make_unique<Rpg::RpgLogicClientTutorial::Tutorial>();
	tutorial->character = url.path().mid(1);
	tutorial->terrain = QStringLiteral("map_tutorial");
	tutorial->power = 8;
	tutorial->duration = 2*60*60;

	tutorial->addTower(17);
	tutorial->addTower(33);
	tutorial->addEmitter(19);


	tutorial->fnFirst = [character = tutorial->character](Rpg::RpgLogicClientTutorial *logic) {
		Q_ASSERT(logic);

		logic->npcAddToPoint(QStringLiteral("soldier02"), "entry2", RpgStream::TeamB);

		Rpg::RpgLogicScope scope = logic->getScope();

		Rpg::EventMpCreate ev2;
		ev2.emitter = scope.entityFromIdTag(logic->getId(19));
		ev2.mpCount = 8;
		ev2.setTick(120);

		logic->eventStore(std::move(ev2));


		logic->towerSet(17, RpgStream::TeamB, 100);

		RpgPlayerDefinition def = RpgGame::characters().value(character);

		if (def.name.isEmpty()) {
			LOG_CERROR("game") << "Invalid character" << character;
			return;
		}

		///logic->defenderAddToPoint("entry3", RpgStream::BaseDefenderObject::Questionnaire, RpgStream::TeamB);

		for (const RpgStream::BaseDefenderObject::Type &d : def.defender) {
			logic->defenderAddToTower(17, d, RpgStream::TeamB);
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
	tutorial->terrain = QStringLiteral("map_tutorial");
	tutorial->power = 1;
	tutorial->duration = CFG_GAME_DURATION;

	tutorial->addTower(17);
	tutorial->addEmitter(19);

	tutorial->noChests();

	//tutorial->addChest("entry3");
	//tutorial->addChest("entry8");

	RpgStream::Quest q;
	q.setQuestion(4);
	q.setStreak(2);
	q.setPts(400);

	tutorial->questList.emplace_back(std::move(q));


	tutorial->fnFirst = [](Rpg::RpgLogicClientTutorial *logic) {
		Q_ASSERT(logic);

		logic->towerSet(17, RpgStream::TeamA, 70);
	};

	{
		Rpg::RpgLogicClientTutorial::Tutorial::Step step;

		step.message = QObject::tr("Menj a Power Generator mellé");

		step.infoIcon = "qrc:/Qaterial/Icons/flash-circle.svg";
		step.infoTitle = QObject::tr("Power Generator");
		step.infoText = QObject::tr("Power Pointokat a Power Generator segítségével tudsz termelni. Menj oda, és aktiváld.");

		step.addTargetControlEvent([](TiledObjectBody *obj) {
			if (dynamic_cast<RpgTower*>(obj))
				return true;
			else
				return false;
		});

		tutorial->steps.emplace_back(std::move(step));
	}



	{
		Rpg::RpgLogicClientTutorial::Tutorial::Step step;

		step.message = QObject::tr("Aktiváld a Power Generatort");

		step.infoIcon = "qrc:/Qaterial/Icons/flash-circle.svg";
		step.infoTitle = QObject::tr("Aktiválás");
		step.infoText = QObject::tr("A Power Generatort az ENTER lenyomásával, vagy a joystickra kattintással tudod aktiválni. "
									"Az aktiváláshoz helyes válasz szükséges.");

		step.addTowerEvent([](Rpg::RpgLogicClientTutorial *logic, const Rpg::EventTowerActiveChanged &event) {
			Rpg::RpgLogicScope scope = logic->getScope();

			auto ent = scope.entityFromIdTag(logic->getId(17));

			return (event.tower == ent) && event.active && event.team == RpgStream::TeamA;
		});

		step.fnNext = [](Rpg::RpgLogicClientTutorial *logic, const quint32 &tick) {
			Q_ASSERT(logic);

			Rpg::RpgLogicScope scope = logic->getScope();

			Rpg::EventMpCreate ev;
			ev.emitter = scope.entityFromIdTag(logic->getId(19));
			ev.mpCount = 8;
			ev.setTick(tick+1);

			logic->eventStore(std::move(ev));

			logic->npcAddToPoint(QStringLiteral("soldier01"), "entry2");

			//logic->chestAddToPoint(QStringLiteral("entry8"));

		};

		tutorial->steps.emplace_back(std::move(step));

	}

	{
		Rpg::RpgLogicClientTutorial::Tutorial::Step step;

		step.message = QObject::tr("Gyűjts össze %1 MP-t")
					   .arg(cfgRequiredMpDefender.value(RpgStream::BaseDefenderObject::Multiplier1));

		step.infoIcon = "qrc:/Qaterial/Icons/shimmer.svg";
		step.infoTitle = QObject::tr("MP");
		step.infoText = QObject::tr("A Power Generator megvédéséhez szükséged van töltényre, védőeszközre, vagy speciális képességre. "
									"Ezeket elegendő MP esetén tudod megszerezni. Menj és gyűjtsd össze az MP-ket");

		/*auto ev = std::make_unique<RpgStream::EventStageChanged>();
		ev->config().setStage(RpgStream::GameConfig::StageMain);

		step.inputEvents.emplace_back(std::move(ev));*/

		step.addPlayerEvent([](RpgPlayer *player, const RpgStream::EventPlayer &event) {
			return event.type() == RpgStream::EventPlayer::EventMpPick && player &&
					player->mp() >= cfgRequiredMpDefender.value(RpgStream::BaseDefenderObject::Multiplier1)-1;
		});

		/*step.addTargetControlEvent([](TiledObjectBody *obj) {
			if (dynamic_cast<RpgTower*>(obj))
				return true;
			else
				return false;
		});*/



		/*step.fnNext = [](Rpg::RpgLogicClientTutorial *logic, const quint32 &) {
			Q_ASSERT(logic);
			logic->npcAddToPoint(QStringLiteral("soldier01"), "entry2");
		};*/



		tutorial->steps.emplace_back(std::move(step));
	}



	{
		Rpg::RpgLogicClientTutorial::Tutorial::Step step;

		step.message = QObject::tr("Tölts be egy Multiplicatort");

		step.infoIcon = "qrc:/Qaterial/Icons/creation.svg";
		step.infoTitle = QObject::tr("MP konvertálás");
		step.infoText = QObject::tr("Váltsd át az összegyűjtött MP-ket Multiplicator védőeszközre. "
									"Kattints hozzá az MP konvertáló gombra.");

		step.addPlayerEvent([](RpgPlayer *, const RpgStream::EventPlayer &event) {
			return event.type() == RpgStream::EventPlayer::EventChangeDefender;
		});


		tutorial->steps.emplace_back(std::move(step));
	}


	{
		Rpg::RpgLogicClientTutorial::Tutorial::Step step;

		step.message = QObject::tr("Helyezd el a Multiplicatort");

		step.infoIcon = "qrc:/Qaterial/Icons/fan-speed-2.svg";
		step.infoTitle = QObject::tr("Védőeszköz elhelyezése");
		step.infoText = QObject::tr("Helyezd el a Multiplicatort a Power Generator mellett. "
									"Menj a Generatorhoz, és kattints a zöld joystickra. "
									"Ha szükséges, előtte aktviáld újra a Generatort.");

		step.addPlayerEvent([](RpgPlayer *, const RpgStream::EventPlayer &event) {
			return event.type() == RpgStream::EventPlayer::EventDefender;
		});


		step.fnNext = [](Rpg::RpgLogicClientTutorial *logic, const quint32 &tick) {
			Q_ASSERT(logic);

			Rpg::RpgLogicScope scope = logic->getScope();

			Rpg::EventMpCreate ev;
			ev.emitter = scope.entityFromIdTag(logic->getId(19));
			ev.mpCount = CFG_MP_CHANGE_BULLET;
			ev.setTick(tick+1);

			logic->eventStore(std::move(ev));
		};


		tutorial->steps.emplace_back(std::move(step));
	}


	{
		Rpg::RpgLogicClientTutorial::Tutorial::Step step;

		step.message = QObject::tr("Gyűjts össze %1 MP-t").arg(CFG_MP_CHANGE_BULLET);

		step.infoIcon = "qrc:/Qaterial/Icons/shimmer.svg";
		step.infoTitle = QObject::tr("MP");
		step.infoText = QObject::tr("Töltény megszerzéséhez menj és gyűjtsd össze az MP-ket");


		step.addPlayerEvent([](RpgPlayer *player, const RpgStream::EventPlayer &event) {
			return event.type() == RpgStream::EventPlayer::EventMpPick && player && player->mp() >= CFG_MP_CHANGE_BULLET-1;
		});


		tutorial->steps.emplace_back(std::move(step));
	}


	{
		Rpg::RpgLogicClientTutorial::Tutorial::Step step;

		step.message = QObject::tr("Töltsd fel a töltényeket");

		step.infoIcon = "qrc:/Qaterial/Icons/bullet.svg";
		step.infoTitle = QObject::tr("MP konvertálás");
		step.infoText = QObject::tr("Váltsd át az összegyűjtött MP-ket töltényekre. "
									"Kattints hozzá az MP konvertáló gombra.");

		step.addPlayerEvent([](RpgPlayer *, const RpgStream::EventPlayer &event) {
			return event.type() == RpgStream::EventPlayer::EventChangeBullet;
		});


		tutorial->steps.emplace_back(std::move(step));
	}


	{
		Rpg::RpgLogicClientTutorial::Tutorial::Step step;

		step.message = QObject::tr("Támadd meg az ellenfelet");

		step.infoIcon = "qrc:/internal/game/target1.svg";
		step.infoTitle = QObject::tr("Támadás");
		step.infoText = QObject::tr("Célozni és lőni a piros joystick segítségével tudsz. "
									"Ha hosszan lenyomod az MP konvertáló gombot, és van elég MP-d, tudsz váltani a joystick funkciói között.");

		step.addPlayerEvent([](RpgPlayer *player, const RpgStream::EventPlayer &event) {
			return event.type() == RpgStream::EventPlayer::EventAttackPlayer && player->bullet() <= 3;
		});


		step.fnNext = [](Rpg::RpgLogicClientTutorial *logic, const quint32 &tick) {
			Q_ASSERT(logic);

			Rpg::RpgLogicScope scope = logic->getScope();

			Rpg::EventMpCreate ev;
			ev.emitter = scope.entityFromIdTag(logic->getId(19));
			ev.mpCount = CFG_MP_CHANGE_BULLET;
			ev.setTick(tick+5*60);

			logic->eventStore(std::move(ev));

			logic->chestAddToPoint(QStringLiteral("entry8"));
		};


		tutorial->steps.emplace_back(std::move(step));
	}



	{
		Rpg::RpgLogicClientTutorial::Tutorial::Step step;

		step.message = QObject::tr("Nyisd ki a ládát");

		step.infoIcon = "qrc:/internal/game/target1.svg";
		step.infoTitle = QObject::tr("Upgrade danger");
		step.infoText = QObject::tr("A pályán nehezíteni egy-egy láda kinyitásával tudsz. "
									"Minden kinyitott láda +50%-kal fogja növelni a jutalmat sikeres teljesítés esetén.");

		step.addPlayerEvent([](RpgPlayer *, const RpgStream::EventPlayer &event) {
			return event.type() == RpgStream::EventPlayer::EventUseControl;
		});


		step.fnNext = [t = tutorial.get()](Rpg::RpgLogicClientTutorial *logic, const quint32 &tick) {
			Q_ASSERT(logic);
			Q_ASSERT(t);

			Rpg::RpgLogicScope scope = logic->getScope();

			RpgStream::GameState *state = scope.getCtx<RpgStream::GameState>();

			Q_ASSERT(state);

			int pts = (std::ceil(state->ptsA() / 100.) * 100) + 150;

			for (auto e : scope.view<Rpg::Player>()) {
				Rpg::Player *p = scope.try_get<Rpg::Player>(e);
				p->playerData.quest().setPts(pts);

				{
					Rpg::RpgLogicClientTutorial::Tutorial::Step step;

					step.infoIcon = "qrc:/internal/game/target1.svg";
					step.infoTitle = QObject::tr("Power Point termelés");
					step.infoText = QObject::tr("Védd meg a Generatort és termelj %1 Power Pointot").arg(pts);

					t->steps.emplace_back(std::move(step));
				}
				break;
			}

			Rpg::EventMpCreate ev;
			ev.emitter = scope.entityFromIdTag(logic->getId(19));
			ev.mpCount = CFG_MP_CHANGE_BULLET;
			ev.setTick(tick+5*60);

			logic->eventStore(std::move(ev));

			logic->npcAddToPoint("soldier04", QStringList{}, 2, 60);

			{
				Rpg::EventMpCreate ev2;
				ev2.emitter = scope.entityFromIdTag(logic->getId(19));
				ev2.mpCount = CFG_MP_CHANGE_BULLET;
				ev2.setTick(tick+30*60);

				logic->eventStore(std::move(ev2));
			}

			{
				Rpg::EventMpCreate ev2;
				ev2.emitter = scope.entityFromIdTag(logic->getId(19));
				ev2.mpCount = CFG_MP_CHANGE_BULLET;
				ev2.setTick(tick+45*60);

				logic->eventStore(std::move(ev2));
			}

			{
				Rpg::EventMpCreate ev2;
				ev2.emitter = scope.entityFromIdTag(logic->getId(19));
				ev2.mpCount = CFG_MP_CHANGE_BULLET;
				ev2.setTick(tick+60*60);

				logic->eventStore(std::move(ev2));
			}

			{
				Rpg::EventMpCreate ev2;
				ev2.emitter = scope.entityFromIdTag(logic->getId(19));
				ev2.mpCount = CFG_MP_CHANGE_BULLET;
				ev2.setTick(tick+90*60);

				logic->eventStore(std::move(ev2));
			}

			{
				Rpg::EventMpCreate ev2;
				ev2.emitter = scope.entityFromIdTag(logic->getId(19));
				ev2.mpCount = CFG_MP_CHANGE_BULLET;
				ev2.setTick(tick+120*60);

				logic->eventStore(std::move(ev2));
			}
		};


		tutorial->steps.emplace_back(std::move(step));
	}


	return tutorial;
}
