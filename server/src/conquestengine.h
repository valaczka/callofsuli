/*
 * ---- Call of Suli ----
 *
 * conquestengine.h
 *
 * Created on: 2024. 01. 21.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ConquestEngine
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

#ifndef CONQUESTENGINE_H
#define CONQUESTENGINE_H

#include "abstractengine.h"
#include "conquestconfig.h"
#include "qelapsedtimer.h"
#include "qjsonarray.h"
#include "qlambdathreadworker.h"


class ConquestEngine;

/**
 * @brief The ConquestQuestion class
 */

class ConquestQuestion
{
public:
	ConquestQuestion(ConquestEngine *engine)
		: m_engine(engine)
	{}

	void check();
	void upload(const QJsonArray &list);
	QJsonValue next();
	bool hasQuestion();
	const QJsonArray &array() const { return m_questionArray; }

private:
	ConquestEngine *m_engine = nullptr;
	QLambdaThreadWorker m_worker;
	QMutex m_mutex;
	QJsonArray m_questionArray;
	qint64 m_lastRequest = 0;
};



/**
 * @brief The Player class
 */

class ConquestEnginePlayer : public ConquestPlayer
{
	Q_GADGET

public:
	ConquestEnginePlayer(const int &_id, const QString &_user, const QString &_fullNickName, WebSocketStream *_stream)
		: ConquestPlayer(_id, _user)
		, stream(_stream)
	{
		fullNickName = _fullNickName;
	}
	ConquestEnginePlayer(const int &_id, const QString &_user, WebSocketStream *_stream)
		: ConquestEnginePlayer(_id, _user, QStringLiteral(""), _stream) {}
	ConquestEnginePlayer(const int &_id, WebSocketStream *_stream)
		: ConquestEnginePlayer(_id, QStringLiteral(""), _stream) {}
	ConquestEnginePlayer(const int &_id)
		: ConquestEnginePlayer(_id, nullptr) {}
	ConquestEnginePlayer()
		: ConquestEnginePlayer(-1) {}
	virtual ~ConquestEnginePlayer() = default;

	WebSocketStream *stream = nullptr;
	int gameId = -1;
};


/**
 * @brief The ConquestEngine class
 */

class ConquestEngine : public AbstractEngine
{
	Q_OBJECT

public:
	explicit ConquestEngine(EngineHandler *handler, QObject *parent = nullptr);
	virtual ~ConquestEngine();

	static void handleWebSocketMessage(WebSocketStream *stream, const QJsonValue &message, EngineHandler *handler);

	static std::shared_ptr<ConquestEngine> createEngine(WebSocketStream *stream, EngineHandler *handler);
	static std::weak_ptr<AbstractEngine> connectToEngine(const int &id, WebSocketStream *stream, EngineHandler *handler);
	static int restoreEngines(ServerService *service);

	QJsonObject cmdStart(WebSocketStream *stream, const QJsonObject &message);
	QJsonObject cmdPrepare(WebSocketStream *stream, const QJsonObject &message);
	QJsonObject cmdPlay(WebSocketStream *stream, const QJsonObject &message);

	bool userCanConnect(const QString &username) const;


	virtual bool canDelete(const int &useCount) override;
	virtual void timerTick() override;
	virtual void triggerEvent() override;

	qint64 currentTick() const;

	static int increaseNextId() { return ++m_nextId; }
	static int setNextId(const int &id) { m_nextId = id+1; return m_nextId; }

	int playerEnroll(WebSocketStream *stream, const QString &character);
	bool playerLeave(WebSocketStream *stream, const bool &forced);
	bool playerConnectStream(const int &playerId, WebSocketStream *stream);
	void playerDisconnectStream(const int &playerId, WebSocketStream *stream = nullptr);
	void playerDisconnectStream(WebSocketStream *stream = nullptr) { playerDisconnectStream(-1, stream); }
	int playerGetId(const QString &username) const;
	WebSocketStream *playerGetStream(const QString &username) const;

	WebSocketStream *hostStream() const;
	void setHostStream(WebSocketStream *newHostStream);

	ConquestConfig config() const;
	void setConfig(const ConquestConfig &newConfig);

protected:
	static void sendStreamJson(WebSocketStream *stream, const QJsonValue &value);

