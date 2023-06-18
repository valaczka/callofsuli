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
#include "Logger.h"
#include "application.h"
#include "actiongame.h"
#include "gameenemy.h"
#include "gameplayerposition.h"
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
	LOG_CDEBUG("scene") << "Player created:" << this;

	m_defaultShotSound = QStringLiteral("qrc:/sound/sfx/shot.wav");


	m_soundEffectHash.insert(QStringLiteral("run"), QStringList({
																	QStringLiteral("qrc:/sound/sfx/run1.mp3"),
																	QStringLiteral("qrc:/sound/sfx/run2.mp3"),
																}));

	m_soundEffectHash.insert(QStringLiteral("walk"), QStringList({
																	 QStringLiteral("qrc:/sound/sfx/step1.mp3"),
																	 QStringLiteral("qrc:/sound/sfx/step2.mp3"),
																 }));

	m_soundEffectHash.insert(QStringLiteral("climb"), QStringList({
																	  QStringLiteral("qrc:/sound/sfx/ladderup1.mp3"),
																	  QStringLiteral("qrc:/sound/sfx/ladderup2.mp3"),
																  }));

	m_soundEffectHash.insert(QStringLiteral("ladder"), QStringList({
																	   QStringLiteral("qrc:/sound/sfx/ladder.mp3"),
																   }));

	m_soundEffectHash.insert(QStringLiteral("pain"), QStringList({
																	 QStringLiteral("qrc:/sound/sfx/pain1.mp3"),
																	 QStringLiteral("qrc:/sound/sfx/pain2.mp3"),
																	 QStringLiteral("qrc:/sound/sfx/pain3.mp3"),
																 }));



	// Terrain objects

	m_terrainObjects.insert(QStringLiteral("fire"), nullptr);
	m_terrainObjects.insert(QStringLiteral("fence"), nullptr);
	m_terrainObjects.insert(QStringLiteral("teleport"), nullptr);

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

	connect(this, &GamePlayer::hurt, this, [this]() {
		game()->resetKillStreak();
		QTimer::singleShot(450, this, [this](){ playSoundEffect(QStringLiteral("pain")); });
	});
	connect(this, &GamePlayer::allHpLost, this, [this](){
		setPlayerState(Dead);
		m_scene->playSoundPlayerVoice(QStringLiteral("qrc:/sound/sfx/dead.mp3"));
		emit killed(this);
	});

	connect(this, &GamePlayer::invisibleChanged, this, [this](){
		if (!m_invisible)
			m_scene->playSound("qrc:/sound/sfx/question.mp3");
	});

	connect(this, &GameEntity::isAliveChanged, this, &GamePlayer::onIsAliveChanged);
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

	LOG_CDEBUG("scene") << "Player destroyed:" << this;
}


/**
 * @brief GamePlayer::onSceneConnected
 */

void GamePlayer::onSceneConnected()
{
	const QJsonObject &data = m_scene->levelData().value(QStringLiteral("player")).toObject();

	setRayCastElevation(data.value(QStringLiteral("rayCastElevation")).toDouble());
	setRayCastLength(data.value(QStringLiteral("rayCastLength")).toDouble());
	setHurtFall(data.value(QStringLiteral("hurtFall")).toDouble());
	setDeathlyFall(data.value(QStringLiteral("deathlyFall")).toDouble());
}




/**
 * @brief GamePlayer::onTimingTimerTimeout
 */

