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
	virtual QJsonObject getExtendedData() const override;
	virtual int msecLeft() const override;

	Q_INVOKABLE void playMenuBgMusic();
	Q_INVOKABLE void stopMenuBgMusic();

	Q_INVOKABLE void selectCharacter(const QString &terrain, const QString &character, const QStringList &weaponList);
	Q_INVOKABLE virtual void rpgGameActivated();

	Q_INVOKABLE void finishGame();
	Q_INVOKABLE virtual void gamePrepared();

	Q_INVOKABLE void clearSharedTextures();

	Q_INVOKABLE void addWallet(RpgUserWallet *wallet);

	Q_INVOKABLE void saveTerrainInfo() { RpgGame::saveTerrainInfo(); }

	Q_INVOKABLE static QStringList getDisabledWeapons(const QString &character);

	const RpgConfig &config() const;
	void setConfig(const RpgConfig &newConfig);

	RpgGame *rpgGame() const;
	virtual void setRpgGame(RpgGame *newRpgGame);

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
	void gameModeChanged();

protected:
	virtual void onConfigChanged();
	virtual QQuickItem* loadPage() override;
	virtual void timerEvent(QTimerEvent *) override;
	virtual void connectGameQuestion() override;
	virtual bool gameStartEvent() override;
	virtual bool gameFinishEvent() override;

	virtual void onPlayerDead(RpgPlayer *player);
	virtual void onTimerLeftTimeout() override;

	void onGameTimeout();
	void onGameSuccess();
	void onGameFailed();
	void onGameLoadFailed(const QString &);

	void loadInventory(RpgPlayer *player);
	void loadInventory(RpgPlayer *player, const RpgGameData::PickableBaseData::PickableType &pickableType);
	void loadWeapon(RpgPlayer *player, const RpgGameData::Weapon::WeaponType &type, const int &bullet = 0);

	void updateConfig();
	void setError();

	void downloadGameData(const QString &map, const QList<RpgGameData::CharacterSelect> &players);

	virtual void onTimeStepped();
	virtual bool onBodyStep(TiledObjectBody *body) { Q_UNUSED(body); return false; }
	virtual bool onPlayerPick(RpgPlayer *player, RpgPickableObject *pickable);
	virtual bool onPlayerAttackEnemy(RpgPlayer *player, RpgEnemy *enemy, const RpgGameData::Weapon::WeaponType &weaponType);
	virtual bool onPlayerUseContainer(RpgPlayer *player, RpgContainer *container);
	virtual bool onPlayerUseCast(RpgPlayer *player);
	virtual bool onPlayerCastTimeout(RpgPlayer *player);
	virtual bool onPlayerFinishCast(RpgPlayer *player);
	virtual bool onPlayerHit(RpgPlayer *player, RpgEnemy *enemy, RpgWeapon *weapon);
	virtual bool onPlayerShot(RpgPlayer *player, RpgWeapon *weapon, const qreal &angle);
	virtual bool onEnemyHit(RpgEnemy *enemy, RpgPlayer *player, RpgWeapon *weapon);
	virtual bool onEnemyShot(RpgEnemy *enemy, RpgWeapon *weapon, const qreal &angle);
	virtual bool onEnemyAttackPlayer(RpgEnemy *enemy, RpgPlayer *player, const RpgGameData::Weapon::WeaponType &weaponType);
	virtual bool onBulletImpact(RpgBullet *bullet, TiledObjectBody *other);
	virtual void onBulletDelete(IsometricBullet *bullet);
	virtual void onQuestionSuccess(RpgPlayer *player, RpgEnemy *enemy, RpgContainer *container, int xp);
	virtual void onQuestionFailed(RpgPlayer *player, RpgEnemy *enemy, RpgContainer *container);

private:
	void rpgGameActivated_();
	void onMsecLeftChanged();

	void downloadLoadableContentDict(const QStringList &fileList);
	void downloadLoadableContent(const QStringList &fileList);


protected:
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

	qint64 m_elapsedTick = 0;
	qint64 m_deadlineTick = 0;


	struct PlayerResurrect {
		QPointer<RpgPlayer> player;
		qint64 time = -1;
	};

	PlayerResurrect m_playerResurrect;

	friend class RpgQuestion;
	friend class RpgGame;
};

#endif // ACTIONRPGGAME_H
