#include "multiplayerengine.h"
#include "Logger.h"
#include "serverservice.h"


/**
 * @brief MultiPlayerEngine::MultiPlayerEngine
 * @param parent
 */

MultiPlayerEngine::MultiPlayerEngine(QObject *parent)
	: AbstractEngine{EngineMultiPlayer, parent}
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

		QMutexLocker locker(&s->mutex());

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

	QMutexLocker locker(&stream->mutex());

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
			auto ptr = std::make_shared<MultiPlayerEngine>();

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
}


/**
 * @brief MultiPlayerEngine::streamDisconnectedEvent
 * @param stream
 */

void MultiPlayerEngine::streamDisconnectedEvent(WebSocketStream *stream)
{
	LOG_CTRACE("engine") << "MultiPlayerEngine stream disconnected:" << stream << (stream ? stream->credential().username() : "");

	if (m_hostStream == stream) {
		LOG_CTRACE("engine") << "HOST STREAM DISCONNECTED";

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
	}
}


/**
 * @brief MultiPlayerEngine::streamTriggerEvent
 * @param stream
 */

void MultiPlayerEngine::streamTriggerEvent(WebSocketStream *stream)
{
	LOG_CINFO("engine") << "Stream trigger" << this << stream;

	QJsonObject ret;

	ret.insert(QStringLiteral("cmd"), QStringLiteral("state"));
	ret.insert(QStringLiteral("engine"), m_id);
	ret.insert(QStringLiteral("state"), m_gameState);
	ret.insert(QStringLiteral("host"), (m_hostStream == stream ? true : false));

	QJsonArray streams;

	for (const auto &s : m_streams) {
		if (!s) continue;
		streams.append(s->credential().username());
	}

	ret.insert(QStringLiteral("users"), streams);

	sendStreamJson(stream, ret);
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
