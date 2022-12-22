/*
 * ---- Call of Suli ----
 *
 * gameplayer.cpp
 *
 * Created on: 2022. 12. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GamePlayer
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

#include "gameplayer.h"
#include "application.h"
#include "actiongame.h"
#include "gameenemy.h"
#include "gamescene.h"

#ifndef Q_OS_WASM
#include "desktopclient.h"
#endif



/**
 * @brief GamePlayer::GamePlayer
 * @param parent
 */

GamePlayer::GamePlayer(QQuickItem *parent)
	: GameEntity(parent)
{
	qCDebug(lcScene).noquote() << tr("Player created:") << this;

	m_defaultShotSound = "qrc:/sound/sfx/shot.wav";


	m_soundEffectHash.insert("run", QStringList({
													"qrc:/sound/sfx/run1.mp3",
													"qrc:/sound/sfx/run2.mp3",
												}));

	m_soundEffectHash.insert("walk", QStringList({
													 "qrc:/sound/sfx/step1.mp3",
													 "qrc:/sound/sfx/step2.mp3",
												 }));

	m_soundEffectHash.insert("climb", QStringList({
													  "qrc:/sound/sfx/ladderup1.mp3",
													  "qrc:/sound/sfx/ladderup2.mp3",
												  }));

	m_soundEffectHash.insert("ladder", QStringList({
													   "qrc:/sound/sfx/ladder.mp3",
												   }));

	m_soundEffectHash.insert("pain", QStringList({
													 "qrc:/sound/sfx/pain1.mp3",
													 "qrc:/sound/sfx/pain2.mp3",
													 "qrc:/sound/sfx/pain3.mp3",
												 }));



	// Terrain objects

	m_terrainObjects.insert("fire", nullptr);
	m_terrainObjects.insert("fence", nullptr);
	m_terrainObjects.insert("teleport", nullptr);

	setCategoryFixture(CATEGORY_PLAYER);
	setCategoryRayCast(CATEGORY_ENEMY);
	setCategoryCollidesWith(CATEGORY_GROUND|CATEGORY_ITEM|CATEGORY_OTHER);
	setRayCastEnabled(true);


	connect(this, &GameObject::sceneConnected, this, &GamePlayer::onSceneConnected);
	connect(this, &GameObject::timingTimerTimeout, this, &GamePlayer::onTimingTimerTimeout);
	connect(this, &GameEntity::isOnGroundChanged, this, &GamePlayer::onIsOnGroundChanged);
	connect(this, &GameEntity::beginContact, this, &GamePlayer::onBeginContact);
	connect(this, &GameEntity::endContact, this, &GamePlayer::onEndContact);
	connect(this, &GameEntity::baseGroundContact, this, &GamePlayer::onBaseGroundContacted);

#ifndef Q_OS_WASM
	DesktopClient *client = qobject_cast<DesktopClient*>(Application::instance()->client());

	if (client) {
		m_soundEffectShot = client->newSoundEffect();
		m_soundEffectShot->setSource(shotSound());
		connect(this, &GamePlayer::attack, m_soundEffectShot, &QSoundEffect::play);
		connect(this, &GamePlayer::shotSoundChanged, m_soundEffectShot, &QSoundEffect::setSource);

		m_soundEffectGeneral = client->newSoundEffect();
	}
#endif

	connect(this, &GamePlayer::movingFlagsChanged, this, &GamePlayer::onMovingFlagsChanged);

	connect(this, &GamePlayer::hurt, this, [this]() { QTimer::singleShot(450, this, [this](){ playSoundEffect("pain"); }); });
	connect(this, &GamePlayer::allHpLost, this, [this](){
		setPlayerState(Dead);
		m_scene->playSoundPlayerVoice("qrc:/sound/sfx/dead.mp3");
		emit killed();
	});

}


/**
 * @brief GamePlayer::~GamePlayer
 */

