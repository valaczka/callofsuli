/*
 * ---- Call of Suli ----
 *
 * actiongame.cpp
 *
 * Created on: 2022. 12. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ActionGame
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

#include "gameenemysniper.h"
#include "gameenemysoldier.h"
#include "gameplayer.h"
#include "gameenemy.h"
#include "actiongame.h"
#include "client.h"
#include "gamequestion.h"
#include "qdiriterator.h"
#include "utils_.h"
#include <QRandomGenerator>
#include <QtMath>
#include <Logger.h>

#if defined(Q_OS_ANDROID) || defined (Q_OS_IOS)
#include "qapplication.h"
#include "qscreen.h"
#endif

#ifndef Q_OS_WASM
#include "standaloneclient.h"
#endif



const int ActionGame::TickTimer::m_interval = 2000;


// Tool dependency

const QHash<QString, QVector<GamePickable::PickableType>>
ActionGame::m_toolDependency({
								 { QStringLiteral("fence"), {GamePickable::PickablePliers} },
								 { QStringLiteral("fire"), {GamePickable::PickableWater} },
								 { QStringLiteral("teleport"), {GamePickable::PickableTeleporter} }
							 });


QStringList ActionGame::m_availableCharacters;


const QHash<QString, ActionGame::Tooltip> ActionGame::m_tooltips {
	{ QStringLiteral("rotate"), {
			QStringLiteral("qrc:/Qaterial/Icons/phone-rotate-landscape.svg"),
					tr("Fordítsd el a kijelzőt"),
					tr("A játék ideje alatt a kijelzőt vízszintesen érdemes tartani")
		} },

	{ QStringLiteral("pinch"), {
			QStringLiteral("qrc:/Qaterial/Icons/gesture-pinch.svg"),
					tr("Áttekintés"),
					tr("A pálya áttekintéséhez csippentsd össze a képernyőt")
		} },

	{ QStringLiteral("keyboard"), {
			QStringLiteral("qrc:/Qaterial/Icons/keyboard.svg"),
					tr("Billentyűk használata"),
					tr("<b>NYILAK</b> Játékos mozgatása<br/>"
					   "<b>SPACE</b> Lövés leadása<br/>"
					   "<b>ENTER</b> Tárgy felvétele<br/>"
					   "<b>F3</b> Teljes pálya megtekintése"),
		} },

	{ QStringLiteral("shield"), {
			QStringLiteral("qrc:/Qaterial/Icons/shield.svg"),
					tr("A pajzs megvéd attól, hogy XP-t veszíts"),
					tr("Ha az ellenség meglő, akkor amíg van pajzsod, abból veszítesz XP helyett")
		} },

	{ QStringLiteral("water"), {
			QStringLiteral("qrc:/internal/game/drop.png"),
					tr("A vízzel elolthatod a tüzet"),
					tr("Közelítsd meg óvatosan a tüzet, majd nyomd meg a vizet ábrázoló gombot")
		} },

	{ QStringLiteral("pliers"), {
			QStringLiteral("qrc:/Qaterial/Icons/pliers.svg"),
					tr("A drótvágóval átvághatod a kerítést"),
					tr("Közelítsd meg a kerítést, majd nyomd meg a drótvágót ábrázoló gombot")
		} },

	{ QStringLiteral("teleporter"), {
			QStringLiteral("qrc:/Qaterial/Icons/remote.svg"),
					tr("Teleportáló"),
					tr("A teleportálóval a teleportok között közlekedhetsz")
		} },

	{ QStringLiteral("camouflage"), {
			QStringLiteral("qrc:/Qaterial/Icons/domino-mask.svg"),
					tr("Az álruhával láthatatlanná válhatsz"),
					tr("Egy álruha használatával 10 másodpercen keresztül nem vesz észre az ellenség")
		} },
};

/**
 * @brief ActionGame::ActionGame
 * @param missionLevel
 * @param client
 */

ActionGame::ActionGame(GameMapMissionLevel *missionLevel, Client *client, const GameMap::GameMode &mode)
	: AbstractLevelGame(mode, missionLevel, client)
{
	Q_ASSERT(missionLevel);

	LOG_CTRACE("game") << "Action game constructed" << mode << this;

	connect(this, &AbstractLevelGame::msecLeftChanged, this, &ActionGame::onMsecLeftChanged);
	connect(this, &AbstractLevelGame::gameTimeout, this, &ActionGame::onGameTimeout);
	connect(this, &ActionGame::toolChanged, this, &ActionGame::toolListIconsChanged);
}


/**
 * @brief ActionGame::~ActionGame
 */

ActionGame::~ActionGame()
{
	m_tickTimer.stop();
	LOG_CTRACE("game") << "Action game destroyed" << this;
}



/**
 * @brief ActionGame::onSceneReady
 */

void ActionGame::onSceneReady()
{
	LOG_CTRACE("game") << "Scene ready" << this;

	createQuestions();
	createEnemyLocations();
	createFixEnemies();
	createInventory();

	pageItem()->setState(QStringLiteral("run"));

	m_scene->playSoundMusic(backgroundMusic());
}



/**
 * @brief ActionGame::onSceneAnimationFinished
 */

void ActionGame::onSceneAnimationFinished()
{
	LOG_CTRACE("game") << "Scene amimation finsihed" << this;

	recreateEnemies();
	createPlayer();
}




/**
 * @brief ActionGame::createPlayer
 */

