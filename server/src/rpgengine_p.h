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



/**
 * @brief The RpgEnginePrivate class
 */

class RpgEnginePrivate
{
private:
	RpgEnginePrivate(RpgEngine *engine)
		: q(engine)
		, m_logger(new Logger(QStringLiteral("engineprivate"), false))
		, m_closeTimer(-1)
	{}



	/**
	 * @brief The RpgPeerData class
	 */

	struct RpgPeerData : public UdpPeerData
	{
		RpgStream::ConnectionToken token;
		RpgStream::Team team = RpgStream::TeamNone;
		UdpServerPeer *peer = nullptr;

		RpgStream::PlayerData data;

		quint32 rpgId = 0;										// A hosszú peerId helyett ez lesz az RpgLogic-ban a player sorszáma (PlayerData::playerId)
		quint32 playerTag = 0;

		Rpg::EventWindowHash acceptedTags;

		void loadToken() {
			token = {};
			token.fromJson(connectionToken);
		}
	};


	quint32 tick() const { return m_elapsedTimer.isValid() ? (m_elapsedTimerReference + m_elapsedTimer.elapsed()*60./1000.) : 0; }
	bool running() const { return m_elapsedTimer.isValid(); }

	void start(const qint64 &startTick = 0) {
		m_elapsedTimerReference = startTick;
		m_elapsedTimer.start();
	}

	quint32 stop() {
		quint32 t = tick();
		m_elapsedTimer.invalidate();
		return t;
	}


	RpgPeerData *getPlayer(UdpServerPeer *peer);

	void render();


	RpgStream::Team nextTeam() const;
	quint32 changeHost();

	void receiveCharacterSelect(RpgEnginePrivate::RpgPeerData *player, RpgStream::EngineDataStream &&stream);
	void sendCharacterSelect(const bool reliable = false);
	void checkCompleted();
	void onAllCompleted();

	void receivePlayerData(RpgEnginePrivate::RpgPeerData *player, RpgStream::EngineDataStream &&stream);

	void receiveWaitingData(RpgEnginePrivate::RpgPeerData *player, RpgStream::EngineDataStream &&stream);
	void sendWaitingData();

	void receiveFull(RpgEnginePrivate::RpgPeerData *player, RpgStream::EngineDataStream &&stream);
	void onDataReceived();
	void sendFull();

	void checkPrepared();
	void onAllPrepared();

	void receiveState(RpgEnginePrivate::RpgPeerData *player, RpgStream::EngineDataStream &&stream);

	void onSelectFinished();
	void onAborted();


private:
	RpgEngine *q;

	std::unique_ptr<Logger> m_logger;
	Logger *_logger() const { return m_logger.get(); }

	QMap<quint32, RpgPeerData> m_players;
	quint32 m_host = 0;

	QElapsedTimer m_elapsedTimer;
	qint64 m_elapsedTimerReference = 0;

	QElapsedTimer m_selectTimer;
	QDeadlineTimer m_closeTimer;

	qint64 m_dtAcc = 0;
	quint32 m_deadlineTick = 0;

	quint32 m_wsCounter = 0;


	inline static quint32 m_engineId = 1;










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






#endif // RPGENGINE_P_H
