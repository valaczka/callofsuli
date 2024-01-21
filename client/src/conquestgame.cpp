/*
 * ---- Call of Suli ----
 *
 * conquestgame.cpp
 *
 * Created on: 2024. 01. 21.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ConquestGame
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

#include "conquestgame.h"
#include "application.h"
#include "client.h"

ConquestGame::ConquestGame(Client *client)
	: AbstractGame(GameMap::Conquest, client)
{
	LOG_CTRACE("game") << "ConquestGame created" << this;

	if (m_client) {
		connect(m_client->httpConnection()->webSocket(), &WebSocket::messageReceived, this, &ConquestGame::onJsonReceived);
		connect(m_client->httpConnection()->webSocket(), &WebSocket::activeChanged, this, &ConquestGame::onActiveChanged);

		m_client->httpConnection()->webSocket()->observerAdd(QStringLiteral("conquest"));

		if (m_client->httpConnection()->webSocket()->active())
			onActiveChanged();
		else
			m_client->httpConnection()->webSocket()->connect();
	}

	connect(&m_timeSyncTimer, &QTimer::timeout, this, &ConquestGame::onTimeSyncTimerTimeout);

	m_timeSyncTimer.setInterval(15000);
	m_timeSyncTimer.start();
}



/**
 * @brief ConquestGame::~ConquestGame
 */

ConquestGame::~ConquestGame()
{
	m_tickTimer.stop();

	if (m_client) {
		m_client->httpConnection()->webSocket()->observerRemove(QStringLiteral("conquest"));
	}

	LOG_CTRACE("game") << "ConquestGame destroyed" << this;
}


/**
 * @brief ConquestGame::sendWebSocketMessage
 * @param data
 */

void ConquestGame::sendWebSocketMessage(const QJsonValue &data)
{
	if (!m_client || !m_client->httpConnection()->webSocket()->active()) {
		LOG_CWARNING("game") << "WebSocket inactive";
		return;
	}

	LOG_CTRACE("game") << "SEND:" << data;

	m_client->httpConnection()->webSocket()->send(QStringLiteral("conquest"), data);
}




/**
 * @brief ConquestGame::gameAbort
 */

void ConquestGame::gameAbort()
{
	setFinishState(Neutral);

	LOG_CINFO("game") << "Game aborted:" << this;

	emit gameFinished(Neutral);
}


/**
 * @brief ConquestGame::loadPage
 * @return
 */

QQuickItem *ConquestGame::loadPage()
{
	return m_client->stackPushPage(QStringLiteral("PageConquestGame.qml"), QVariantMap({
																						   { QStringLiteral("game"), QVariant::fromValue(this) }
																					   }));

}


/**
 * @brief ConquestGame::timerEvent
 */

void ConquestGame::timerEvent(QTimerEvent *)
{
	if (m_config.state != ConquestConfig::StatePlay)
		return;

	LOG_CTRACE("game") << "TIMER EVENT" << m_tickTimer.currentTick();

	/*ObjectStateSnapshot snap;

	foreach (GameObject *o, m_scene->m_gameObjects) {
		const qint64 id = getEntityId(o);

		if (o && id != -1) {
			o->getStateSnapshot(&snap, id);
		}
	}

	m_client->httpConnection()->webSocket()->send(qCompress(snap.toByteArray()));*/
}


/**
 * @brief ConquestGame::connectGameQuestion
 */

void ConquestGame::connectGameQuestion()
{

}


/**
 * @brief ConquestGame::gameStartEvent
 * @return
 */

bool ConquestGame::gameStartEvent()
{
	return true;
}


/**
 * @brief ConquestGame::gameFinishEvent
 * @return
 */

bool ConquestGame::gameFinishEvent()
{
	return true;
}




/**
 * @brief ConquestGame::onTimeSyncTimerTimeout
 */

