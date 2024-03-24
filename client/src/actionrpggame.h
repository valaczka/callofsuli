/*
 * ---- Call of Suli ----
 *
 * actionrpggame.h
 *
 * Created on: 2024. 03. 24.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ActionRpgGame
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

#ifndef ACTIONRPGGAME_H
#define ACTIONRPGGAME_H

#include "abstractlevelgame.h"
#include "rpgconfig.h"
#include "rpggame.h"

class ActionRpgGame : public AbstractLevelGame
{
	Q_OBJECT

	Q_PROPERTY(RpgConfig config READ config WRITE setConfig NOTIFY configChanged FINAL)
	Q_PROPERTY(RpgGame *rpgGame READ rpgGame WRITE setRpgGame NOTIFY rpgGameChanged FINAL)
	Q_PROPERTY(RpgPlayerConfig playerConfig READ playerConfig WRITE setPlayerConfig NOTIFY playerConfigChanged FINAL)

public:
	explicit ActionRpgGame(GameMapMissionLevel *missionLevel, Client *client);
	virtual ~ActionRpgGame();

	virtual void gameAbort() override;

	Q_INVOKABLE void playMenuBgMusic();
	Q_INVOKABLE void stopMenuBgMusic();

	Q_INVOKABLE void selectCharacter(const QString &character);
	Q_INVOKABLE void rpgGameActivated();

	RpgConfig config() const;
	void setConfig(const RpgConfig &newConfig);

	RpgGame *rpgGame() const;
	void setRpgGame(RpgGame *newRpgGame);

	RpgPlayerConfig playerConfig() const;
	void setPlayerConfig(const RpgPlayerConfig &newPlayerConfig);

signals:
	void configChanged();
	void rpgGameChanged();
	void playerConfigChanged();

protected:
	virtual QQuickItem* loadPage() override;
	virtual void timerEvent(QTimerEvent *) override;
	virtual void connectGameQuestion() override;
	virtual bool gameStartEvent() override;
	virtual bool gameFinishEvent() override;

	virtual void onGamePrepared();

	void onGameTimeout();
	void onGameSuccess();
	void onGameFailed();

private:
	void onConfigChanged();

protected:
	RpgConfig m_config;
	RpgGame *m_rpgGame = nullptr;
	RpgPlayerConfig m_playerConfig;

private:
	RpgConfig::GameState m_oldGameState = RpgConfig::StateInvalid;

};

#endif // ACTIONRPGGAME_H
