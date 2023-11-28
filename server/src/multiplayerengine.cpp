#include "multiplayerengine.h"
#include "Logger.h"
#include "objectstate.h"
#include "serverservice.h"


/**
 * @brief MultiPlayerEngine::MultiPlayerEngine
 * @param parent
 */

MultiPlayerEngine::MultiPlayerEngine(ServerService *service, QObject *parent)
	: AbstractEngine{EngineMultiPlayer, service, parent}
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
 * @brief MultiPlayerEngine::find
 * @param service
 * @param id
 */

QVector<std::shared_ptr<AbstractEngine> >::const_iterator MultiPlayerEngine::find(const QVector<std::shared_ptr<AbstractEngine> > &list, const int &id)
{
	for (auto it = list.constBegin(); it != list.constEnd(); ++it) {
		if (it->get() && it->get()->type() == AbstractEngine::EngineMultiPlayer) {
			MultiPlayerEngine *ptr = qobject_cast<MultiPlayerEngine*>(it->get());
			if (ptr && ptr->id() == id)
				return it;
		}
	}

	return list.constEnd();
}


/**
 * @brief MultiPlayerEngine::handleWebSocketTrigger
 * @param list
 * @param service
 */

void MultiPlayerEngine::handleWebSocketTrigger(const QVector<WebSocketStream *> &list, ServerService *service, const int &engineId)
{
	Q_UNUSED(service);

	LOG_CDEBUG("engine") << "WebSocket trigger" << list << engineId;

	for (auto s : list) {
		if (!s)
			continue;

		for (const auto &e : s->engines()) {
			if (e && e->type() == AbstractEngine::EngineMultiPlayer)
				e->trigger(s);
		}
	}

}



/**
 * @brief MultiPlayerEngine::handleWebSocketMessage
 * @param stream
 * @param message
 * @param service
 */

