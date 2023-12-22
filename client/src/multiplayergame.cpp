#include "multiplayergame.h"
#include "Logger.h"
#include "client.h"
#include "gameenemysoldier.h"
#include "gameplayermulti.h"
#include <QRandomGenerator>


/**
 * @brief MultiPlayerGame::MultiPlayerGame
 * @param missionLevel
 * @param client
 */

MultiPlayerGame::MultiPlayerGame(GameMapMissionLevel *missionLevel, Client *client)
    : ActionGame(missionLevel, client, GameMap::MultiPlayer)
{
    if (m_client) {
        connect(m_client->httpConnection()->webSocket(), &WebSocket::messageReceived, this, &MultiPlayerGame::onJsonReceived);
        connect(m_client->httpConnection()->webSocket(), &WebSocket::activeChanged, this, &MultiPlayerGame::onActiveChanged);

        m_client->httpConnection()->webSocket()->observerAdd(QStringLiteral("multiplayer"));

        if (m_client->httpConnection()->webSocket()->active())
            onActiveChanged();
        else
            m_client->httpConnection()->webSocket()->connect();
    }

    connect(&m_timeSyncTimer, &QTimer::timeout, this, &MultiPlayerGame::onTimeSyncTimerTimeout);

    m_timeSyncTimer.setInterval(15000);
    m_timeSyncTimer.start();
}


/**
 * @brief MultiPlayerGame::~MultiPlayerGame
 */

MultiPlayerGame::~MultiPlayerGame()
{
    if (m_client) {
        m_client->httpConnection()->webSocket()->observerRemove(QStringLiteral("multiplayer"));
    }
}



/**
 * @brief MultiPlayerGame::start
 */

void MultiPlayerGame::start()
{
    if (m_pageItem) {
        LOG_CWARNING("game") << "ActionGame page already exists";
        return;
    }

    sendWebSocketMessage(QJsonObject{
        { QStringLiteral("cmd"), QStringLiteral("start") },
        { QStringLiteral("engine"), m_engineId }
    });
}


/**
 * @brief MultiPlayerGame::gameAbort
 */

void MultiPlayerGame::gameAbort()
{
    setFinishState(Neutral);

    LOG_CINFO("game") << "Game aborted:" << this;

    emit gameFinished(Neutral);
}




/**
 * @brief MultiPlayerGame::loadPage
 * @return
 */

QQuickItem *MultiPlayerGame::loadPage()
{
    QQuickItem *page = m_client->stackPushPage(QStringLiteral("PageMultiPlayerGame.qml"), QVariantMap({
                                                                                              { QStringLiteral("game"), QVariant::fromValue(this) }
                                                                                          }));

    return page;
}



/**
 * @brief MultiPlayerGame::onSceneAboutToStart
 */

void MultiPlayerGame::onSceneAboutToStart()
{
    /*if (m_deathmatch) {
        message(tr("LEVEL %1").arg(level()));
        message(tr("SUDDEN DEATH"));
        message(m_multiPlayerMode == MultiPlayerHost ? tr("MULTIPLAYER HOST") : tr("MULTIPLAYER CLIENT"));
        m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/sudden_death.mp3"));
    } else {
        message(tr("LEVEL %1").arg(level()));
        message(m_multiPlayerMode == MultiPlayerHost ? tr("MULTIPLAYER HOST") : tr("MULTIPLAYER CLIENT"));
        m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/begin.mp3"));
    }*/
}



/**
 * @brief MultiPlayerGame::timerEvent
 * @param event
 */

void MultiPlayerGame::timerEvent(QTimerEvent *)
{
    if (!m_scene || m_multiPlayerGameState != StatePlaying)
        return;

    LOG_CTRACE("game") << "TIMER EVENT" << m_tickTimer.currentTick();

    ObjectStateSnapshot snap;

    foreach (GameObject *o, m_scene->m_gameObjects) {
        const qint64 id = getEntityId(o);

        if (o && id != -1) {
            o->getStateSnapshot(&snap, id);
        }
    }

    m_client->httpConnection()->webSocket()->send(qCompress(snap.toByteArray()));
}




