#include "multiplayerengine.h"
#include "Logger.h"
#include "objectstate.h"
#include "qjsonarray.h"
#include "qjsonobject.h"
#include "enginehandler.h"
#include "serverservice.h"


/**
 * @brief MultiPlayerEngine::MultiPlayerEngine
 * @param parent
 */

MultiPlayerEngine::MultiPlayerEngine(EngineHandler *handler, QObject *parent)
    : AbstractEngine{EngineMultiPlayer, handler, parent}
{

}



/**
 * @brief MultiPlayerEngine::~MultiPlayerEngine
 */

MultiPlayerEngine::~MultiPlayerEngine()
{
    LOG_CTRACE("engine") << "MultiPlayerEngine destroy" << this;
}






/**
 * @brief MultiPlayerEngine::handleWebSocketMessage
 * @param stream
 * @param message
 * @param service
 */

void MultiPlayerEngine::handleWebSocketMessage(WebSocketStream *stream, const QJsonValue &message, EngineHandler *handler)
{
    if (!handler || !stream)
        return;

    const QJsonObject &obj = message.toObject();
    const QString &cmd = obj.value(QStringLiteral("cmd")).toString();

    const auto &id = obj.value(QStringLiteral("engine")).toInt(-1);
    MultiPlayerEngine *engine = stream->engineGet<MultiPlayerEngine>(AbstractEngine::EngineMultiPlayer, id);

    LOG_CINFO("engine") << "HANDLE" << cmd << message;

    if (cmd == QStringLiteral("connect")) {
        const auto &eList = handler->engines();

        int eid = -1;

        for (const auto &ptr : eList) {
            if (ptr->type() == EngineMultiPlayer) {
                eid = ptr->id();
                break;
            }
        }

        if (eid == -1) {
            const auto &ptr = createEngine(stream, handler);
            if (!ptr.expired())
                eid = ptr.lock()->id();
        } else {
            auto engine = connectToEngine(eid, stream, handler);
        }

        LOG_CINFO("engine") << "CONNECTED" << eid;

        sendStreamJson(stream, QJsonObject{
                                   { QStringLiteral("cmd"), cmd },
                                   { QStringLiteral("engine"), eid }
                               });

    } else if (cmd == QStringLiteral("state")) {
        handler->engineTrigger(EngineMultiPlayer);
    } else {
        if (!engine) {
            sendStreamJson(stream, QJsonObject{
                                       { QStringLiteral("cmd"), cmd },
                                       { QStringLiteral("error"), QStringLiteral("invalid engine") }
                                   });
            return;
        }

        if (cmd == QStringLiteral("start")) {
            engine->startGame(stream);
        } else if (cmd == QStringLiteral("create")) {
            engine->createGame(stream, obj);
        } else if (cmd == QStringLiteral("enroll")) {
            int pid = engine->enrollPlayer(stream);
            if (pid == -1)
                sendStreamJson(stream, QJsonObject{
                                           { QStringLiteral("cmd"), cmd },
                                           { QStringLiteral("error"), QStringLiteral("enroll failed") }
                                       });
            else
                sendStreamJson(stream, QJsonObject{
                                           { QStringLiteral("cmd"), cmd },
                                           { QStringLiteral("engine"), engine->id() },
                                           { QStringLiteral("playerId"), pid }
                                       });
        } else if (cmd == QStringLiteral("prepare")) {
            engine->prepareGame(stream, obj);
        } else {
            sendStreamJson(stream, QJsonObject{
                                       { QStringLiteral("cmd"), cmd },
                                       { QStringLiteral("error"), QStringLiteral("invalid command") }
                                   });
        }
    }
}



/**
 * @brief MultiPlayerEngine::createEngine
 * @param stream
 * @param service
 * @return
 */

