/*
 * ---- Call of Suli ----
 *
 * rpgengine.cpp
 *
 * Created on: 2025. 01. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgEngine
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "rpgengine.h"
#include "rpgstream.h"
#include "FileAppender.h"
#include "Logger.h"
#include "serverservice.h"
#include "udpserver.h"
#include <QCborArray>
#include <QCborMap>

#include "rpgengine_p.h"


/**
 * @brief RpgEngine::RpgEngine
 * @param handler
 * @param parent
 */

RpgEngine::RpgEngine(EngineHandler *handler, QObject *parent)
	: UdpEngine(EngineRpg, handler, parent)
	, d(new RpgEnginePrivate(this))
{
}


/**
 * @brief RpgEngine::~RpgEngine
 */

RpgEngine::~RpgEngine()
{
	delete d;
	d = nullptr;
}


/**
 * @brief RpgEngine::engineCreate
 * @param handler
 * @return
 */

std::shared_ptr<RpgEngine> RpgEngine::engineCreate(EngineHandler *handler, UdpServer *server)
{
	if (!handler)
		return {};

	LOG_CDEBUG("engine") << "Create RpgEngine" << m_nextId << server;


	auto ptr = std::make_shared<RpgEngine>(handler);
	ptr->setId(m_nextId);
	increaseNextId();

	ptr->m_readableId = QRandomGenerator::global()->bounded(1000, 1000000);

	ptr->setUdpServer(server);

	ptr->setPlayerLimit(4);

	if (const QString &dir = handler->service()->logDir(); !dir.isEmpty()) {
		const QString fname = dir+QStringLiteral("/rpg-%1.log").arg(ptr->id(), 3, 10, '0');

		if (QFile::exists(fname))
			QFile::remove(fname);

		ptr->setLoggerFile(fname);
	}

	handler->engineAdd(ptr);

	return ptr;
}



/**
 * @brief RpgEngine::engineDispatch
 * @param handler
 * @param data
 * @param server
 * @return
 */

std::shared_ptr<RpgEngine> RpgEngine::engineDispatch(EngineHandler *handler, const QJsonObject &connectionToken,
													 UdpPacketRcv &&data)
{
	Q_ASSERT(handler);
	Q_ASSERT(data.peer);
	Q_ASSERT(data.peer->server());


	RpgStream::EngineStream stream(data.data);

	LOG_CDEBUG("engine") << "*********************** operation" << data.peer->peerID() << stream.operation();

	if (stream.operation() == RpgStream::EngineStream::OperationInvalid) {
		LOG_CWARNING("engine") << "Invalid operation" << data.peer->peerID();

		return {};
	}


	RpgStream::ConnectionToken cToken;
	cToken.fromJson(connectionToken);

	if (stream.operation() == RpgStream::EngineStream::OperationList) {
		/////RpgEnginePrivate::sendEngineList(cToken.config, data.peer, handler);
		return {};
	}


	if (stream.operation() == RpgStream::EngineStream::OperationDisconnect) {
		LOG_CWARNING("engine") << "Invalid Disconnect operation" << data.peer->peerID();

		return {};
	}


	// Create

	if (stream.operation() == RpgStream::EngineStream::OperationCreate) {
		/*std::shared_ptr<RpgEngine> engine = engineCreate(handler, cToken.config, data.peer->server());

		LOG_CINFO("engine") << "Create engine" << data.peer->peerID() << "id:" << engine->id();

		data.peer->server()->peerConnectToEngine(data.peer, engine);

		return engine;*/
	}



	/*

	// Connect

	const auto &list = handler->engines();

	const auto it = std::find_if(list.constBegin(),
								 list.constEnd(),
								 [peer](const std::shared_ptr<AbstractEngine> &ptr){
		if (!ptr || ptr->type() != EngineRpg)
			return false;

		return std::dynamic_pointer_cast<RpgEngine>(ptr)->player(peer->peerID()) != nullptr;

	});

	if (it != list.constEnd()) {
		std::shared_ptr<RpgEngine> engine = std::dynamic_pointer_cast<RpgEngine>(*it);

		if (!engine) {
			LOG_CERROR("engine") << "Engine cast error";
			return {};
		}

		if (engine->config() == cToken.config) {
			if (selector.engine <= 0 || selector.engine == engine->id()) {
				peer->server()->peerConnectToEngine(peer, engine);
				return engine;
			} else {
				LOG_CWARNING("engine") << "Engine config mismatch" << peer->peerID() << selector.engine << "vs." << engine->id();
			}
		}
	}


	// Direct connect

	const auto eit = std::find_if(list.constBegin(),
								  list.constEnd(),
								  [&cToken, &selector, peerId = peer->peerID()](const std::shared_ptr<AbstractEngine> &ptr){
		if (!ptr || ptr->type() != AbstractEngine::EngineRpg)
			return false;

		if (!RpgEnginePrivate::canConnect(peerId, cToken.config, std::dynamic_pointer_cast<RpgEngine>(ptr).get()))
			return false;

		return ptr->id() == selector.engine;
	});


	if (eit == list.constEnd()) {
		LOG_CWARNING("engine") << "Invalid engine" << peer->peerID() << selector.engine;
		return {};
	}


	std::shared_ptr<RpgEngine> engine = std::dynamic_pointer_cast<RpgEngine>(*eit);

	peer->server()->peerConnectToEngine(peer, engine);
	return engine;
	*/

	return {};
}


