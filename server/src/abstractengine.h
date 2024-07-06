#ifndef ABSTRACTENGINE_H
#define ABSTRACTENGINE_H

#include "qmutex.h"
#include <QJsonValue>
#include <QObject>

class WebSocketStream;
class ServerService;
class EngineHandler;

class AbstractEngine : public QObject
{
	Q_OBJECT

public:
	enum Type {
		EngineInvalid = 0,
		EnginePeer,
		EngineMultiPlayer,			// DEPRECATED
		EngineExam,
		EngineConquest
	};

	Q_ENUM(Type);

	explicit AbstractEngine(const Type &type, const int &id, EngineHandler *handler, QObject *parent = nullptr);
	explicit AbstractEngine(const Type &type, EngineHandler *handler, QObject *parent = nullptr)
		: AbstractEngine(type, 0, handler, parent) {}
	explicit AbstractEngine(EngineHandler *handler, QObject *parent = nullptr)
		: AbstractEngine(EngineInvalid, handler, parent) {}
	virtual ~AbstractEngine();

	const Type &type() const { return m_type; }

	virtual bool canDelete(const int &useCount);
	virtual bool canConnect() const { return m_connectionLimit == 0 || m_connectionLimit > m_streams.size(); }

	const QString &owner() const;
	void setOwner(const QString &newOwner);

	uint connectionLimit() const;
	void setConnectionLimit(uint newConnectionLimit);

	uint playerLimit() const;
	void setPlayerLimit(uint newPlayerLimit);

	const QVector<WebSocketStream *> &streams() const;

	int id() const;
	void setId(int newId);

	virtual void timerTick() {}
	virtual void timerMinuteTick() {}
	virtual void triggerEvent() { }

protected:
	virtual void streamLinkedEvent(WebSocketStream *stream) { Q_UNUSED(stream); }
	virtual void streamUnlinkedEvent(WebSocketStream *stream) { Q_UNUSED(stream); }
	virtual void onBinaryMessageReceived(const QByteArray &data, WebSocketStream *stream) { Q_UNUSED(data); Q_UNUSED(stream); }

	QRecursiveMutex m_engineMutex;
	QMutex m_playerMutex;

	EngineHandler *const m_handler;
	ServerService *const m_service;
	const Type m_type = EngineInvalid;
	QVector<WebSocketStream*> m_streams;
	QString m_owner;
	int m_id = 0;
	uint m_connectionLimit = 0;
	uint m_playerLimit = 0;

private:
	void streamSet(WebSocketStream *stream);
	void streamUnSet(WebSocketStream *stream);

	friend class EngineHandlerPrivate;
};

#endif // ABSTRACTENGINE_H