std::weak_ptr<MultiPlayerEngine> MultiPlayerEngine::createEngine(WebSocketStream *stream, EngineHandler *handler)
{
    if (!handler)
        return std::weak_ptr<MultiPlayerEngine>();

    LOG_CDEBUG("engine") << "Create MultiPlayerEngine" << stream;

    static int nextId = 0;

    auto ptr = std::make_shared<MultiPlayerEngine>(handler);

    ++nextId;

    ptr->setId(nextId);

    if (stream) {
        ptr->setHostStream(stream);
        LOG_CTRACE("engine") << "SET HOST" << ptr.get() << stream << ptr->m_id;
        ptr->setGameState(StateConnecting);
        handler->websocketEngineLink(stream, ptr);
        ptr->enrollPlayer(stream);
    }

    handler->engineAdd(ptr);

    return ptr;
}



/**
 * @brief MultiPlayerEngine::connectToEngine
 * @param stream
 * @return
 */

std::weak_ptr<AbstractEngine> MultiPlayerEngine::connectToEngine(const int &id, WebSocketStream *stream, EngineHandler *handler)
{
    if (!handler || !stream)
        return std::weak_ptr<AbstractEngine>();

    LOG_CDEBUG("engine") << "Connect to MultiPlayerEngine" << id;

    const auto &ptr = handler->engineGet(AbstractEngine::EngineMultiPlayer, id);

    if (!ptr.expired()) {
        LOG_CTRACE("engine") << "Engine exists" << id;
        handler->websocketEngineLink(stream, ptr.lock());
    }

    return ptr;
}



/**
 * @brief MultiPlayerEngine::createGame
 * @param stream
 */

void MultiPlayerEngine::startGame(WebSocketStream *stream)
{
    if (m_gameState != StateConnecting) {
        return sendStreamJson(stream, QJsonObject{
                                          { QStringLiteral("cmd"), QStringLiteral("start") },
                                          { QStringLiteral("error"), QStringLiteral("invalid state") }
                                      });
    }

    if (stream != m_hostStream) {
        return sendStreamJson(stream, QJsonObject{
                                          { QStringLiteral("cmd"), QStringLiteral("start") },
                                          { QStringLiteral("error"), QStringLiteral("permission denied") }
                                      });
    }

    setGameState(StateCreating);

    m_handler->engineTriggerEngine(this);
}



/**
 * @brief MultiPlayerEngine::createGame
 * @param stream
 */

void MultiPlayerEngine::createGame(WebSocketStream *stream, const QJsonObject &data)
{
    if (!stream)
        return;

    if (m_gameState != StateCreating) {
        return sendStreamJson(stream, QJsonObject{
                                          { QStringLiteral("cmd"), QStringLiteral("create") },
                                          { QStringLiteral("error"), QStringLiteral("invalid state") }
                                      });
    }

    if (stream != m_hostStream) {
        return sendStreamJson(stream, QJsonObject{
                                          { QStringLiteral("cmd"), QStringLiteral("create") },
                                          { QStringLiteral("error"), QStringLiteral("permission denied") }
                                      });
    }

    const QByteArray &baseData = QByteArray::fromBase64(data.value(QStringLiteral("entities")).toString().toUtf8());

    std::optional<ObjectStateSnapshot> snap = ObjectStateSnapshot::fromByteArray(qUncompress(baseData));

    if (!snap) {
        LOG_CWARNING("engine") << "Invalid binary message received";
        return sendStreamJson(stream, QJsonObject{
                                          { QStringLiteral("cmd"), QStringLiteral("create") },
                                          { QStringLiteral("error"), QStringLiteral("invalid data") }
                                      });
    }

    int entityId = 0;


    // Randomize player entities
    QVector<Player*> players;

    players.reserve(m_players.size());
    for (Player &p : m_players)
        players.append(&p);

    LOG_CERROR("engine") << "PLAYERS" << players;

    for (auto it=snap->list.begin(); it != snap->list.end(); ++it) {
        ObjectStateBase ptr = *it;
        ptr.tick = 0;
        ptr.state = ObjectStateBase::StateActive;
        ptr.id = ++entityId;


        Entity e;
        e.type = ptr.type;
        e.renderedStates.push_back(ptr);

        if (ptr.type == ObjectStateBase::TypePlayer) {
            if (players.isEmpty()) {
                LOG_CDEBUG("engine") << "Skip player entity" << ptr.id;
                continue;
            }

            Player *p = players.takeAt(QRandomGenerator::global()->bounded(players.size()));
            p->entityId = ptr.id;
            e.owner = p->username;
        } else {
            e.owner = QStringLiteral("");
        }

        LOG_CINFO("entine") << "++ ADD" << ptr.id << ptr.type << ptr.enemyState << ptr.position << ptr.size << ptr.facingLeft << ptr.subType;



        m_entities.insert({ptr.id, e});
    }


    // Check (?) -> state Error

    for (const auto &p : m_players) {
        LOG_CINFO("engine") << "Player" << p.id << "->" << p.entityId << "=" << p.stream;
    }

    setGameState(StatePreparing);

    m_handler->engineTriggerEngine(this);

}



