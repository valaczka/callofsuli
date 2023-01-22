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
#include <QRandomGenerator>
#include <QtMath>
#include <Logger.h>


// Tool dependency

const QHash<QString, QVector<GamePickable::PickableType>>
ActionGame::m_toolDependency({
								 { QStringLiteral("fence"), {GamePickable::PickablePliers} },
								 { QStringLiteral("fire"), {GamePickable::PickableWater} },
								 { QStringLiteral("teleport"), {GamePickable::PickableTeleporter} }
							 });



/**
 * @brief ActionGame::ActionGame
 * @param missionLevel
 * @param client
 */

ActionGame::ActionGame(GameMapMissionLevel *missionLevel, Client *client)
	: AbstractLevelGame(GameMap::Action, missionLevel, client)
{
	Q_ASSERT(missionLevel);

	LOG_CTRACE("game") << "Action game constructed" << this;

	connect(this, &AbstractLevelGame::msecLeftChanged, this, &ActionGame::onMsecLeftChanged);
	connect(this, &AbstractLevelGame::gameTimeout, this, &ActionGame::onGameTimeout);
	connect(this, &ActionGame::toolChanged, this, &ActionGame::toolListIconsChanged);
}


/**
 * @brief ActionGame::~ActionGame
 */

ActionGame::~ActionGame()
{
	qDeleteAll(m_questions);
	qDeleteAll(m_enemies);
	LOG_CTRACE("game") << "Action game destroyed" << this;
}


/**
 * @brief ActionGame::createQuestions
 */

void ActionGame::createQuestions()
{
	qDeleteAll(m_questions);

	QVector<Question> list = AbstractLevelGame::createQuestions();

	foreach (const Question &q, list) {
		QuestionLocation *ql = new QuestionLocation();
		ql->setQuestion(q);
		ql->setEnemy(nullptr);
		ql->setUsed(0);

		m_questions.append(ql);
	}
}


/**
 * @brief ActionGame::createLocations
 */