/**
 * @brief MultiPlayerGame::sendWebSocketMessage
 * @param cmd
 * @param data
 */

void MultiPlayerGame::sendWebSocketMessage(const QJsonValue &data)
{
    if (!m_client || !m_client->httpConnection()->webSocket()->active()) {
        LOG_CWARNING("game") << "WebSocket inactive";
        return;
    }

    LOG_CTRACE("game") << "SEND:" << data;

    m_client->httpConnection()->webSocket()->send(QStringLiteral("multiplayer"), data);
}



/**
 * @brief MultiPlayerGame::getServerState
 */

void MultiPlayerGame::getServerState()
{
    m_client->httpConnection()->webSocket()->send(QStringLiteral("multiplayer"), QJsonObject{
                                                                                     { QStringLiteral("cmd"), QStringLiteral("state") }
                                                                                 });

}



/**
 * @brief MultiPlayerGame::onTimeSyncTimerTimeout
 */

void MultiPlayerGame::onTimeSyncTimerTimeout()
{
    if (!m_client)
        return;

    if (!m_client->httpConnection()->webSocket()->active()) {
        LOG_CWARNING("game") << "WebSocket inactive";
        return;
    }

    LOG_CTRACE("game") << "Sync server time" << QDateTime::currentDateTime();

    m_client->httpConnection()->webSocket()->send(QStringLiteral("timeSync"), QJsonObject{
                                                                                  { QStringLiteral("clientTime"), QDateTime::currentMSecsSinceEpoch() }
                                                                              });

}



/**
 * @brief MultiPlayerGame::onActiveChanged
 */

void MultiPlayerGame::onActiveChanged()
{
    auto ws = m_client->httpConnection()->webSocket();

    const bool active = ws->active();
    LOG_CTRACE("game") << "MultiPlayerGame WebSocket active changed" << active;

    if (active && !m_binarySignalConnected) {
        connect(ws->socket(), &QWebSocket::binaryMessageReceived, this, &MultiPlayerGame::updateGameTrigger);
        m_binarySignalConnected = true;
    }

    sendWebSocketMessage(QJsonObject{
        { QStringLiteral("cmd"), QStringLiteral("connect") }
    });


}



/**
 * @brief MultiPlayerGame::onJsonReceived
 * @param operation
 * @param data
 */

void MultiPlayerGame::onJsonReceived(const QString &operation, const QJsonValue &data)
{
    if (operation == QStringLiteral("timeSync")) {
        const QJsonObject &obj = data.toObject();
        qint64 clientTime = QDateTime::currentMSecsSinceEpoch() - obj.value(QStringLiteral("clientTime")).toInteger();

        m_tickTimer.setLatency(clientTime/2);

        LOG_CTRACE("game") << "ACTUAL SERVER LATENCY" << m_tickTimer.latency();
    } else if (operation == QStringLiteral("multiplayer")) {
        const QJsonObject &obj = data.toObject();
        const QString &cmd = obj.value(QStringLiteral("cmd")).toString();

        if (obj.contains(QStringLiteral("error"))) {
            m_client->messageError(obj.value(QStringLiteral("error")).toString(), tr("Belső hiba"));
        } else if (cmd == QStringLiteral("connect")) {
            setEngineId(obj.value(QStringLiteral("engine")).toInt(-1));
            getServerState();
        } else if (cmd == QStringLiteral("state")) {
            setEngineId(obj.value(QStringLiteral("engine")).toInt(-1));
            setMultiPlayerMode(obj.value(QStringLiteral("host")).toVariant().toBool() ? MultiPlayerHost : MultiPlayerClient);
            if (obj.contains(QStringLiteral("gameState"))) {
                setMultiPlayerGameState(obj.value(QStringLiteral("gameState")).toVariant().value<MultiPlayerGameState>());
            }
            if (obj.contains(QStringLiteral("playerId"))) {
                setPlayerId(obj.value(QStringLiteral("playerId")).toInt());
            }
            if (obj.contains(QStringLiteral("playerEntityId"))) {
                setPlayerEntityId(obj.value(QStringLiteral("playerEntityId")).toInt());
            }
            if (qint64 tick = obj.value(QStringLiteral("tick")).toInteger(-1); tick != -1) {
                m_tickTimer.start(this, tick);
            }
            if (obj.contains(QStringLiteral("interval"))) {
                setServerInterval(obj.value(QStringLiteral("interval")).toInt());
            }
        } else if (cmd == QStringLiteral("create")) {

        } else if (cmd == QStringLiteral("prepare")) {
            if (obj.value(QStringLiteral("engine")).toInt() != m_engineId) {
                LOG_CERROR("game") << "Invalid engineId";
            } else {
                updateGameTrigger(QByteArray::fromBase64(obj.value(QStringLiteral("entities")).toString().toUtf8()));
            }
        } else if (cmd == QStringLiteral("start")) {

        }
    }
}




