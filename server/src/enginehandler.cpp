#include "enginehandler.h"
#include "enginehandler_p.h"
#include "Logger.h"
#include "peerengine.h"
#include "udpserver.h"



#define ENGINE_HANDLER_FPS		120


/**
 * @brief EngineHandler::EngineHandler
 * @param service
 */

EngineHandler::EngineHandler(ServerService *service)
	: m_service(service)
{
	LOG_CTRACE("service") << "EngineHandler created";

	d = new EngineHandlerPrivate(this);
	d->moveToThread(&m_dThread);
	QObject::connect(&m_dThread, &QThread::finished, d, &QObject::deleteLater);
	m_dThread.start();

	initEngines();
}


/**
 * @brief EngineHandler::~EngineHandler
 */

EngineHandler::~EngineHandler()
{
	d = nullptr;
	m_dThread.quit();
	m_dThread.wait();

	LOG_CTRACE("service") << "EngineHandler destroyed";
}



/**
 * @brief EngineHandler::running
 * @return
 */

bool EngineHandler::running() const
{
	return m_running;
}

void EngineHandler::setRunning(bool newRunning)
{
	m_running = newRunning;
}



/**
 * @brief EngineHandler::engineGet
 * @param type
 * @param id
 * @return
 */

std::weak_ptr<AbstractEngine> EngineHandler::engineGet(const AbstractEngine::Type &type, const int &id)
{
	for (const auto &e : d->m_engines) {
		if (e && e->type() == type && e->id() == id) {
			return e;
		}
	}
	return std::weak_ptr<AbstractEngine>();
}




/**
 * Queued private functions
 */

const QVector<std::shared_ptr<AbstractEngine> > &EngineHandler::engines() const { return d->m_engines; }

void EngineHandler::engineAdd(const std::shared_ptr<AbstractEngine> &engine) {
	if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::engineAdd, d, engine), Qt::QueuedConnection);
}

void EngineHandler::engineRemove(AbstractEngine *engine)
{
	if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::engineRemove, d, engine), Qt::QueuedConnection);
}

void EngineHandler::engineTriggerEngine(AbstractEngine *engine) {
	if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::engineTriggerEngine, d, engine), Qt::QueuedConnection);
}

void EngineHandler::engineTrigger(const AbstractEngine::Type &type) {
	if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::engineTrigger, d, type), Qt::QueuedConnection);
}

void EngineHandler::engineTrigger(const AbstractEngine::Type &type, const int &id) {
	if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::engineTriggerId, d, type, id), Qt::QueuedConnection);
}

void EngineHandler::engineRemoveUnused() {
	if (m_running) QMetaObject::invokeMethod(d, &EngineHandlerPrivate::engineRemoveUnused, Qt::QueuedConnection);
}

void EngineHandler::websocketAdd(QWebSocket *socket) {
	if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::websocketAdd, d, socket), Qt::QueuedConnection);
}

void EngineHandler::websocketCloseAll() {
	if (m_running) QMetaObject::invokeMethod(d, &EngineHandlerPrivate::websocketCloseAll, Qt::QueuedConnection);
}

void EngineHandler::websocketDisconnected(WebSocketStream *stream) {
	if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::websocketDisconnected, d, stream), Qt::QueuedConnection);
}

void EngineHandler::websocketTrigger(WebSocketStream *stream) {
	if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::websocketTrigger, d, stream), Qt::QueuedConnection);
}

void EngineHandler::websocketObserverAdded(WebSocketStream *stream, const AbstractEngine::Type &type) {
	if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::websocketObserverAdded, d, stream, type), Qt::QueuedConnection);
}

void EngineHandler::websocketObserverRemoved(WebSocketStream *stream, const AbstractEngine::Type &type) {
	if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::websocketObserverRemoved, d, stream, type), Qt::QueuedConnection);
}

void EngineHandler::websocketEngineLink(WebSocketStream *stream, const std::shared_ptr<AbstractEngine> &engine) {
	if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::websocketEngineLink, d, stream, engine), Qt::QueuedConnection);
}

void EngineHandler::websocketEngineUnlink(WebSocketStream *stream, AbstractEngine *engine) {
	if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::websocketEngineUnlink, d, stream, engine), Qt::QueuedConnection);
}

void EngineHandler::udpDataReceived(UdpServerPeer *peer, QByteArray data)
{
	if (m_running) QMetaObject::invokeMethod(d, std::bind(&EngineHandlerPrivate::udpDataReceived, d, peer, data), Qt::QueuedConnection);
}





/**
 * @brief EngineHandler::initEngines
 */

