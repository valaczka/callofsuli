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

#include "qelapsedtimer.h"
#include "qloggingcategory.h"
#include "qquickitem.h"
#include "gamemap.h"
#include <QObject>

class Client;
class GameQuestion;

/**
 * @brief The Game class
 */

class AbstractGame : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QQuickItem *pageItem READ pageItem WRITE setPageItem NOTIFY pageItemChanged)
	Q_PROPERTY(GameMap* map READ map WRITE setMap NOTIFY mapChanged)
	Q_PROPERTY(GameQuestion *gameQuestion READ gameQuestion WRITE setGameQuestion NOTIFY gameQuestionChanged)

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
		QString objective;
		bool success;
		int elapsed;
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
	void addStatistics(const QString &uuid, const bool &success, const int &elapsed);
	QJsonArray takeStatistics();

	GameQuestion *gameQuestion() const;
	void setGameQuestion(GameQuestion *newGameQuestion);

	int elapsedMsec() const;

	void setFinishState(const FinishState &newFinishState);
	const FinishState &finishState() const;

	bool readyToDestroy() const;
	void setReadyToDestroy(bool newReadyToDestroy);


public slots:
	bool load();
	bool gameStart();
	bool gameFinish();

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

};

Q_DECLARE_LOGGING_CATEGORY(lcGame)

#endif // ABSTRACTGAME_H