GamePlayer::~GamePlayer()
{
#ifndef Q_OS_WASM
	if (m_soundEffectShot)
		m_soundEffectShot->deleteLater();
	if (m_soundEffectGeneral)
		m_soundEffectGeneral->deleteLater();
#endif

	qCDebug(lcScene).noquote() << tr("Player destroyed:") << this;
}


/**
 * @brief GamePlayer::onSceneConnected
 */

void GamePlayer::onSceneConnected()
{
	const QJsonObject &data = m_scene->levelData().value("player").toObject();

	setRayCastElevation(data.value("rayCastElevation").toDouble());
	setRayCastLength(data.value("rayCastLength").toDouble());
	setHurtFall(data.value("hurtFall").toDouble());
	setDeathlyFall(data.value("deathlyFall").toDouble());
}




/**
 * @brief GamePlayer::onTimingTimerTimeout
 */

void GamePlayer::onTimingTimerTimeout()
{
	/*if ((m_playerState == Run || m_playerState == Walk || m_playerState == ClimbUp) && !m_movingFlags) {
		jumpToSprite("idle");
		return;
	}*/

	terrainObjectSignalEmit();

	if (m_playerState == Dead || m_playerState == Burn || !isAlive())
		return;

	if (m_ladderFall && !m_groundFixtures.isEmpty()) {
		m_ladderFall = false;
	}


	if (m_ladderState == LadderInactive) {
		setLadder(nullptr);
	}



	m_soundElapsedMsec += m_scene->timingTimerTimeoutMsec();

	if (m_playerState == Walk) {
		if (m_soundElapsedMsec >= 400) {
			m_soundElapsedMsec = 0;
			playSoundEffect("walk");
		}
	} else 	if (m_playerState == Run) {
		if (m_soundElapsedMsec >= 300) {
			m_soundElapsedMsec = 0;
			playSoundEffect("run");
		}
	} else if (m_playerState == ClimbUp || m_playerState == ClimbDown) {
		if (m_soundElapsedMsec >= 1700) {
			m_soundElapsedMsec = 0;
			playSoundEffect("climb");
		}
	} else {
		m_soundElapsedMsec = 0;
	}


	if (m_playerState == ClimbUp && m_ladder) {
		ladderMove(true);
		return;
	}

	if (m_playerState == ClimbDown && m_ladder) {
		ladderMove(false);
		return;
	}

	if (m_playerState == ClimbPause && m_ladder) {

		return;
	}


	if (m_playerState == Fall && m_groundFixtures.isEmpty())
		return;

	if (m_groundFixtures.isEmpty()) {
		m_fallStart = y();
		setPlayerState(Fall);
		return;
	} else if (m_playerState == Fall) {
		qreal d = y()-m_fallStart;
		qCDebug(lcScene).noquote() << tr("Player fell:") << d;

		if (d >= m_deathlyFall || m_hp == 1) {
			setPlayerState(Dead);
			m_scene->playSoundPlayerVoice("qrc:/sound/sfx/falldead.mp3");
			kill();
			return;
		} else if (d >= m_hurtFall) {
			decreaseHp();
		}

		setPlayerState(Idle);
		return;
	}





	if (m_playerState == Shot) {
		if (m_lastCurrentSprite == "shot")
			return;
		else
			jumpToSprite("idle");

		onMovingFlagsChanged();
	}


	if (m_playerState == Run || m_playerState == Walk) {
		body()->setBodyType(Box2DBody::Kinematic);

		if (m_playerState == Run) {
			if (m_facingLeft)
				setX(x() - runSize());
			else
				setX(x() + runSize());
		} else {
			if (m_facingLeft)
				setX(x() - m_walkSize);
			else
				setX(x() + m_walkSize);
		}

		body()->setBodyType(Box2DBody::Dynamic);
	} else if (m_playerState == Fall) {

	} else if (m_lastCurrentSprite != "idle") {
		jumpToSprite("idle");
	}



}


/**
 * @brief GamePlayer::onBeginContact
 * @param other
 */