void ConquestGame::onTimeSyncTimerTimeout()
{
	if (!m_client)
		return;

	if (!m_client->httpConnection()->webSocket()->active()) {
		LOG_CWARNING("game") << "WebSocket inactive";
		return;
	}

	LOG_CTRACE("game") << "Sync server time" << QDateTime::currentDateTime();

	m_client->httpConnection()->webSocket()->send(QStringLiteral("timeSync"), QJsonObject{
													  { QStringLiteral("clientTime"), QDateTime::currentMSecsSinceEpoch() }
												  });
}



/**
 * @brief ConquestGame::onActiveChanged
 */

void ConquestGame::onActiveChanged()
{
	auto ws = m_client->httpConnection()->webSocket();

	const bool active = ws->active();
	LOG_CTRACE("game") << "MultiPlayerGame WebSocket active changed" << active;

	/*if (active && !m_binarySignalConnected) {
		connect(ws->socket(), &QWebSocket::binaryMessageReceived, this, &MultiPlayerGame::updateGameTrigger);
		m_binarySignalConnected = true;
	}*/

	QJsonObject o = m_config.toJson();
	o.insert(QStringLiteral("cmd"), QStringLiteral("connect"));

	sendWebSocketMessage(o);

}



/**
 * @brief ConquestGame::onJsonReceived
 * @param operation
 * @param data
 */

void ConquestGame::onJsonReceived(const QString &operation, const QJsonValue &data)
{
	if (operation == QStringLiteral("timeSync")) {
		const QJsonObject &obj = data.toObject();
		qint64 clientTime = QDateTime::currentMSecsSinceEpoch() - JSON_TO_INTEGER(obj.value(QStringLiteral("clientTime")));

		m_tickTimer.setLatency(clientTime/2);

		LOG_CTRACE("game") << "ACTUAL SERVER LATENCY" << m_tickTimer.latency();
	} else if (operation == QStringLiteral("conquest")) {
		const QJsonObject &obj = data.toObject();
		const QString &cmd = obj.value(QStringLiteral("cmd")).toString();

		using fnDef = void (ConquestGame::*)(const QJsonObject &);

		static const QHash<std::string, fnDef> fMap = {
			{ "state", &ConquestGame::cmdState },
			{ "start", &ConquestGame::cmdStart },
			{ "connect", &ConquestGame::cmdConnect },
			{ "prepare", &ConquestGame::cmdPrepare },
			{ "questionRequest", &ConquestGame::cmdQuestionRequest },
			{ "test", &ConquestGame::cmdTest },
		};

		auto fn = fMap.value(cmd.toStdString());

		if (obj.contains(QStringLiteral("error"))) {
			m_client->messageError(obj.value(QStringLiteral("error")).toString(), tr("Belső hiba"));
		} else if (fn) {
			std::invoke(fn, this, obj);
		} else {
			LOG_CWARNING("game") << "Invalid command?" << cmd;
		}
	}
}



/**
 * @brief ConquestGame::onGameStateChanged
 */

void ConquestGame::onGameStateChanged()
{
	LOG_CINFO("game") << "ConquestGame state:" << m_config.state;

	if (m_config.state == ConquestConfig::StatePrepare) {
		LOG_CDEBUG("game") << "Prepare world:" << qPrintable(m_config.world);

	}

	/*sendWebSocketMessage(QJsonObject{
		{ QStringLiteral("cmd"), QStringLiteral("prepare") },
		{ QStringLiteral("engine"), m_engineId },
		{ QStringLiteral("ready"), true }
	});*/
}



/**
 * @brief ConquestGame::cmdState
 * @param data
 */

