#ifndef ENGINEHANDLER_H
#define ENGINEHANDLER_H

#include "abstractengine.h"
#include "qthread.h"
#include <QObject>
#include "websocketstream.h"

class ServerService;
class EngineHandlerPrivate;
class UdpServerPeer;



/**
 * @brief The EngineHandler class
 */

class EngineHandler
{
public:
	explicit EngineHandler(ServerService *service);
	virtual ~EngineHandler();

	const QVector<std::shared_ptr<AbstractEngine> > &engines() const;

	ServerService *service() const { return m_service; }

	bool running() const;
	void setRunning(bool newRunning);

	template<typename T>
	T *engineGet(const AbstractEngine::Type &type, const int &id);

	template<typename T>
	QList<T*> engineGet(const AbstractEngine::Type &type);

	std::weak_ptr<AbstractEngine> engineGet(const AbstractEngine::Type &type, const int &id);

	void engineAdd(const std::shared_ptr<AbstractEngine> &engine);
	void engineRemove(AbstractEngine *engine);
	void engineTriggerEngine(AbstractEngine *engine);
	void engineTrigger(const AbstractEngine::Type &type);
	void engineTrigger(const AbstractEngine::Type &type, const int &id);
	void engineRemoveUnused();

	void websocketAdd(QWebSocket *socket);
	void websocketCloseAll();
	void websocketDisconnected(WebSocketStream *stream);
	void websocketTrigger(WebSocketStream *stream);
	void websocketObserverAdded(WebSocketStream *stream, const AbstractEngine::Type &type);
	void websocketObserverRemoved(WebSocketStream *stream, const AbstractEngine::Type &type);
	void websocketEngineLink(WebSocketStream *stream, const std::shared_ptr<AbstractEngine> &engine);
	void websocketEngineUnlink(WebSocketStream *stream, AbstractEngine *engine);

	void udpDataReceived(UdpServerPeer *peer, QByteArray data);

	void timerEvent();
	void timerMinuteEvent();


private:
	[[deprecated]] void initEngines();

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
	for (const auto &e : engines()) {
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

	for (const auto &e : engines()) {
		if (e && e->type() == type) {
			auto t = qobject_cast<T*>(e.get());
			if (t)
				list.append(t);
		}
	}
	return list;
}


#endif // ENGINEHANDLER_H
