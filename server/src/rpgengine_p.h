/*
 * ---- Call of Suli ----
 *
 * rpgengine_p.h
 *
 * Created on: 2025. 07. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#ifndef RPGENGINE_P_H
#define RPGENGINE_P_H


#include "rpgengine.h"
#include "userapi.h"




/**
 * @brief The RpgEngineMessage class
 */

class RpgEngineMessage
{
public:
	RpgEngineMessage(const RpgGameData::Message &msg, const QList<int> &players = {}, const bool &inverse = false)
		: m_message(msg)
		, m_players(players)
		, m_inverse(inverse)
	{}

	const RpgGameData::Message &message() const { return m_message; }
	const QList<int> &players() const { return m_players; }
	const bool &inverse() const { return m_inverse; }

	const QSet<int> &sent() const { return m_sent; }
	void addSent(const int &id) { m_sent.insert(id); }

	bool canSend(const int &id) const {
		if (m_sent.contains(id))
			return false;

		const bool &ct = m_players.contains(id);

		return (m_players.isEmpty() ||
				(!m_inverse && ct) ||
				(m_inverse && !ct))
				;

	}

private:
	RpgGameData::Message m_message;
	QList<int> m_players;
	bool m_inverse;
	QSet<int> m_sent;
};






/**
 * @brief The RpgEnginePrivate class
 */

class RpgEnginePrivate
{
private:
	RpgEnginePrivate(RpgEngine *engine)
		: m_removeTimer(-1)
		, q(engine)
		, m_logger(new Logger(QStringLiteral("engineprivate"), false))
	{}


	static void sendEngineList(const RpgConfigBase &config, UdpServerPeer *peer, EngineHandler *handler);

	static bool canConnect(const qint64 &peerID, const RpgConfigBase &config, RpgEngine *engine);

	RpgEnginePlayer* getPlayer(UdpServerPeer *peer) const;
	RpgEnginePlayer* getPlayer(const int &playerId) const;

	void dataReceived(RpgEnginePlayer *player, const QByteArray &data, const qint64 &diff);
	void dataReceivedChrSel(RpgEnginePlayer *player, const QByteArray &data);
	void dataReceivedPrepare(RpgEnginePlayer *player, const QByteArray &data);
	void dataReceivedPlay(RpgEnginePlayer *player, const QByteArray &data, const qint64 &diff);
	void dataReceivedFinished(RpgEnginePlayer *player, const QByteArray &data, const qint64 &diff);

	bool updatePlayer(RpgEnginePlayer *player, const RpgGameData::CharacterSelect &data);

	enum SendMode {
		SendNone = 0,
		SendChrSel,
		SendPrepare,
		SendReconnect
	};

	void dataSend(const SendMode &mode, RpgEnginePlayer *player = nullptr);
	void dataSendPlay();
	void dataSendFinished();

	void insertBaseMapData(QCborMap *dst, RpgEnginePlayer *player);
	bool insertMessages(QCborMap *dst, RpgEnginePlayer *player);
	void clearMessages();

	void updateState();

	void updatePeers();
	bool reconnectPeer(UdpServerPeer *peer);
	bool abortPlayer(const quint32 &peerId);
	bool banOutPlayer(RpgEnginePlayer *player);



	void createEnemies(const RpgGameData::CurrentSnapshot &snapshot);
	void createControls(const RpgGameData::CurrentSnapshot &snapshot);
	void createCollection();
	int relocateCollection(const RpgGameData::ControlCollectionBaseData &base, const qint64 &tick, QPointF *ptr);
	bool finishCollection(const RpgGameData::ControlCollectionBaseData &base, const int &idx);

	void createRandomizer();


	template <typename T, typename ...Args,
			  typename = std::enable_if<std::is_base_of<RpgEventBase, T>::value>::type>
	void eventAdd(Args &&...args);

	template <typename T, typename ...Args,
			  typename = std::enable_if<std::is_base_of<RpgEventBase, T>::value>::type>
	void eventAddLater(Args &&...args);

	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<RpgEventBase, T>::value>::type>
	T* eventFind(const T2 &baseData);

	void eventRemove(RpgEventBase *event);


	RpgGameData::CurrentSnapshot processEvents(const qint64 &tick);

	UserAPI::UserGame toUserGame() const;
	bool gameCreate(RpgEnginePlayer *player);
	bool gameFinish();
	bool gameUpdate(RpgEnginePlayer *player);
	bool gameAbort(RpgEnginePlayer *player);


	RpgGameData::GameConfig m_gameConfig;
	std::optional<RpgGameData::Randomizer> m_randomizer;

	QElapsedTimer m_elapsedTimer;
	qint64 m_elapsedTimerReference = 0;
	qint64 m_lastSentTick = -1;
	qint64 m_lastReliable = -1;
	int m_lastMyObjectId = 0;

