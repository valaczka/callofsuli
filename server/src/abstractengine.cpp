#include "abstractengine.h"
#include "Logger.h"
#include "enginehandler.h"


/**
 * @brief AbstractEngine::AbstractEngine
 * @param type
 * @param parent
 */

AbstractEngine::AbstractEngine(const Type &type, const int &id, EngineHandler *handler, QObject *parent)
	: QObject{parent}
	, m_handler(handler)
	, m_service(handler ? handler->m_service : nullptr)
	, m_type(type)
	, m_id(id)
{
	Q_ASSERT(handler);

	LOG_CDEBUG("engine") << "Abstract engine created:" << m_type << m_id << this;
}



/**
 * @brief AbstractEngine::~AbstractEngine
 */

AbstractEngine::~AbstractEngine()
{
	LOG_CDEBUG("engine") << "Abstract engine destroyed:" << m_type << m_id << this;
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

	if (!m_streams.contains(stream)) {
		m_streams.append(stream);
		streamLinkedEvent(stream);
	}

	LOG_CTRACE("engine") << "Engine stream set finsihed" << this << stream << m_streams;
}


/**
 * @brief AbstractEngine::streamUnSet
 * @param stream
 */

void AbstractEngine::streamUnSet(WebSocketStream *stream)
{
	if (!stream)
		return;

	LOG_CTRACE("engine") << "Engine stream unset check" << stream;

	streamUnlinkedEvent(stream);

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

uint AbstractEngine::connectionLimit() const
{
	return m_connectionLimit;
}

void AbstractEngine::setConnectionLimit(uint newConnectionLimit)
{
	m_connectionLimit = newConnectionLimit;
}

const QVector<WebSocketStream *> &AbstractEngine::streams() const
{
	return m_streams;
}


int AbstractEngine::id() const
{
	return m_id;
}

void AbstractEngine::setId(int newId)
{
	m_id = newId;
}

QString AbstractEngine::dumpEngine() const
{
	return QStringLiteral(">>> AbstractEngine %1 <<<\n \n \n").arg(m_id);
}

uint AbstractEngine::playerLimit() const
{
	return m_playerLimit;
}

void AbstractEngine::setPlayerLimit(uint newPlayerLimit)
{
	m_playerLimit = newPlayerLimit;
}