void ActionGame::createPlayer()
{
	LOG_CDEBUG("game") << "Create player";

	if (!m_scene) {
		LOG_CWARNING("game") << "Missing scene";
		return;
	}

	if (m_player) {
		LOG_CWARNING("game") << "Player already exists";
		return;
	}

	QString character = m_client->server() ? m_client->server()->user()->character() : QStringLiteral("");

	GamePlayer *player = GamePlayer::create(m_scene, character);
	GameTerrain::PlayerPositionData pos = m_scene->getPlayerPosition();
	pos.point.setY(pos.point.y()-player->height());

	player->setPosition(pos.point);
	player->setMaxHp(startHP());
	player->setHp(startHP());
	connect(player, &GamePlayer::died, this, &ActionGame::onPlayerDied);
	connect(this, &ActionGame::runningChanged, player, &GamePlayer::onMovingFlagsChanged);

	setPlayer(player);
}


/**
 * @brief ActionGame::createQuestions
 */

void ActionGame::createQuestions()
{
	QVector<Question> list = AbstractLevelGame::createQuestions();

	foreach (const Question &q, list) {
		auto ql = std::make_unique<QuestionLocation>();
		ql->setQuestion(q);
		ql->setEnemy(nullptr);
		ql->setUsed(0);

		m_questions.push_back(std::move(ql));
	}
}


/**
 * @brief ActionGame::createLocations
 */

void ActionGame::createEnemyLocations()
{
	if (!m_scene) {
		LOG_CWARNING("game") << "Missing scene, don't created any location";
		return;
	}

	foreach (const GameTerrain::EnemyData &e, m_scene->terrain().enemies())
		m_enemies.push_back(std::make_unique<EnemyLocation>(e));

	LOG_CDEBUG("game") << m_enemies.size() << " enemy places created";

	emit activeEnemiesChanged();
}


/**
 * @brief ActionGame::createFixEnemies
 */

void ActionGame::createFixEnemies()
{
	if (!m_scene) {
		LOG_CWARNING("game") << "Missing scene";
		return;
	}

	LOG_CDEBUG("game") << "Create fix enemies";

	int n = 0;

	for (auto &el : m_enemies) {
		const GameTerrain::EnemyData &e = el->enemyData();
		const GameTerrain::EnemyType &type = el->enemyData().type;

		if (el->enemy())
			continue;

		if (type == GameTerrain::EnemySniper) {

			GameEnemySniper *sniper= GameEnemySniper::create(m_scene, e);

			sniper->setFacingLeft(QRandomGenerator::global()->generate() % 2);

			sniper->setX(e.rect.left() + e.rect.width()/2);
			sniper->setY(e.rect.bottom()-sniper->height());

			sniper->startMovingAfter(2500);

			el->setEnemy(sniper);

			connect(sniper, &GameEntity::killed, this, &ActionGame::onEnemyDied);

			const QString &levelKey = QString::number((int)m_missionLevel->level());

			int hp = sniper->dataObject().value(QStringLiteral("hp")).toObject().value(levelKey).toInt(3);

			sniper->setMaxHp(hp);
			sniper->setHp(hp);

			++n;
		}
	}

	LOG_CDEBUG("game") << n << " fix enemies created";

	emit activeEnemiesChanged();
}


/**
 * @brief ActionGame::recreateEnemies
 */

void ActionGame::recreateEnemies()
{
	if (!m_scene) {
		LOG_CWARNING("game") << "Missing scene";
		return;
	}

	LOG_CDEBUG("game") << "Recreate enemies";

	QList<GameEnemy *> soldiers;

	for (auto &el : m_enemies) {
		const GameTerrain::EnemyData &e = el->enemyData();
		const GameTerrain::EnemyType &type = el->enemyData().type;

		if (el->enemy())
			continue;

		if (type != GameTerrain::EnemySoldier || m_closedBlocks.contains(e.block))
			continue;

		ObjectStateEnemySoldier state = GameEnemySoldier::createState(e);
		GameEnemySoldier *soldier = GameEnemySoldier::create(m_scene, e, QString::fromUtf8(state.subType));
		GameObject::updateStateQuickItem(&state, soldier);
		soldier->setCurrentState(state);

		soldier->startMovingAfter(2500);

		el->setEnemy(soldier);

		connect(soldier, &GameEntity::killed, this, &ActionGame::onEnemyDied);

		soldiers.append(soldier);
	}

	LOG_CDEBUG("game") << soldiers.size() << " new enemies created";

	linkQuestionToEnemies(soldiers);
	linkPickablesToEnemies(soldiers);

	emit activeEnemiesChanged();
}



/**
 * @brief ActionGame::createInventory
 */

void ActionGame::createInventory()
{
	m_inventory.clear();

	if (!m_missionLevel) {
		LOG_CWARNING("game") << "Missing game map, don't created any pickable";
		return;
	}

	LOG_CDEBUG("game") << "Create inventory";

	const QHash<QString, GamePickable::GamePickableData> &list = GamePickable::pickableDataHash();

	foreach (GameMapInventory *inventory, m_missionLevel->inventories()) {
		if (!list.contains(inventory->module())) {
			LOG_CWARNING("game") << "Invalid pickable module:" << inventory->module();
			continue;
		}

		Inventory inv(list.value(inventory->module()), inventory->block());
		m_inventory.append(inv);
	}

	LOG_CDEBUG("game") << "Inventory loaded " << m_inventory.size() << " items";

	foreach (const Inventory &inv, m_inventory) {
		LOG_CTRACE("game") << "- " << inv.first.name << " (block: " << inv.second << ")";
	}
}



/**
 * @brief ActionGame::createPickableFromEnemy
 * @param enemy
 */

