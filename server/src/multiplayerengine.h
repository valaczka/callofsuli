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
	explicit MultiPlayerEngine(ServerService *service, QObject *parent = nullptr);
	virtual ~MultiPlayerEngine();

	enum GameState {
		StateInvalid = 0,
		StateConnecting,
		StateCreating,
		StatePreparing,
		StatePlaying,
		StateFinished
	};

	Q_ENUM(GameState);

	static QVector<std::shared_ptr<AbstractEngine> >::const_iterator find(const QVector<std::shared_ptr<AbstractEngine> > &list, const int &id);
	static void handleWebSocketTrigger(const QVector<WebSocketStream *> &list, ServerService *service, const int &engineId);
	static void handleWebSocketMessage(WebSocketStream *stream, const QJsonValue &message, ServerService *service);

	virtual bool canDelete(const int &useCount) override;

	int id() const;
	void setId(int newId);

	const GameState &gameState() const;
	void setGameState(const GameState &newGameState);

	WebSocketStream *hostStream() const;
	void setHostStream(WebSocketStream *newHostStream);

	virtual void timerTick() override;

protected:
	static void sendStreamJson(WebSocketStream *stream, const QJsonValue &value);
	virtual void streamConnectedEvent(WebSocketStream *stream) override;
	virtual void streamDisconnectedEvent(WebSocketStream *stream) override;
	virtual void streamTriggerEvent(WebSocketStream *stream) override;

private:
	void onBinaryMessageReceived(const QByteArray &data, WebSocketStream *sender);
	void renderStates();
	void sendStates();


	/**
	 * @brief The Entity class
	 */

	struct Entity {
		Entity() {}
		/*Entity(const ObjectStateBase::ObjectType &_type, const QString &_owner, std::unique_ptr<ObjectStateBase> &ptr)
			: type(_type)
			, owner(_owner)
			, renderedState(ptr.release())
		{}
		Entity(const ObjectStateBase::ObjectType &_type, const QString &_owner, ObjectStateBase *state)
			: type(_type)
			, owner(_owner)
			, renderedState(state)
		{}*/
		virtual ~Entity() = default;

		ObjectStateBase::ObjectType type = ObjectStateBase::TypeInvalid;
		QString owner;
		std::unique_ptr<ObjectStateBase> renderedState;
	};

	/**
	 * @brief The EntityState class
	 */

	struct EntityState {
		EntityState() {}
		EntityState(std::unique_ptr<ObjectStateBase> &ptr, const QString &_sender)
			: sender(_sender)
		{
			if (ptr) {
				std::unique_ptr<ObjectStateBase> p(ptr->clone());
				state = std::move(p);
			}
		}

		virtual ~EntityState() = default;

		QString sender;
		std::unique_ptr<ObjectStateBase> state;

		qint64 id() const { return state ? state->id : -1; }
	};


	void updateState(EntityState *dest, const EntityState &from);
	void updateState(EntityState *dest, EntityState *from) {
		if (!from || !dest)
			return;
		updateState(dest, *from);
	}



	int m_id = 1;
	GameState m_gameState = StateInvalid;
	WebSocketStream *m_hostStream = nullptr;

	std::map<qint64, Entity> m_entities;
	std::map<qint64, std::vector<std::unique_ptr<EntityState>>> m_states;

	qint64 m_lastRenderedState = 0;
	qint64 m_lastCachedState = 0;
	qint64 m_lastSentState = 0;


	QHash<WebSocketStream*, QMetaObject::Connection> m_signalHelper;
	int m_t = 0;			/// tmp
};


#endif // MULTIPLAYERENGINE_H