	//virtual void streamLinkedEvent(WebSocketStream *stream) override;
	virtual void streamUnlinkedEvent(WebSocketStream *stream) override;
	//virtual void onBinaryMessageReceived(const QByteArray &data, WebSocketStream *stream) override;

private:
	static QJsonObject cmdList(WebSocketStream *stream, const QJsonObject &message, EngineHandler *handler);
	static QJsonObject cmdCreate(WebSocketStream *stream, const QJsonObject &message, EngineHandler *handler);
	static QJsonObject cmdConnect(WebSocketStream *stream, EngineHandler *handler, const int &id, const bool &forced);

	void engineBackup();
	bool engineRestore(const QJsonObject &data);
	static QString engineBackupFile(const ServerService *service, const int &id, const bool &createDir = false);
	QString engineBackupFile(const bool &createDir = false) const {
		return engineBackupFile(m_service, m_id, createDir);
	}

	QJsonObject cmdState(WebSocketStream *stream, const QJsonObject &message);
	QJsonObject cmdEnroll(WebSocketStream *stream, const QJsonObject &message);
	QJsonObject cmdLeave(WebSocketStream *stream, const QJsonObject &message);
	QJsonObject cmdQuestionRequest(WebSocketStream *stream, const QJsonObject &message);
	QJsonObject cmdPick(WebSocketStream *stream, const QJsonObject &message);
	QJsonObject cmdAnswer(WebSocketStream *stream, const QJsonObject &message);

	auto playerFind(const QString &username);
	auto playerFind(const QString &username) const;
	auto playerFind(const int &id);
	auto playerFind(WebSocketStream *stream);

	bool playerExists(const QString &username) const;

	void onPlayerPrepared(WebSocketStream *stream);
	QJsonArray playersToArray() const;
	QString userGetFullNickName(const QString &username);

	bool prepareWorld(const QString &world);
	void pickLands();
	void prepareTurns(const ConquestTurn::Stage &stage);
	void preparePlayerOrder();
	bool checkTurn();
	void nextSubStage();
	void playBegin();
	void gameFinish(const bool &hasError);

	bool questionNext();
	void questionClear();
	bool nextPick(const bool &subStage);
	bool nextBattle(const bool &subStage, const bool &isLastRound);
	QStringList getPickableLands(const int &playerId);
	void checkAndRemovePlayersTurns();
	bool checkOtherPlayerAvailable();
	void resetPlayers();


	WebSocketStream *m_hostStream = nullptr;
	QElapsedTimer m_elapsedTimer;
	bool m_elapsedTimerStartRequired = false;
	qint64 m_startedAt = -1;
	qint64 m_tickBegin = 0;
	std::vector<ConquestEnginePlayer> m_players;
	ConquestConfig m_config;
	ConquestWordListHelper m_worldListHelper;
	static int m_nextId;
	std::unique_ptr<ConquestQuestion> m_question;
	qint64 m_lastStateSent = 0;

	friend class ConquestQuestion;
};




/**
 * @brief ConquestEngine::playerFind
 * @param username
 */

inline auto ConquestEngine::playerFind(const QString &username)
{
	return std::find_if(m_players.begin(), m_players.end(), [username](const ConquestEnginePlayer &p) {
		return (p.username == username);
	});
}


/**
 * @brief ConquestEngine::playerFind
 * @param username
 */

inline auto ConquestEngine::playerFind(const QString &username) const
{
	return std::find_if(m_players.cbegin(), m_players.cend(), [username](const ConquestEnginePlayer &p) {
		return (p.username == username);
	});
}


/**
 * @brief ConquestEngine::playerFind
 * @param id
 */

inline auto ConquestEngine::playerFind(const int &id)
{
	return std::find_if(m_players.begin(), m_players.end(), [id](const ConquestEnginePlayer &p) {
		return (p.playerId == id);
	});
}


/**
 * @brief ConquestEngine::playerFind
 * @param stream
 */

inline auto ConquestEngine::playerFind(WebSocketStream *stream)
{
	return std::find_if(m_players.begin(), m_players.end(), [stream](const ConquestEnginePlayer &p) {
		return (p.stream == stream);
	});
}

#endif // CONQUESTENGINE_H
