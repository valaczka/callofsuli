#include "multiplayergame.h"
#include "Logger.h"
#include "client.h"
#include "gameenemysoldier.h"
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

    loadGamePage();
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
    if (m_deathmatch) {
        message(tr("LEVEL %1").arg(level()));
        message(tr("SUDDEN DEATH"));
        message(m_multiPlayerMode == MultiPlayerHost ? tr("MULTIPLAYER HOST") : tr("MULTIPLAYER CLIENT"));
        m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/sudden_death.mp3"));
    } else {
        message(tr("LEVEL %1").arg(level()));
        message(m_multiPlayerMode == MultiPlayerHost ? tr("MULTIPLAYER HOST") : tr("MULTIPLAYER CLIENT"));
        m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/begin.mp3"));
    }
}



/**
 * @brief MultiPlayerGame::timerEvent
 * @param event
 */

void MultiPlayerGame::timerEvent(QTimerEvent *)
{
    if (!m_scene)
        return;

    LOG_CTRACE("game") << "TIMER EVENT" << m_tickTimer.currentTick();

    ObjectStateSnapshot snap;

    foreach (GameObject *o, m_scene->m_gameObjects) {
        const qint64 id = getObjectId(o);

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

        if (cmd == QStringLiteral("connect")) {
            setEngineId(obj.value(QStringLiteral("engine")).toInt(-1));
            getServerState();
        } else if (cmd == QStringLiteral("state")) {
            setEngineId(obj.value(QStringLiteral("engine")).toInt(-1));
            setMultiPlayerMode(obj.value(QStringLiteral("host")).toVariant().toBool() ? MultiPlayerHost : MultiPlayerClient);
            if (obj.contains(QStringLiteral("gameState"))) {
                setMultiPlayerGameState(obj.value(QStringLiteral("gameState")).toVariant().value<MultiPlayerGameState>());
            }
            if (qint64 tick = obj.value(QStringLiteral("tick")).toInteger(-1); tick != -1) {
                m_tickTimer.start(this, tick);
            }
            if (obj.contains(QStringLiteral("interval"))) {
                setServerInterval(obj.value(QStringLiteral("interval")).toInt());
            }
        } else if (cmd == QStringLiteral("create")) {
            if (obj.contains(QStringLiteral("error"))) {
                m_client->messageError(obj.value(QStringLiteral("error")).toString(), tr("Belső hiba"));
            }
        } else if (cmd == QStringLiteral("prepare")) {
            if (obj.contains(QStringLiteral("error"))) {
                m_client->messageError(obj.value(QStringLiteral("error")).toString(), tr("Belső hiba"));
            } else if (obj.value(QStringLiteral("engine")).toInt() != m_engineId) {
                LOG_CERROR("game") << "Invalid engineId";
            } else {
                updateGameTrigger(QByteArray::fromBase64(obj.value(QStringLiteral("entities")).toString().toUtf8()));
            }
        } else if (cmd == QStringLiteral("start")) {
            if (obj.contains(QStringLiteral("error"))) {
                m_client->messageError(obj.value(QStringLiteral("error")).toString(), tr("Belső hiba"));
            }
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
    if (m_multiPlayerMode != MultiPlayerHost || m_multiPlayerGameState != StateConnecting)
        return;

    LOG_CINFO("game") << "Create multiplayer enemies";

    ObjectStateSnapshot snap;

    int num = 0;

    foreach (const GameTerrain::EnemyData &e, m_scene->terrain().enemies()) {
        if (e.type != GameTerrain::EnemySoldier)
            continue;

        ObjectStateEnemySoldier state = GameEnemySoldier::createState(e);

        std::unique_ptr<ObjectStateBase> ptr(state.clone());
        QByteArray b;
        state.toReadable(&b);
        LOG_CTRACE("game") << "+++STATE" << b.constData();
        snap.append(ptr);

        if (++num > 1)
            break;
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
    if (m_multiPlayerGameState != StatePreparing && m_multiPlayerGameState != StatePlaying)
        return;

    if (data.isEmpty() && m_entities.empty()) {
        LOG_CDEBUG("game") << "Prepare multiplayer enemies";
        sendWebSocketMessage(QJsonObject{
            { QStringLiteral("cmd"), QStringLiteral("prepare") },
            { QStringLiteral("engine"), m_engineId }
        });
        return;
    }


    LOG_CTRACE("game") << "Binary message received";

    const std::optional<ObjectStateSnapshot> &snap = ObjectStateSnapshot::fromByteArray(qUncompress(data));

    if (!snap) {
        LOG_CWARNING("game") << "Invalid binary message received";
        return;
    }

    QVector<qint64> existingIdList;

    for (const auto &s : snap->list) {
        auto it = m_entities.find(s->id);

        LOG_CINFO("game") << "---SNAP" << s->id << s->tick;

        if (it == m_entities.end()) {
            LOG_CINFO("game") << "Create entity" << s->id << s->type;

            if (s->type == ObjectStateBase::TypeEnemySoldier) {
                ObjectStateEnemySoldier *_ptr = dynamic_cast<ObjectStateEnemySoldier*>(s.get());
                if (!_ptr) {
                    LOG_CERROR("game") << "Invalid pointer";
                    continue;
                }

                LOG_CINFO("game") << "---" << _ptr->id << _ptr->enemyState << _ptr->position << _ptr->facingLeft << _ptr->enemyRect;

                GameTerrain::EnemyData d;
                d.type = GameTerrain::EnemySoldier;
                d.rect = _ptr->enemyRect;
                GameEnemySoldier *soldier = GameEnemySoldier::create(m_scene, d, QString::fromUtf8(_ptr->subType));
                updateBody(soldier, m_multiPlayerMode == MultiPlayerHost);

                soldier->setCurrentState(*_ptr, m_multiPlayerMode == MultiPlayerClient);

                auto &e = m_entities[s->id];
                e.reset(soldier);
            }
        } else {
            LOG_CTRACE("game") << "Update entity" << s->id << s->type << s->tick << "<>" << currentTick();
            it->second->setStateFromSnapshot(s.get(), currentTick(), m_multiPlayerMode == MultiPlayerClient);
        }
    }

}



/**
 * @brief MultiPlayerGame::playGameTrigger
 */

void MultiPlayerGame::playGameTrigger()
{
    if (m_multiPlayerGameState != StatePlaying)
        return;

    LOG_CTRACE("game") << "Play game trigger";

    if (m_multiPlayerMode != MultiPlayerHost)
        return;

    for (auto &e : m_entities) {
        GameEnemy *enemy = dynamic_cast<GameEnemy*>(e.second.get());

        if (enemy && enemy->enemyState() == GameEnemy::Invalid) {
            LOG_CWARNING("game") << "MOVE ENEMY" << enemy << e.first;
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
 * @brief MultiPlayerGame::getObjectId
 * @param object
 * @return
 */

qint64 MultiPlayerGame::getObjectId(GameObject *object)
{
    if (!object)
        return -1;

    for (const auto &[id, obj] : m_entities) {
        if (obj.get() == object)
            return id;
    }

    return -1;
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

    if (m_multiPlayerGameState != StateConnecting && !m_pageItem) {
        LOG_CERROR("game") << "+++PAGE";
        start();
    }

    if (m_multiPlayerGameState == StateCreating)
        createGameTrigger();
    else if (m_multiPlayerGameState == StatePlaying)
        playGameTrigger();
    /*else if (m_multiPlayerGameState == StatePreparing)
        prepareGameTrigger({});*/

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

    if (m_multiPlayerMode == MultiPlayerHost) {
        foreach (GameObject *o, m_scene->m_gameObjects) {
            if (o) {
                o->onTimingTimerTimeout(msec, delayFactor);
                o->cacheCurrentState();
            }
        }
    } else {
        const qint64 tick = currentTick();
        foreach (GameObject *o, m_scene->m_gameObjects) {
            if (o) {
                o->interpolateState(tick);
            }
        }
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

    m_scene->playSoundMusic(backgroundMusic());

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