/**
 * @brief MultiPlayerGame::loadGamePage
 */

void MultiPlayerGame::loadGamePage()
{
    QQuickItem *page = m_client->stackPushPage(QStringLiteral("PageActionGame.qml"), QVariantMap({
                                                                                         { QStringLiteral("game"), QVariant::fromValue(this) }
                                                                                     }));

    connect(page, &QQuickItem::destroyed, this, [this](){
            LOG_CINFO("game") << "Multiplayer page destroyed";
            setScene(nullptr);
            setPageItem(nullptr);
        }, Qt::DirectConnection);

    const QVariant &scene = page->property("scene");

    setScene(qvariant_cast<GameScene*>(scene));

    LOG_CTRACE("game") << "LOAD GAME PAGE" << m_scene << m_pageItem << page;

    setPageItem(page);
}




/**
 * @brief MultiPlayerGame::createGameTrigger
 */

void MultiPlayerGame::createGameTrigger()
{
    if (m_multiPlayerMode != MultiPlayerHost || m_multiPlayerGameState != StateCreating || !m_scene)
        return;

    LOG_CINFO("game") << "Create multiplayer enemies";

    ObjectStateSnapshot snap;

    int num = 0;

    foreach (const GameTerrain::EnemyData &e, m_scene->terrain().enemies()) {
        if (e.type != GameTerrain::EnemySoldier)
            continue;

        if (num > 3)
            break;
        else if (num > 1) {
            ObjectStateBase state = GamePlayerMulti::createState(e.rect.bottomLeft());
            const auto &chList = ActionGame::availableCharacters();
            if (chList.size() > (num-2)) {
                state.fields.setFlag(ObjectStateBase::FieldSubType);
                state.subType = chList.at(num-2).toUtf8();
            }

            LOG_CTRACE("game") << "+++STATE" << state.toReadable();
            snap.append(state);
        } else {
            ObjectStateBase state = GameEnemySoldier::createState(e);

            LOG_CTRACE("game") << "+++STATE" << state.toReadable();
            snap.append(state);
        }

        ++num;

    }

    sendWebSocketMessage(QJsonObject{
        { QStringLiteral("cmd"), QStringLiteral("create") },
        { QStringLiteral("engine"), m_engineId },
        { QStringLiteral("entities"), QString::fromUtf8(qCompress(snap.toByteArray()).toBase64()) }
    });
}



/**
 * @brief MultiPlayerGame::updateGameTrigger
 */

