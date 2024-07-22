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
#include "downloader.h"
#include "rpgconfig.h"
#include "rpggame.h"
#include "rpguserwallet.h"


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
	Q_PROPERTY(Downloader *downloader READ downloader CONSTANT FINAL)
	Q_PROPERTY(int gameid READ gameid CONSTANT FINAL)

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

	Q_INVOKABLE void selectCharacter(const QString &terrain, const QString &character, const QStringList &weaponList);
	Q_INVOKABLE void rpgGameActivated();

	Q_INVOKABLE void finishGame();
	Q_INVOKABLE void gamePrepared();

	Q_INVOKABLE void clearSharedTextures();

	Q_INVOKABLE void addWallet(RpgUserWallet *wallet);

	Q_INVOKABLE void saveTerrainInfo() { RpgGame::saveTerrainInfo(); }

	RpgConfig config() const;
	void setConfig(const RpgConfig &newConfig);

	RpgGame *rpgGame() const;
	void setRpgGame(RpgGame *newRpgGame);

	RpgPlayerConfig playerConfig() const;
	void setPlayerConfig(const RpgPlayerConfig &newPlayerConfig);

	GameMode gameMode() const;
	void setGameMode(const GameMode &newGameMode);

	Downloader *downloader() const;

	int gameid() const;

signals:
	void marketRequest();
	void marketLoaded();
	void marketUnloaded();
	void finishDialogRequest(QString text, QString icon, bool success);
	void configChanged();
	void rpgGameChanged();
	void playerConfigChanged();
	void gameModeChanged();

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
	void onGameLoadFailed(const QString &);


private:
	void rpgGameActivated_();
	void updateConfig();
	void onConfigChanged();
	void downloadGameData();
	void setError();
	void onMsecLeftChanged();

	void downloadLoadableContentDict(const QStringList &fileList);
	void downloadLoadableContent(const QStringList &fileList);

	void loadInventory(RpgPlayer *player);
	void loadInventory(RpgPlayer *player, const RpgPickableObject::PickableType &pickableType);
	void loadWeapon(RpgPlayer *player, const TiledWeapon::WeaponType &type, const int &bullet = 0);
	void loadBullet(RpgPlayer *player, const RpgPickableObject::PickableType &bulletType, const int &count);

	bool onPlayerPick(RpgPlayer *player, RpgPickableObject *pickable);
	bool onPlayerAttackEnemy(RpgPlayer *player, IsometricEnemy *enemy, const TiledWeapon::WeaponType &weaponType);
	bool onPlayerUseContainer(RpgPlayer *player, TiledContainer *container);
	bool onPlayerUseCast(RpgPlayer *player);
	bool onPlayerFinishCast(RpgPlayer *player);
	void onQuestionSuccess(RpgPlayer *player, IsometricEnemy *enemy, TiledContainer *container, int xp);
	void onQuestionFailed(RpgPlayer *player, IsometricEnemy *enemy, TiledContainer *container);

private:
	GameMode m_gameMode = SinglePlayer;
	RpgConfig m_config;
	RpgGame *m_rpgGame = nullptr;
	RpgPlayerConfig m_playerConfig;
	std::unique_ptr<RpgQuestion> m_rpgQuestion;
	std::unique_ptr<Downloader> m_downloader;

	QJsonObject m_loadableContentDict;
	QVector<Server::DynamicContent> m_loadableContentListBase;

	RpgConfig::GameState m_oldGameState = RpgConfig::StateInvalid;

	int m_msecNotifyAt = 0;
	int m_tmpSoundSfxVolume = 0;

	friend class RpgQuestion;
};

#endif // ACTIONRPGGAME_H
