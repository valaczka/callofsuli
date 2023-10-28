/*
 * ---- Call of Suli ----
 *
 * mapplaycampaign.h
 *
 * Created on: 2023. 04. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapPlayCampaign
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

#ifndef MAPPLAYCAMPAIGN_H
#define MAPPLAYCAMPAIGN_H

#include "mapplay.h"
#include "studentmaphandler.h"
#include "campaign.h"
#include "actiongame.h"
#include "litegame.h"
#include "testgame.h"


/**
 * @brief The MapPlayCampaign class
 */

class MapPlayCampaign : public MapPlay
{
	Q_OBJECT

public:
	explicit MapPlayCampaign(StudentMapHandler *handler, QObject *parent = nullptr);
	virtual ~MapPlayCampaign();

	bool load(Campaign *campaign, StudentMap *map);

	Q_INVOKABLE virtual void updateSolver() override;

signals:
	void gameIdChanged();

protected:
	virtual AbstractLevelGame *createLevelGame(MapPlayMissionLevel *level, const GameMap::GameMode &mode) override;
	virtual void onCurrentGamePrepared() override;
	virtual void onCurrentGameFinished() override;

private slots:
	void onUpdateTimerTimeout();

private:
	void destroyCurrentGame();

	QPointer<StudentMapHandler> m_handler = nullptr;
	QPointer<Campaign> m_campaign = nullptr;
	QTimer m_updateTimer;
	int m_lastXP = -1;
};




/**
 * @brief The CampaignGameIface class
 */

class CampaignGameIface
{
public:
	CampaignGameIface() {}

	const int &gameId() const { return m_gameId; }
	void setGameId(int newGameId) { m_gameId = newGameId; }

	const QJsonObject &serverExtended() const { return m_serverExtended; }
	void setServerExtended(const QJsonObject &newServerExtended) { m_serverExtended = newServerExtended; }

protected:
	int m_gameId = -1;
	QJsonObject m_serverExtended;
};



/**
 * @brief The CampaignLevelGame class
 */

class CampaignActionGame : public ActionGame, public CampaignGameIface
{
	Q_OBJECT

public:
	explicit CampaignActionGame(GameMapMissionLevel *missionLevel, Client *client)
		: ActionGame(missionLevel, client) {}
	virtual ~CampaignActionGame() {}

	virtual QJsonObject getServerExtendedData() const { return m_serverExtended; };
};




/**
 * @brief The CampaignActionGame class
 */

class CampaignLiteGame : public LiteGame, public CampaignGameIface
{
	Q_OBJECT

public:
	explicit CampaignLiteGame(GameMapMissionLevel *missionLevel, Client *client, const bool &isPractice = false)
		: LiteGame(missionLevel, client, isPractice) {}
	virtual ~CampaignLiteGame() {}
};






/**
 * @brief The CampaignTestGame class
 */

class CampaignTestGame : public TestGame, public CampaignGameIface
{
	Q_OBJECT

public:
	explicit CampaignTestGame(GameMapMissionLevel *missionLevel, Client *client)
		: TestGame(missionLevel, client) {}
	virtual ~CampaignTestGame() {}
};



#endif // MAPPLAYCAMPAIGN_H