void GamePlayer::onBeginContact(Box2DFixture *other)
{
	QVariant object = other->property("targetObject");
	QVariantMap data = other->property("targetData").toMap();

	if (!object.isValid()) {
		return;
	}

	GameObject *gameObject = qvariant_cast<GameObject*>(object);


	if (data.value("fireDie", false).toBool()) {
		qCDebug(lcScene).noquote() << tr("Player is on fire");

		setPlayerState(Burn);
		return;
	}



	foreach (const QString &key, m_terrainObjects.keys()) {
		if (data.value(key, false).toBool()) {
			//qDebug() << "TERRAIN OBJECT" << key << (gameObject ? "+++" : "-");
			setTerrainObject(key, gameObject);
			return;
		}
	}




	GameLadder *ladder = qvariant_cast<GameLadder *>(object);

	if (ladder && m_ladderState != LadderActive && m_ladderState != LadderTopSprite) {
		const QString &dir = data.value("direction").toString();

		if (dir == "up") {
			setLadderState(LadderUpAvailable);
			setLadder(ladder);
		} else if (dir == "down") {
			setLadderState(LadderDownAvailable);
			setLadder(ladder);
		} else {
			qCWarning(lcScene).noquote() << tr("Invalid ladder direction:") << dir;
		}
	}


}





/**
 * @brief GamePlayer::onEndContact
 * @param other
 */

void GamePlayer::onEndContact(Box2DFixture *other)
{
	QVariantMap data = other->property("targetData").toMap();
	QVariant object = other->property("targetObject");
	GameLadder *ladder = qvariant_cast<GameLadder *>(object);

	if (!object.isValid()) {
		//qCWarning(lcScene).noquote() << tr("Invalid target object:") << other;
		return;
	}

	//GameObject *gameObject = qvariant_cast<GameObject*>(object);


	foreach (const QString &key, m_terrainObjects.keys()) {
		if (data.value(key, false).toBool()) {
			//qDebug() << "TERRAIN OBJECT" << key << gameObject;
			setTerrainObject(key, nullptr);
			return;
		}
	}



	if (ladder && m_ladderState != LadderActive && m_ladderState != LadderTopSprite) {
		setLadderState(LadderInactive);
	}
}






/**
 * @brief GamePlayer::onGroundTouched
 */

void GamePlayer::onBaseGroundContacted()
{
	qCDebug(lcScene).noquote() << tr("Player fell to base ground");

	setPlayerState(Dead);
	m_scene->playSoundPlayerVoice("qrc:/sound/sfx/falldead.mp3");
	kill();
}


/**
 * @brief GamePlayer::ladderUp
 */

