/*
 * ---- Call of Suli ----
 *
 * gameenemysoldier.cpp
 *
 * Created on: 2022. 12. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemySoldier
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

#include "gameenemysoldier.h"
#include "gamescene.h"
#include "qdiriterator.h"
#include "actiongame.h"
#include "gameplayer.h"
#include "Logger.h"
#include <QRandomGenerator>

GameEnemySoldier::GameEnemySoldier(QQuickItem *parent)
    : GameEnemy(parent)
{
    connect(this, &GameEnemy::attack, this, &GameEnemySoldier::onAttack);
    connect(this, &GameObject::sceneConnected, this, &GameEnemySoldier::onSceneConnected);

}


/**
 * @brief GameEnemy::~GameEnemy
 */

GameEnemySoldier::~GameEnemySoldier()
{

}


/**
 * @brief GameEnemySoldier::onAttack
 */

void GameEnemySoldier::onAttack()
{
    LOG_CDEBUG("scene") << "Enemy soldier attack" << this;

    jumpToSprite(QStringLiteral("shot"));
}





/**
 * @brief GameEnemySoldier::cacheCurrentState
 */

void GameEnemySoldier::cacheCurrentState()
{
    ActionGame *_game = game();

    if (!_game)
        return;

    if (!m_cachedStates.empty()) {
        ObjectStateBase &state = m_cachedStates.back();

        if (m_stateHash.value(state.enemyState, Invalid) == m_enemyState &&
            state.facingLeft == m_facingLeft &&
            state.hp == m_hp &&
            state.maxHp == m_maxHp) {
            state = getCurrentState();
            state.tick = _game->currentTick();
            return;
        }
    }

    ObjectStateBase state = getCurrentState();
    state.tick = _game->currentTick();

    m_cachedStates.push_back(state);
}










/**
 * @brief GameEnemySoldier::setStateFromSnapshot
 * @param ptr
 * @param currentTick
 * @param force
 */

void GameEnemySoldier::setStateFromSnapshot(const ObjectStateBase &ptr, const qint64 &currentTick, const bool &force)
{
    if (ptr.type != ObjectStateBase::TypeEnemySoldier) {
        LOG_CERROR("game") << "Invalid ObjectState" << ptr.id << ptr.type;
        return;
    }

    if (!force) {
        const auto &s = GameObject::stateReconciliation(ptr);

        if (s.has_value()) {
            const ObjectStateBase &curr = getCurrentState();
            if (!stateReconciliation(*s, curr)) {
                setCurrentState(*s, true);
            }
        } else {
            setCurrentState(ptr, true);
        }
    } else {
        removeOldAuthoritativeStates(currentTick);
        m_authoritativeStates.insert({ptr.tick, ptr});

        interpolateState(currentTick);
    }
}



/**
 * @brief GameEnemySoldier::getCurrentState
 * @return
 */

ObjectStateBase GameEnemySoldier::getCurrentState() const
{
    ObjectStateBase b = GameEnemy::getCurrentState();
    b.type = ObjectStateBase::TypeEnemySoldier;

    b.fields.setFlag(ObjectStateBase::FieldTurnElapsedMSec);
    b.fields.setFlag(ObjectStateBase::FieldAttackElapsedMSec);

    b.turnElapsedMsec = m_turnElapsedMsec;
    b.attackElapsedMsec = m_attackElapsedMsec;

    return b;
}




/**
 * @brief GameEnemySoldier::setCurrentState
 * @param state
 * @param force
 */

void GameEnemySoldier::setCurrentState(const ObjectStateBase &state, const bool &force)
{
    GameEnemy::setCurrentState(state, force);

    if (force) {
        if (state.fields.testFlag(ObjectStateBase::FieldTurnElapsedMSec))
            setTurnElapsedMsec(state.turnElapsedMsec);

        if (state.fields.testFlag(ObjectStateBase::FieldAttackElapsedMSec))
            m_attackElapsedMsec = state.attackElapsedMsec;
    }
}






/**
 * @brief GameEnemySoldier::onTimingTimerTimeout
 * @param msec
 * @param delayFactor
 */

