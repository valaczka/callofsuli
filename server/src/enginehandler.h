#ifndef ENGINEHANDLER_H
#define ENGINEHANDLER_H

#include "abstractengine.h"
#include "qmutex.h"
#include "qthread.h"
#include <QObject>
#include "websocketstream.h"

class ServerService;
class EngineHandler;

/**
 * @brief The EngineHandlerPrivate class
 */

class EngineHandlerPrivate : public QObject
{
	Q_OBJECT

public:
	explicit EngineHandlerPrivate(EngineHandler *handler);
	virtual ~EngineHandlerPrivate();

private:
	const QVector<std::shared_ptr<AbstractEngine> > &engines() const { return m_engines; }
	void engineAdd(const std::shared_ptr<AbstractEngine> &engine);
	void engineRemove(const std::shared_ptr<AbstractEngine> &engine);
	void engineRemove(AbstractEngine *engine);

	void engineAddStream(WebSocketStream *stream, const std::shared_ptr<AbstractEngine> &engine);
	void engineRemoveStream(WebSocketStream *stream, AbstractEngine *engine);

	void engineTrigger(const AbstractEngine::Type &type);
	void engineTriggerId(const AbstractEngine::Type &type, const int &id);
	void engineTriggerEngine(AbstractEngine *engine);


	void websocketAdd(QWebSocket *socket);
	void websocketRemove(WebSocketStream *stream);
	void websocketCloseAll();
	void websocketDisconnected(WebSocketStream *stream);
	void websocketTrigger(WebSocketStream *stream);
	void websocketObserverAdded(WebSocketStream *stream, const AbstractEngine::Type &type);
	void websocketObserverRemoved(WebSocketStream *stream, const AbstractEngine::Type &type);
	void websocketEngineLink(WebSocketStream *stream, const std::shared_ptr<AbstractEngine> &engine);
	void websocketEngineUnlink(WebSocketStream *stream, AbstractEngine *engine);

	void timerEventRun();
	void timerMinuteEventRun();

	void onBinaryDataReceived(WebSocketStream *stream, const QByteArray &data);


private:
	EngineHandler *q = nullptr;
	QRecursiveMutex m_mutex;

	QVector<std::shared_ptr<AbstractEngine>> m_engines;
	std::vector<std::unique_ptr<WebSocketStream>> m_streams;


	friend class EngineHandler;
};





/**
 * @brief The EngineHandler class
 */

class EngineHandler
{
public:
	explicit EngineHandler(ServerService *service);
	virtual ~EngineHandler();

	const QVector<std::shared_ptr<AbstractEngine> > &engines() const { return d->m_engines; }

	bool running() const;
	void setRunning(bool newRunning);

	template<typename T>
	T *engineGet(const AbstractEngine::Type &type, const int &id);

	template<typename T>
	QList<T*> engineGet(const AbstractEngine::Type &type);

	std::weak_ptr<AbstractEngine> engineGet(const AbstractEngine::Type &type, const int &id);


	void engineAdd(const std::shared_ptr<AbstractEngine> &engine) {
		if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::engineAdd, d, engine), Qt::QueuedConnection);
	}

	void engineTriggerEngine(AbstractEngine *engine) {
		if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::engineTriggerEngine, d, engine), Qt::QueuedConnection);
	}

	void engineTrigger(const AbstractEngine::Type &type) {
		if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::engineTrigger, d, type), Qt::QueuedConnection);
	}

	void engineTrigger(const AbstractEngine::Type &type, const int &id) {
		if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::engineTriggerId, d, type, id), Qt::QueuedConnection);
	}


	void websocketAdd(QWebSocket *socket) {
		if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::websocketAdd, d, socket), Qt::QueuedConnection);
	}

	void websocketCloseAll() {
		if (m_running) QMetaObject::invokeMethod(d, &EngineHandlerPrivate::websocketCloseAll, Qt::QueuedConnection);
	}

	void websocketDisconnected(WebSocketStream *stream) {
		if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::websocketDisconnected, d, stream), Qt::QueuedConnection);
	}

	void websocketTrigger(WebSocketStream *stream) {
		if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::websocketTrigger, d, stream), Qt::QueuedConnection);
	}

	void websocketObserverAdded(WebSocketStream *stream, const AbstractEngine::Type &type) {
		if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::websocketObserverAdded, d, stream, type), Qt::QueuedConnection);
	}

	void websocketObserverRemoved(WebSocketStream *stream, const AbstractEngine::Type &type) {
		if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::websocketObserverRemoved, d, stream, type), Qt::QueuedConnection);
	}

	void websocketEngineLink(WebSocketStream *stream, const std::shared_ptr<AbstractEngine> &engine) {
		if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::websocketEngineLink, d, stream, engine), Qt::QueuedConnection);
	}

	void websocketEngineUnlink(WebSocketStream *stream, AbstractEngine *engine) {
		if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::websocketEngineUnlink, d, stream, engine), Qt::QueuedConnection);
	}


	void timerEvent() {
		if (m_running) QMetaObject::invokeMethod(d, &EngineHandlerPrivate::timerEventRun, Qt::QueuedConnection);
	}

	void timerMinuteEvent() {
		if (m_running) QMetaObject::invokeMethod(d, &EngineHandlerPrivate::timerMinuteEventRun, Qt::QueuedConnection);
	}


private:
	void initEngines();

	ServerService *const m_service;
	EngineHandlerPrivate *d = nullptr;
	QThread m_dThread;
	bool m_running = true;

	friend class EngineHandlerPrivate;
	friend class AbstractEngine;
	friend class WebSocketStream;
};





/**
 * @brief EngineHandler::engineGet
 * @param type
 * @param id
 * @return
 */

template<typename T>
T *EngineHandler::engineGet(const AbstractEngine::Type &type, const int &id)
{
	for (const auto &e : d->m_engines) {
		if (e && e->type() == type && e->id() == id) {
			return qobject_cast<T*>(e.get());
		}
	}
	return nullptr;
}



/**
 * @brief EngineHandler::engineGet
 * @param type
 * @return
 */

template<typename T>
QList<T*> EngineHandler::engineGet(const AbstractEngine::Type &type)
{
	QList<T*> list;

	for (const auto &e : d->m_engines) {
		if (e && e->type() == type) {
			auto t = qobject_cast<T*>(e.get());
			if (t)
				list.append(t);
		}
	}
	return list;
}


#endif // ENGINEHANDLER_H