void MultiPlayerGame::updateGameTrigger(const QByteArray &data)
{
    if (m_multiPlayerGameState != StateCreating && m_multiPlayerGameState != StatePlaying && m_multiPlayerGameState != StatePreparing)
        return;

    if (data.isEmpty() && m_entities.empty()) {
        LOG_CDEBUG("game") << "Prepare multiplayer enemies";
        sendWebSocketMessage(QJsonObject{
            { QStringLiteral("cmd"), QStringLiteral("prepare") },
            { QStringLiteral("engine"), m_engineId }
        });
        return;
    }


    bool _sendReady = m_entities.empty();

    LOG_CTRACE("game") << "Binary message received" << "??" << _sendReady;

    const std::optional<ObjectStateSnapshot> &snap = ObjectStateSnapshot::fromByteArray(qUncompress(data));

    if (!snap) {
        LOG_CWARNING("game") << "Invalid binary message received";
        return;
    }

    ObjectStateBase prev;

    for (const auto &sn : snap->list) {
        if (prev.type != sn.type || prev.id != sn.id) {
            prev = sn;
        } else {
            if (!prev.patch(sn)) {
                LOG_CERROR("game") << "State patch error" << sn.type << sn.id;
                continue;
            }
        }

        auto it = std::find_if(m_entities.cbegin(), m_entities.cend(), [&prev](const Entity &e) {
            return e.id == prev.id;
        });

        const bool isSelf = (prev.type == ObjectStateBase::TypePlayer && prev.id == m_playerEntityId) ||
                            (prev.type == ObjectStateBase::TypeEnemySoldier && m_multiPlayerMode == MultiPlayerHost);

        if (it == m_entities.cend()) {
            LOG_CINFO("game") << "Create entity" << prev.id << prev.type;

            if (prev.type == ObjectStateBase::TypeEnemySoldier) {
                LOG_CINFO("game") << "---" << prev.toReadable();

                GameTerrain::EnemyData d;
                d.type = GameTerrain::EnemySoldier;
                d.rect = prev.enemyRect;
                GameEnemySoldier *soldier = GameEnemySoldier::create(m_scene, d, QString::fromUtf8(prev.subType));
                updateBody(soldier, isSelf);

                soldier->setCurrentState(prev, !isSelf);

                m_entities.push_back({prev.id, prev.type, soldier});
            } else if (prev.type == ObjectStateBase::TypePlayer) {
                LOG_CINFO("game") << "--- PLAYER" << isSelf << prev.toReadable();

                GamePlayerMulti *player = GamePlayer::create<GamePlayerMulti>(m_scene, QStringLiteral("GamePlayerMulti.qml"), prev.subType);
                updateBody(player, isSelf);
                player->setIsSelf(isSelf);
                player->init(this);
                player->setCurrentState(prev, !isSelf);

                if (isSelf)
                    setPlayer(player);

                m_entities.push_back({prev.id, prev.type, player});
            }
        } else {
            LOG_CTRACE("game") << "Update entity" << prev.id << prev.type << prev.tick << "<>" << currentTick();
            if (it->object) {
                it->object->setAuthoritativeStateInterval(m_serverInterval);
                it->object->setStateFromSnapshot(prev, currentTick(), !isSelf);
            } else
                LOG_CERROR("game") << "Invalid object pointer" << prev.id;
        }
    }



    if (_sendReady)
        sendWebSocketMessage(QJsonObject{
            { QStringLiteral("cmd"), QStringLiteral("prepare") },
            { QStringLiteral("engine"), m_engineId },
            { QStringLiteral("ready"), true }
        });
}



/**
 * @brief MultiPlayerGame::playGameTrigger
 */

void MultiPlayerGame::playGameTrigger()
{
    if (m_multiPlayerGameState != StatePlaying)
        return;

    message(m_multiPlayerMode == MultiPlayerHost ? tr("MULTIPLAYER HOST") : tr("MULTIPLAYER CLIENT"));
    m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/begin.mp3"));


    LOG_CTRACE("game") << "Play game trigger";

    if (m_multiPlayerMode != MultiPlayerHost)
        return;

    for (const auto &e : m_entities) {
        GameEnemy *enemy = dynamic_cast<GameEnemy*>(e.object.get());

        if (enemy && enemy->enemyState() == GameEnemy::Invalid) {
            LOG_CWARNING("game") << "MOVE ENEMY" << enemy << e.id;
            //enemy->setEnemyState(GameEnemy::Move);
            enemy->startMovingAfter(1500);
        }

    }
}