	float m_avgCollectionMsec = 0;
	int m_collectionGenerated = -1;

	bool m_playerDataModified = false;

	std::vector<std::unique_ptr<RpgEventBase>> m_events;
	std::vector<std::unique_ptr<RpgEventBase>> m_eventsLater;
	QList<RpgEventBase*> m_eventsRemove;
	bool m_eventsProcessing = false;

	QDeadlineTimer m_removeTimer;

	bool m_locked = false;
	QList<qint64> m_banList;
	QList<qint64> m_abortList;


	QList<RpgEngineMessage> m_messages;

	RpgEngine *q;

	std::unique_ptr<Logger> m_logger;
	Logger *_logger() const { return m_logger.get(); }



	/// ---- MEASURE ----


	enum Measure {
		Invalid,
		Received,
		Render,
		RenderFull,
		TimerTick,
		TimerUpd,
		BinaryRcv
	};

	struct MeasureData {
		qint64 min = -1;
		qint64 max = -1;
		qint64 med = -1;
		QList<qint64> data;

		qint64 avg() const {
			if (data.size() > 0) {
				qint64 sum = std::accumulate(data.constBegin(), data.constEnd(), 0);
				return sum/data.size();
			} else {
				return 0;
			}
		}

		void add(const qint64 &ms) {
			if (data.size() > limit)
				data.erase(data.constBegin(), data.constBegin()+(data.size()-limit-1));
			data.append(ms);

			QList<qint64> tmp = data;

			std::sort(tmp.begin(), tmp.end());

			if (const auto &s = tmp.size(); s % 2 == 0)
				med = (tmp.at(s / 2 - 1) + tmp.at(s / 2)) / 2;
			else
				med = tmp.at(s / 2);

			if (min < 0 || ms < min)
				min = ms;

			if (max < 0 || ms > max)
				max = ms;
		}

		int limit = 120;
	};

	QHash<Measure, MeasureData> m_renderData;

	QElapsedTimer m_renderTimer;

	void renderTimerStart() {
		if (m_renderTimer.isValid())
			m_renderTimer.restart();
		else
			m_renderTimer.start();
	}

	void renderTimerMeausure(const Measure &measure) {
		m_renderData[measure].add(m_renderTimer.restart());
	}

	void renderTimerMeausure(const Measure &measure, const qint64 &msec) {
		m_renderData[measure].add(msec);
	}

	QString renderTimerDump() const;
	QString engineDump() const;

	friend class RpgEngine;
};









/**
 * @brief RpgEnginePrivate::eventAddLater
 * @param args
 */
template<typename T, typename ...Args, typename T3>
void RpgEnginePrivate::eventAddLater(Args &&...args)
{
	std::unique_ptr<T> e(new T(q, std::forward<Args>(args)...));

	for (const auto &ptr : m_events) {
		if (ptr->isUnique() && ptr->isEqual(e.get())) {
			ELOG_WARNING << "Event unique constraint failed";
			return;
		}
	}

	for (const auto &ptr : m_eventsLater) {
		if (ptr->isUnique() && ptr->isEqual(e.get())) {
			ELOG_WARNING << "Event unique constraint failed";
			return;
		}
	}

	m_eventsLater.push_back(std::move(e));
}




/**
 * @brief RpgEnginePrivate::eventAdd
 * @param args
 */

template<typename T, typename ...Args, typename T3>
void RpgEnginePrivate::eventAdd(Args &&...args)
{
	if (m_eventsProcessing) {
		eventAddLater<T>(std::forward<Args>(args)...);
		return;
	}

	std::unique_ptr<T> e(new T(q, std::forward<Args>(args)...));

	for (const auto &ptr : m_events) {
		if (ptr->isUnique() && ptr->isEqual(e.get())) {
			ELOG_ERROR << "Event unique constraint failed";
			return;
		}
	}

	m_events.push_back(std::move(e));
}



/**
 * @brief RpgEnginePrivate::eventFind
 * @param baseData
 * @return
 */

template<typename T, typename T2, typename T3>
T *RpgEnginePrivate::eventFind(const T2 &baseData)
{
	for (const auto &ptr : m_events) {
		if (T* e = dynamic_cast<T*>(ptr.get()); e && e->baseData() == baseData)
			return e;
	}

	return nullptr;
}





/**
 * @brief RpgEngine::eventAdd
 * @param args
 */

template<typename T, typename ...Args, typename T3>
void RpgEngine::eventAdd(Args &&...args)
{
	d->eventAdd<T>(std::forward<Args>(args)...);
}



template<typename T, typename ...Args, typename T3>
void RpgEngine::eventAddLater(Args &&...args)
{
	d->eventAddLater<T>(std::forward<Args>(args)...);
}



template<typename T, typename T2, typename T3>
T *RpgEngine::eventFind(const T2 &data)
{
	return d->eventFind<T>(data);
}



#endif // RPGENGINE_P_H