void ActionGame::createPickable(GameEnemy *enemy)
{
	if (!enemy) {
		LOG_CERROR("game") << "Invalid enemy";
		return;
	}

	const GamePickable::GamePickableData &pickable = enemy->pickable();

	LOG_CDEBUG("game") << "Create pickable from enemy:" << enemy->terrainEnemyData().type;

	QPointF pos = enemy->position();
	const QRectF br = enemy->bodyRect();

	pos.setX(pos.x() + br.x() + br.width()/2);
	pos.setY(pos.y() + br.y() + br.height());

	createPickable(pickable, pos);
}




/**
 * @brief ActionGame::createPickable
 * @param data
 * @param bottomPoint
 */


void ActionGame::createPickable(const GamePickable::GamePickableData &data, const QPointF &bottomPoint)
{
	if (!m_scene) {
		LOG_CERROR("game") << "Invalid scene";
		return;
	}

	LOG_CDEBUG("game") << "Create pickable:" << data.type << bottomPoint;

	GamePickable *object = qobject_cast<GamePickable*>(GameObject::createFromFile("GamePickable.qml", m_scene, false));

	if (!object) {
		LOG_CERROR("scene") << "Pickable creation error:" << data.type;
		return;
	}

	object->setParentItem(m_scene);
	object->setScene(m_scene);
	object->setBottomPoint(bottomPoint);
	object->bodyComplete();

	object->setPickableData(data);

}


/**
 * @brief ActionGame::linkQuestionToEnemies
 * @param enemies
 */

void ActionGame::linkQuestionToEnemies(QList<GameEnemy *> enemies)
{
	LOG_CDEBUG("game") << "Link questions to enemies";

	typedef QVector<QuestionLocation *> QVL;

	QMap<int, QVL> usedList;

	for (auto &ql : std::as_const(m_questions)) {
		if (!ql->enemy()) {
			const int &used = ql->used();
			if (usedList.contains(used)) {
				usedList[used].append(ql.get());
			} else {
				usedList.insert(used, {ql.get()});
			}
		}
	}

	QList<GameEnemy *> notUsed = enemies;

	for (auto it = usedList.begin(); it != usedList.end(); ++it) {
		LOG_CTRACE("game") << "Link questions used " << it.key() << " times: " << it.value().size();

		while (!it.value().isEmpty() && !enemies.isEmpty()) {
			QuestionLocation *ql = it.value().takeAt(QRandomGenerator::global()->bounded(it.value().size()));

			while (!enemies.isEmpty()) {
				GameEnemy *e = enemies.takeAt(QRandomGenerator::global()->bounded(enemies.size()));

				const qreal &block = e->terrainEnemyData().block;

				qreal blockMax = 0;
				int blockQuestions = 0;

				for (auto &el : m_enemies) {
					if (el->enemyData().block != block) {
						continue;
					}

					++blockMax;

					if (el->enemy()->question())
						++blockQuestions;
				}

				if (blockQuestions >= qFloor(blockMax * m_missionLevel->questions())) {
					LOG_CTRACE("game") << "Block oversize, skip:" << block;
					continue;
				}


				ql->setEnemy(e);
				ql->setUsed(ql->used()+1);
				e->setQuestion(ql);
				e->setMaxHp(1);
				e->setHp(1);
				notUsed.removeAll(e);

				break;
			}
		}

		if (enemies.isEmpty())
			break;
	}


	// Set HP

	const QString &levelKey = QString::number((int)m_missionLevel->level());

	foreach (GameEnemy *enemy, notUsed) {
		int hp = enemy->dataObject().value(QStringLiteral("hp")).toObject().value(levelKey).toInt(3);

		enemy->setMaxHp(hp);
		enemy->setHp(hp);
	}

}


/**
 * @brief ActionGame::linkPickablesToEnemies
 * @param enemies
 */

void ActionGame::linkPickablesToEnemies(QList<GameEnemy *> enemies)
{
	LOG_CDEBUG("game") << "Link pickables to enemies:" << enemies.size();

	QVector<int> eBlocks;

	for (auto &el : m_enemies) {
		if (!el->enemy())
			continue;

		int b = el->enemyData().block;

		if (!eBlocks.contains(b))
			eBlocks.append(b);
	}

	int n = 0;

	foreach (const int &block, eBlocks) {
		QVector<GameEnemy *> elist;

		foreach (GameEnemy *e, enemies) {
			if (!e->hasPickable() && e->terrainEnemyData().block == block)
				elist.append(e);
		}

		while (!elist.isEmpty()) {
			QVector<int> idx;
			for (int i=0; i<m_inventory.size(); ++i) {
				if (m_inventory.at(i).second == block)
					idx.append(i);
			}

			if (idx.isEmpty())
				break;

			int x = idx.takeAt(QRandomGenerator::global()->bounded(idx.size()));
			GameEnemy *e = elist.takeAt(QRandomGenerator::global()->bounded(elist.size()));
			enemies.removeAll(e);

			Inventory data = m_inventory.takeAt(x);

			e->setPickable(data.first);
			++n;
		}
	}


	// To everywhere

	QVector<GameEnemy *> elist;

	foreach (GameEnemy *e, enemies) {
		if (!e->hasPickable())
			elist.append(e);
	}

	while (!elist.isEmpty() && !m_inventory.isEmpty()) {
		Inventory data = m_inventory.takeAt(QRandomGenerator::global()->bounded(m_inventory.size()));
		GameEnemy *e = elist.takeAt(QRandomGenerator::global()->bounded(elist.size()));

		e->setPickable(data.first);
		++n;
	}

	LOG_CDEBUG("game") << n << "pickables linked";
}


/**
 * @brief ActionGame::relinkQuestionToEnemy
 * @param enemy
 */