void GamePlayer::ladderMove(const bool &up)
{
	if (!m_ladder) {
		qCWarning(lcScene).noquote() << tr("Missing ladder");
		return;
	}

	if (m_ladderState == LadderUpAvailable || m_ladderState == LadderDownAvailable) {
		if (up)
			qCDebug(lcScene).noquote() << tr("Begin climbing up on ladder:") << m_ladder->boundRect();
		else
			qCDebug(lcScene).noquote() << tr("Begin climbing down on ladder:") << m_ladder->boundRect();

		if (up)
			setLadderState(LadderActive);
		else
			setLadderState(LadderTopSprite);

		qreal _x = m_ladder->boundRect().x()+(m_ladder->boundRect().width()-width())/2;
		body()->setBodyType(Box2DBody::Kinematic);
		setX(_x);

		jumpToSprite(up ? "climbup" : "climbdown");

		if (up)
			playSoundEffect("ladder");

	} else if ((m_ladderState == LadderActive || m_ladderState == LadderTopSprite) && up) {
		qreal _y = y() -climbSize();

		if (m_ladderState == LadderActive && !QStringList({"climbup", "climbup2", "climbup3"}).contains(m_lastCurrentSprite))
			jumpToSprite("climbup2");

		if (_y < m_ladder->boundRect().top() - height()) {
			_y = m_ladder->boundRect().top()-height();
			qCDebug(lcScene).noquote() << tr("Finish climbing up on ladder:") << m_ladder->boundRect();
			setLadderState(LadderInactive);
			setY(_y);
			body()->setBodyType(Box2DBody::Dynamic);
			setPlayerState(Idle);
		} else if (_y >= m_ladder->boundRect().top() - height()) {
			setY(_y);

			if (m_ladderState == LadderActive && _y < m_ladder->boundRect().top()) {
				qCDebug(lcScene).noquote() << tr("Ladder top area reached");
				setLadderState(LadderTopSprite);
				jumpToSprite("climbupend");
			}
		}
	} else if ((m_ladderState == LadderActive || m_ladderState == LadderTopSprite) && !up) {
		qreal _y = y() +climbSize();

		if (!QStringList({"climbdown", "climbdown2", "climbdown3"}).contains(m_lastCurrentSprite))
			jumpToSprite("climbdown2");

		if (m_ladderState == LadderTopSprite && _y >= m_ladder->boundRect().top())  {
			qCDebug(lcScene).noquote() << tr("Ladder top area over");
			setLadderState(LadderActive);
			onMovingFlagsChanged();
		}

		if (_y+height() >= m_ladder->boundRect().bottom()) {
			_y = m_ladder->boundRect().bottom()-height();
			qCDebug(lcScene).noquote() << tr("Finish climbing down on ladder:") << m_ladder->boundRect();
			m_ladderFall = true;
			setLadderState(LadderInactive);
			setY(_y);
			body()->setBodyType(Box2DBody::Dynamic);
			setPlayerState(Idle);
		} else if (_y+height() < m_ladder->boundRect().bottom()) {
			setY(_y);
		}

	}

}



/**
 * @brief GamePlayer::terrainObjectSignalAddToPool
 * @param type
 * @param object
 */

void GamePlayer::terrainObjectSignalAddToPool(const QString &type, GameObject *object)
{
	if (m_terrainObjectsPrevious.contains(type))
		return;

	m_terrainObjectsPrevious.insert(type, object);
}




/**
 * @brief GamePlayer::terrainObjectSignalEmit
 */

void GamePlayer::terrainObjectSignalEmit()
{
	for (auto it = m_terrainObjectsPrevious.begin(); it != m_terrainObjectsPrevious.end(); ++it) {
		GameObject *old = it.value();
		GameObject *actual = m_terrainObjects.value(it.key());

		if (old != actual) {
			it.value() = actual;
			emit terrainObjectChanged(it.key(), actual);
		}
	}
}


/**
 * @brief GamePlayer::terrainObjects
 * @return
 */

const QHash<QString, QPointer<GameObject>> &GamePlayer::terrainObjects() const
{
	return m_terrainObjects;
}



/**
 * @brief GamePlayer::terrainObject
 * @param type
 * @return
 */

GameObject *GamePlayer::terrainObject(const QString &type) const
{
	return m_terrainObjects.value(type, nullptr);
}




/**
 * @brief GamePlayer::ladderState
 * @return
 */

GamePlayer::LadderState GamePlayer::ladderState() const
{
	return m_ladderState;
}

void GamePlayer::setLadderState(LadderState newLadderState)
{
	if (m_ladderState == newLadderState)
		return;
	m_ladderState = newLadderState;
	emit ladderStateChanged();
}




/**
 * @brief GamePlayer::hurtFall
 * @return
 */

qreal GamePlayer::hurtFall() const
{
	return m_hurtFall;
}

void GamePlayer::setHurtFall(qreal newHurtFall)
{
	if (qFuzzyCompare(m_hurtFall, newHurtFall))
		return;
	m_hurtFall = newHurtFall;
	emit hurtFallChanged();
}




qreal GamePlayer::deathlyFall() const
{
	return m_deathlyFall;
}

void GamePlayer::setDeathlyFall(qreal newDeathlyFall)
{
	if (qFuzzyCompare(m_deathlyFall, newDeathlyFall))
		return;
	m_deathlyFall = newDeathlyFall;
	emit deathlyFallChanged();
}