void MultiPlayerEngine::handleWebSocketMessage(WebSocketStream *stream, const QJsonValue &message, ServerService *service)
{
	if (!service || !stream)
		return;

	const QJsonObject &obj = message.toObject();
	const QString &cmd = obj.value(QStringLiteral("cmd")).toString();

	LOG_CINFO("engine") << "HANDLE" << cmd;

	if (cmd == QStringLiteral("connect")) {
		static int nextId = 0;

		MultiPlayerEngine *myEngine = nullptr;

		const auto &eList = service->engines();
		auto ptr = find(eList, nextId);

		if (ptr != eList.constEnd()) {
			MultiPlayerEngine *engine = qobject_cast<MultiPlayerEngine*>(ptr->get());
			const auto &l = stream->engines();
			if (auto it = std::find_if(l.constBegin(), l.constEnd(),
									   [engine](const std::shared_ptr<AbstractEngine> &e) { return (e.get() == engine); });
					it == l.constEnd())
			{
				stream->engineAdd(*ptr);
				(*ptr)->streamSet(stream);
			}
			myEngine = engine;

		} else {
			auto ptr = std::make_shared<MultiPlayerEngine>(service);

			ptr->setId(++nextId);
			ptr->setHostStream(stream);

			myEngine = ptr.get();
			stream->engineAdd(ptr);
			ptr->streamSet(stream);
			service->engineAdd(std::move(ptr));
		}

		LOG_CINFO("engine") << "CONNECTED" << myEngine << (myEngine ? myEngine->id() : -1);

		sendStreamJson(stream, QJsonObject{
						   { QStringLiteral("cmd"), cmd },
						   { QStringLiteral("engine"), (myEngine ? myEngine->id() : -1) }
					   });

	} else if (cmd == QStringLiteral("state")) {
		for (auto &e : stream->engines()) {
			if (e && e->type() == EngineMultiPlayer)
				e->trigger(stream);
		}
	}
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
 * @brief MultiPlayerEngine::id
 * @return
 */


int MultiPlayerEngine::id() const
{
	return m_id;
}

void MultiPlayerEngine::setId(int newId)
{
	m_id = newId;
}

const MultiPlayerEngine::GameState &MultiPlayerEngine::gameState() const
{
	return m_gameState;
}

void MultiPlayerEngine::setGameState(const GameState &newGameState)
{
	m_gameState = newGameState;
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
 * @brief MultiPlayerEngine::streamConnectedEvent
 * @param stream
 */

void MultiPlayerEngine::streamConnectedEvent(WebSocketStream *stream)
{
	LOG_CTRACE("engine") << "MultiPlayerEngine stream connected:" << stream << (stream ? stream->credential().username() : "");

	if (stream && stream->socket()) {
		auto sig = connect(stream->socket(), &QWebSocket::binaryMessageReceived,
						   this, std::bind(&MultiPlayerEngine::onBinaryMessageReceived, this, std::placeholders::_1, stream));

		m_signalHelper[stream] = sig;

		LOG_CINFO("engine") << "SIGNAL HELPER ADD" << stream << sig;
	}
}


/**
 * @brief MultiPlayerEngine::streamDisconnectedEvent
 * @param stream
 */

void MultiPlayerEngine::streamDisconnectedEvent(WebSocketStream *stream)
{
	LOG_CTRACE("engine") << "MultiPlayerEngine stream disconnected:" << stream << (stream ? stream->credential().username() : "");

	if (stream && stream->socket()) {
		if (m_signalHelper.contains(stream)) {
			LOG_CINFO("engine") << "SIGNAL HELPER REMOVE" << stream << m_signalHelper.value(stream);
			QObject::disconnect(m_signalHelper.value(stream));
		}
		m_signalHelper.remove(stream);
	}

	if (m_hostStream == stream) {
		LOG_CTRACE("engine") << "HOST STREAM DISCONNECTED";

		m_worker.execInThread([this, stream](){
			LOG_CERROR("app") << "MUTEX LOCKER" << &m_mutex << QThread::currentThread();
			QMutexLocker locker(&m_mutex);

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

			triggerAll();
		});
	}
}


/**
 * @brief MultiPlayerEngine::streamTriggerEvent
 * @param stream
 */

void MultiPlayerEngine::streamTriggerEvent(WebSocketStream *stream)
{
	LOG_CINFO("engine") << "Stream trigger" << this << stream;

	m_worker.execInThread([this, stream](){
		LOG_CERROR("app") << "MUTEX LOCKER" << &m_mutex << QThread::currentThread();
		QMutexLocker locker(&m_mutex);
		QJsonObject ret;

		ret.insert(QStringLiteral("cmd"), QStringLiteral("state"));
		ret.insert(QStringLiteral("engine"), m_id);
		ret.insert(QStringLiteral("state"), m_gameState);
		ret.insert(QStringLiteral("interval"), m_service->mainTimerInterval());
		ret.insert(QStringLiteral("host"), (m_hostStream == stream ? true : false));

		QJsonArray streams;

		for (const auto &s : m_streams) {
			if (!s) continue;
			streams.append(s->credential().username());
		}

		ret.insert(QStringLiteral("users"), streams);

		sendStreamJson(stream, ret);
	});
}



/**
 * @brief MultiPlayerEngine::onBinaryMessageReceived
 * @param data
 */

void MultiPlayerEngine::onBinaryMessageReceived(const QByteArray &data, WebSocketStream *sender)
{
	LOG_CTRACE("engine") << "Binary message received" << data.size();

	m_worker.execInThread([this, sender, data]() mutable {
		LOG_CERROR("app") << "MUTEX LOCKER" << &m_mutex << QThread::currentThread();
		QMutexLocker locker(&m_mutex);

		std::optional<ObjectStateSnapshot> snap = ObjectStateSnapshot::fromByteArray(qUncompress(data));

		if (!snap) {
			LOG_CWARNING("engine") << "Invalid binary message received";
			return;
		}

		LOG_CTRACE("engine") << "Sorting snap";

		for (auto it=snap->list.begin(); it != snap->list.end(); ++it) {
			if (!it->get())
				continue;

			const qint64 &tick = (*it)->tick;
			std::unique_ptr<EntityState> estate(new EntityState(*it, sender ? sender->credential().username() : QStringLiteral("")));

			auto &v = m_states[tick];
			v.push_back(std::move(estate));
		}

	});
}



/**
 * @brief MultiPlayerEngine::renderStates
 */

void MultiPlayerEngine::renderStates()
{
	LOG_CTRACE("engine") << "Rendering states";

	m_worker.execInThread([this](){
		LOG_CERROR("app") << "MUTEX LOCKER" << &m_mutex << QThread::currentThread();
		QMutexLocker locker(&m_mutex);

		// create entities

		for (const auto& [tick, sList] : m_states) {
			for (const auto &s : sList) {
				const auto &id = s->id();
				if (id == -1 || !s->state)
					continue;

				if (m_entities.find(id) == m_entities.end()) {
					auto &e = m_entities[id];
					e.type = s->state->type;
					e.owner = s->sender;
					e.renderedState.reset(s->state->clone());

					LOG_CTRACE("engine") << "-----CREATED" << id << e.type;
				}
			}
		}


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

		m_states.clear();

	});

}



/**
 * @brief MultiPlayerEngine::sendStates
 */

void MultiPlayerEngine::sendStates()
{
	LOG_CTRACE("engine") << "Sending states";

	m_worker.execInThread([this](){
		LOG_CERROR("app") << "MUTEX LOCKER" << &m_mutex << QThread::currentThread();
		QMutexLocker locker(&m_mutex);

		std::vector<ObjectStateBase*> snap;
		//const qint64 tick =

		snap.reserve(m_entities.size());

		for (auto & [entityId, entity] : m_entities) {
			ObjectStateBase *ptr = entity.renderedState.get();
			ptr->id = entityId;
			//ptr->tick
			snap.push_back(ptr);
		}

		const QByteArray &data = qCompress(ObjectStateSnapshot::toByteArray(snap));

		for (auto ws : m_streams) {
			if (ws && ws->socket())
				ws->socket()->sendBinaryMessage(data);
		}

	});
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

	dest->sender = from.sender;
	dest->state.reset(from.state->clone());
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

	m_worker.execInThread([this](){
		LOG_CERROR("app") << "MUTEX LOCKER" << &m_mutex << QThread::currentThread();
		QMutexLocker locker(&m_mutex);

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

	});
}

