/*
 * ---- Call of Suli ----
 *
 * game.h
 *
 * Created on: 2022. 12. 11.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Game
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

#ifndef ABSTRACTGAME_H
#define ABSTRACTGAME_H

#include "qbasictimer.h"
#include "qelapsedtimer.h"
#include "qquickitem.h"
#include "gamemap.h"
#include <QObject>

class Client;
class GameQuestion;


#if QT_VERSION >= 0x060000

#ifndef OPAQUE_PTR_Client
#define OPAQUE_PTR_Client
  Q_DECLARE_OPAQUE_POINTER(Client*)
#endif

#ifndef OPAQUE_PTR_GameQuestion
#define OPAQUE_PTR_GameQuestion
  Q_DECLARE_OPAQUE_POINTER(GameQuestion*)
#endif

#endif

/**
 * @brief The Game class
 */

class AbstractGame : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QQuickItem *pageItem READ pageItem WRITE setPageItem NOTIFY pageItemChanged)
	Q_PROPERTY(GameMap* map READ map WRITE setMap NOTIFY mapChanged)
	Q_PROPERTY(GameQuestion *gameQuestion READ gameQuestion WRITE setGameQuestion NOTIFY gameQuestionChanged)
	Q_PROPERTY(GameMap::GameMode mode READ mode CONSTANT)
	Q_PROPERTY(FinishState finishState READ finishState WRITE setFinishState NOTIFY finishStateChanged)

public:

	enum FinishState {
		Invalid = 0,
		Neutral,
		Success,
		Fail
	};

	Q_ENUM(FinishState);

	// Statistics

	struct Statistics {
		int id = -1;
		QString module;
		QString objective;
		bool success = false;
		int elapsed = 0;
		bool uploaded = false;
	};


	// TickTimer

	class TickTimer {
	public:
		TickTimer() {}

		void start(AbstractGame *game, const qint64 &startTick = 0) {
			m_reference.invalidate();
			m_reference.start();
			m_startTick = startTick;
			if (!m_timer.isActive())
				m_timer.start(m_interval, Qt::PreciseTimer, game);
		}

		void stop() {
			m_reference.invalidate();
			m_timer.stop();
		}

		bool isValid() const { return m_reference.isValid(); }

		const qint64 &latency() const { return m_latency; }
		void setLatency(const qint64 &latency) { m_latency = latency; }

		qint64 currentTick() const {
			if (!m_reference.isValid())
				return -1;

			const qint64 &elapsed = m_reference.elapsed();
			return m_startTick+elapsed+m_latency;
		}

		static int interval() { return m_interval; }

	private:
		QBasicTimer m_timer;
		QElapsedTimer m_reference;
		qint64 m_startTick = 0;
		qint64 m_latency = 0;
		static const int m_interval;
	};

	explicit AbstractGame(const GameMap::GameMode &mode, Client *client);
	virtual ~AbstractGame();

	QQuickItem *pageItem() const;
	void setPageItem(QQuickItem *newPageItem);

	const GameMap::GameMode &mode() const;

	GameMap *map() const;
	void setMap(GameMap *newMap);

	Q_INVOKABLE void unloadPageItem();

	void addStatistics(const Statistics &stat);
	void addStatistics(const QString &module, const QString &uuid, const bool &success, const int &elapsed);
	QJsonArray getStatistics();
	void clearStatistics(const QJsonArray &list, const bool &revert = false);

	virtual QJsonObject getExtendedData() const { return QJsonObject(); };
	virtual QJsonObject getServerExtendedData() const { return QJsonObject(); };

	GameQuestion *gameQuestion() const;
	void setGameQuestion(GameQuestion *newGameQuestion);

	int elapsedMsec() const;

	bool readyToDestroy() const;
	void setReadyToDestroy(bool newReadyToDestroy);

	const FinishState &finishState() const;
	void setFinishState(const FinishState &newFinishState);

public slots:
	bool load();
	bool gameStart();
	bool gameFinish();
	virtual void gameAbort() {}

protected:
	virtual QQuickItem *loadPage() = 0;
	virtual void connectGameQuestion() = 0;
	virtual bool gameStartEvent() { return true; };
	virtual bool gameFinishEvent() { return true; };
	virtual void gameDestroy();

private slots:
	void onPageItemDestroyed();

signals:
	void pageItemChanged();
	void mapChanged();
	void gameQuestionChanged();
	void gameFinished(AbstractGame::FinishState state);
	void gameDestroyRequest();
	void finishStateChanged();

protected:
	Client *m_client = nullptr;
	QQuickItem *m_pageItem = nullptr;
	const GameMap::GameMode m_mode;
	GameMap *m_map = nullptr;
	GameQuestion *m_gameQuestion = nullptr;
	QVector<Statistics> m_statistics;
	int m_elapsedMsec = -1;

private:
	void elapsedTimeStart();
	void elapsedTimeStop();

	QElapsedTimer m_elapsedTimer;
	FinishState m_finishState = Invalid;
	bool m_readyToDestroy = false;
	int m_statId = 0;
};

#endif // ABSTRACTGAME_H