/**
 * @brief MultiPlayerGame::updateBody
 * @param object
 */

void MultiPlayerGame::updateBody(GameObject *object, const bool &owned)
{
    if (!object)
        return;

    const float g = object->body()->gravityScale();
    const float _g = owned ? DEFAULT_ENTITY_GRAVITY_SCALE : 0.0;

    if (g != _g)
        object->body()->setGravityScale(_g);
}


/**
 * @brief MultiPlayerGame::getEntityId
 * @param object
 * @return
 */

qint64 MultiPlayerGame::getEntityId(GameObject *object) const
{
    if (!object)
        return -1;

    for (const auto &e : m_entities) {
        if (e.object.get() == object)
            return e.id;
    }

    return -1;
}


/**
 * @brief MultiPlayerGame::getEntity
 * @param entityId
 * @return
 */

GameObject *MultiPlayerGame::getEntity(const qint64 &entityId) const
{
    if (entityId < 0)
        return nullptr;

    for (const auto &e : m_entities) {
        if (e.id == entityId)
            return e.object.get();
    }

    return nullptr;
}

qint64 MultiPlayerGame::playerEntityId() const
{
    return m_playerEntityId;
}

void MultiPlayerGame::setPlayerEntityId(qint64 newPlayerEntityId)
{
    if (m_playerEntityId == newPlayerEntityId)
        return;
    m_playerEntityId = newPlayerEntityId;
    emit playerEntityIdChanged();
}




/**
 * @brief MultiPlayerGame::enemyAttackPlayer
 * @param enemy
 * @param canProtect
 * @param player
 */

void MultiPlayerGame::enemyAttackPlayer(GameEnemy *enemy, const bool &canProtect, GamePlayer *player)
{
    LOG_CERROR("game")    << "Enemy attack player" << enemy << canProtect << player;
}



/**
 * @brief MultiPlayerGame::enemyKillPlayer
 * @param enemy
 * @param player
 */

void MultiPlayerGame::enemyKillPlayer(GameEnemy *enemy, GamePlayer *player)
{
    LOG_CERROR("game")    << "Enemy kill player" << enemy << player;
}



/**
 * @brief MultiPlayerGame::tryAttack
 * @param player
 * @param enemy
 */

void MultiPlayerGame::tryAttack(GamePlayer *player, GameEnemy *enemy)
{
    LOG_CERROR("game")    << "Player try attack" << player << enemy;
}





/**
 * @brief MultiPlayerGame::playerId
 * @return
 */

int MultiPlayerGame::playerId() const
{
    return m_playerId;
}

void MultiPlayerGame::setPlayerId(int newPlayerId)
{
    if (m_playerId == newPlayerId)
        return;
    m_playerId = newPlayerId;
    emit playerIdChanged();
}



/**
 * @brief MultiPlayerGame::serverInterval
 * @return
 */

int MultiPlayerGame::serverInterval() const
{
    return m_serverInterval;
}

void MultiPlayerGame::setServerInterval(int newServerInterval)
{
    m_serverInterval = newServerInterval;
}


/**
 * @brief MultiPlayerGame::multiPlayerGameState
 * @return
 */

MultiPlayerGameState MultiPlayerGame::multiPlayerGameState() const
{
    return m_multiPlayerGameState;
}