void GameEnemySoldier::onTimingTimerTimeout(const int &msec, const qreal &delayFactor)
{
    if (m_terrainEnemyData.type != GameTerrain::EnemySoldier) {
        LOG_CWARNING("scene") << "Invalid enemy type";
        return;
    }

    if (m_enemyState == Dead || !isAlive())
        return;

    ActionGame *_game = game();

    if (!_game || !_game->running())
        return;


    if (m_enemyState == Idle) {
        m_turnElapsedMsec += msec * delayFactor;

        if (m_turnElapsedMsec >= m_msecBeforeTurn) {
            setFacingLeft(!facingLeft());
            setEnemyState(Move);
            m_turnElapsedMsec = -1;
        }
    } else if (m_enemyState == Move) {
        qreal posX = x();
        qreal delta = m_walkSize;

        if (facingLeft()) {
            if ((posX-(delta*delayFactor) < m_terrainEnemyData.rect.left()) && (posX-delta >= m_terrainEnemyData.rect.left())) {
                setX(posX-delta);
            } else if (posX-(delta*delayFactor) < m_terrainEnemyData.rect.left()) {
                setEnemyState(Idle);
                m_turnElapsedMsec = 0;
            } else {
                setX(posX-(delta*delayFactor));
            }
        } else {
            if ((posX+(delta*delayFactor) > m_terrainEnemyData.rect.right() - width()) && (posX+delta <= m_terrainEnemyData.rect.right() - width())) {
                setX(posX+delta);
            } else if (posX+(delta*delayFactor) > m_terrainEnemyData.rect.right() - width()) {
                setEnemyState(Idle);
                m_turnElapsedMsec = 0;
            } else {
                setX(posX+(delta*delayFactor));
            }
        }

        m_body->setAwake(true);

    } else if (m_enemyState == WatchPlayer) {
        m_attackElapsedMsec += msec * delayFactor;

        setMsecLeftToAttack(qMax((int)m_msecBeforeAttack-m_attackElapsedMsec, 0));

        if (m_attackElapsedMsec >= m_msecBeforeAttack) {
            setEnemyState(Attack);
            attackPlayer();
            m_attackElapsedMsec = 0;
        }
    } else if (m_enemyState == Attack) {
        m_attackElapsedMsec += msec * delayFactor;

        if (m_attackElapsedMsec >= m_msecBetweenAttack) {
            attackPlayer();
            m_attackElapsedMsec = 0;
        }
    }
}



/**
 * @brief GameEnemySoldier::onTimingTimerTimeoutMulti
 * @param hosted
 * @param msec
 * @param delayFactor
 */

void GameEnemySoldier::onTimingTimerTimeoutMulti(const bool &hosted, const int &msec, const qreal &delayFactor)
{
    if (hosted)
        onTimingTimerTimeout(msec, delayFactor);
}









/**
 * @brief GameEnemySoldier::turnElapsedMsec
 * @return
 */

int GameEnemySoldier::turnElapsedMsec() const
{
    return m_turnElapsedMsec;
}

void GameEnemySoldier::setTurnElapsedMsec(int newTurnElapsedMsec)
{
    if (m_turnElapsedMsec == newTurnElapsedMsec)
        return;
    m_turnElapsedMsec = newTurnElapsedMsec;
    emit turnElapsedMsecChanged();
}





/**
 * @brief GameEnemySoldier::create
 * @param scene
 * @param type
 * @return
 */

GameEnemySoldier *GameEnemySoldier::create(GameScene *scene, const GameTerrain::EnemyData &enemyData, const QString &type)
{
    LOG_CDEBUG("scene") << "Create enemy soldier";

    GameEnemySoldier *soldier = qobject_cast<GameEnemySoldier*>(GameObject::createFromFile(QStringLiteral("GameEnemySoldier.qml"), scene, false));

    if (!soldier) {
        LOG_CERROR("scene") << "Enemy soldier creation error";
        return nullptr;
    }

    soldier->setParentItem(scene);
    soldier->setScene(scene);
    soldier->createSpriteItem();

    QDirIterator it(QStringLiteral(":/soldiers"), {QStringLiteral("data.json")}, QDir::Files, QDirIterator::Subdirectories);
    QStringList list;

    while (it.hasNext())
        list.append(it.next().section('/',-2,-2));

    if (list.isEmpty()) {
        qFatal("Enemy soldier directory is empty");
    }

    if (type.isEmpty()) {
        const QString &s = list.at(QRandomGenerator::global()->bounded(list.size()));
        soldier->setDataDir(QStringLiteral(":/soldiers/%1").arg(s));
    } else if (list.contains(type)) {
        soldier->setDataDir(QStringLiteral(":/soldiers/%1").arg(type));
    } else {
        LOG_CWARNING("scene") << "Invalid enemy soldier type:" << type;
        soldier->setDataDir(QStringLiteral(":/soldiers/%1").arg(list.first()));
    }


    soldier->loadFromJsonFile();
    soldier->setTerrainEnemyData(enemyData);

    return soldier;
}




/**
 * @brief GameEnemySoldier::createState
 * @param enemyData
 * @return
 */

ObjectStateBase GameEnemySoldier::createState(const GameTerrain::EnemyData &enemyData)
{
    QDirIterator it(QStringLiteral(":/soldiers"), {QStringLiteral("data.json")}, QDir::Files, QDirIterator::Subdirectories);
    QStringList list;

    while (it.hasNext())
        list.append(it.next().section('/',-2,-2));

    if (list.isEmpty()) {
        qFatal("Enemy soldier directory is empty");
    }

    ObjectStateBase state;

    state.type = ObjectStateBase::TypeEnemySoldier;

    state.fields = ObjectStateBase::FieldHp|ObjectStateBase::FieldMaxHp|ObjectStateBase::FieldEnemyState|ObjectStateBase::FieldEnemyRect|
                   ObjectStateBase::FieldSubType|ObjectStateBase::FieldFacingLeft|ObjectStateBase::FieldPosition;

    state.hp = 1;
    state.maxHp = 1;
    state.enemyRect = enemyData.rect;
    state.enemyState = ObjectStateBase::Idle;
    state.subType = list.at(QRandomGenerator::global()->bounded(list.size())).toUtf8();
    state.facingLeft = QRandomGenerator::global()->generate() % 2;
    state.position = QPointF(enemyData.rect.left() + enemyData.rect.width()/2,
                             enemyData.rect.bottom());

    return state;
}