/**
 * @brief GamePlayer::create
 * @param scene
 * @return
 */

GamePlayer *GamePlayer::create(GameScene *scene, const QString &type)
{
	qCDebug(lcScene).noquote() << tr("Create player");

	GamePlayer *player = qobject_cast<GamePlayer*>(GameObject::createFromFile("GamePlayer.qml", scene));

	if (!player) {
		qCCritical(lcScene).noquote() << tr("Player creation error");
		return nullptr;
	}

	player->setParentItem(scene);
	player->setScene(scene);
	player->createSpriteItem();

	QDirIterator it(":/character", {"data.json"}, QDir::Files, QDirIterator::Subdirectories);
	QStringList list;

	while (it.hasNext())
		list.append(it.next().section('/',-2,-2));

	if (list.isEmpty()) {
		qFatal("Player character directory is empty");
	}

	if (list.contains(type)) {
		player->setDataDir(QString(":/character/%1").arg(type));
	} else if (list.contains("default")) {
		player->setDataDir(":/character/default");
	} else {
		player->setDataDir(QString(":/character/%1").arg(list.first()));
	}

	player->loadFromJsonFile();

	return player;
}



/**
 * @brief GamePlayer::setMovingFlag
 * @param flag
 * @param on
 */

void GamePlayer::setMovingFlag(const MovingFlag &flag, const bool &on)
{
	MovingFlags f = m_movingFlags;
	f.setFlag(flag, on);
	setMovingFlags(f);
}


/**
 * @brief GamePlayer::standbyMovingFlags
 */

void GamePlayer::standbyMovingFlags()
{
	setMovingFlags(Standby);
}






/**
 * @brief GamePlayer::moveTo
 * @param point
 * @param forced
 */

void GamePlayer::moveTo(const QPointF &point, const bool &forced)
{
	body()->setBodyType(Box2DBody::Kinematic);

	if (forced) {
		setPosition(point);
	}

	body()->setBodyType(Box2DBody::Dynamic);
}


/**
 * @brief GamePlayer::tryAttack
 */

void GamePlayer::shot()
{
	if (!isAlive() || m_ladderState == LadderActive || m_ladderState == LadderTopSprite)
		return;

	setPlayerState(Shot);
	emit attack();
	jumpToSprite("shot");		// Mindenképp kérjük

	if (!m_enemy)
		return;

	m_scene->game()->tryAttack(this, m_enemy);
}


/**
 * @brief GamePlayer::turnLeft
 */

void GamePlayer::turnLeft()
{
	if (!m_scene || !m_scene->game() || !m_scene->game()->running() ||!isAlive())
		return;

	setFacingLeft(true);
}




/**
 * @brief GamePlayer::turnRight
 */

void GamePlayer::turnRight()
{
	if (!m_scene || !m_scene->game() || !m_scene->game()->running() ||!isAlive())
		return;

	setFacingLeft(false);
}





/**
 * @brief GamePlayer::hurtByEnemy
 * @param enemy
 * @param canProtect
 */

void GamePlayer::hurtByEnemy(GameEnemy *enemy, const bool &canProtect)
{
	emit underAttack();

	/*if (m_cosGame && m_cosGame->gameMatch() && m_cosGame->gameMatch()->invincible()) {
					return;
			}

			if (canProtect && m_shield > 0) {
					setShield(m_shield-1);
			} else {*/
	decreaseHp();

	/*if (m_cosGame && m_cosGame->gameMatch()) {
							m_cosGame->gameMatch()->setIsFlawless(false);
					}*/


}


/**
 * @brief GamePlayer::killByEnemy
 * @param enemy
 */

void GamePlayer::killByEnemy(GameEnemy *enemy)
{
	/*if (m_cosGame && m_cosGame->gameMatch() && m_cosGame->gameMatch()->invincible()) {
			return;
	}*/

	kill();

	/*if (m_cosGame && m_cosGame->gameMatch()) {
			m_cosGame->gameMatch()->setIsFlawless(false);
	}

	emit killedByEnemy(enemy);*/

}