void ConquestGame::cmdState(const QJsonObject &data)
{
	//setEngineId(data.value(QStringLiteral("engine")).toInt(-1));

	if (data.value(QStringLiteral("engine")).toInt(-1) != m_engineId) {
		LOG_CWARNING("game") << "Game engine mismatch";
	}

	setHostMode(data.value(QStringLiteral("host")).toVariant().toBool() ? HostMode::ModeHost : HostMode::ModeGuest);
	setConfig(ConquestConfig::fromJson(data));

	if (data.contains(QStringLiteral("playerId")))
		setPlayerId(data.value(QStringLiteral("playerId")).toInt());


	/*if (data.contains(QStringLiteral("gameState"))) {
		setMultiPlayerGameState(obj.value(QStringLiteral("gameState")).toVariant().value<MultiPlayerGameState>());
	}

	if (obj.contains(QStringLiteral("playerEntityId"))) {
		setPlayerEntityId(obj.value(QStringLiteral("playerEntityId")).toInt());
	}*/

	if (qint64 tick = JSON_TO_INTEGER_Y(data.value(QStringLiteral("tick")), -1); tick != -1) {
		m_tickTimer.start(this, tick);
	}

	/*if (data.contains(QStringLiteral("interval"))) {
		setServerInterval(data.value(QStringLiteral("interval")).toInt());
	}*/
}


/**
 * @brief ConquestGame::cmdConnect
 * @param data
 */

void ConquestGame::cmdConnect(const QJsonObject &data)
{
	setEngineId(data.value(QStringLiteral("engine")).toInt(-1));
}


/**
 * @brief ConquestGame::cmdStart
 * @param data
 */

void ConquestGame::cmdStart(const QJsonObject &data)
{

}


/**
 * @brief ConquestGame::cmdPrepare
 * @param data
 */

void ConquestGame::cmdPrepare(const QJsonObject &data)
{

}



/**
 * @brief ConquestGame::cmdQuestionRequest
 * @param data
 */

void ConquestGame::cmdQuestionRequest(const QJsonObject &)
{
	LOG_CDEBUG("game") << "Generate questions";

	m_client->messageInfo("generated");

	QJsonArray tmp = {6,3,2,6};

	sendWebSocketMessage(QJsonObject{
							 { QStringLiteral("cmd"), QStringLiteral("questionRequest") },
							 { QStringLiteral("engine"), m_engineId },
							 { QStringLiteral("list"), tmp }
						 });
}



/**
 * @brief ConquestGame::cmdTest
 * @param data
 */

void ConquestGame::cmdTest(const QJsonObject &data)
{
	if (data.contains(QStringLiteral("stateId"))) {
		emit testImage(data.value(QStringLiteral("stateId")).toInt(), data.value(QStringLiteral("value")).toBool());
	}
}



/**
 * @brief ConquestGame::playerId
 * @return
 */

int ConquestGame::playerId() const
{
	return m_playerId;
}

void ConquestGame::setPlayerId(int newPlayerId)
{
	if (m_playerId == newPlayerId)
		return;
	m_playerId = newPlayerId;
	emit playerIdChanged();
}



/**
 * @brief ConquestGame::engineId
 * @return
 */

int ConquestGame::engineId() const
{
	return m_engineId;
}

void ConquestGame::setEngineId(int newEngineId)
{
	if (m_engineId == newEngineId)
		return;
	m_engineId = newEngineId;
	emit engineIdChanged();
}


/**
 * @brief ConquestGame::hostMode
 * @return
 */

ConquestGame::HostMode ConquestGame::hostMode() const
{
	return m_hostMode;
}

void ConquestGame::setHostMode(const HostMode &newHostMode)
{
	if (m_hostMode == newHostMode)
		return;
	m_hostMode = newHostMode;
	emit hostModeChanged();
}



/**
 * @brief ConquestGame::config
 * @return
 */

ConquestConfig ConquestGame::config() const
{
	return m_config;
}

void ConquestGame::setConfig(const ConquestConfig &newConfig)
{
	if (m_config == newConfig)
		return;
	m_config = newConfig;
	emit configChanged();
	onGameStateChanged();
}




/**
 * @brief ConquestGame::handler
 * @return
 */

StudentMapHandler *ConquestGame::handler() const
{
	return m_handler;
}

void ConquestGame::setHandler(StudentMapHandler *newHandler)
{
	m_handler = newHandler;
}