/**
 * @brief MultiPlayerEngine::prepareGame
 * @param stream
 * @param data
 */

void MultiPlayerEngine::prepareGame(WebSocketStream *stream, const QJsonObject &message)
{
    if (m_gameState != StatePreparing) {
        return sendStreamJson(stream, QJsonObject{
                                          { QStringLiteral("cmd"), QStringLiteral("prepare") },
                                          { QStringLiteral("error"), QStringLiteral("invalid state") }
                                      });
    }

    if (message.value(QStringLiteral("ready")).toBool()) {
        playerPrepared(stream);
        return;
    }

    const QString &baseData = QString::fromUtf8(getStates().toBase64());

    return sendStreamJson(stream, QJsonObject{
                                      { QStringLiteral("cmd"), QStringLiteral("prepare") },
                                      { QStringLiteral("engine"), m_id },
                                      { QStringLiteral("entities"), baseData }
                                  });
}



/**
 * @brief MultiPlayerEngine::finishGame
 */

void MultiPlayerEngine::finishGame()
{
    LOG_CDEBUG("engine") << "Finish multiplayer game:" << m_id;

    setGameState(StateFinished);
}



/**
 * @brief MultiPlayerEngine::canDelete
 * @param useCount
 * @return
 */

bool MultiPlayerEngine::canDelete(const int &useCount)
{
    return (useCount == 1 && m_gameState == StateFinished);
}




/**
 * @brief MultiPlayerEngine::sendStreamJson
 * @param stream
 * @param value
 */

void MultiPlayerEngine::sendStreamJson(WebSocketStream *stream, const QJsonValue &value)
{
    if (!stream)
        return;

    stream->sendJson("multiplayer", value);
}



/**
 * @brief MultiPlayerEngine::streamDisconnectedEvent
 * @param stream
 */

void MultiPlayerEngine::streamUnlinkedEvent(WebSocketStream *stream)
{
    LOG_CTRACE("engine") << "MultiPlayerEngine stream disconnected:" << stream << (stream ? stream->credential().username() : "");

    disconnectPlayerStream(stream);

    if (m_hostStream == stream) {
        LOG_CTRACE("engine") << "Host stream disconnected";

        WebSocketStream *next = nullptr;

        for (auto &s : m_streams) {
            if (s == stream)
                continue;

            next = s;
            break;
        }

        if (!next) {
            LOG_CWARNING("engine") << "All stream dismissed";
            setHostStream(nullptr);
            finishGame();
        } else {
            LOG_CINFO("engine") << "Next host stream:" << next;
            setHostStream(next);
        }

        m_handler->engineTriggerEngine(this);
    }
}


/**
 * @brief MultiPlayerEngine::streamTriggerEvent
 * @param stream
 */

