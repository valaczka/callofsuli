#include "abstractengine.h"
#include "Logger.h"


/**
 * @brief AbstractEngine::AbstractEngine
 * @param type
 * @param parent
 */

AbstractEngine::AbstractEngine(const Type &type, QObject *parent)
	: QObject{parent}
	, m_type(type)
{
	LOG_CTRACE("engine") << "Abstract engine created:" << type << this;
}



/**
 * @brief AbstractEngine::~AbstractEngine
 */

AbstractEngine::~AbstractEngine()
{
	LOG_CTRACE("engine") << "Abstract engine destroyed:" << m_type << this;
}


/**
 * @brief AbstractEngine::streamSet
 * @param stream
 */

void AbstractEngine::streamSet(WebSocketStream *stream)
{
	if (!stream)
		return;

	LOG_CTRACE("engine") << "Engine stream set:" << this << stream;
	QMutexLocker locker(&m_mutex);

	if (!m_streams.contains(stream)) {
		m_streams.append(stream);
		streamConnectedEvent(stream);
	}
}


/**
 * @brief AbstractEngine::streamUnSet
 * @param stream
 */

void AbstractEngine::streamUnSet(WebSocketStream *stream)
{
	if (!stream)
		return;

	LOG_CTRACE("engine") << "Engine stream unset:" << this << stream;
	QMutexLocker locker(&m_mutex);

	streamDisconnectedEvent(stream);

	m_streams.removeAll(stream);
}



/**
 * @brief AbstractEngine::canDelete
 * @param useCount
 * @return
 */

bool AbstractEngine::canDelete(const int &useCount)
{
	return (useCount == 1);
}

const QString &AbstractEngine::owner() const
{
	return m_owner;
}

void AbstractEngine::setOwner(const QString &newOwner)
{
	m_owner = newOwner;
}

int AbstractEngine::connectionLimit() const
{
	return m_connectionLimit;
}

void AbstractEngine::setConnectionLimit(int newConnectionLimit)
{
	m_connectionLimit = newConnectionLimit;
}

const QVector<WebSocketStream *> &AbstractEngine::streams() const
{
	return m_streams;
}



/**
 * @brief AbstractEngine::triggerAll
 */

void AbstractEngine::triggerAll()
{
	for (const auto &s : m_streams) {
		if (s)
			streamTriggerEvent(s);
	}
}


/**
 * @brief AbstractEngine::trigger
 * @param stream
 */

void AbstractEngine::trigger(WebSocketStream *stream)
{
	if (stream && m_streams.contains(stream))
		streamTriggerEvent(stream);
}