void MultiPlayerGame::setMultiPlayerGameState(MultiPlayerGameState newMultiPlayerGameState)
{
    if (m_multiPlayerGameState == newMultiPlayerGameState)
        return;
    m_multiPlayerGameState = newMultiPlayerGameState;
    emit multiPlayerGameStateChanged();

    LOG_CINFO("game") << "MULTIPLAYER STATE CHANGED" << m_multiPlayerGameState << m_pageItem;


    if (m_multiPlayerMode == MultiPlayerHost) {
        if ((m_multiPlayerGameState == StatePreparing || m_multiPlayerGameState == StateCreating || m_multiPlayerGameState == StatePlaying) && !m_pageItem) {
            LOG_CERROR("game") << "+++PAGE";
            loadGamePage();
        }
    } else {
        if ((m_multiPlayerGameState == StatePreparing || m_multiPlayerGameState == StatePlaying) && !m_pageItem) {
            LOG_CERROR("game") << "+++PAGE";
            loadGamePage();
        }
    }


    /*if (m_multiPlayerGameState == StateCreating)
        createGameTrigger();
    else if (m_multiPlayerGameState == StatePreparing)
        updateGameTrigger({});
    else*/ if (m_multiPlayerGameState == StatePlaying)
        playGameTrigger();

}




int MultiPlayerGame::engineId() const
{
    return m_engineId;
}

void MultiPlayerGame::setEngineId(int newEngineId)
{
    if (m_engineId == newEngineId)
        return;
    m_engineId = newEngineId;
    emit engineIdChanged();
}



/**
 * @brief MultiPlayerGame::multiPlayerMode
 * @return
 */

const MultiPlayerGame::Mode &MultiPlayerGame::multiPlayerMode() const
{
    return m_multiPlayerMode;
}

void MultiPlayerGame::setMultiPlayerMode(const Mode &newMultiPlayerMode)
{
    if (m_multiPlayerMode == newMultiPlayerMode)
        return;
    m_multiPlayerMode = newMultiPlayerMode;
    emit multiPlayerModeChanged();
}



/**
 * @brief MultiPlayerGame::sceneTimerTimeout
 * @param msec
 * @param delayFactor
 */

void MultiPlayerGame::sceneTimerTimeout(const int &msec, const qreal &delayFactor)
{
    if (!m_scene) {
        LOG_CWARNING("game") << "Missing scene";
        return;
    }

    if (m_multiPlayerGameState != StatePlaying)
        return;

    const qint64 tick = currentTick();

    for (const auto &e : m_entities) {
        if (!e.object) {
            LOG_CERROR("game") << "Entity missing:" << e.id;
            continue;
        }

        bool hosted = false;

        if ((e.type == ObjectStateBase::TypeEnemySniper && m_multiPlayerMode == MultiPlayerHost) ||
            (e.type == ObjectStateBase::TypeEnemySoldier && m_multiPlayerMode == MultiPlayerHost) ||
            (e.type == ObjectStateBase::TypePlayer && e.id == m_playerEntityId))
            hosted = true;

        e.object->onTimingTimerTimeoutMulti(hosted, msec, delayFactor);

        if (hosted)
            e.object->cacheCurrentState();
        else
            e.object->interpolateState(tick);
    }


    foreach (GameObject *o, m_scene->m_gameObjects) {
        GameEntity *e = qobject_cast<GameEntity*>(o);
        if (e)
            e->performRayCast();
    }
}



/**
 * @brief MultiPlayerGame::onSceneReady
 */

void MultiPlayerGame::onSceneReady()
{
    LOG_CTRACE("game") << "Multiplayer scene ready" << this;

    createQuestions();
    /*createEnemyLocations();
    if (m_multiPlayerMode == MultiPlayerHost) {
        createFixEnemies();
        createInventory();
    }*/

    pageItem()->setState(QStringLiteral("run"));

    ////m_scene->playSoundMusic(backgroundMusic());

    LOG_CTRACE("game") << "!!!!" << m_multiPlayerMode << m_multiPlayerGameState;

    if (m_multiPlayerMode == MultiPlayerHost)
        createGameTrigger();
}



/**
 * @brief MultiPlayerGame::onSceneAnimationFinished
 */

void MultiPlayerGame::onSceneAnimationFinished()
{
    LOG_CTRACE("game") << "Multiplayer scene amimation finsihed" << this;

    updateGameTrigger({});
}