/**
 * @brief GameEnemySoldier::attackPlayer
 */

void GameEnemySoldier::attackPlayer()
{
    emit attack();

    //jumpToSprite(QStringLiteral("shot"));

    if (player() && player()->isAlive())
        player()->hurtByEnemy(this, true);
}




/**
 * @brief GameEnemySoldier::rayCastReport
 * @param items
 */

void GameEnemySoldier::rayCastReport(const QMultiMap<qreal, GameEntity *> &items)
{
    GamePlayer *_player = nullptr;

    qreal fraction = -1.0;

    for (auto it = items.constBegin(); it != items.constEnd(); ++it) {
        GamePlayer *e = qobject_cast<GamePlayer *>(it.value());

        if (e && e->isAlive() && !e->invisible()) {
            _player = e;
            fraction = it.key();
            break;
        }
    }

    GamePlayer *oldPlayer = player();

    setPlayer(_player);

    if ((m_enemyState == Attack || m_enemyState == WatchPlayer) && !_player) {
        if (oldPlayer)
            turnToPlayer(oldPlayer);
        else
            setEnemyState(Move);
    } else if (_player && m_enemyState != Attack && m_enemyState != WatchPlayer) {
        if (fraction != -1.0 && fraction < m_castAttackFraction) {
            setEnemyState(Attack);
            attackPlayer();
        } else
            setEnemyState(WatchPlayer);
    }

}


/**
 * @brief GameEnemySoldier::enemyStateModified
 * @param newEnemyState
 */

void GameEnemySoldier::enemyStateModified()
{
    switch (m_enemyState) {
    case Invalid:
    case Idle:
        jumpToSprite(QStringLiteral("idle"));
        break;
    case Move:
        jumpToSprite(QStringLiteral("walk"));
        break;
    case WatchPlayer:
        m_attackElapsedMsec = 0;
        setMsecLeftToAttack(m_msecBeforeAttack);
        jumpToSprite(QStringLiteral("idle"));
        break;

    case Attack:
        setMsecLeftToAttack(0);
        break;

    case Dead:
        jumpToSprite(QStringLiteral("idle"));
        jumpToSprite(QStringLiteral("dead"));
        break;
    }
}



/**
 * @brief GameEnemySoldier::interpolate
 * @param t
 * @param from
 * @param to
 * @return
 */

ObjectStateBase GameEnemySoldier::interpolate(const qreal &t, const ObjectStateBase &from, const ObjectStateBase &to)
{
    ObjectStateBase b = GameEnemy::interpolate(t, from, to);

    if (t < 1.0) {
        if (from.fields.testFlag(ObjectStateBase::FieldTurnElapsedMSec))
            b.turnElapsedMsec = std::lerp(from.turnElapsedMsec, to.turnElapsedMsec, t);
        if (from.fields.testFlag(ObjectStateBase::FieldAttackElapsedMSec))
            b.attackElapsedMsec = std::lerp(from.attackElapsedMsec, to.attackElapsedMsec, t);
    } else {
        if (from.fields.testFlag(ObjectStateBase::FieldTurnElapsedMSec))
            b.turnElapsedMsec = to.turnElapsedMsec;
        if (from.fields.testFlag(ObjectStateBase::FieldAttackElapsedMSec))
            b.attackElapsedMsec = to.attackElapsedMsec;
    }

    return b;
}





/**
 * @brief GameEnemySoldier::onSceneConnected
 */

void GameEnemySoldier::onSceneConnected()
{
    const QJsonObject &data = m_scene->levelData().value(QStringLiteral("enemy")).toObject().value(QStringLiteral("soldier")).toObject();

    setRayCastElevation(data.value(QStringLiteral("rayCastElevation")).toDouble());
    setRayCastLength(data.value(QStringLiteral("rayCastLength")).toDouble());
    setMsecBeforeTurn(data.value(QStringLiteral("msecBeforeTurn")).toDouble());
    setCastAttackFraction(data.value(QStringLiteral("castAttackFraction")).toDouble());
    setMsecBeforeAttack(data.value(QStringLiteral("msecBeforeAttack")).toDouble());
    setMsecBetweenAttack(data.value(QStringLiteral("msecBetweenAttack")).toDouble());
}


int GameEnemySoldier::msecBeforeTurn() const
{
    return m_msecBeforeTurn;
}

void GameEnemySoldier::setMsecBeforeTurn(int newMsecBeforeTurn)
{
    if (m_msecBeforeTurn == newMsecBeforeTurn)
        return;
    m_msecBeforeTurn = newMsecBeforeTurn;
    emit msecBeforeTurnChanged();
}