void MultiPlayerEngine::streamTriggerEvent(WebSocketStream *stream)
{
    LOG_CINFO("engine") << "Stream trigger" << this << stream << currentTick();

    QJsonObject ret;

    ret.insert(QStringLiteral("cmd"), QStringLiteral("state"));
    ret.insert(QStringLiteral("engine"), m_id);
    ret.insert(QStringLiteral("gameState"), m_gameState);
    ret.insert(QStringLiteral("interval"), m_service->mainTimerInterval());
    ret.insert(QStringLiteral("host"), (m_hostStream == stream ? true : false));

    if (m_startedAt != -1) {
        ret.insert(QStringLiteral("startedAt"), m_startedAt);
    }

    if (m_elapsedTimer.isValid()) {
        ret.insert(QStringLiteral("tick"), currentTick());
    }


    const auto &player = getPlayer(stream);

    if (player) {
        ret.insert(QStringLiteral("playerId"), player->id);
        ret.insert(QStringLiteral("playerEntityId"), player->entityId);
    }


    QJsonArray streams;

    int waitForPrepare = 0;

    for (const auto &p : m_players) {
        if (!p.prepared)
            ++waitForPrepare;

        streams.append(p.username);
    }

    if (waitForPrepare > 0)
        ret.insert(QStringLiteral("wait"), waitForPrepare);

    ret.insert(QStringLiteral("users"), streams);

    sendStreamJson(stream, ret);
}



/**
 * @brief MultiPlayerEngine::onBinaryMessageReceived
 * @param data
 */

void MultiPlayerEngine::onBinaryMessageReceived(const QByteArray &data, WebSocketStream *stream)
{
    LOG_CTRACE("engine") << "Binary message received" << data.size();

    if (m_gameState != StatePlaying) {
        LOG_CWARNING("engine") << "Binary message not accepted, state:" << m_gameState;
        return;
    }

    std::optional<ObjectStateSnapshot> snap = ObjectStateSnapshot::fromByteArray(qUncompress(data));

    if (!snap) {
        LOG_CWARNING("engine") << "Invalid binary message received from:"
                               << qPrintable(stream ? stream->credential().username() : QStringLiteral("?"));
        return;
    }

    LOG_CTRACE("engine") << "Sorting snap";

    ObjectStateBase prev;

    for (auto &s : snap->list) {
        EntityState estate;
        if (stream)
            estate.sender = stream->credential().username();

        if (prev.type == s.type && prev.id == s.id) {
            estate.state = prev;
            if (!estate.state.patch(s)) {
                LOG_CERROR("engine") << "Patch error" << s.type << s.id;
                estate.state = s;
                prev = s;
            } else {
                prev = estate.state;
            }
        } else {
            estate.state = s;
            prev = s;
        }

        const qint64 &tick = s.tick;

        /*if (m_lastSentState > 0 && s.tick < m_lastSentState) {
            LOG_CWARNING("engine") << "Received and skipped state from past:" << s.id << s.tick
                                   << qPrintable(stream ? stream->credential().username() : QStringLiteral("?"));
            continue;
        }*/

        estate.state.fields = ObjectStateBase::FieldAll;

        LOG_CINFO("engine") << "Add state to tick" << tick << "id:" <<
            estate.state.id << estate.state.enemyState << estate.state.position << estate.state.size << estate.state.subType;

        auto &v = m_states[tick];
        v.push_back(estate);
    }
}



/**
 * @brief MultiPlayerEngine::renderStates
 */

void MultiPlayerEngine::renderStates()
{
    LOG_CTRACE("engine") << "Rendering states";

    for (auto & [entityId, entity] : m_entities) {
        LOG_CTRACE("engine") << "Render......." << entityId << entity.type;

        if (entity.renderedStates.empty()) {
            LOG_CERROR("engine") << "Missing rendered state" << entityId << entity.type;
            continue;
        }

        for (const auto& [tick, esList] : m_states) {
            if (auto eIt = std::find_if(esList.cbegin(), esList.cend(),
                                        [&e = entityId](const auto &esPtr){ return (esPtr.id() == e); });
                eIt != esList.cend()) {

                entity.addRenderedState(this, *eIt);
            }
        }
    }

    m_states.clear();
}



