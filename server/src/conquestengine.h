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

private:
	ConquestEngine *m_engine = nullptr;
	QLambdaThreadWorker m_worker;
	QMutex m_mutex;
	QJsonArray m_questionArray;
	qint64 m_lastRequest = 0;
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

	/**
	 * @brief The Player class
	 */

	struct Player {
		Player(const int &_id, const QString &_user, WebSocketStream *_stream) : id(_id), username(_user), stream(_stream) {}
		Player(const int &_id, WebSocketStream *_stream) : id(_id), stream(_stream) {}
		Player(const int &_id) : id(_id) {}
		virtual ~Player() = default;

		QJsonObject toJson() const {
			QJsonObject o;
			o[QStringLiteral("id")] = id;
			o[QStringLiteral("username")] = username;
			o[QStringLiteral("prepared")] = prepared;
			o[QStringLiteral("xp")] = xp;
			return o;
		}

		int id = -1;
		QString username;
		WebSocketStream *stream = nullptr;
		bool prepared = false;
		int xp = 0;
	};


	static void handleWebSocketMessage(WebSocketStream *stream, const QJsonValue &message, EngineHandler *handler);

	static std::weak_ptr<ConquestEngine> createEngine(WebSocketStream *stream, EngineHandler *handler);
	static std::weak_ptr<AbstractEngine> connectToEngine(const int &id, WebSocketStream *stream, EngineHandler *handler);


	QJsonObject gameFinish(WebSocketStream *stream, const QJsonObject &message);
	QJsonObject gameStart(WebSocketStream *stream, const QJsonObject &message);
	QJsonObject gamePrepare(WebSocketStream *stream, const QJsonObject &message);
	QJsonObject gamePlay(WebSocketStream *stream, const QJsonObject &message);


	virtual bool canDelete(const int &useCount) override;
	virtual void timerTick() override;
	virtual void streamTriggerEvent(WebSocketStream *stream) override;

	qint64 currentTick() const;

	static int increaseNextId() { return ++m_nextId; }
	static int setNextId(const int &id) { m_nextId = id+1; return m_nextId; }

	int playerEnroll(WebSocketStream *stream);
	bool playerConnectStream(const int &playerId, WebSocketStream *stream);
	void playerDisconnectStream(const int &playerId, WebSocketStream *stream = nullptr);
	void playerDisconnectStream(WebSocketStream *stream = nullptr) { playerDisconnectStream(-1, stream); }
	std::optional<Player> playerGet(WebSocketStream *stream) const;
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
	static QJsonObject cmdList(const QJsonObject &message, EngineHandler *handler);
	static QJsonObject cmdCreate(WebSocketStream *stream, const QJsonObject &message, EngineHandler *handler);
	static QJsonObject cmdConnect(WebSocketStream *stream, EngineHandler *handler, const int &id);

	QJsonObject cmdState(WebSocketStream *stream, const QJsonObject &message);
	QJsonObject cmdEnroll(WebSocketStream *stream, const QJsonObject &message);
	QJsonObject cmdQuestionRequest(WebSocketStream *stream, const QJsonObject &message);
	QJsonObject cmdTest(WebSocketStream *stream, const QJsonObject &message);

	auto playerFind(const QString &username) const;
	auto playerFind(const int &id) const;
	auto playerFind(WebSocketStream *stream) const;
	void onPlayerPrepared(WebSocketStream *stream);

	void updatePlayerLimit();


	WebSocketStream *m_hostStream = nullptr;
	QElapsedTimer m_elapsedTimer;
	qint64 m_startedAt = -1;
	std::vector<Player> m_players;
	ConquestConfig m_config;
	QList<ConquestWorld> m_worldList;
	static int m_nextId;
	std::unique_ptr<ConquestQuestion> m_question;

	friend class ConquestQuestion;
};




/**
 * @brief ConquestEngine::playerFind
 * @param username
 */

inline auto ConquestEngine::playerFind(const QString &username) const
{
		return std::find_if(m_players.begin(), m_players.end(), [username](const Player &p) {
			return (p.username == username);
		});
}


/**
 * @brief ConquestEngine::playerFind
 * @param id
 */

inline auto ConquestEngine::playerFind(const int &id) const
{
	return std::find_if(m_players.begin(), m_players.end(), [id](const Player &p) {
		return (p.id == id);
	});
}


/**
 * @brief ConquestEngine::playerFind
 * @param stream
 */

inline auto ConquestEngine::playerFind(WebSocketStream *stream) const
{
	return std::find_if(m_players.begin(), m_players.end(), [stream](const Player &p) {
		return (p.stream == stream);
	});
}

#endif // CONQUESTENGINE_H
