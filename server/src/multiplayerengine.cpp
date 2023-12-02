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

    LOG_CINFO("engine") << "HANDLE" << cmd << message;

    if (cmd == QStringLiteral("connect")) {
        const auto &eList = handler->engines();

        int eid = -1;

        if (eList.isEmpty()) {
            const auto &ptr = createEngine(stream, handler);
            if (!ptr.expired())
                eid = ptr.lock()->id();
        } else {
            const auto &ptr = eList.first();
            auto engine = connectToEngine(ptr->id(), stream, handler);
            if (!engine.expired())
                eid = engine.lock()->id();
        }

        LOG_CINFO("engine") << "CONNECTED" << eid;

        sendStreamJson(stream, QJsonObject{
                                   { QStringLiteral("cmd"), cmd },
                                   { QStringLiteral("engine"), eid }
                               });

    } else if (cmd == QStringLiteral("state")) {
        handler->engineTrigger(EngineMultiPlayer);
    } else if (cmd == QStringLiteral("start")) {
        const auto &id = message.toObject().value(QStringLiteral("engine")).toInt(-1);
        MultiPlayerEngine *engine = stream->engineGet<MultiPlayerEngine>(AbstractEngine::EngineMultiPlayer, id);
        if (engine) {
            engine->startGame(stream);
        } else {
            sendStreamJson(stream, QJsonObject{
                                       { QStringLiteral("cmd"), cmd },
                                       { QStringLiteral("error"), QStringLiteral("invalid engine") }
                                   });
        }

    } else if (cmd == QStringLiteral("create")) {
        const QJsonObject &obj = message.toObject();
        const auto &id = obj.value(QStringLiteral("engine")).toInt(-1);
        LOG_CTRACE("engine") << "GET ENGINE" << id;
        MultiPlayerEngine *engine = stream->engineGet<MultiPlayerEngine>(AbstractEngine::EngineMultiPlayer, id);
        if (engine) {
            engine->createGame(stream, obj);
        } else {
            sendStreamJson(stream, QJsonObject{
                                       { QStringLiteral("cmd"), cmd },
                                       { QStringLiteral("error"), QStringLiteral("invalid engine") }
                                   });
        }

    } else if (cmd == QStringLiteral("prepare")) {
        const QJsonObject &obj = message.toObject();
        const auto &id = obj.value(QStringLiteral("engine")).toInt(-1);
        MultiPlayerEngine *engine = stream->engineGet<MultiPlayerEngine>(AbstractEngine::EngineMultiPlayer, id);
        if (engine) {
            engine->prepareGame(stream);
        } else {
            sendStreamJson(stream, QJsonObject{
                                       { QStringLiteral("cmd"), cmd },
                                       { QStringLiteral("error"), QStringLiteral("invalid engine") }
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
        handler->websocketEngineLink(stream, ptr.lock());
        return ptr;
    } else {
        return createEngine(stream, handler);
    }
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

    if (m_gameState != StateCreating && m_gameState != StateConnecting) {
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

    // Start id from 1000
    int entityId = 1000;

    for (auto it=snap->list.begin(); it != snap->list.end(); ++it) {
        if (!it->get())
            continue;

        ObjectStateBase *ptr = it->get()->clone();
        ptr->tick = 0;
        ptr->state = ObjectStateBase::StateActive;


        auto &e = m_entities[++entityId];

        if (ObjectStateEnemySoldier *soldier = dynamic_cast<ObjectStateEnemySoldier*>(ptr); soldier != nullptr) {
            LOG_CINFO("entine") << "++ ADD" << entityId << soldier->enemyState << soldier->position << soldier->size << soldier->facingLeft;
        } else {
            LOG_CTRACE("entine") << "++ ADD" << entityId << ptr->position;
        }

        e.owner = QStringLiteral("");
        e.type = ptr->type;
        e.renderedState.reset(ptr);
    }


    // PLAYERS?

    setGameState(StatePreparing);

    m_handler->engineTriggerEngine(this);

}



/**
 * @brief MultiPlayerEngine::prepareGame
 * @param stream
 * @param data
 */

void MultiPlayerEngine::prepareGame(WebSocketStream *stream)
{
    if (m_gameState != StatePreparing) {
        return sendStreamJson(stream, QJsonObject{
                                          { QStringLiteral("cmd"), QStringLiteral("prepare") },
                                          { QStringLiteral("error"), QStringLiteral("invalid state") }
                                      });
    }

    const QString &baseData = QString::fromUtf8(getStates().toBase64());

    if (m_hostStream == stream)
        QTimer::singleShot(5000, this, &MultiPlayerEngine::playGame);

    return sendStreamJson(stream, QJsonObject{
                                      { QStringLiteral("cmd"), QStringLiteral("prepare") },
                                      { QStringLiteral("engine"), m_id },
                                      { QStringLiteral("entities"), baseData }
                                  });
}



/**
 * @brief MultiPlayerEngine::canDelete
 * @param useCount
 * @return
 */

bool MultiPlayerEngine::canDelete(const int &useCount)
{
    LOG_CTRACE("engine") << "MultiPlayer use" << useCount << m_t;
    if (useCount == 1)
        ++m_t;

    if (m_t > 2)
        return true;

    return false;
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

    if (m_hostStream == stream) {
        LOG_CTRACE("engine") << "HOST STREAM DISCONNECTED";

        WebSocketStream *next = nullptr;

        for (auto &s : m_streams) {
            if (s == stream)
                continue;

            next = s;
            break;
        }

        if (!next) {
            LOG_CWARNING("engine") << "Host stream dismissed";
            setHostStream(nullptr);
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

    QJsonArray streams;

    for (const auto &s : m_streams) {
        if (!s) continue;
        streams.append(s->credential().username());
    }

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
        LOG_CWARNING("engine") << "Binary message not accepted";
        return;
    }

    std::optional<ObjectStateSnapshot> snap = ObjectStateSnapshot::fromByteArray(qUncompress(data));

    if (!snap) {
        LOG_CWARNING("engine") << "Invalid binary message received";
        return;
    }

    LOG_CTRACE("engine") << "Sorting snap";

    LOG_CTRACE("engine") << snap->toReadable();

    for (auto &s : snap->list) {
        if (!s)
            continue;

        const qint64 &tick = s->tick;
        std::unique_ptr<EntityState> estate(new EntityState(s, stream ? stream->credential().username() : QStringLiteral("")));


        if (ObjectStateEnemySoldier *soldier = dynamic_cast<ObjectStateEnemySoldier*>(estate->state.get()); soldier != nullptr) {
            LOG_CINFO("engine") << "Add state to tick" << tick << soldier->id << soldier->enemyState << soldier->position << soldier->size << soldier->facingLeft;
        }

        auto &v = m_states[tick];
        v.push_back(std::move(estate));
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

        EntityState statePtr(entity.renderedState, entity.owner);

        for (const auto& [tick, esList] : m_states) {
            if (auto eIt = std::find_if(esList.cbegin(), esList.cend(),
                                        [&e = entityId](const auto &esPtr){ return (esPtr && esPtr->id() == e); });
                eIt != esList.cend()) {

                updateState(&statePtr, eIt->get());

                LOG_CTRACE("engine") << "   - " << tick;
            }
        }

        LOG_CTRACE("engine") << "   ===" << statePtr.sender << statePtr.state->id << statePtr.state->state << statePtr.state->position;

        entity.renderedState.reset(statePtr.state->clone());
    }

    m_lastRenderedState = currentTick();

    m_states.clear();
}



/**
 * @brief MultiPlayerEngine::sendStates
 */

void MultiPlayerEngine::sendStates()
{
    LOG_CTRACE("engine") << "Sending states";

    const QByteArray &data = getStates();

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
    QByteArray data;

    std::vector<ObjectStateBase*> snap;

    snap.reserve(m_entities.size());

    for (auto & [entityId, entity] : m_entities) {
        ObjectStateBase *ptr = entity.renderedState.get();
        ptr->id = entityId;
        snap.push_back(ptr);
    }

    data = qCompress(ObjectStateSnapshot::toByteArray(snap));

    return data;
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
 * @brief MultiPlayerEngine::updateState
 * @param dest
 * @param from
 */

void MultiPlayerEngine::updateState(EntityState *dest, const EntityState &from)
{
    if (!dest)
        return;

    if (ObjectStateEnemySoldier *soldier = dynamic_cast<ObjectStateEnemySoldier*>(from.state.get()); soldier != nullptr) {
        LOG_CTRACE("engine") << "UPDATE STATE" << dest << from.id() << from.state->tick << soldier->position << soldier->enemyState << soldier->size << soldier->facingLeft;
    } else {
        LOG_CTRACE("engine") << "UPDATE STATE" << dest << from.id() << from.state->tick << from.state->position;
    }

    dest->sender = from.sender;
    dest->state.reset(from.state->clone());
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

    renderStates();
    sendStates();

    QByteArray content;

    for (auto & [entityId, entity] : m_entities) {
        content.append("=========================================\n");
        content.append(QString("%1 - %2\n").arg(entityId).arg(entity.type).toUtf8());
        content.append(entity.owner.toUtf8()).append("\n");
        content.append("=========================================\n");

        entity.renderedState->toReadable(&content);
        content.append("---------------------------------\n");
    }

    QFile f("/tmp/_state.txt");
    f.open(QIODevice::WriteOnly);
    f.write(content);
    f.close();

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

