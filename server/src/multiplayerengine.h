#ifndef MULTIPLAYERENGINE_H
#define MULTIPLAYERENGINE_H

#include "abstractengine.h"
#include "qlambdathreadworker.h"

class ServerService;

class MultiPlayerEngine : public AbstractEngine
{
	Q_OBJECT

public:
	explicit MultiPlayerEngine(QObject *parent = nullptr);
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

protected:
	static void sendStreamJson(WebSocketStream *stream, const QJsonValue &value);
	virtual void streamConnectedEvent(WebSocketStream *stream) override;
	virtual void streamDisconnectedEvent(WebSocketStream *stream) override;
	virtual void streamTriggerEvent(WebSocketStream *stream) override;

private:
	struct OwnerMap {
		int64_t id;
		QString username;
		WebSocketStream *stream = nullptr;
	};


	QLambdaThreadWorker m_worker;
	QRecursiveMutex m_mutex;

	int m_id = 1;
	GameState m_gameState = StateInvalid;
	WebSocketStream *m_hostStream = nullptr;
	QVector<OwnerMap> m_ownerMap;

	int m_t = 0;
};

#endif // MULTIPLAYERENGINE_H