void GamePlayer::onTimingTimerTimeout()
{
	if (game() && !game()->running())
		return;

	if (m_playerState == Dead || m_playerState == Burn || !isAlive())
		return;

	if (m_ladderFall && !m_groundFixtures.isEmpty()) {
		m_ladderFall = false;
	}


	if (m_ladderState == LadderInactive) {
		setLadder(nullptr);
	}


	if (m_invisibleTime > 0) {
		if (!m_invisible)
			setInvisible(true);

		setInvisibleTime(qMax(m_invisibleTime - m_scene->timingTimerTimeoutMsec(), 0));
	}

	setInvisible(m_invisibleTime > 0);


	if (m_playerState == MoveToOperate || m_playerState == Operate) {
		if (!m_operatingObject) {
			setPlayerState(Idle);
		} else if (m_playerState == MoveToOperate) {
			const QPointF &left = m_operatingObject->property("operatingPointLeft").toPointF() + m_operatingObject->position();
			const QPointF &right = m_operatingObject->property("operatingPointRight").toPointF() + m_operatingObject->position();

			qreal myLeft = x()+m_bodyRect.x();
			qreal myRight = x()+m_bodyRect.x()+m_bodyRect.width();

			if (myRight < left.x()) {
				setFacingLeft(false);
				if (left.x()-myRight > 5) {
					setX(x() + m_walkSize);
					m_body->setAwake(true);				// Különben nem reagál a mozgásra
				} else {
					setPlayerState(Operate);
				}

			} else if (myLeft > right.x()) {
				setFacingLeft(true);
				if (myLeft - right.x() > 5) {
					setX(x() - m_walkSize);
					m_body->setAwake(true);				// Különben nem reagál a mozgásra
				} else {
					setPlayerState(Operate);
				}
			} else {
				setPlayerState(Operate);
			}
		}

		// else wait for operate

		return;
	}


	m_soundElapsedMsec += m_scene->timingTimerTimeoutMsec();

	if (m_playerState == Walk) {
		if (m_soundElapsedMsec >= 400) {
			m_soundElapsedMsec = 0;
			playSoundEffect(QStringLiteral("walk"));
		}
	} else 	if (m_playerState == Run) {
		if (m_soundElapsedMsec >= 300) {
			m_soundElapsedMsec = 0;
			playSoundEffect(QStringLiteral("run"));
		}
	} else if (m_playerState == ClimbUp || m_playerState == ClimbDown) {
		if (m_soundElapsedMsec >= 1700) {
			m_soundElapsedMsec = 0;
			playSoundEffect(QStringLiteral("climb"));
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
		LOG_CDEBUG("scene") << "Player fell:" << d;

		if (d >= m_deathlyFall || m_hp == 1) {
			setPlayerState(Dead);
			m_scene->playSoundPlayerVoice(QStringLiteral("qrc:/sound/sfx/falldead.mp3"));
			kill();
			return;
		} else if (d >= m_hurtFall) {
			decreaseHp();
		}

		setPlayerState(Idle);
		return;
	}





	if (m_playerState == Shot) {
		if (m_lastCurrentSprite == QStringLiteral("shot"))
			return;
		else
			jumpToSprite(QStringLiteral("idle"));

		onMovingFlagsChanged();
	}


	if (m_playerState == Run || m_playerState == Walk) {
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

		m_body->setAwake(true);				// Különben nem reagál a mozgásra



	} else if (m_playerState == Fall) {

	} else if (m_lastCurrentSprite != QStringLiteral("idle")) {
		jumpToSprite(QStringLiteral("idle"));
	}



}


/**
 * @brief GamePlayer::onBeginContact
 * @param other
 */

void GamePlayer::onBeginContact(Box2DFixture *other)
{
	GameObject *gameObject = qobject_cast<GameObject *>(other->getBody()->target());
	const QVariantMap &data = other->property("targetData").toMap();

	if (!gameObject) {
		return;
	}

	GamePlayerPosition *position = qobject_cast<GamePlayerPosition *>(gameObject);

	if (position) {
		m_scene->setPlayerPosition(position);
		return;
	}


	GamePickable *pickable = qobject_cast<GamePickable *>(gameObject);

	if (pickable) {
		game()->pickableAdd(pickable);
		return;
	}

	GameLadder *ladder = qobject_cast<GameLadder *>(gameObject);

	if (ladder && m_ladderState != LadderActive && m_ladderState != LadderTopSprite) {
		const QString &dir = data.value(QStringLiteral("direction")).toString();

		if (dir == QStringLiteral("up")) {
			setLadderState(LadderUpAvailable);
			setLadder(ladder);
		} else if (dir == QStringLiteral("down")) {
			setLadderState(LadderDownAvailable);
			setLadder(ladder);
		} else {
			LOG_CWARNING("scene") << "Invalid ladder direction:" << dir;
		}
	}



	if (data.value(QStringLiteral("fireDie"), false).toBool()) {
		LOG_CDEBUG("scene") << "Player is on fire";

		setPlayerState(Burn);
		return;
	}


	if (!gameObject->objectType().isEmpty()) {
		foreach (const QString &key, m_terrainObjects.keys()) {
			if (gameObject->objectType() == key) {
				setTerrainObject(key, gameObject);
				return;
			}
		}
	}





}





/**
 * @brief GamePlayer::onEndContact
 * @param other
 */

void GamePlayer::onEndContact(Box2DFixture *other)
{
	GameObject *gameObject = qobject_cast<GameObject *>(other->getBody()->target());

	if (!gameObject) {
		return;
	}

	GamePickable *pickable = qobject_cast<GamePickable *>(gameObject);

	if (pickable) {
		game()->pickableRemove(pickable);
		return;
	}

	if (!gameObject->objectType().isEmpty()) {
		foreach (const QString &key, m_terrainObjects.keys()) {
			if (gameObject->objectType() == key) {
				setTerrainObject(key, nullptr);
				return;
			}
		}
	}

	GameLadder *ladder = qobject_cast<GameLadder *>(gameObject);

	if (ladder && m_ladderState != LadderActive && m_ladderState != LadderTopSprite) {
		setLadderState(LadderInactive);
	}
}






/**
 * @brief GamePlayer::onGroundTouched
 */

void GamePlayer::onBaseGroundContacted()
{
	LOG_CDEBUG("scene") << "Player fell to base ground";

	setPlayerState(Dead);
	m_scene->playSoundPlayerVoice(QStringLiteral("qrc:/sound/sfx/falldead.mp3"));
	kill();
}


/**
 * @brief GamePlayer::onIsAliveChanged
 */

void GamePlayer::onIsAliveChanged()
{
	if (m_enemy)
		m_enemy->setAimedByPlayer(false);
}


/**
 * @brief GamePlayer::onHpOrShieldChanged
 */

void GamePlayer::onHpOrShieldChanged()
{
	if (m_shield)
		setHpProgressValue(m_shield);
	else
		setHpProgressValue(m_hp);
}


/**
 * @brief GamePlayer::ladderUp
 */

void GamePlayer::ladderMove(const bool &up)
{
	if (!m_ladder) {
		LOG_CWARNING("scene") << "Missing ladder";
		return;
	}

	if (m_ladderState == LadderUpAvailable || m_ladderState == LadderDownAvailable) {
		if (up)
			LOG_CDEBUG("scene") << "Begin climbing up on ladder:" << m_ladder->boundRect();
		else
			LOG_CDEBUG("scene") << "Begin climbing down on ladder:" << m_ladder->boundRect();

		if (up)
			setLadderState(LadderActive);
		else
			setLadderState(LadderTopSprite);

		qreal _x = m_ladder->boundRect().x()+(m_ladder->boundRect().width()-width())/2;
		body()->setBodyType(Box2DBody::Kinematic);
		setX(_x);

		jumpToSprite(up ? QStringLiteral("climbup") : QStringLiteral("climbdown"));

		if (up)
			playSoundEffect(QStringLiteral("ladder"));

	} else if ((m_ladderState == LadderActive || m_ladderState == LadderTopSprite) && up) {
		qreal _y = y() -climbSize();

		if (m_ladderState == LadderActive && !QStringList({QStringLiteral("climbup"), QStringLiteral("climbup2"), QStringLiteral("climbup3")}).contains(m_lastCurrentSprite))
			jumpToSprite(QStringLiteral("climbup2"));

		if (_y < m_ladder->boundRect().top() - height()) {
			_y = m_ladder->boundRect().top()-height();
			LOG_CDEBUG("scene") << "Finish climbing up on ladder:" << m_ladder->boundRect();
			setLadderState(LadderInactive);
			setY(_y);
			body()->setBodyType(Box2DBody::Dynamic);
			setPlayerState(Idle);
		} else if (_y >= m_ladder->boundRect().top() - height()) {
			setY(_y);

			if (m_ladderState == LadderActive && _y < m_ladder->boundRect().top()) {
				LOG_CDEBUG("scene") << "Ladder top area reached";
				setLadderState(LadderTopSprite);
				jumpToSprite(QStringLiteral("climbupend"));
			}
		}
	} else if ((m_ladderState == LadderActive || m_ladderState == LadderTopSprite) && !up) {
		qreal _y = y() +climbSize();

		if (!QStringList({QStringLiteral("climbdown"), QStringLiteral("climbdown2"), QStringLiteral("climbdown3")}).contains(m_lastCurrentSprite))
			jumpToSprite(QStringLiteral("climbdown2"));

		if (m_ladderState == LadderTopSprite && _y >= m_ladder->boundRect().top())  {
			LOG_CDEBUG("scene") << "Ladder top area over";
			setLadderState(LadderActive);
			onMovingFlagsChanged();
		}

		if (_y+height() >= m_ladder->boundRect().bottom()) {
			_y = m_ladder->boundRect().bottom()-height();
			LOG_CDEBUG("scene") << "Finish climbing down on ladder:" << m_ladder->boundRect();
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
 * @brief GamePlayer::shield
 * @return
 */

int GamePlayer::shield() const
{
	return m_shield;
}

void GamePlayer::setShield(int newShield)
{
	if (m_shield == newShield)
		return;
	m_shield = newShield;
	emit shieldChanged();
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
	LOG_CDEBUG("scene") << "Create player";

	GamePlayer *player = qobject_cast<GamePlayer*>(GameObject::createFromFile(QStringLiteral("GamePlayer.qml"), scene));

	if (!player) {
		LOG_CERROR("scene") << "Player creation error";
		return nullptr;
	}

	player->setParentItem(scene);
	player->setScene(scene);
	player->createSpriteItem();

	const QStringList &list = ActionGame::availableCharacters();

	if (list.isEmpty()) {
		qFatal("Player character directory is empty");
	}

	if (list.contains(type)) {
		player->setDataDir(QStringLiteral(":/character/%1").arg(type));
	} else if (list.contains(QStringLiteral("default"))) {
		player->setDataDir(QStringLiteral(":/character/default"));
	} else {
		player->setDataDir(QStringLiteral(":/character/%1").arg(list.first()));
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
	if (forced) {
		setPosition(point);
	}

	m_body->setAwake(true);
}


/**
 * @brief GamePlayer::tryAttack
 */

void GamePlayer::shot()
{
	if (!isAlive() || m_ladderState == LadderActive || m_ladderState == LadderTopSprite)
		return;

	if (m_playerState == Operate || m_playerState == MoveToOperate)
		return;

	setPlayerState(Shot);
	emit attack();
	jumpToSprite(QStringLiteral("shot"));		// Mindenképp kérjük

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
 * @brief GamePlayer::operate
 * @param object
 */

void GamePlayer::operate(GameObject *object)
{
	if (!object || object->objectType().isEmpty()) {
		LOG_CWARNING("game") << "Invalid operating object " << (object ? object->objectType() : "") << object;
		return;
	}

	setOperatingObject(object);
	setPlayerState(MoveToOperate);
}



/**
 * @brief GamePlayer::startInvisibility
 * @param msec
 */

void GamePlayer::startInvisibility(const int &msec)
{
	setInvisibleTime(m_invisibleTime+msec);
	emit invisibleTimeChanged();
}





/**
 * @brief GamePlayer::hurtByEnemy
 * @param enemy
 * @param canProtect
 */

void GamePlayer::hurtByEnemy(GameEnemy *, const bool &canProtect)
{
	emit underAttack();

	/*if (m_cosGame && m_cosGame->gameMatch() && m_cosGame->gameMatch()->invincible()) {
					return;
			}
*/

	if (canProtect && m_shield > 0) {
		setShield(m_shield-1);
	} else {
		decreaseHp();
	}
}


/**
 * @brief GamePlayer::killByEnemy
 * @param enemy
 */

void GamePlayer::killByEnemy(GameEnemy *)
{
	/*if (m_cosGame && m_cosGame->gameMatch() && m_cosGame->gameMatch()->invincible()) {
			return;
	}*/

	setPlayerState(Dead);
	m_scene->playSoundPlayerVoice(QStringLiteral("qrc:/sound/sfx/dead.mp3"));

	kill();
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

	m_terrainObjects[type] = object;

	emit terrainObjectChanged(type, object);
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
		LOG_CWARNING("scene") << "Invalid sound effect:" << effect;
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
 * @brief GamePlayer::hpProgressValueSetup
 */

void GamePlayer::hpProgressValueSetup()
{
	connect(this, &GamePlayer::hpChanged, this, &GamePlayer::onHpOrShieldChanged);
	connect(this, &GamePlayer::shieldChanged, this, &GamePlayer::onHpOrShieldChanged);
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
	//LOG_CDEBUG("scene") << "Moving flags:" << m_movingFlags;

	if (!m_scene || !m_scene->game() || !m_scene->game()->running())
		return;

	if (!isAlive())
		return;

	if (m_playerState == Operate || m_playerState == MoveToOperate)
		return;

	if (m_movingFlags.testFlag(MoveLeft))
		setFacingLeft(true);
	else if (m_movingFlags.testFlag(MoveRight))
		setFacingLeft(false);

	if (m_ladderState == LadderTopSprite)
		return;

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

	LOG_CDEBUG("scene") << "Player state changed to:" << m_playerState;

	switch (m_playerState) {
	case Walk:
	case MoveToOperate:
		jumpToSprite(QStringLiteral("walk"));
		break;
	case Run:
		jumpToSprite(QStringLiteral("run"));
		break;
	case Fall:
		if (!m_ladderFall)
			jumpToSprite(QStringLiteral("fall"));
		break;
	case Shot:
		break;
	case ClimbUp:
		break;
	case ClimbDown:
		break;
	case ClimbPause:
		body()->setBodyType(Box2DBody::Kinematic);
		jumpToSprite(QStringLiteral("climbpause"));
		break;
	case Operate:
		jumpToSprite(QStringLiteral("operate"));
		game()->operateReal(this, m_operatingObject);
		break;
	case Burn:
		jumpToSprite(QStringLiteral("burn"));
		m_scene->playSoundPlayerVoice(QStringLiteral("qrc:/sound/sfx/dead.mp3"));
		kill();
		break;
	case Dead:
		jumpToSprite(QStringLiteral("dead"));
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
	return m_dataObject.value(QStringLiteral("run")).toDouble(m_walkSize);
}


/**
 * @brief GamePlayer::climbSize
 * @return
 */

qreal GamePlayer::climbSize() const
{
	return m_dataObject.value(QStringLiteral("climb")).toDouble(m_walkSize);
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



/**
 * @brief GamePlayer::invisible
 * @return
 */


bool GamePlayer::invisible() const
{
	return m_invisible;
}

void GamePlayer::setInvisible(bool newInvisible)
{
	if (m_invisible == newInvisible)
		return;
	m_invisible = newInvisible;
	emit invisibleChanged();
}

GameObject *GamePlayer::operatingObject() const
{
	return m_operatingObject;
}

void GamePlayer::setOperatingObject(GameObject *newOperatingObject)
{
	if (m_operatingObject == newOperatingObject)
		return;
	m_operatingObject = newOperatingObject;
	emit operatingObjectChanged();
}


/**
 * @brief GamePlayer::setInvisibleTime
 * @param msec
 */

void GamePlayer::setInvisibleTime(const int &msec)
{
	if (m_invisibleTime == msec)
		return;
	m_invisibleTime = msec;
	emit invisibleTimeChanged();
}




/**
 * @brief GamePlayer::invisibleTime
 * @return
 */

const int &GamePlayer::invisibleTime() const
{
	return m_invisibleTime;
}
