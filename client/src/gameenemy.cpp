/*
 * ---- Call of Suli ----
 *
 * gameenemy.cpp
 *
 * Created on: 2022. 12. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemy
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

#include "gameenemy.h"
#include "Logger.h"
#include "gamescene.h"
#include "application.h"
#include "gameplayer.h"
#include <qtimer.h>


// State hash

const QHash<ObjectStateBase::EnemyState, GameEnemy::EnemyState> GameEnemy::m_stateHash = {
    { ObjectStateBase::Idle, GameEnemy::Idle },
    { ObjectStateBase::Move, GameEnemy::Move },
    { ObjectStateBase::WatchPlayer, GameEnemy::WatchPlayer },
    { ObjectStateBase::Attack, GameEnemy::Attack },
    { ObjectStateBase::Dead, GameEnemy::Dead },
    };



/**
 * @brief GameEnemy::GameEnemy
 * @param parent
 */

GameEnemy::GameEnemy(QQuickItem *parent)
    : GameEntity(parent)
{
    m_defaultShotSound = QStringLiteral("qrc:/sound/sfx/enemyshot.wav");

    setCategoryFixture(CATEGORY_ENEMY);
    setCategoryRayCast(CATEGORY_PLAYER);
    setRayCastEnabled(true);

    connect(this, &GameObject::sceneConnected, this, &GameEnemy::onSceneConnected);
    connect(this, &GameEnemy::attack, this, &GameEnemy::playAttackSound);

    connect(this, &GameEnemy::allHpLost, this, [this](){
        emit killed(this);
    });

    connect(this, &GameEnemy::isAliveChanged, this, [this](){
        if (!isAlive())
            setEnemyState(Dead);
    });
}


/**
 * @brief GameEnemy::~GameEnemy
 */

GameEnemy::~GameEnemy()
{

}


/**
 * @brief GameEnemy::onSceneChanged
 */

void GameEnemy::onSceneConnected()
{
    connect(m_scene, &GameScene::zoomOverviewChanged, this, &GameEntity::setOverlayEnabled);
}



/**
 * @brief GameEnemy::attackedByPlayerEvent
 * @param player
 */

void GameEnemy::attackedByPlayerEvent(GamePlayer *player, const bool &isQuestionEmpty)
{
    Q_ASSERT (player);

    if (!isQuestionEmpty || !player->isAlive()) {
        turnToPlayer(player);
    }
}


/**
 * @brief GameEnemy::interpolate
 * @param t
 * @param from
 * @param to
 * @return
 */

ObjectStateBase GameEnemy::interpolate(const qreal &t, const ObjectStateBase &from, const ObjectStateBase &to)
{
    ObjectStateBase b = GameEntity::interpolate(t, from, to);

    if (from.fields.testFlag(ObjectStateBase::FieldEnemyState)) {
        if (t < 1.0) {
            if (to.position.x() != from.position.x())
                b.enemyState = ObjectStateBase::Move;
            else if (from.position.x() == to.position.x() && from.enemyState == ObjectStateBase::Move)
                b.enemyState = to.enemyState == ObjectStateBase::Move ? ObjectStateBase::Idle : to.enemyState;
        } else {
            b.enemyState = to.enemyState;
        }
    }

    return b;
}


/**
 * @brief GameEnemy::stateReconciliation
 * @param from
 * @param to
 * @return
 */

bool GameEnemy::stateReconciliation(const ObjectStateBase &from, const ObjectStateBase &to)
{
    if (!GameEntity::stateReconciliation(from, to))
        return false;

    if (from.enemyState == ObjectStateBase::Dead)
        return false;

    return true;
}




/**
 * @brief GameEnemy::playAttackSound
 */

void GameEnemy::playAttackSound()
{
    Application::instance()->client()->sound()->playSound(shotSound(), Sound::SfxChannel);
}


const GamePickable::GamePickableData &GameEnemy::pickable() const
{
    return m_pickable;
}

void GameEnemy::setPickable(const GamePickable::GamePickableData &newPickable)
{
    m_pickable = newPickable;
    emit pickableChanged();
}




/**
 * @brief GameEnemy::getCurrentState
 * @return
 */

ObjectStateBase GameEnemy::getCurrentState() const
{
    ObjectStateBase b = GameEntity::getCurrentState();
    b.type = ObjectStateBase::TypeEnemy;

    b.fields.setFlag(ObjectStateBase::FieldEnemyState);
    b.fields.setFlag(ObjectStateBase::FieldEnemyRect);
    b.fields.setFlag(ObjectStateBase::FieldMSecToAttack);

    b.enemyState = m_stateHash.key(m_enemyState, ObjectStateBase::Invalid);
    b.enemyRect = m_terrainEnemyData.rect;
    b.msecLeftToAttack = m_msecLeftToAttack;

    return b;
}




/**
 * @brief GameEnemy::setCurrentState
 * @param state
 * @param force
 */

void GameEnemy::setCurrentState(const ObjectStateBase &state, const bool &force)
{
    GameEntity::setCurrentState(state, force);

    if (force) {
        if (state.fields.testFlag(ObjectStateBase::FieldEnemyState))
            setEnemyState(m_stateHash.value(state.enemyState, Invalid));

        if (state.fields.testFlag(ObjectStateBase::FieldEnemyRect))
            m_terrainEnemyData.rect = state.enemyRect;

        if (state.fields.testFlag(ObjectStateBase::FieldMSecToAttack))
            setMsecLeftToAttack(state.msecLeftToAttack);
    }
}





