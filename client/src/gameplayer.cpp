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


	setCategoryFixture(CATEGORY_PLAYER);
	setCategoryRayCast(CATEGORY_ENEMY);
	setCategoryCollidesWith(CATEGORY_GROUND|CATEGORY_ITEM|CATEGORY_OTHER);
	setRayCastEnabled(true);

	connect(this, &GameObject::sceneConnected, this, &GamePlayer::onSceneConnected);
	connect(this, &GameObject::timingTimerTimeout, this, &GamePlayer::onTimingTimerTimeout);
	connect(this, &GameEntity::isOnGroundChanged, this, &GamePlayer::onIsOnGroundChanged);

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
	connect(this, &GamePlayer::isAliveChanged, this, [this](){
		if (!isAlive())
			setPlayerState(Dead);
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

	if (m_playerState == Dead || !isAlive())
		return;


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
	/*if (m_ladderMode == LadderClimb || m_ladderMode == LadderClimbFinish)
					return;*/

	if (!isAlive())
		return;

	setPlayerState(Shot);
	emit attack();
	jumpToSprite("shot");		// Mindenképp kérjük

	if (!m_enemy)
		return;

	m_scene->game()->tryAttack(this, m_enemy);
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
	qCDebug(lcScene).noquote() << tr("Moving flags:") << m_movingFlags;

	if (!m_scene || !m_scene->game() || !m_scene->game()->running())
		return;

	if (!isAlive())
		return;

	if (m_movingFlags.testFlag(MoveLeft))
		setFacingLeft(true);
	else if (m_movingFlags.testFlag(MoveRight))
		setFacingLeft(false);

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
		jumpToSprite("fall");
		break;
	case Shot:
		//jumpToSprite("shot");
		break;
	case ClimbUp:
		jumpToSprite("climbup");
		break;
	case ClimbDown:
		jumpToSprite("climbdown");
		break;
	case ClimbPause:
		jumpToSprite("climbpause");
		break;
	case Operate:
		jumpToSprite("operate");
		break;
	case Burn:
		jumpToSprite("burn");
		break;
	case Dead:
		jumpToSprite("dead");
		m_scene->playSoundPlayerVoice("qrc:/sound/sfx/dead.mp3");
		emit killed();
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
	emit movingFlagsChanged();
}