void ActionGame::createEnemyLocations()
{
	qDeleteAll(m_enemies);

	if (!m_scene) {
		LOG_CWARNING("game") << "Missing scene, don't created any location";
		return;
	}

	foreach (const GameTerrain::EnemyData &e, m_scene->terrain().enemies())
		m_enemies.append(new EnemyLocation(e));

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

	foreach (EnemyLocation *el, m_enemies) {
		const GameTerrain::EnemyData &e = el->enemyData();
		const GameTerrain::EnemyType &type = el->enemyData().type;

		if (el->enemy())
			continue;

		if (type == GameTerrain::EnemySniper) {

			GameEnemySniper *sniper= GameEnemySniper::create(m_scene, e);

			sniper->setFacingLeft(QRandomGenerator::global()->generate() % 2);

			sniper->setX(e.rect.left() + e.rect.width()/2);
			sniper->setY(e.rect.bottom()-sniper->height());

			m_scene->addChildItem(sniper);

			sniper->startMovingAfter(2500);

			el->setEnemy(sniper);

			connect(sniper, &GameEntity::killed, this, &ActionGame::onEnemyDied);

			const QString &levelKey = QString::number((int)m_missionLevel->level());

			int hp = sniper->dataObject().value(QStringLiteral("hp")).toObject().value(levelKey).toInt(3);

			sniper->setMaxHp(hp);
			sniper->setHp(hp);

			QCoreApplication::processEvents();

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

	foreach (EnemyLocation *el, m_enemies) {
		const GameTerrain::EnemyData &e = el->enemyData();
		const GameTerrain::EnemyType &type = el->enemyData().type;

		if (el->enemy())
			continue;

		if (type != GameTerrain::EnemySoldier || m_closedBlocks.contains(e.block))
			continue;

		GameEnemySoldier *soldier = GameEnemySoldier::create(m_scene, e);

		soldier->setFacingLeft(QRandomGenerator::global()->generate() % 2);

		soldier->setX(e.rect.left() + e.rect.width()/2);
		soldier->setY(e.rect.bottom()-soldier->height());

		m_scene->addChildItem(soldier);

		soldier->startMovingAfter(2500);

		el->setEnemy(soldier);

		connect(soldier, &GameEntity::killed, this, &ActionGame::onEnemyDied);

		QCoreApplication::processEvents();

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

	GamePickable *object = qobject_cast<GamePickable*>(GameObject::createFromFile("GamePickable.qml", m_scene));

	if (!object) {
		qCCritical(lcScene).noquote() << tr("Pickable creation error:") << data.type;
		return;
	}

	object->setParentItem(m_scene);
	object->setScene(m_scene);
	object->setBottomPoint(bottomPoint);
	object->bodyComplete();

	m_scene->addChildItem(object);

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

	foreach (QuestionLocation *ql, m_questions) {
		if (!ql->enemy()) {
			const int &used = ql->used();
			if (usedList.contains(used)) {
				usedList[used].append(ql);
			} else {
				usedList.insert(used, {ql});
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

				foreach (EnemyLocation *el, m_enemies) {
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

	foreach (EnemyLocation *el, m_enemies) {
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

	foreach (QuestionLocation *ql, m_questions) {
		if (!ql->enemy()) {
			const int &used = ql->used();
			if (usedList.contains(used)) {
				usedList[used].append(ql);
			} else {
				usedList.insert(used, {ql});
			}
		}
	}

	if (m_questions.isEmpty()) {
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

	LOG_CDEBUG("game") << "Setup new player";

	m_player->setMaxHp(startHP());
	m_player->setHp(startHP());
	connect(m_player, &GamePlayer::died, this, &ActionGame::onPlayerDied);
	connect(this, &ActionGame::runningChanged, newPlayer, &GamePlayer::onMovingFlagsChanged);
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
 * @brief ActionGame::onSceneStarted
 */

void ActionGame::onSceneStarted()
{
	timeNotifySendReset();
	startWithRemainingTime(m_missionLevel->duration()*1000);

	if (m_deathmatch) {
		message(tr("LEVEL %1 SUDDEN DEATH").arg(level()));
		m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/sudden_death.mp3"));
	} else {
		message(tr("LEVEL %1").arg(level()));
		m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/begin.mp3"));
	}

	//dialogMessageTooltip("Ez a szvöeg am ia éajer oadjf lkéasdjf", "qrc:/Qaterial/Icons/message-question.svg");
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
	addStatistics(m_gameQuestion->objectiveUuid(), true, m_gameQuestion->elapsedMsec());

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
	if (m_player) {
		addStatistics(m_gameQuestion->objectiveUuid(), false, m_gameQuestion->elapsedMsec());

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
	m_scene->stopSoundMusic(backgroundMusic());

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
	m_scene->stopSoundMusic(backgroundMusic());

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
	m_scene->stopSoundMusic(backgroundMusic());

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
 * @brief ActionGame::toolDependency
 * @return
 */

const QHash<QString, QVector<GamePickable::PickableType> > &ActionGame::toolDependency() const
{
	return m_toolDependency;
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
 * @brief ActionGame::killAllEnemy
 */

void ActionGame::killAllEnemy()
{
	foreach (EnemyLocation *e, m_enemies) {
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



void ActionGame::testQuestion()
{
	m_gameQuestion->setPostponeEnabled(true);
	m_scene->playSound(QStringLiteral("qrc:/sound/sfx/question.mp3"));
	m_gameQuestion->loadQuestion(QUrl(QStringLiteral("qrc:/GameQuestionDefaultComponent.qml")), {
									 {QStringLiteral("question"), QStringLiteral("Na ez már jó kérdés")},
									 //{"decimalEnabled", true},
									 //{"twoLine", true},
									 {QStringLiteral("answer"), QVariantMap({
										   {"first", 2 },
										  {"second", 3 }
									  })},
									 {QStringLiteral("options"), QStringList({
										  "egy",
										  "kettő afaiodf aélsd fwioe alékfj alésdjfioweéaklsjf léaksfiowe éajkdf éalskdfoweir aéldkfjd alésdfowier éaklsdfj aseofa dfwei aédfjk aoir asdklf",
										  "három saklf weio alkdjfl askdjf",
										  "négy",
										  "öt",
										  "hat"
									  }) },

									 {"image", "file:///home/valaczka/Letöltések/bg.jpg"}
									 /*{"imageAnswers", true},
																																																																																																																				  {"options", QStringList({
																																																																																																																					   "file:///home/valaczka/Letöltések/bg.jpg",
																																																																																																																					   "file:///home/valaczka/Letöltések/3centiho.jpg",
																																																																																																																					   "file:///home/valaczka/Letöltések/logo_gb.jpg",
																																																																																																																					   "file:///home/valaczka/Letöltések/vitorlas.jpg",
																																																																																																																				   })}*/
								 });
	setRunning(false);
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

	foreach (EnemyLocation *e, m_enemies) {
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
	LOG_CDEBUG("game") << "Player died";
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

	m_scene->createPlayer();
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

	int block = -1;

	foreach (EnemyLocation *el, m_enemies) {
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

	foreach (EnemyLocation *el, m_enemies) {
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

	p->pick(this);

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

	QMetaObject::invokeMethod(m_pageItem, "messageTooltip", Qt::DirectConnection,
							  Q_ARG(QString, text),
							  Q_ARG(QString, icon),
							  Q_ARG(QString, title));
}



/**
 * @brief ActionGame::dialogMessageTooltip
 * @param msgId
 * @param title
 */

void ActionGame::dialogMessageTooltipById(const QString &msgId, const QString &title)
{

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
	m_scene->stopSoundMusic(backgroundMusic());
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