/**
 * @brief GamePlayer::setTerrainObject
 * @param type
 * @param object
 */

void GamePlayer::setTerrainObject(const QString &type, GameObject *object)
{
	if (m_terrainObjects.value(type) == object)
		return;

	terrainObjectSignalAddToPool(type, m_terrainObjects.value(type));

	m_terrainObjects[type] = object;

	//emit terrainObjectChanged(type, object);
}




/**
 * @brief GamePlayer::playSoundEffect
 * @param effect
 * @param from
 */

void GamePlayer::playSoundEffect(const QString &effect, int from)
{
	if (!m_scene)
		return;

	if (!m_soundEffectHash.contains(effect)) {
		qCWarning(lcScene).noquote() << tr("Invalid sound effect:") << effect;
		return;
	}

	if (from == -1)
		from = m_soundEffectNum.value(effect, 0);

	const QStringList &list = m_soundEffectHash.value(effect);

	if (from >= list.size())
		from = 0;

	m_scene->playSoundPlayerVoice(list.at(from));

	m_soundEffectNum[effect] = from+1;

}


/**
 * @brief GamePlayer::rayCastReport
 * @param items
 */

void GamePlayer::rayCastReport(const QMultiMap<qreal, GameEntity *> &items)
{
	if (m_ladderState == LadderActive || m_ladderState == LadderTopSprite) {
		setEnemy(nullptr);
		return;
	}

	GameEnemy *enemy = nullptr;

	foreach(GameEntity *item, items) {
		GameEnemy *e = qobject_cast<GameEnemy *>(item);

		if (e && e->isAlive()) {
			enemy = e;
			break;
		}
	}

	setEnemy(enemy);

}


/**
 * @brief GamePlayer::onEnemyKilled
 */

void GamePlayer::onEnemyKilled()
{
	setEnemy(nullptr);
}


/**
 * @brief GamePlayer::onMovingFlagsChanged
 */

void GamePlayer::onMovingFlagsChanged()
{
	//qCDebug(lcScene).noquote() << tr("Moving flags:") << m_movingFlags;

	if (!m_scene || !m_scene->game() || !m_scene->game()->running())
		return;

	if (!isAlive())
		return;

	if (m_movingFlags.testFlag(MoveLeft))
		setFacingLeft(true);
	else if (m_movingFlags.testFlag(MoveRight))
		setFacingLeft(false);

	if (m_ladderState == LadderTopSprite)
		return;

	qDebug() << "TEST" << m_movingFlags << m_ladderState << m_ladder;

	if (m_movingFlags.testFlag(MoveUp) && m_ladder && (m_ladderState == LadderActive || m_ladderState == LadderUpAvailable)) {
		setPlayerState(ClimbUp);
		return;
	} else if (m_movingFlags.testFlag(MoveDown) && m_ladder && (m_ladderState == LadderActive || m_ladderState == LadderDownAvailable)) {
		setPlayerState(ClimbDown);
		return;
	}

	if (m_ladderState == LadderActive && !m_movingFlags.testFlag(MoveUp) && !m_movingFlags.testFlag(MoveDown)) {
		setPlayerState(ClimbPause);
		return;
	}


	if (m_movingFlags.testFlag(MoveLeft) || m_movingFlags.testFlag(MoveRight)) {
		if (m_movingFlags.testFlag(SlowModifier))
			setPlayerState(Walk);
		else
			setPlayerState(Run);
	} else {
		setPlayerState(Idle);
	}

	/*if (ep.ladderMode == GamePlayerPrivate.LadderClimb ||
					ep.ladderMode == GamePlayerPrivate.LadderClimbFinish)
				return*/

	/*if (!root.isRunning)
				spriteSequence.jumpTo("run")*/

}


/**
 * @brief GamePlayer::onIsOnGroundChanged
 */