void ActionGame::relinkQuestionToEnemy(GameEnemy *enemy)
{
	Q_ASSERT (enemy);

	if (!enemy->question()) {
		LOG_CWARNING("game") << "Enemy hasn't question location:" << enemy;
	} else {
		enemy->question()->setEnemy(nullptr);
	}

	typedef QVector<QuestionLocation *> QVL;

	QMap<int, QVL> usedList;

	for (auto &ql : std::as_const(m_questions)) {
		if (!ql->enemy()) {
			const int &used = ql->used();
			if (usedList.contains(used)) {
				usedList[used].append(ql.get());
			} else {
				usedList.insert(used, {ql.get()});
			}
		}
	}

	if (m_questions.empty()) {
		LOG_CWARNING("game") << "Question location unavailable";
		return;
	}

	auto it = usedList.begin();

	LOG_CTRACE("game") << "Relink questions used " << it.key() << " times:" << it.value().size();

	QuestionLocation *ql = it.value().takeAt(QRandomGenerator::global()->bounded(it.value().size()));

	ql->setEnemy(enemy);
	ql->setUsed(ql->used()+1);
	enemy->setQuestion(ql);
	enemy->setMaxHp(1);
	enemy->setHp(1);

}


/**
 * @brief ActionGame::tryAttack
 * @param player
 * @param enemy
 */

void ActionGame::tryAttack(GamePlayer *player, GameEnemy *enemy)
{
	if (!player) {
		LOG_CWARNING("game") << "Invalid player to try attack";
		return;
	}

	if (!enemy) {
		LOG_CWARNING("game") << "Invalid enemy to try attack";
		return;
	}

	LOG_CDEBUG("game") << "Try attack";

	if (enemy->hasQuestion()) {
		m_attackedEnemy = enemy;
		m_scene->playSound(QStringLiteral("qrc:/sound/sfx/question.mp3"));
		m_gameQuestion->loadQuestion(enemy->question()->question());
		setRunning(false);
		player->standbyMovingFlags();
	} else {
		enemy->decreaseHp();
	}

}


/**
 * @brief ActionGame::operate
 * @param object
 */

void ActionGame::operateReal(GamePlayer *player, GameObject *object)
{
	if (!player) {
		LOG_CERROR("game") << "Missing player";
		return;
	}

	QString objectType = object ? object->objectType() : "";

	if (objectType.isEmpty()) {
		LOG_CWARNING("game") << "Invalid operating object type:" << objectType;
		player->setOperatingObject(nullptr);
		return;
	}

	connect(object, &GameObject::destroyed, player, [objectType, player](){
		if (player)
			emit player->terrainObjectChanged(objectType, nullptr);
	});

	QMetaObject::invokeMethod(object, "operate", Qt::QueuedConnection);
}



/**
 * @brief ActionGame::canOperate
 * @param type
 * @return
 */

bool ActionGame::canOperate(const QString &type) const
{
	if (!m_toolDependency.contains(type))
		return true;

	const QVector<GamePickable::PickableType> &list = m_toolDependency.value(type);

	foreach (const GamePickable::PickableType &t, list) {
		if (toolCount(t) < 1)
			return false;
	}

	return true;
}


/**
 * @brief ActionGame::canOperate
 * @param object
 * @return
 */

bool ActionGame::canOperate(GameObject *object) const
{
	return object ? canOperate(object->objectType()) : true;
}


/**
 * @brief ActionGame::player
 * @return
 */

GamePlayer *ActionGame::player() const
{
	return qobject_cast<GamePlayer*>(m_player);
}


/**
 * @brief ActionGame::setPlayer
 * @param newPlayer
 */

void ActionGame::setPlayer(GamePlayer *newPlayer)
{
	if (m_player == newPlayer)
		return;
	m_player = newPlayer;
	emit playerChanged();

	if (!m_player)
		return;
}




/**
 * @brief ActionGame::loadPage
 * @return
 */

QQuickItem *ActionGame::loadPage()
{
	QQuickItem *page = m_client->stackPushPage(QStringLiteral("PageActionGame.qml"), QVariantMap({
																									 { QStringLiteral("game"), QVariant::fromValue(this) }
																								 }));

	const QVariant &scene = page->property("scene");

	setScene(qvariant_cast<GameScene*>(scene));
	return page;
}


/**
 * @brief ActionGame::connectGameQuestion
 */

void ActionGame::connectGameQuestion()
{
	connect(m_gameQuestion, &GameQuestion::success, this, &ActionGame::onGameQuestionSuccess);
	connect(m_gameQuestion, &GameQuestion::failed, this, &ActionGame::onGameQuestionFailed);

	connect(m_gameQuestion, &GameQuestion::started, this, &ActionGame::onGameQuestionStarted);
	connect(m_gameQuestion, &GameQuestion::finished, this, &ActionGame::onGameQuestionFinished);
}


/**
 * @brief ActionGame::gameFinishEvent
 * @return
 */

bool ActionGame::gameFinishEvent()
{
	if (m_closedSuccesfully)
		return false;

	setRunning(false);
	m_closedSuccesfully = true;
	return true;
}





/**
 * @brief ActionGame::onSceneAboutToStart
 */