/**
 * @brief RpgEngine::peerFind
 * @param server
 * @param username
 * @return
 */

std::shared_ptr<RpgEngine> RpgEngine::peerFind(UdpServer *server, const QString &username, quint32 *idPtr)
{
	Q_ASSERT(server);

	return std::dynamic_pointer_cast<RpgEngine>(server->findEngineForUser(AbstractEngine::EngineRpg, username, idPtr));
}



/**
 * @brief RpgEngine::peerAbort
 * @param peerId
 * @return
 */

bool RpgEngine::peerAbort(const quint32 &peerId)
{
	////return d->abortPlayer(peerId);
	return false;
}



/**
 * @brief RpgEngine::canDelete
 * @param useCount
 * @return
 */

bool RpgEngine::canDelete(const int &useCount)
{
	if (!d->m_removeTimer.isForever()) {
		return d->m_removeTimer.hasExpired();
	} /*else if (m_config.gameState == RpgConfig::StatePlay) {
		// Ha van még, aki nincs kész, akkor nem zárjuk le

		const auto it = std::find_if(m_player.cbegin(),
									 m_player.cend(),
									 [](const auto &ptr) {
			return !ptr->config().finished;
		});

		const bool hasNoFinished = (it != m_player.cend());

		return !hasNoFinished;

	}*/ else {
		return AbstractEngine::canDelete(useCount);
	}
}



/**
 * @brief RpgEngine::binaryDataReceived
 * @param data
 */

void RpgEngine::binaryDataReceived(const UdpServerPeerReceivedList &data)
{
	for (const auto &pair : data)
		binaryDataReceived(pair);

	/*QElapsedTimer t2;
	t2.start();

	d->updateState();

	d->renderTimerMeausure(RpgEnginePrivate::TimerUpd, t2.elapsed());

	if (m_config.gameState == RpgConfig::StateCharacterSelect)
		d->dataSend(RpgEnginePrivate::SendChrSel);
	else if (m_config.gameState == RpgConfig::StatePrepare)
		d->dataSend(RpgEnginePrivate::SendPrepare);
	else if (m_config.gameState == RpgConfig::StatePlay)
		d->dataSendPlay();
	else if (m_config.gameState == RpgConfig::StateFinished)
		d->dataSendFinished();

	d->renderTimerMeausure(RpgEnginePrivate::TimerTick, t2.elapsed());*/
}






/**
 * @brief RpgEngine::binaryDataReceived
 * @param peer
 * @param data
 */

void RpgEngine::binaryDataReceived(const UdpPacketRcv &recv)
{
	Q_ASSERT(recv.peer);

	/*RpgEnginePlayer *player = d->getPlayer(recv.peer);

	if (!player) {
		ELOG_ERROR << "Player not found" << recv.peer;
		return;
	}

	d->dataReceived(player, recv.data);*/
}




/**
 * @brief RpgEngine::udpPeerAdd
 * @param peer
 */