void EngineHandler::initEngines()
{
	LOG_CTRACE("service") << "Init engines";

	auto ptr = std::make_shared<PeerEngine>(this);
	engineAdd(std::move(ptr));
}



/**
 * @brief EngineHandlerPrivate::engineAdd
 * @param engine
 */

EngineHandlerPrivate::EngineHandlerPrivate(EngineHandler *handler)
	: QObject{}
	, q(handler)
{
	startTimer(1000./ENGINE_HANDLER_FPS, Qt::PreciseTimer);
}



/**
 * @brief EngineHandlerPrivate::~EngineHandlerPrivate
 */

EngineHandlerPrivate::~EngineHandlerPrivate()
{
	q->m_running = false;
	q = nullptr;
}




/**
 * @brief EngineHandlerPrivate::engineAdd
 * @param engine
 */

void EngineHandlerPrivate::engineAdd(const std::shared_ptr<AbstractEngine> &engine)
{
	if (!engine)
		return;

	LOG_CTRACE("service") << "Add engine:" << engine.get()->type() << engine.get();

	QMutexLocker locker(&m_mutex);

	m_engines.append(std::move(engine));
}


/**
 * @brief EngineHandlerPrivate::engineRemove
 * @param engine
 */

/*void EngineHandlerPrivate::engineRemove(const std::shared_ptr<AbstractEngine> &engine)
{
	LOG_CTRACE("service") << "Remove engine:" << engine->type() << engine->id();

	QMutexLocker locker(&m_mutex);

	for (auto it = m_engines.begin(); it != m_engines.end(); ) {
		if (*it == engine) {
			if (!it->get()->m_engineMutex.tryLock(10000)) {
				LOG_CERROR("service") << "Unable to unlock mutex" << it->get();
				++it;
			} else {
				it->get()->m_engineMutex.unlock();
				it = m_engines.erase(it);
			}
		} else {
			++it;
		}
	}
}*/


/**
 * @brief EngineHandlerPrivate::engineRemove
 * @param engine
 */

void EngineHandlerPrivate::engineRemove(AbstractEngine *engine)
{
	if (!engine)
		return;

	LOG_CTRACE("service") << "Remove engine:" << engine->type() << engine;

	QMutexLocker locker(&m_mutex);

	for (auto it = m_engines.begin(); it != m_engines.end(); ) {
		if (it->get() == engine) {
			if (!it->get()->m_engineMutex.tryLock(10000)) {
				LOG_CERROR("service") << "Unable to unlock mutex" << it->get();
				++it;
			} else {
				it->get()->m_engineMutex.unlock();
				it = m_engines.erase(it);
			}
		} else {
			++it;
		}
	}
}


/**
 * @brief EngineHandlerPrivate::engineRemoveUnused
 */

void EngineHandlerPrivate::engineRemoveUnused()
{
	//LOG_CTRACE("service") << "Remove unused engines";

	QMutexLocker locker(&m_mutex);

	for (auto it = m_engines.begin(); it != m_engines.end(); ) {
		auto e = it->get();
		if (e->canDelete(it->use_count())) {
			if (!e->m_engineMutex.tryLock(10000)) {
				LOG_CERROR("service") << "Unable to unlock mutex" << e;
				++it;
			} else {
				e->m_engineMutex.unlock();
				LOG_CDEBUG("service") << "Remove unused engine" << e->type() << e->id();
				it = m_engines.erase(it);
			}
		} else {
			++it;
		}
	}
}



/**
 * @brief EngineHandlerPrivate::engineAdd
 * @param stream
 * @param engine
 */

void EngineHandlerPrivate::engineAddStream(WebSocketStream *stream, const std::shared_ptr<AbstractEngine> &engine)
{
	if (!stream) return;
	QMutexLocker locker(&m_mutex);
	stream->engineAdd(engine);
}


/**
 * @brief EngineHandlerPrivate::engineRemove
 * @param stream
 * @param engine
 */

void EngineHandlerPrivate::engineRemoveStream(WebSocketStream *stream, AbstractEngine *engine)
{
	if (!stream) return;
	QMutexLocker locker(&m_mutex);
	stream->engineRemove(engine);
}


/**
 * @brief EngineHandlerPrivate::engineTrigger
 * @param type
 */

void EngineHandlerPrivate::engineTrigger(const AbstractEngine::Type &type)
{
	LOG_CTRACE("service") << "Engine trigger" << type;

	QMutexLocker locker(&m_mutex);

	for (const auto &e : m_engines) {
		if (e->type() != type)
			continue;

		e->triggerEvent();
	}
}


