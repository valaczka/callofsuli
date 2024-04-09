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


/**
 * @brief The ActionRpgGame class
 */

class ActionRpgGame : public AbstractLevelGame
{
	Q_OBJECT

	Q_PROPERTY(GameMode gameMode READ gameMode WRITE setGameMode NOTIFY gameModeChanged FINAL)
	Q_PROPERTY(RpgConfig config READ config WRITE setConfig NOTIFY configChanged FINAL)
	Q_PROPERTY(RpgGame *rpgGame READ rpgGame WRITE setRpgGame NOTIFY rpgGameChanged FINAL)
	Q_PROPERTY(RpgPlayerConfig playerConfig READ playerConfig WRITE setPlayerConfig NOTIFY playerConfigChanged FINAL)
	Q_PROPERTY(qreal downloadProgress READ downloadProgress NOTIFY downloadProgressChanged FINAL)
	Q_PROPERTY(QVariantList characterList READ characterList CONSTANT FINAL)

public:
	explicit ActionRpgGame(GameMapMissionLevel *missionLevel, Client *client);
	virtual ~ActionRpgGame();

	enum GameMode {
		SinglePlayer = 0,
		MultiPlayerHost,
		MultiPlayerGuest
	};

	Q_ENUM(GameMode);

	virtual void gameAbort() override;

	Q_INVOKABLE void playMenuBgMusic();
	Q_INVOKABLE void stopMenuBgMusic();

	Q_INVOKABLE void selectCharacter(const QString &character);
	Q_INVOKABLE void rpgGameActivated();

	Q_INVOKABLE void finishGame();
	Q_INVOKABLE void gamePrepared();

	RpgConfig config() const;
	void setConfig(const RpgConfig &newConfig);

	RpgGame *rpgGame() const;
	void setRpgGame(RpgGame *newRpgGame);

	RpgPlayerConfig playerConfig() const;
	void setPlayerConfig(const RpgPlayerConfig &newPlayerConfig);

	GameMode gameMode() const;
	void setGameMode(const GameMode &newGameMode);

	qreal downloadProgress() const;

	QVariantList characterList() const;

signals:
	void finishDialogRequest(QString text, QString icon, bool success);
	void configChanged();
	void rpgGameChanged();
	void playerConfigChanged();
	void gameModeChanged();
	void downloadProgressChanged();

protected:
	virtual QQuickItem* loadPage() override;
	virtual void timerEvent(QTimerEvent *) override;
	virtual void connectGameQuestion() override;
	virtual bool gameStartEvent() override;
	virtual bool gameFinishEvent() override;

	virtual void onPlayerDead(RpgPlayer *player);

	void onGameTimeout();
	void onGameSuccess();
	void onGameFailed();


private:
	void updateConfig();
	void onConfigChanged();
	void downloadGameData();
	void setError();
	void onMsecLeftChanged();

	void loadInventory(RpgPlayer *player);
	void loadInventory(RpgPlayer *player, const RpgPickableObject::PickableType &pickableType);

	bool onPlayerPick(RpgPlayer *player, RpgPickableObject *pickable);
	bool onPlayerAttackEnemy(RpgPlayer *player, IsometricEnemy *enemy, const TiledWeapon::WeaponType &weaponType);
	void onQuestionSuccess(RpgPlayer *player, IsometricEnemy *enemy, int xp);
	void onQuestionFailed(RpgPlayer *player, IsometricEnemy *enemy);

private:
	GameMode m_gameMode = SinglePlayer;
	RpgConfig m_config;
	RpgGame *m_rpgGame = nullptr;
	RpgPlayerConfig m_playerConfig;
	std::unique_ptr<RpgQuestion> m_rpgQuestion;

	RpgConfig::GameState m_oldGameState = RpgConfig::StateInvalid;
	int m_loadableContentCount = 0;
	qreal m_downloadProgress = 0.;

	int m_msecNotifyAt = 0;

	friend class RpgQuestion;
};

#endif // ACTIONRPGGAME_H