/**
 * @brief MultiPlayerEngine::sendStates
 */

void MultiPlayerEngine::sendStates()
{
    LOG_CTRACE("engine") << "Sending states";

    const QByteArray &data = getStates();

    m_lastSentState = currentTick();

    for (auto ws : m_streams) {
        if (ws)
            ws->sendBinaryMessage(data);
    }
}



/**
 * @brief MultiPlayerEngine::getStates
 * @return
 */

QByteArray MultiPlayerEngine::getStates()
{
    renderStates();

    ObjectStateSnapshot snap;

    snap.list.reserve(m_entities.size());

    for (auto &[entityId, entity] : m_entities) {
        ObjectStateBase prev;
        for (const auto &rs: entity.renderedStates) {
            if (prev.type != ObjectStateBase::TypeInvalid) {
                auto s = prev.diff(rs);
                if (!s) {
                    LOG_CERROR("engine") << "State diff error" << entityId << entity.type << rs.tick;
                    continue;
                }
                s->id = entityId;
                snap.append(*s);
            } else {
                ObjectStateBase ptr = rs;
                ptr.id = entityId;
                snap.append(ptr);
            }
            prev = rs;
        }

        if (entity.renderedStates.size() > 1) {
            LOG_CTRACE("engine") << "   remove rendered states" << entityId << entity.type;
            entity.renderedStates.erase(entity.renderedStates.cbegin(), entity.renderedStates.cend()-1);
        }
    }

    return qCompress(snap.toByteArray());
}



/**
 * @brief MultiPlayerEngine::playGame
 */

void MultiPlayerEngine::playGame()
{
    if (m_gameState != StatePreparing)
        return;

    LOG_CDEBUG("engine") << "Play MultiPlayerGame" << m_id << this;

    m_startedAt = QDateTime::currentMSecsSinceEpoch();
    m_elapsedTimer.start();
    setGameState(StatePlaying);

    m_handler->engineTriggerEngine(this);

}



/**
 * @brief MultiPlayerEngine::enrollPlayer
 * @param stream
 * @return
 */

int MultiPlayerEngine::enrollPlayer(WebSocketStream *stream)
{
    if (!stream)
        return -1;

    if (m_playerLimit > 0 && m_players.size() >= m_playerLimit) {
        LOG_CWARNING("engine") << "Players number full";
        return -1;
    }

    int playerId = 1;

    for (const auto &p : m_players) {
        playerId = std::max(p.id+1, playerId);
    }

    const QString &username = stream->credential().username();

    LOG_CDEBUG("engine") << "Enroll player" << playerId << stream << qPrintable(username);

    m_players.push_back({playerId, username, stream});

    m_handler->engineTriggerEngine(this);

    return playerId;
}



/**
 * @brief MultiPlayerEngine::connectPlayerStream
 * @param playerId
 * @param stream
 */

bool MultiPlayerEngine::connectPlayerStream(const int &playerId, WebSocketStream *stream)
{
    for (Player &p : m_players) {
        if (p.id == playerId) {
            if (p.stream == nullptr) {
                p.stream = stream;
                return true;
            } else {
                LOG_CWARNING("engine") << "Can't connect player stream" << stream << "to id" << playerId;
                return false;
            }
        }
    }

    return false;
}


/**
 * @brief MultiPlayerEngine::disconnectPlayerStream
 * @param playerId
 * @param stream
 */

void MultiPlayerEngine::disconnectPlayerStream(const int &playerId, WebSocketStream *stream)
{
    for (auto &p : m_players) {
        if ((playerId == -1 && p.stream == stream) || (p.id == playerId))
            p.stream = nullptr;
    }
}



/**
 * @brief MultiPlayerEngine::getPlayer
 * @param stream
 * @return
 */

std::optional<MultiPlayerEngine::Player> MultiPlayerEngine::getPlayer(WebSocketStream *stream) const
{
    if (!stream)
        return std::nullopt;

    for (const auto &p : m_players) {
        if (p.stream == stream)
            return p;
    }

    return std::nullopt;
}



