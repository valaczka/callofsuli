#ifndef MULTIPLAYERENGINE_H
#define MULTIPLAYERENGINE_H

#include "abstractengine.h"
#include "objectstate.h"
//#include "websocketstream.h"

class ServerService;


/**
 * @brief The MultiPlayerEngine class
 */

class MultiPlayerEngine : public AbstractEngine
{
    Q_OBJECT

public:
    explicit MultiPlayerEngine(EngineHandler *handler, QObject *parent = nullptr);
    virtual ~MultiPlayerEngine();

    static void handleWebSocketMessage(WebSocketStream *stream, const QJsonValue &message, EngineHandler *handler);

    static std::weak_ptr<MultiPlayerEngine> createEngine(WebSocketStream *stream, EngineHandler *handler);
    static std::weak_ptr<AbstractEngine> connectToEngine(const int &id, WebSocketStream *stream, EngineHandler *handler);

    void startGame(WebSocketStream *stream);
    void createGame(WebSocketStream *stream, const QJsonObject &data);
    void prepareGame(WebSocketStream *stream);

    virtual bool canDelete(const int &useCount) override;

    WebSocketStream *hostStream() const;
    void setHostStream(WebSocketStream *newHostStream);

    virtual void timerTick() override;

    qint64 currentTick() const;

    MultiPlayerGameState gameState() const;
    void setGameState(MultiPlayerGameState newGameState);

    virtual void streamTriggerEvent(WebSocketStream *stream) override;

protected:
    static void sendStreamJson(WebSocketStream *stream, const QJsonValue &value);
    //virtual void streamLinkedEvent(WebSocketStream *stream) override;
    virtual void streamUnlinkedEvent(WebSocketStream *stream) override;
    virtual void onBinaryMessageReceived(const QByteArray &data, WebSocketStream *stream) override;

private:
    void renderStates();
    void sendStates();
    QByteArray getStates();
    void playGame();


    /**
     * @brief The Entity class
     */

    struct Entity {
        Entity() {}
        virtual ~Entity() = default;

        ObjectStateBase::ObjectType type = ObjectStateBase::TypeInvalid;
        QString owner;
        std::vector<ObjectStateBase> renderedStates;
    };

    /**
     * @brief The EntityState class
     */

    struct EntityState {
        EntityState() {}
        EntityState(const ObjectStateBase &_state, const QString &_sender)
            : sender(_sender)
            , state(_state)
        {  }

        virtual ~EntityState() = default;

        QString sender;
        ObjectStateBase state;

        qint64 id() const { return state.id; }

        bool updateFrom(const ObjectStateBase &state, const QString &sender);
        bool updateFrom(const EntityState &state) { return updateFrom(state.state, state.sender); }
    };



    MultiPlayerGameState m_gameState = StateInvalid;
    WebSocketStream *m_hostStream = nullptr;
    QElapsedTimer m_elapsedTimer;
    qint64 m_startedAt = -1;

    std::map<qint64, Entity> m_entities;
    std::map<qint64, std::vector<EntityState>> m_states;

    qint64 m_lastSentState = 0;


    int m_t = 0;			/// tmp
};


#endif // MULTIPLAYERENGINE_H