/**
 * @brief EngineHandlerPrivate::engineTriggerId
 * @param type
 * @param id
 */

void EngineHandlerPrivate::engineTriggerId(const AbstractEngine::Type &type, const int &id)
{
	LOG_CTRACE("service") << "Engine trigger" << type << id;

	QMutexLocker locker(&m_mutex);

	for (const auto &e : m_engines) {
		if (e->type() != type || e->id() != id)
			continue;

		e->triggerEvent();
	}
}




/**
 * @brief EngineHandlerPrivate::engineTriggerEngine
 * @param engine
 */

void EngineHandlerPrivate::engineTriggerEngine(AbstractEngine *engine)
{
	QMutexLocker locker(&m_mutex);

	if (!engine)
		return;

	LOG_CTRACE("service") << "Engine trigger" << engine;


	engine->triggerEvent();
}



/**
 * @brief EngineHandlerPrivate::websocketAdd
 * @param socket
 */

void EngineHandlerPrivate::websocketAdd(QWebSocket *socket)
{
	LOG_CTRACE("service") << "Add WebSocket" << socket;

	QMutexLocker locker(&m_mutex);

	const auto &it = std::find_if(m_streams.begin(), m_streams.end(), [socket](const std::unique_ptr<WebSocketStream> &s) {
		return (s->m_socket.get() == socket);
	});

	if (it != m_streams.end()) {
		LOG_CERROR("service") << "WebSocketStream already exists" << socket;
		return;
	}

	m_streams.push_back(std::make_unique<WebSocketStream>(q, socket));

	auto &ws = m_streams.back();

	auto wsocket = ws->m_socket.get();

	connect(wsocket, &QWebSocket::disconnected, ws.get(), &WebSocketStream::onWebSocketDisconnected);
	connect(wsocket, &QWebSocket::textMessageReceived, ws.get(), &WebSocketStream::onTextReceived);
	connect(wsocket, &QWebSocket::binaryMessageReceived, ws.get(), &WebSocketStream::onBinaryDataReceived);
	connect(wsocket, &QWebSocket::binaryMessageReceived, this,
			std::bind(&EngineHandlerPrivate::onBinaryDataReceived, this, ws.get(), std::placeholders::_1));


	LOG_CTRACE("service") << "WebSocketStream added" << &ws;

	ws->sendHello();
}



/**
 * @brief EngineHandlerPrivate::websocketRemove
 * @param socket
 */

void EngineHandlerPrivate::websocketRemove(WebSocketStream *stream)
{
	if (!stream) return;

	LOG_CTRACE("service") << "Remove WebSocket" << stream;

	QMutexLocker locker(&m_mutex);

	for (auto it = m_streams.begin(); it != m_streams.end(); ) {
		if (auto ws = it->get(); ws == stream) {
			auto wsocket = ws->m_socket.get();
			/*disconnect(wsocket, &QWebSocket::disconnected, ws, &WebSocketStream::onWebSocketDisconnected);
			disconnect(wsocket, &QWebSocket::textMessageReceived, ws, &WebSocketStream::onTextReceived);
			disconnect(wsocket, &QWebSocket::binaryMessageReceived, ws, &WebSocketStream::onBinaryDataReceived);*/
			wsocket->disconnect(ws);
			wsocket->disconnect(this);

			it = m_streams.erase(it);
		} else
			++it;
	}
}



/**
 * @brief EngineHandlerPrivate::websocketCloseAll
 */

void EngineHandlerPrivate::websocketCloseAll()
{
	LOG_CTRACE("service") << "Close all WebSocket";

	QMutexLocker locker(&m_mutex);

	for (auto it = m_streams.begin(); it != m_streams.end(); ) {
		auto ws = it->get();

		for (const auto &e : m_engines)
			e.get()->streamUnSet(ws);

		auto wsocket = ws->m_socket.get();

		LOG_CTRACE("service") << "Close WebSocket" << wsocket;

		/*disconnect(wsocket, &QWebSocket::disconnected, ws, &WebSocketStream::onWebSocketDisconnected);
		disconnect(wsocket, &QWebSocket::textMessageReceived, ws, &WebSocketStream::onTextReceived);
		disconnect(wsocket, &QWebSocket::binaryMessageReceived, ws, &WebSocketStream::onBinaryDataReceived);*/

		wsocket->disconnect(ws);
		wsocket->disconnect(this);

		it = m_streams.erase(it);
	}
}



/**
 * @brief EngineHandlerPrivate::websocketDisconnected
 * @param stream
 */