void ActionGame::onSceneAboutToStart()
{
	startWithRemainingTime(m_missionLevel->duration()*1000);

	if (m_deathmatch) {
		message(tr("LEVEL %1").arg(level()));
		message(tr("SUDDEN DEATH"));
		m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/sudden_death.mp3"));
	} else {
		message(tr("LEVEL %1").arg(level()));
		m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/begin.mp3"));
	}

	const QJsonObject &data = getServerExtendedData();

	for (auto it=data.constBegin(); it != data.constEnd(); ++it) {
		const int num = it.value().toInt();
		if (num <= 0) {
			LOG_CWARNING("game") << "Server inventory skipped:" << it.key() << it.value();
			continue;
		}

		if (it.key() == QStringLiteral("hp")) {
			if (m_player) {
				LOG_CINFO("game") << "Server inventory HP:" << num;
				message(tr("+%1 HP gained").arg(num));
				m_player->setHp(m_player->hp()+num);
			} else {
				m_client->messageError(tr("Missing player, server inventory HP lost"), tr("Belső hiba"));
			}

			continue;
		} else if (it.key() == QStringLiteral("shield")) {
			if (player()) {
				LOG_CINFO("game") << "Server inventory shield:" << num;
				message(tr("+%1 shield gained").arg(num));
				player()->setShield(player()->shield()+num);
			} else {
				m_client->messageError(tr("Missing player, server inventory shield lost"), tr("Belső hiba"));
			}

			continue;
		}

		const GamePickable::GamePickableData &data = GamePickable::pickableDataHash().value(it.key());

		if (data.type != GamePickable::PickableInvalid) {
			toolAdd(data.type, num);
			message(tr("+1 %1 gained").arg(data.messageName.isEmpty() ? data.name : data.messageName), data.iconColor);
		} else {
			LOG_CWARNING("game") << "Server inventory skipped:" << it.key() << it.value();
		}

	}

}



/**
 * @brief ActionGame::onSceneStarted
 */

void ActionGame::onSceneStarted()
{
	timeNotifySendReset();

	onSceneAboutToStart();

#if defined(Q_OS_ANDROID) || defined (Q_OS_IOS)
	QScreen *screen = QApplication::primaryScreen();
	if (screen &&
			(screen->primaryOrientation() == Qt::ScreenOrientation::PortraitOrientation ||
			 screen->primaryOrientation() == Qt::ScreenOrientation::InvertedPortraitOrientation)) {
		if (dialogMessageTooltipById(QStringLiteral("rotate")))
			dialogMessageTooltipById(QStringLiteral("pinch"));
	} else {
		dialogMessageTooltipById(QStringLiteral("pinch"));
	}
#elif defined(Q_OS_WIN) || (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_MACOS) || defined(Q_OS_WASM)
	dialogMessageTooltipById(QStringLiteral("keyboard"));
#endif

}


/**
 * @brief ActionGame::onMsecLeftChanged
 * @param diff
 */

void ActionGame::onMsecLeftChanged()
{
	const int &msec = msecLeft();

	if (m_timeNotifySendNext > msec) {
		QString msg;
		QString voice;

		if (msec <= 30000) {
			msg = tr("You have 30 seconds left");
			voice = QStringLiteral("qrc:/sound/voiceover/final_round.mp3");
			m_timeNotifySendNext = -1;
		} else if (msec <= 60000) {
			msg = tr("You have 60 seconds left");
			voice = QStringLiteral("qrc:/sound/voiceover/time.mp3");
			m_timeNotifySendNext = 30000;
		}

		if (!msg.isEmpty()) {
			message(msg, QStringLiteral("#00bcd4"));
			m_scene->playSoundVoiceOver(voice);

			emit timeNotify();
			return;
		}
	}

}




/**
 * @brief ActionGame::onGameQuestionSuccess
 * @param answer
 */

void ActionGame::onGameQuestionSuccess(const QVariantMap &answer)
{
	addStatistics(m_gameQuestion->module(), m_gameQuestion->objectiveUuid(), true, m_gameQuestion->elapsedMsec());

	int xp = m_gameQuestion->questionData().value(QStringLiteral("xpFactor"), 0.0).toReal() * (qreal) ACTION_GAME_BASE_XP;
	setXp(m_xp+xp);

	m_scene->playSound(QStringLiteral("qrc:/sound/sfx/correct.mp3"));
	m_gameQuestion->answerReveal(answer);
	m_gameQuestion->setMsecBeforeHide(0);
	m_gameQuestion->finish();

	m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/winner.mp3"));

	if (m_attackedEnemy) {
		m_attackedEnemy->kill();
		m_attackedEnemy = nullptr;
	}

}


/**
 * @brief ActionGame::onGameQuestionFailed
 * @param answer
 */

void ActionGame::onGameQuestionFailed(const QVariantMap &answer)
{
#ifndef Q_OS_WASM
	StandaloneClient *client = qobject_cast<StandaloneClient*>(m_client);
	if (client)
		client->performVibrate();
#endif

	addStatistics(m_gameQuestion->module(), m_gameQuestion->objectiveUuid(), false, m_gameQuestion->elapsedMsec());

	if (m_player)
		player()->hurtByEnemy(nullptr, false);

	m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/loser.mp3"));

	m_gameQuestion->answerReveal(answer);
	m_gameQuestion->setMsecBeforeHide(1250);

	GameEnemy *enemy = qobject_cast<GameEnemy*>(m_attackedEnemy);

	if (enemy) {
		m_attackedEnemy = nullptr;
		enemy->missedByPlayer(player());
		relinkQuestionToEnemy(enemy);
	}

	m_gameQuestion->finish();

}


/**
 * @brief ActionGame::onGameQuestionStarted
 */

void ActionGame::onGameQuestionStarted()
{
	m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/fight.mp3"));

}


/**
 * @brief ActionGame::onGameQuestionFinished
 */

void ActionGame::onGameQuestionFinished()
{
	setRunning(true);
	m_scene->forceActiveFocus(Qt::OtherFocusReason);
}


