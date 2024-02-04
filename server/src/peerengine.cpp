#include "peerengine.h"
#include "Logger.h"
#include "qjsonarray.h"
#include "qjsonobject.h"
#include "enginehandler.h"

PeerEngine::PeerEngine(EngineHandler *handler, QObject *parent)
	: AbstractEngine{EnginePeer, handler, parent}
{

}


/**
 * @brief PeerEngine::logPeerUser
 * @param user
 * @return
 */

bool PeerEngine::logPeerUser(const PeerUser &user)
{
	bool ret = PeerUser::addOrUpdate(&m_peerUser, user);
	if (ret)
		m_handler->engineTrigger(EnginePeer);

	return ret;
}


/**
 * @brief PeerEngine::timerMinuteTick
 */

void PeerEngine::timerMinuteTick()
{
	if (PeerUser::clear(&m_peerUser))
		m_handler->engineTrigger(EnginePeer);
}


/**
 * @brief PeerEngine::triggerEvent
 * @param stream
 */

void PeerEngine::triggerEvent()
{
	const QJsonArray &list = PeerUser::toJson(m_peerUser);

	for (const auto &stream : m_streams) {
		stream->sendJson("peers", list);
	}
}



/**
 * @brief PeerEngine::streamConnectedEvent
 * @param stream
 */

void PeerEngine::streamLinkedEvent(WebSocketStream *stream)
{
	LOG_CTRACE("service") << "PeerEngine stream connected" << stream;

	const auto &list = m_handler->engineGet<PeerEngine>(AbstractEngine::EnginePeer);

	if (list.isEmpty()) {

	}
}





/**
 * @brief PeerUser::add
 * @param list
 * @param user
 */

QVector<PeerUser>::iterator PeerUser::find(QVector<PeerUser> *list, const PeerUser &user)
{
	Q_ASSERT(list);

	QVector<PeerUser>::iterator it = list->begin();

	for (; it != list->end(); ++it) {
		if (*it == user)
			break;
	}

	return it;
}


/**
 * @brief PeerUser::add
 * @param list
 * @param user
 */

bool PeerUser::addOrUpdate(QVector<PeerUser> *list, const PeerUser &user)
{
	Q_ASSERT(list);

	if (user.m_username.isEmpty())
		return false;

	auto it = find(list, user);

	if (it == list->end()) {
		list->append(user);
		return true;
	} else
		it->setTimestamp(QDateTime::currentDateTime());

	return false;
}


/**
 * @brief PeerUser::remove
 * @param list
 * @param user
 * @return
 */

bool PeerUser::remove(QVector<PeerUser> *list, const PeerUser &user)
{
	Q_ASSERT(list);

	if (user.m_username.isEmpty())
		return false;

	bool ret = false;

	for (auto it = list->constBegin(); it != list->constEnd(); ) {
		if (*it == user) {
			it = list->erase(it);
			ret = true;
		} else
			++it;
	}

	return ret;
}



/**
 * @brief PeerUser::clear
 * @param list
 */

bool PeerUser::clear(QVector<PeerUser> *list, const qint64 &sec)
{
	Q_ASSERT(list);

	const QDateTime &dt = QDateTime::currentDateTime();

	bool ret = false;

	for (auto it = list->constBegin(); it != list->constEnd(); ) {
		if (!(it->m_timestamp).isValid() || it->m_timestamp.secsTo(dt) > sec) {
			it = list->erase(it);
			ret = true;
		} else
			++it;
	}

	return ret;
}


/**
 * @brief PeerUser::toJson
 * @param list
 * @return
 */

QJsonArray PeerUser::toJson(const QVector<PeerUser> &list)
{
	QJsonArray ret;

	foreach (const PeerUser &user, list) {
		QJsonObject o;
		o.insert(QStringLiteral("username"), user.m_username);
		o.insert(QStringLiteral("host"), user.m_host.toString());
		o.insert(QStringLiteral("agent"), user.m_agent);
		o.insert(QStringLiteral("timestamp"), user.m_timestamp.toSecsSinceEpoch());
		ret.append(o);
	}

	return ret;
}




const QString &PeerUser::username() const
{
	return m_username;
}

void PeerUser::setUsername(const QString &newUsername)
{
	m_username = newUsername;
}

const QString &PeerUser::familyName() const
{
	return m_familyName;
}

void PeerUser::setFamilyName(const QString &newFamilyName)
{
	m_familyName = newFamilyName;
}

const QString &PeerUser::givenName() const
{
	return m_givenName;
}

void PeerUser::setGivenName(const QString &newGivenName)
{
	m_givenName = newGivenName;
}

const QHostAddress &PeerUser::host() const
{
	return m_host;
}

void PeerUser::setHost(const QHostAddress &newHost)
{
	m_host = newHost;
}

const QDateTime &PeerUser::timestamp() const
{
	return m_timestamp;
}

void PeerUser::setTimestamp(const QDateTime &newTimestamp)
{
	m_timestamp = newTimestamp;
}

const QString &PeerUser::agent() const
{
	return m_agent;
}

void PeerUser::setAgent(const QString &newAgent)
{
	m_agent = newAgent;
}
