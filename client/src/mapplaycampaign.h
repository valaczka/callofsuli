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

	Q_INVOKABLE void reloadSolver();

signals:
	void gameIdChanged();

protected:
	virtual AbstractLevelGame *createLevelGame(MapPlayMissionLevel *level, const GameMap::GameMode &mode) override;
	virtual void onCurrentGamePrepared() override;
	virtual void onCurrentGameFinished() override;

private:
	void destroyCurrentGame();

	QPointer<StudentMapHandler> m_handler = nullptr;
	QPointer<Campaign> m_campaign = nullptr;
};




/**
 * @brief The CampaignLevelGame class
 */

class CampaignActionGame : public ActionGame
{
	Q_OBJECT

	Q_PROPERTY(int gameId READ gameId WRITE setGameId NOTIFY gameIdChanged)

public:
	explicit CampaignActionGame(GameMapMissionLevel *missionLevel, Client *client);
	virtual ~CampaignActionGame();

	int gameId() const;
	void setGameId(int newGameId);

signals:
	void gameIdChanged();

private:
	int m_gameId = -1;
};

#endif // MAPPLAYCAMPAIGN_H