/**
 * @brief ActionGame::onGameTimeout
 */

void ActionGame::onGameTimeout()
{
	m_scene->stopSoundMusic();

	setFinishState(Fail);
	gameFinish();
	m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"));
	m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/you_lose.mp3"));
	dialogMessageFinish(tr("Lejárt az idő"), "qrc:/Qaterial/Icons/timer-sand.svg", false);

}


/**
 * @brief ActionGame::onGameSuccess
 */

void ActionGame::onGameSuccess()
{
	m_scene->stopSoundMusic();

	setFinishState(Success);
	gameFinish();
	m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/sfx/win.mp3"));
	m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"));
	m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/you_win.mp3"));

	QTimer::singleShot(2000, this, [this](){
		dialogMessageFinish(m_isFlawless ? tr("Mission completed\nHibátlan győzelem!") : tr("Mission completed"), "qrc:/Qaterial/Icons/trophy.svg", true);
	});
}




/**
 * @brief ActionGame::onGameFailed
 */

void ActionGame::onGameFailed()
{
	m_scene->stopSoundMusic();

	setFinishState(Fail);
	gameFinish();
	m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"));
	m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/you_lose.mp3"));
	dialogMessageFinish(tr("Your man has died"), "qrc:/Qaterial/Icons/skull-crossbones.svg", false);
}



/**
 * @brief ActionGame::timeNotifySendReset
 */

void ActionGame::timeNotifySendReset()
{
	m_timeNotifySendNext = 60000;
}


/**
 * @brief ActionGame::availableCharacters
 * @return
 */

const QStringList &ActionGame::availableCharacters()
{
	return m_availableCharacters;
}



/**
 * @brief ActionGame::getExtendedData
 * @return
 */

QJsonObject ActionGame::getExtendedData() const
{
	QJsonObject data;

	if (m_deathmatch)
		return data;

	if (player()) {
		data.insert(QStringLiteral("hp"), qMin(m_player->hp()-1, 2));
		data.insert(QStringLiteral("shield"), player()->shield());
	}

	foreach (const GamePickable::GamePickableData &d, GamePickable::pickableDataTypes()) {
		const int cnt = toolCount(d.type);

		if (cnt > 0)
			data.insert(d.id, cnt);
	}

	return data;
}



/**
 * @brief ActionGame::sceneTimerTimeout
 */

void ActionGame::sceneTimerTimeout(const int &msec, const qreal &delayFactor)
{
	if (!m_scene) {
		LOG_CWARNING("game") << "Missing scene";
		return;
	}

	foreach (GameObject *o, m_scene->m_gameObjects)
		if (o)
			o->onTimingTimerTimeout(msec, delayFactor);


	foreach (GameObject *o, m_scene->m_gameObjects) {
		GameEntity *e = qobject_cast<GameEntity*>(o);
		if (e)
			e->performRayCast();
	}
}


/**
 * @brief ActionGame::toolDependency
 * @return
 */

const QHash<QString, QVector<GamePickable::PickableType> > &ActionGame::toolDependency() const
{
	return m_toolDependency;
}



/**
 * @brief ActionGame::reloadAvailableCharacters
 */

void ActionGame::reloadAvailableCharacters()
{
	LOG_CDEBUG("game") << "Reload available characters...";

	m_availableCharacters.clear();

	QDirIterator it(QStringLiteral(":/character"), {QStringLiteral("data.json")}, QDir::Files, QDirIterator::Subdirectories);

	while (it.hasNext())
		m_availableCharacters.append(it.next().section('/',-2,-2));

	LOG_CDEBUG("game") << "...loaded " << m_availableCharacters.size() << " characters";
}





/**
 * @brief ActionGame::pickable
 * @return
 */

GamePickable *ActionGame::pickable() const
{
	return m_pickableStack.isEmpty() ? nullptr : m_pickableStack.top();
}


/**
 * @brief ActionGame::resetKillStreak
 */

void ActionGame::resetKillStreak()
{
	m_killStreak = 0;
}



/**
 * @brief ActionGame::killAllEnemy
 */

void ActionGame::killAllEnemy()
{
	for (auto &e : m_enemies) {
		if (e->enemy())
			e->enemy()->kill();
	}

	emit activeEnemiesChanged();
}



/**
 * @brief ActionGame::toolCount
 * @param type
 */

int ActionGame::toolCount(const GamePickable::PickableType &type) const
{
	return m_tools.value(type, 0);
}


/**
 * @brief ActionGame::toolAdd
 * @param type
 * @param count
 */

void ActionGame::toolAdd(const GamePickable::PickableType &type, const int &count)
{
	int s = m_tools.value(type, 0);
	s = qMax(s+count, 0);
	m_tools.insert(type, s);
	emit toolChanged(type, s);
}


/**
 * @brief ActionGame::toolRemove
 * @param type
 * @param count
 */

void ActionGame::toolRemove(const GamePickable::PickableType &type, const int &count)
{
	int s = m_tools.value(type, 0);
	s = qMax(s-count, 0);
	m_tools.insert(type, s);
	emit toolChanged(type, s);
}


/**
 * @brief ActionGame::toolClear
 * @param type
 */

void ActionGame::toolClear(const GamePickable::PickableType &type)
{
	m_tools.insert(type, 0);
	emit toolChanged(type, 0);
}


/**
 * @brief ActionGame::toolList
 * @return
 */

