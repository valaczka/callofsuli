#ifndef ABSTRACTENGINE_H
#define ABSTRACTENGINE_H

#include "qlambdathreadworker.h"
#include "qmutex.h"
#include <QJsonValue>
#include <QObject>

class WebSocketStream;
class ServerService;

class AbstractEngine : public QObject
{
	Q_OBJECT

public:
	enum Type {
		EngineInvalid = 0,
		EngineMultiPlayer
	};

	Q_ENUM(Type);

	explicit AbstractEngine(const Type &type, ServerService *service, QObject *parent = nullptr);
	explicit AbstractEngine(ServerService *service, QObject *parent = nullptr) : AbstractEngine(EngineInvalid, service, parent) {}
	virtual ~AbstractEngine();

	const Type &type() const { return m_type; }
	void streamSet(WebSocketStream *stream);
	void streamUnSet(WebSocketStream *stream);

	virtual bool canDelete(const int &useCount);
	virtual bool canConnect() const { return m_connectionLimit == 0 || m_connectionLimit > m_streams.size(); }

	const QString &owner() const;
	void setOwner(const QString &newOwner);

	int connectionLimit() const;
	void setConnectionLimit(int newConnectionLimit);

	const QVector<WebSocketStream *> &streams() const;
	void triggerAll();
	void trigger(WebSocketStream *stream);

	virtual void timerTick() {}

signals:

protected:
	virtual void streamConnectedEvent(WebSocketStream *stream) { Q_UNUSED(stream); }
	virtual void streamDisconnectedEvent(WebSocketStream *stream) { Q_UNUSED(stream); }
	virtual void streamTriggerEvent(WebSocketStream *stream) { Q_UNUSED(stream); }

	ServerService *const m_service;
	const Type m_type = EngineInvalid;
	QVector<WebSocketStream*> m_streams;
	QString m_owner;
	int m_connectionLimit = 0;

	QLambdaThreadWorker m_worker;
	QRecursiveMutex m_mutex;

};

#endif // ABSTRACTENGINE_H