void RpgEngine::udpPeerAdd(UdpServerPeer *peer)
{
	if (!peer)
		return;
}



/**
 * @brief RpgEngine::udpPeerRemove
 * @param peer
 */

void RpgEngine::udpPeerRemove(UdpServerPeer *peer)
{

}



/**
 * @brief RpgEngine::disconnectUnusedPeer
 * @param peer
 */

void RpgEngine::disconnectUnusedPeer(UdpServerPeer *peer)
{
	/*if (m_config.gameState == RpgConfig::StateFinished && !d->m_removeTimer.isForever() && d->m_removeTimer.hasExpired()) {
		ELOG_INFO << "Disconnect peer" << peer->peerID() << "from engine";

		if (peer->peer()) {
			enet_peer_disconnect_later(peer->peer(), 0);
		} else {
			LOG_CERROR("engine") << "Missing ENetPeer" << peer << "in peer" << peer->peerID();
		}
	}*/
}


/**
 * @brief RpgEngine::isPeerValid
 * @param peerId
 * @return
 */

bool RpgEngine::isPeerValid(const quint32 &peerId) const
{
	/*if (m_config.gameState == RpgConfig::StateError || m_config.gameState == RpgConfig::StateFinished)
		return false;

	if (d->m_abortList.contains(peerId))
		return false;

	if (player(peerId))
		return true;*/

	return false;
}



/**
 * @brief RpgEngine::dumpEngine
 * @return
 */

QString RpgEngine::dumpEngine() const
{
	return d->engineDump();
}


/**
 * @brief RpgEngine::toStream
 * @return
 */

RpgStream::Engine RpgEngine::toStream() const
{
	RpgStream::Engine e;

	e.setId(m_id);
	e.setReadableId(m_readableId);
	e.setMaxPlayer(m_playerLimit);

	/*if (m_hostPlayer) {
		e.owner().setUserName(m_hostPlayer->config().username.toUtf8());
		e.owner().setNickName(m_hostPlayer->config().nickname.toUtf8());
	}

	for (const auto &p : m_player) {
		if (!p.get())
			continue;

		if (m_hostPlayer && p.get() == m_hostPlayer)
			continue;

		e.players().emplace_back(p->config().username.toUtf8(), p->config().nickname.toUtf8());
	}*/

	return e;
}


/**
 * @brief RpgEngine::_logger
 * @return
 */

Logger *RpgEngine::_logger() const
{
	return d->m_logger.get();
}



/**
 * @brief RpgEngine::setLoggerFile
 * @param fname
 */

void RpgEngine::setLoggerFile(const QString &fname)
{
	FileAppender* appender = new FileAppender(fname);

	appender->setFormat(QString::fromStdString( "%{time}{hh:mm:ss.zzz} [%{TypeOne}] %{message}\n"));

#ifndef QT_NO_DEBUG
	//appender->setFormat(QString::fromStdString( "%{time}{yyyy-MM-dd hh:mm:ss.zzz} [%{TypeOne}] %{message} <%{function} %{file}:%{line}>\n"));
	appender->setDetailsLevel(Logger::Trace);
#else
	//appender->setFormat(QString::fromStdString( "%{time}{hh:mm:ss.zzz} [%{TypeOne}] %{message}\n"));
	appender->setDetailsLevel(Logger::Debug);
#endif

	d->m_logger->registerAppender(appender);
}


















/**
 * @brief RpgEnginePrivate::getPlayer
 * @param peer
 * @return
 */

void RpgEnginePrivate::sendEngineList(UdpServerPeer *peer, EngineHandler *handler)
{
	Q_ASSERT(handler);
	Q_ASSERT(peer);
	Q_ASSERT(peer->server());

	LOG_CINFO("engine") << "++++++++++ send engine list" << peer->peerID() << peer->peer();

	RpgStream::EngineList list;


	for (const auto &ptr : handler->engines()) {
		if (!ptr || ptr->type() != AbstractEngine::EngineRpg)
			continue;

		const auto &e = std::dynamic_pointer_cast<RpgEngine>(ptr);

		if (!canConnect(peer->peerID(), e.get()))
			continue;

		list.engines().push_back(e->toStream());

	}

	const int max = std::max(1, handler->service()->settings()->udpMaxEngines());

	list.setCanCreate(handler->engines().size() < max);

	peer->send(list.toStream().data(), false);
}