QVariantList ActionGame::tools()
{
	QVariantList l;

	foreach (const GamePickable::GamePickableData &d, GamePickable::pickableDataTypes()) {
		if (!d.icon.isEmpty()) {
			QStringList depList;

			for (auto it = m_toolDependency.constBegin(); it != m_toolDependency.constEnd(); ++it) {
				if (it.value().contains(d.type))
					depList.append(it.key());
			}

			l.append(QVariantMap({
									 { QStringLiteral("id"), d.id },
									 { QStringLiteral("icon"), d.icon },
									 { QStringLiteral("iconColor"), d.iconColor },
									 { QStringLiteral("dependency"), depList },
									 { QStringLiteral("type"), d.type },
								 }));
		}
	}

	return l;
}




/**
 * @brief ActionGame::closedBlocks
 * @return
 */

const QVector<int> &ActionGame::closedBlocks() const
{
	return m_closedBlocks;
}



GameScene *ActionGame::scene() const
{
	return m_scene;
}

void ActionGame::setScene(GameScene *newScene)
{
	if (m_scene == newScene)
		return;
	m_scene = newScene;
	emit sceneChanged();

	if (m_scene)
		connect(m_scene, &GameScene::sceneStarted, this, &ActionGame::onSceneStarted);
}

bool ActionGame::running() const
{
	return m_running;
}

void ActionGame::setRunning(bool newRunning)
{
	if (m_running == newRunning)
		return;
	m_running = newRunning;
	emit runningChanged();
}


/**
 * @brief ActionGame::EnemyLocation::~EnemyLocation
 */

ActionGame::EnemyLocation::EnemyLocation(const GameTerrain::EnemyData &enemyData)
	: m_enemyData(enemyData)
{

}



ActionGame::EnemyLocation::~EnemyLocation()
{
	if (m_enemy)
		m_enemy->deleteLater();
}



const GameTerrain::EnemyData &ActionGame::EnemyLocation::enemyData() const
{
	return m_enemyData;
}

void ActionGame::EnemyLocation::setEnemyData(const GameTerrain::EnemyData &newEnemyData)
{
	m_enemyData = newEnemyData;
}

GameEnemy *ActionGame::EnemyLocation::enemy() const
{
	return qobject_cast<GameEnemy*>(m_enemy);
}

void ActionGame::EnemyLocation::setEnemy(GameEnemy *newEnemy)
{
	m_enemy = newEnemy;
}


/**
 * @brief ActionGame::activeEnemies
 * @return
 */

int ActionGame::activeEnemies() const
{
	int n = 0;

	for (auto &e : m_enemies) {
		if (e->enemy())
			++n;
	}

	return n;
}


/**
 * @brief ActionGame::onPlayerDied
 */

void ActionGame::onPlayerDied(GameEntity *)
{
	LOG_CINFO("game") << "Player died";
	setPlayer(nullptr);

	message(tr("Your man has died"), QStringLiteral("#e53935"));

	pickableRemoveAll();

	if (m_gameQuestion)
		m_gameQuestion->forceDestroy();

	if (m_deathmatch) {
		onGameFailed();
		return;
	}

	recreateEnemies();

	createPlayer();
}




/**
 * @brief ActionGame::onEnemyDied
 */

void ActionGame::onEnemyDied(GameEntity *entity)
{
	GameEnemy *enemy = qobject_cast<GameEnemy*>(entity);

	if (!enemy) {
		LOG_CERROR("game") << "Invalid enemy entity:" << entity;
		return;
	}

	m_killStreak++;

	LOG_CINFO("game") << "Kill streak:" << m_killStreak;

	int xp = 0;

	if (m_killStreak % 5 == 0) {
		xp = m_killStreak * 15;
		message(QStringLiteral("Kill streak: %1").arg(m_killStreak), QStringLiteral("#FFC107"));
		message(QStringLiteral("+%1 XP").arg(xp), QStringLiteral("#FFC107"));
	}

	setXp(m_xp + ACTION_GAME_ENEMY_KILL_XP + xp);

	int block = -1;

	for (auto &el : m_enemies) {
		if (el->enemy() == enemy) {
			block = el->enemyData().block;
			el->setEnemy(nullptr);
			break;
		}
	}

	emit activeEnemiesChanged();

	if (!activeEnemies()) {
		onGameSuccess();
		LOG_CDEBUG("game") << "Mission completed";
		return;
	}

	// Create pickable

	if (enemy->hasPickable()) {
		createPickable(enemy);
	}

	// Close blocks

	for (auto &el : m_enemies) {
		if (el->enemyData().block == block && el->enemy()) {
			return;
		}
	}

	if (m_closedBlocks.contains(block))
		return;

	m_closedBlocks.append(block);

	LOG_CDEBUG("game") << "Block closed:" << block;

	if (m_scene)
		m_scene->activateLaddersInBlock(block);

}


/**
 * @brief ActionGame::pickableAdd
 * @param pickable
 */

void ActionGame::pickableAdd(GamePickable *pickable)
{
	if (!pickable)
		return;

	m_pickableStack.push(pickable);
	connect(pickable, &QObject::destroyed, this, [this, pickable]() {
		m_pickableStack.removeAll(pickable);
		emit pickableChanged();
	});
	emit pickableChanged();
}


/**
 * @brief ActionGame::pickableRemove
 * @param pickable
 */

void ActionGame::pickableRemove(GamePickable *pickable)
{
	m_pickableStack.removeAll(pickable);
	emit pickableChanged();
}


/**
 * @brief ActionGame::pickableRemoveAll
 */

void ActionGame::pickableRemoveAll()
{
	m_pickableStack.clear();
	emit pickableChanged();
}


/**
 * @brief ActionGame::pickablePick
 */