/**
 * @brief GameEnemy::question
 * @return
 */

ActionGame::QuestionLocation *GameEnemy::question() const
{
    return m_question;
}

void GameEnemy::setQuestion(ActionGame::QuestionLocation *newQuestion)
{
    if (m_question == newQuestion)
        return;
    m_question = newQuestion;
    emit questionChanged();
}


/**
 * @brief GameEnemy::enemyState
 * @return
 */

const GameEnemy::EnemyState &GameEnemy::enemyState() const
{
    return m_enemyState;
}

void GameEnemy::setEnemyState(const EnemyState &newEnemyState)
{
    if (m_enemyState == newEnemyState)
        return;
    m_enemyState = newEnemyState;
    emit enemyStateChanged();

    enemyStateModified();
}


/**
 * @brief GameEnemy::msecBetweenAttack
 * @return
 */

qreal GameEnemy::msecBetweenAttack() const
{
    return m_msecBetweenAttack;
}

void GameEnemy::setMsecBetweenAttack(qreal newMsecBetweenAttack)
{
    if (qFuzzyCompare(m_msecBetweenAttack, newMsecBetweenAttack))
        return;
    m_msecBetweenAttack = newMsecBetweenAttack;
    emit msecBetweenAttackChanged();
}

qreal GameEnemy::msecBeforeAttack() const
{
    return m_msecBeforeAttack;
}

void GameEnemy::setMsecBeforeAttack(qreal newMsecBeforeAttack)
{
    if (qFuzzyCompare(m_msecBeforeAttack, newMsecBeforeAttack))
        return;
    m_msecBeforeAttack = newMsecBeforeAttack;
    emit msecBeforeAttackChanged();
}

qreal GameEnemy::castAttackFraction() const
{
    return m_castAttackFraction;
}

void GameEnemy::setCastAttackFraction(qreal newCastAttackFraction)
{
    if (qFuzzyCompare(m_castAttackFraction, newCastAttackFraction))
        return;
    m_castAttackFraction = newCastAttackFraction;
    emit castAttackFractionChanged();
}





/**
 * @brief GameEnemy::startMovingAfter
 * @param msec
 */

void GameEnemy::startMovingAfter(const int &msec)
{
    QTimer::singleShot(msec, this, [this]() { setEnemyState(Move); });
}


/**
 * @brief GameEnemy::setTerrainEnemyData
 * @param newTerrainEnemyData
 */

void GameEnemy::setTerrainEnemyData(const GameTerrain::EnemyData &newTerrainEnemyData)
{
    m_terrainEnemyData = newTerrainEnemyData;
}


/**
 * @brief GameEnemy::terrainEnemyData
 * @return
 */

const GameTerrain::EnemyData &GameEnemy::terrainEnemyData() const
{
    return m_terrainEnemyData;
}


bool GameEnemy::aimedByPlayer() const
{
    return m_aimedByPlayer;
}

void GameEnemy::setAimedByPlayer(bool newAimedByPlayer)
{
    if (m_aimedByPlayer == newAimedByPlayer)
        return;
    m_aimedByPlayer = newAimedByPlayer;
    emit aimedByPlayerChanged();
}


/**
 * @brief GameEnemy::player
 * @return
 */

GamePlayer *GameEnemy::player() const
{
    return qobject_cast<GamePlayer*>(m_player);
}

void GameEnemy::setPlayer(GamePlayer *newPlayer)
{
    if (m_player == newPlayer)
        return;
    m_player = newPlayer;
    emit playerChanged();
}

qreal GameEnemy::msecLeftToAttack() const
{
    return m_msecLeftToAttack;
}

void GameEnemy::setMsecLeftToAttack(qreal newMsecLeftToAttack)
{
    if (qFuzzyCompare(m_msecLeftToAttack, newMsecLeftToAttack))
        return;
    m_msecLeftToAttack = newMsecLeftToAttack;
    emit msecLeftToAttackChanged();
}




/**
 * @brief GameEnemy::attackByPlayer
 * @param player
 * @param questionEmpty
 */

void GameEnemy::attackByPlayer(GamePlayer *player, const bool &questionEmpty)
{
    decreaseHp();

    attackedByPlayerEvent(player, questionEmpty);

    if (isAlive())
        return;

    setAimedByPlayer(false);
}


/**
 * @brief GameEnemy::missedByPlayer
 * @param player
 */

void GameEnemy::missedByPlayer(GamePlayer *player)
{
    LOG_CDEBUG("scene") << "Missed by player:" << this;

    game()->setIsFlawless(false);

    turnToPlayer(player);

    emit killMissed();

    player->hurtByEnemy(this, false);
}





/**
 * @brief GameEnemy::turnToPlayer
 * @param player
 */

void GameEnemy::turnToPlayer(GamePlayer *player)
{
    if (!player) {
        LOG_CWARNING("game") << tr("Missing player");
        return;
    }

    const qreal &playerX = player->x();

    if (playerX <= x() && !m_facingLeft)
        setFacingLeft(true);
    else if (playerX > x() && m_facingLeft)
        setFacingLeft(false);
}