/**
 * @brief MultiPlayerEngine::playerPrepared
 * @param stream
 */

void MultiPlayerEngine::playerPrepared(WebSocketStream *stream)
{
    if (!stream)
        return;

    LOG_CDEBUG("engine") << "Player prepared" << stream << qPrintable(stream->credential().username());

    int w = 0;

    for (auto &p : m_players) {
        if (p.stream == stream)
            p.prepared = true;
        else if (!p.prepared)
            ++w;
    }

    if (w) {
        LOG_CTRACE("engine") << "Wait for" << w << "players to be prepared";
        return;
    }

    playGame();
}





/**
 * @brief MultiPlayerEngine::gameState
 * @return
 */

MultiPlayerGameState MultiPlayerEngine::gameState() const
{
    return m_gameState;
}

void MultiPlayerEngine::setGameState(MultiPlayerGameState newGameState)
{
    m_gameState = newGameState;
}






/**
 * @brief MultiPlayerEngine::hostStream
 * @return
 */

WebSocketStream *MultiPlayerEngine::hostStream() const
{
    return m_hostStream;
}

void MultiPlayerEngine::setHostStream(WebSocketStream *newHostStream)
{
    m_hostStream = newHostStream;
}



/**
 * @brief MultiPlayerEngine::timerTick
 */

void MultiPlayerEngine::timerTick()
{
    LOG_CTRACE("engine") << "Timer tick" << this;

    if (m_gameState != StatePlaying)
        return;

    QByteArray content;

    for (const auto & [entityId, entity] : m_entities) {
        content.append("=========================================\n");
        content.append(QString("%1 - %2\n").arg(entityId).arg(entity.type).toUtf8());
        content.append(entity.owner.toUtf8()).append("\n");
        content.append("=========================================\n");
        for (const auto &rs : entity.renderedStates) {
            content.append(rs.toReadable());
            content.append("---------------------------------\n");
        }
    }

    QFile f("/tmp/_state.txt");
    f.open(QIODevice::WriteOnly);
    f.write(content);
    f.close();

    sendStates();

    LOG_CTRACE("engine") << "States saved";
}



/**
 * @brief MultiPlayerEngine::currentTick
 * @return
 */

qint64 MultiPlayerEngine::currentTick() const
{
    if (m_elapsedTimer.isValid())
        return m_elapsedTimer.elapsed();
    else
        return 0;
}




/**
 * @brief MultiPlayerEngine::Entity::addRenderedState
 * @param state
 * @return
 */


bool MultiPlayerEngine::Entity::addRenderedState(MultiPlayerEngine *engine, const EntityState &state)
{
    Q_ASSERT(engine);

    ObjectStateBase old = renderedStates.back();

    if (state.state.tick < old.tick) {
        LOG_CWARNING("engine") << "Cached state older than last rendered state" << state.state.id << state.state.type << state.sender;
        return false;
    }


    /// Server-hosted entities

    if (owner.isEmpty()) {
        if (!engine->m_hostStream) {
            LOG_CWARNING("engine") << "Host stream missing";
            return false;
        }

        if (engine->m_hostStream->credential().username() != state.sender) {
            LOG_CWARNING("engine") << "Invalid state sender for server-hosted entity:" << state.state.id << state.state.type << state.sender;
            return false;
        }
    } else {
        if (owner != state.sender) {
            LOG_CWARNING("engine") << "Unauthorized state changes";
            return false;
        }
    }

    ObjectStateBase::Fields field = ObjectStateBase::FieldAll;
    field &= ~ObjectStateBase::FieldSubType;

    const bool success = old.patch(state.state, field);

    if (success) {
        LOG_CTRACE("engine") << "   - " << state.state.tick << state.sender << state.state.position << state.state.enemyState;
        renderedStates.push_back(old);
    } else
        LOG_CERROR("engine") << "   ! " << state.state.tick << state.sender << state.state.position << state.state.enemyState;

    return success;
}
