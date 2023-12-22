/*
 * ---- Call of Suli ----
 *
 * gameenemysniper.cpp
 *
 * Created on: 2022. 12. 29.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemySniper
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
#include "Logger.h"
#include "gameplayer.h"
#include "qdiriterator.h"
#include <QRandomGenerator>


/**
 * @brief GameEnemySniper::GameEnemySniper
 * @param parent
 */

GameEnemySniper::GameEnemySniper(QQuickItem *parent)
    : GameEnemy(parent)
{
    m_defaultShotSound = QStringLiteral("qrc:/sound/sfx/rifle.wav");

    connect(this, &GameEnemy::attack, this, &GameEnemySniper::onAttack);
}


/**
 * @brief GameEnemySniper::~GameEnemySniper
 */

GameEnemySniper::~GameEnemySniper()
{

}


/**
 * @brief GameEnemySniper::create
 * @param scene
 * @param enemyData
 * @param type
 * @return
 */

GameEnemySniper *GameEnemySniper::create(GameScene *scene, const GameTerrain::EnemyData &enemyData, const QString &type)
{
    LOG_CDEBUG("scene") << "Create enemy sniper";

    GameEnemySniper *sniper = qobject_cast<GameEnemySniper*>(GameObject::createFromFile(QStringLiteral("GameEnemySniper.qml"), scene, false));

    if (!sniper) {
        LOG_CERROR("scene") << "Enemy sniper creation error";
        return nullptr;
    }

    sniper->setParentItem(scene);
    sniper->setScene(scene);
    sniper->createSpriteItem();

    QDirIterator it(QStringLiteral(":/snipers"), {QStringLiteral("data.json")}, QDir::Files, QDirIterator::Subdirectories);
    QStringList list;

    while (it.hasNext())
        list.append(it.next().section('/',-2,-2));

    if (list.isEmpty()) {
        qFatal("Enemy sniper directory is empty");
    }

    if (type.isEmpty()) {
        const QString &s = list.at(QRandomGenerator::global()->bounded(list.size()));
        sniper->setDataDir(QStringLiteral(":/snipers/%1").arg(s));
    } else if (list.contains(type)) {
        sniper->setDataDir(QStringLiteral(":/snipers/%1").arg(type));
    } else {
        LOG_CWARNING("scene") << "Invalid enemy sniper type:" << type;
        sniper->setDataDir(QStringLiteral(":/snipers/%1").arg(list.first()));
    }


    sniper->loadFromJsonFile();
    sniper->setTerrainEnemyData(enemyData);


    return sniper;
}




/**
 * @brief GameEnemySniper::attackPlayer
 */


void GameEnemySniper::attackPlayer()
{
    emit attack();

    if (game() && player() && player()->isAlive())
        game()->enemyKillPlayer(this, player());
}



/**
 * @brief GameEnemySniper::rayCastReport
 * @param items
 */

void GameEnemySniper::rayCastReport(const QMultiMap<qreal, GameEntity *> &items)
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
 * @brief GameEnemySniper::enemyStateModified
 */

void GameEnemySniper::enemyStateModified()
{
    switch (m_enemyState) {
    case Invalid:
    case Idle:
    case Move:
        jumpToSprite(QStringLiteral("idle"));
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
 * @brief GameEnemySniper::onSceneConnected
 */

void GameEnemySniper::onSceneConnected()
{
    const QJsonObject &data = m_scene->levelData().value(QStringLiteral("enemy")).toObject().value(QStringLiteral("sniper")).toObject();

    setRayCastElevation(data.value(QStringLiteral("rayCastElevation")).toDouble());
    setRayCastLength(data.value(QStringLiteral("rayCastLength")).toDouble());
    setMsecBeforeTurn(data.value(QStringLiteral("msecBeforeTurn")).toDouble());
    setCastAttackFraction(data.value(QStringLiteral("castAttackFraction")).toDouble());
    setMsecBeforeAttack(data.value(QStringLiteral("msecBeforeAttack")).toDouble());
    setMsecBetweenAttack(data.value(QStringLiteral("msecBetweenAttack")).toDouble());
}


/**
 * @brief GameEnemySniper::onAttack
 */

void GameEnemySniper::onAttack()
{
    LOG_CDEBUG("scene") << "Enemy sniper attack" << this;

    jumpToSprite(QStringLiteral("shot"));
}





/**
 * @brief GameEnemySniper::onTimingTimerTimeout
 * @param msec
 * @param delayFactor
 */

void GameEnemySniper::onTimingTimerTimeout(const int &msec, const qreal &delayFactor)
{
    if (m_terrainEnemyData.type != GameTerrain::EnemySniper) {
        LOG_CWARNING("scene") << "Invalid enemy type";
        return;
    }

    if (m_enemyState == Dead || !isAlive())
        return;

    if (!game() || !game()->running()) {
        return;
    }

    if (m_enemyState == Move) {
        m_turnElapsedMsec += msec * delayFactor;

        if (m_turnElapsedMsec >= m_msecBeforeTurn) {
            setFacingLeft(!facingLeft());
            m_turnElapsedMsec = -1;
        }
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
 * @brief GameEnemySniper::turnElapsedMsec
 * @return
 */

int GameEnemySniper::turnElapsedMsec() const
{
    return m_turnElapsedMsec;
}

void GameEnemySniper::setTurnElapsedMsec(int newTurnElapsedMsec)
{
    if (m_turnElapsedMsec == newTurnElapsedMsec)
        return;
    m_turnElapsedMsec = newTurnElapsedMsec;
    emit turnElapsedMsecChanged();
}





int GameEnemySniper::msecBeforeTurn() const
{
    return m_msecBeforeTurn;
}

void GameEnemySniper::setMsecBeforeTurn(int newMsecBeforeTurn)
{
    if (m_msecBeforeTurn == newMsecBeforeTurn)
        return;
    m_msecBeforeTurn = newMsecBeforeTurn;
    emit msecBeforeTurnChanged();
}