void ActionGame::pickablePick()
{
	if (!pickable())
		return;

	GamePickable *p = m_pickableStack.pop();
	emit pickableChanged();

	if (!p) {
		LOG_CERROR("game") << "Invalid pickable";
		return;
	}

	QString tooltip;

	switch (p->type()) {
	case GamePickable::PickableShield1:
	case GamePickable::PickableShield2:
	case GamePickable::PickableShield3:
	case GamePickable::PickableShield5:
		tooltip = QStringLiteral("shield");
		break;

	case GamePickable::PickableWater:
		tooltip = QStringLiteral("water");
		break;

	case GamePickable::PickablePliers:
		tooltip = QStringLiteral("pliers");
		break;

	case GamePickable::PickableCamouflage:
		tooltip = QStringLiteral("camouflage");
		break;

	case GamePickable::PickableTeleporter:
		tooltip = QStringLiteral("teleporter");
		break;

	default:
		break;
	}

	p->pick(this);

	if (!tooltip.isEmpty())
		dialogMessageTooltipById(tooltip);

	m_scene->playSound(QStringLiteral("qrc:/sound/sfx/pick.mp3"));
}


/**
 * @brief ActionGame::message
 * @param text
 * @param color
 */

void ActionGame::message(const QString &text, const QColor &color)
{
	if (!m_scene || !m_scene->messageList()) {
		LOG_CINFO("game") << text;
		return;
	}

	LOG_CDEBUG("game") << text;

	QMetaObject::invokeMethod(m_scene->messageList(), "message", Qt::DirectConnection,
							  Q_ARG(QVariant, text),
							  Q_ARG(QVariant, color.name()));
}


/**
 * @brief ActionGame::addMSec
 * @param msec
 */

void ActionGame::addMSec(const qint64 &msec)
{
	addToDeadline(msec);
	timeNotifySendReset();
	emit timeNotify();
}



/**
 * @brief ActionGame::dialogMessageTooltip
 * @param text
 * @param icon
 * @param title
 */

void ActionGame::dialogMessageTooltip(const QString &text, const QString &icon, const QString &title)
{
	if (!m_pageItem) {
		LOG_CINFO("game") << title << text;
		return;
	}

	LOG_CDEBUG("game") << title << text;

	if (player())
		player()->standbyMovingFlags();

	QMetaObject::invokeMethod(m_pageItem, "messageTooltip", Qt::DirectConnection,
							  Q_ARG(QString, text),
							  Q_ARG(QString, icon),
							  Q_ARG(QString, title));
}





/**
 * @brief ActionGame::dialogMessageTooltipById
 * @param msgId
 * @return bool: true - ha már volt korábban, false - ha még nem volt
 */


bool ActionGame::dialogMessageTooltipById(const QString &msgId)
{
	if (msgId.isEmpty())
		return true;

	const QString &id = QStringLiteral("notification/")+msgId;

	if (Utils::settingsGet(id, false).toBool())
		return true;

	if (!m_tooltips.contains(msgId))
		dialogMessageTooltip(msgId, QStringLiteral("qrc:/Qaterial/Icons/information-slab-box-outline.svg"));
	else {
		const Tooltip &t = m_tooltips.value(msgId);
		dialogMessageTooltip(t.text, t.icon, t.title);
	}

	Utils::settingsSet(id, true);

	return false;
}






/**
 * @brief ActionGame::dialogMessageFinish
 * @param text
 * @param icon
 * @param success
 */

void ActionGame::dialogMessageFinish(const QString &text, const QString &icon, const bool &success)
{
	if (!m_pageItem) {
		LOG_CINFO("game") << text;
		return;
	}

	LOG_CDEBUG("game") << text;

	if (player())
		player()->standbyMovingFlags();

	QMetaObject::invokeMethod(m_pageItem, "messageFinish", Qt::DirectConnection,
							  Q_ARG(QString, text),
							  Q_ARG(QString, icon),
							  Q_ARG(bool, success));
}


/**
 * @brief ActionGame::toolUse
 * @param type
 */

void ActionGame::toolUse(const GamePickable::PickableType &type)
{
	LOG_CDEBUG("game") << "Use tool:" << type;

	GamePickable::operate(this, type);
}


/**
 * @brief ActionGame::gameAbort
 */

void ActionGame::gameAbort()
{
	m_scene->stopSoundMusic();
	setFinishState(Fail);

	LOG_CINFO("game") << "Game aborted:" << this;

	gameFinish();
	m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"));
	m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/you_lose.mp3"));
}




/**
 * @brief ActionGame::QuestionLocation::question
 * @return
 */


const Question &ActionGame::QuestionLocation::question() const
{
	return m_question;
}

void ActionGame::QuestionLocation::setQuestion(const Question &newQuestion)
{
	m_question = newQuestion;
}


int ActionGame::QuestionLocation::used() const
{
	return m_used;
}

void ActionGame::QuestionLocation::setUsed(int newUsed)
{
	m_used = newUsed;
}

GameEnemy *ActionGame::QuestionLocation::enemy() const
{
	return qobject_cast<GameEnemy*>(m_enemy);
}

void ActionGame::QuestionLocation::setEnemy(GameEnemy *newEnemy)
{
	m_enemy = newEnemy;
}


/**
 * @brief ActionGame::toolList
 * @return
 */

QVariantList ActionGame::toolListIcons() const
{
	QVariantList l;

	foreach (const QVariant &v, tools()) {
		const QVariantMap &m = v.toMap();

		for (int i=0; i<toolCount(m.value("type").value<GamePickable::PickableType>()); ++i) {
			l.append(m);
		}
	}

	return l;
}