void GamePlayer::onIsOnGroundChanged()
{

}


#ifndef Q_OS_WASM

/**
 * @brief GamePlayer::soundEffectShot
 * @return
 */

QSoundEffect *GamePlayer::soundEffectShot() const
{
	return m_soundEffectShot;
}


/**
 * @brief GamePlayer::soundEffectGeneral
 * @return
 */

QSoundEffect *GamePlayer::soundEffectGeneral() const
{
	return m_soundEffectGeneral;
}

#endif

GameEnemy *GamePlayer::enemy() const
{
	return m_enemy;
}


/**
 * @brief GamePlayer::setEnemy
 * @param newEnemy
 */

void GamePlayer::setEnemy(GameEnemy *newEnemy)
{
	if (m_enemy && m_enemy != newEnemy) {
		//disconnect(m_enemy, &GameEnemy::killed, this, &GamePlayer::onEnemyKilled);
		m_enemy->setAimedByPlayer(false);
	}

	if (m_enemy == newEnemy)
		return;
	m_enemy = newEnemy;
	emit enemyChanged();

	if (m_enemy) {
		m_enemy->setAimedByPlayer(true);
		//connect(m_enemy, &GameEnemy::killed, this, &GamePlayer::onEnemyKilled);
	}

}


/**
 * @brief GamePlayer::playerState
 * @return
 */

const GamePlayer::PlayerState &GamePlayer::playerState() const
{
	return m_playerState;
}


/**
 * @brief GamePlayer::setPlayerState
 * @param newPlayerState
 */

void GamePlayer::setPlayerState(const PlayerState &newPlayerState)
{
	if (m_playerState == newPlayerState)
		return;
	m_playerState = newPlayerState;
	emit playerStateChanged();

	qCDebug(lcScene).noquote() << tr("Player state changed to:") << m_playerState;

	switch (m_playerState) {
	case Walk:
		jumpToSprite("walk");
		break;
	case Run:
		jumpToSprite("run");
		break;
	case Fall:
		if (!m_ladderFall)
			jumpToSprite("fall");
		break;
	case Shot:
		break;
	case ClimbUp:
		break;
	case ClimbDown:
		break;
	case ClimbPause:
		body()->setBodyType(Box2DBody::Kinematic);
		jumpToSprite("climbpause");
		break;
	case Operate:
		jumpToSprite("operate");
		break;
	case Burn:
		jumpToSprite("burn");
		m_scene->playSoundPlayerVoice("qrc:/sound/sfx/dead.mp3");
		kill();
		break;
	case Dead:
		jumpToSprite("dead");
		break;
	case Idle:
	case Invalid:
		// Nem ugrunk oda, csak az timeout után
		//jumpToSprite("idle");
		break;
	}


}



/**
 * @brief GamePlayer::runSize
 * @return
 */

qreal GamePlayer::runSize() const
{
	return m_dataObject.value("run").toDouble(m_walkSize);
}


/**
 * @brief GamePlayer::climbSize
 * @return
 */

qreal GamePlayer::climbSize() const
{
	return m_dataObject.value("climb").toDouble(m_walkSize);
}


/**
 * @brief GamePlayer::movingFlags
 * @return
 */

const GamePlayer::MovingFlags &GamePlayer::movingFlags() const
{
	return m_movingFlags;
}

void GamePlayer::setMovingFlags(const MovingFlags &newMovingFlags)
{
	if (m_movingFlags == newMovingFlags)
		return;
	m_movingFlags = newMovingFlags;
	emit movingFlagsChanged(m_movingFlags);
}


/**
 * @brief GamePlayer::ladder
 * @return
 */

GameLadder *GamePlayer::ladder() const
{
	return m_ladder;
}

void GamePlayer::setLadder(GameLadder *newLadder)
{
	if (m_ladder == newLadder)
		return;
	m_ladder = newLadder;
	emit ladderChanged();

	if (m_ladder)
		onMovingFlagsChanged();
}