void EngineHandlerPrivate::websocketDisconnected(WebSocketStream *stream)
{
	LOG_CTRACE("service") << "WebSocketStream disconnected" << stream;

	QMutexLocker locker(&m_mutex);

	for (const auto &e : m_engines)
		e.get()->streamUnSet(stream);


	websocketRemove(stream);
}


/**
 * @brief EngineHandlerPrivate::websocketTrigger
 * @param stream
 */

void EngineHandlerPrivate::websocketTrigger(WebSocketStream *stream)
{
	if (!stream)
		return;

	QMutexLocker locker(&m_mutex);

	for (const auto &e : stream->engines()) {
		e->triggerEvent();
	}
}



/**
 * @brief EngineHandlerPrivate::websocketObserverAdded
 * @param stream
 * @param type
 */

void EngineHandlerPrivate::websocketObserverAdded(WebSocketStream *stream, const AbstractEngine::Type &type)
{
	/*if (!stream)
		return;

	QMutexLocker locker(&m_mutex);

	for (const auto &e: m_engines) {
		if (e->type() == type) {
			stream->engineAdd(e);
		}
	}*/
}


/**
 * @brief EngineHandlerPrivate::websocketObserverRemoved
 * @param stream
 * @param type
 */

void EngineHandlerPrivate::websocketObserverRemoved(WebSocketStream *stream, const AbstractEngine::Type &type)
{
	if (!stream)
		return;

	QMutexLocker locker(&m_mutex);

	for (const auto &e: m_engines) {
		if (e->type() == type) {
			stream->engineRemove(e.get());
		}
	}
}


/**
 * @brief EngineHandlerPrivate::websocketEngineAdd
 * @param stream
 * @param engine
 */

void EngineHandlerPrivate::websocketEngineLink(WebSocketStream *stream, const std::shared_ptr<AbstractEngine> &engine)
{
	if (!stream)
		return;

	QMutexLocker locker(&m_mutex);

	stream->engineAdd(engine);
	engine->streamSet(stream);
}



/**
 * @brief EngineHandlerPrivate::websocketEngineUnlink
 * @param stream
 * @param engine
 */

void EngineHandlerPrivate::websocketEngineUnlink(WebSocketStream *stream, AbstractEngine *engine)
{
	if (!stream || !engine)
		return;

	QMutexLocker locker(&m_mutex);

	stream->engineRemove(engine);
	engine->streamUnSet(stream);
}



/**
 * @brief EngineHandlerPrivate::timerEvent
 * @param event
 */

void EngineHandlerPrivate::timerEvent(QTimerEvent */*event*/)
{
	if (!q->running())
		return;

	const QDateTime &current = QDateTime::currentDateTime();
	const QDateTime dtMinute(current.date(), QTime(current.time().hour(), current.time().minute()));

	timerEventRun();

	if (!m_timerLastMinute.isNull() && dtMinute <= m_timerLastMinute)
		return;

	m_timerLastMinute = dtMinute;

	timerMinuteEventRun();
}



/**
 * @brief EngineHandlerPrivate::timerEvent
 */

void EngineHandlerPrivate::timerEventRun()
{
	QMutexLocker locker(&m_mutex);

	for (const std::shared_ptr<AbstractEngine> &e : m_engines) {
		e->timerTick();
	}
}




/**
 * @brief EngineHandlerPrivate::timerMinuteEventRun
 */

void EngineHandlerPrivate::timerMinuteEventRun()
{
	QMutexLocker locker(&m_mutex);

	for (const std::shared_ptr<AbstractEngine> &e : m_engines) {
		e->timerMinuteTick();
	}

	engineRemoveUnused();
}




/**
 * @brief EngineHandlerPrivate::onBinaryDataReceived
 * @param data
 */

void EngineHandlerPrivate::onBinaryDataReceived(WebSocketStream *stream, const QByteArray &data)
{
	if (!stream) {
		LOG_CERROR("service") << "Invalid stream";
		return;
	}

	QMutexLocker locker(&m_mutex);

	for (const std::shared_ptr<AbstractEngine> &e : m_engines) {
		e->onBinaryMessageReceived(data, stream);
	}
}


/**
 * @brief EngineHandlerPrivate::udpDataReceived
 * @param peer
 * @param data
 * @param engine
 */

void EngineHandlerPrivate::udpDataReceived(UdpServerPeer *peer, const QByteArray &data)
{
	if (!peer) {
		LOG_CERROR("service") << "Invalid peer";
		return;
	}

	if (!peer->engine()) {
		LOG_CERROR("service") << "Invalid engine";
		return;
	}

	QMutexLocker locker(&m_mutex);

	peer->engine()->binaryDataReceived(peer, data);
}