/**
 * @brief RpgEnginePrivate::canConnect
 * @param config
 * @param engine
 * @return
 */


bool RpgEnginePrivate::canConnect(const qint64 &peerID, RpgEngine *engine)
{
	return (engine &&
			/*(engine->config().gameState == RpgConfig::StateConnect ||
					 engine->config().gameState == RpgConfig::StateCharacterSelect) &&
					engine->config() == config &&*/
			!engine->d->m_locked /*&&
					!engine->d->m_banList.contains(peerID) &&
					!engine->d->m_abortList.contains(peerID) &&
					(engine->m_playerLimit <= 0 || engine->m_player.size() < engine->m_playerLimit)*/
			);
}







/**
 * @brief RpgEnginePrivate::renderTimerDump
 * @return
 */

QString RpgEnginePrivate::renderTimerDump() const
{
	QString txt;

	static const QHash<Measure, QString> hash = {
		{ Invalid, QStringLiteral("Invalid") },
		{ Received, QStringLiteral("Received") },
		{ Render, QStringLiteral("Render") },
		{ RenderFull, QStringLiteral("RenderFull") },
		{ TimerTick, QStringLiteral("TimerTick") },
		{ TimerUpd, QStringLiteral("TimerUpd") },
		{ BinaryRcv, QStringLiteral("BinaryRcv") },
	};

	for (const auto &[key, d] : m_renderData.asKeyValueRange()) {
		QString t;
		txt += QStringLiteral("%1: ").arg(hash.value(key, QStringLiteral("???")), 16);

		txt += QStringLiteral("avg: %1 med: %2 max: %3")
			   .arg(d.avg(), 3)
			   .arg(d.med, 3)
			   .arg(d.max, 3)
			   ;

		if (!d.data.isEmpty()) {
			const auto [min, max] = std::minmax_element(d.data.constBegin(), d.data.constEnd());
			txt += QStringLiteral(" (min: %1 max: %2)").arg(*min, 3).arg(*max, 3);
		}

		txt += '\n';
	}

	txt += QStringLiteral(" \n");

	/*txt += '\n';

	for (const auto &[key, d] : m_renderData.asKeyValueRange()) {
		txt += hash.value(key, QStringLiteral("???"));
		txt += QStringLiteral("\n-------------------------------------\n");

		for (int i=0; i<d.data.size() && i<40; ++i)
			txt += QStringLiteral("%1\n").arg(d.data.at(i), 5);

		txt += '\n';
	}*/

	return txt;
}



/**
 * @brief RpgEnginePrivate::engineDump
 * @return
 */

QString RpgEnginePrivate::engineDump() const
{
	QString txt;

	txt += QStringLiteral("[ENGINE %1] - %2\n").arg(q->m_id).arg(q->m_readableId, 6, 10, '0');
	txt += QStringLiteral("------------------------------------------------------------------\n");

	txt += renderTimerDump();

	/*txt += QStringLiteral("State: %1 | Players: %2 | Tick: %3\n")
		   .arg(q->m_config.gameState, 2).arg(q->m_player.size(), 2)
		   .arg(q->m_currentTick, 5)
		   ;

	txt += QStringLiteral("------------------------------------------------------------------\n");

	for (const auto &ptr : q->m_player) {
		if (ptr->isHost())
			txt += QStringLiteral("(*) ");
		else
			txt += QStringLiteral("( ) ");

		txt += QStringLiteral("P%1 [%2] ")
			   .arg(ptr->playerId())
			   .arg(ptr->peerID(), 12)
			   ;


		if (UdpServerPeer *peer = ptr->udpPeer()) {
			txt += QStringLiteral("%1 | ").arg(peer->address(), 21);
			txt += QStringLiteral("RTT %1 | FPS: %2 | Peer FPS: %3")
				   .arg(peer->currentRtt(), 2)
				   .arg(peer->currentFps(), 2)
				   .arg(peer->peerFps(), 2)
				   ;
		}

		txt += '\n';
	}*/

	txt += QStringLiteral(" \n \n");

	return txt;
}



