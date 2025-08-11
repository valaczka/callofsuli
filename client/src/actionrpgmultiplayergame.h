/*
 * ---- Call of Suli ----
 *
 * actionrpgmultiplayergame.h
 *
 * Created on: 2025. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ActionRpgMultiplayerGame
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

#ifndef ACTIONRPGMULTIPLAYERGAME_H
#define ACTIONRPGMULTIPLAYERGAME_H

#include "actionrpggame.h"
#include "qslistmodel.h"
#include "rpgudpengine.h"
#include <QObject>


class ActionRpgMultiplayerGamePrivate;


/**
 * @brief The ActionRpgMultiplayerGame class
 */

class ActionRpgMultiplayerGame : public ActionRpgGame
{
	Q_OBJECT

	Q_PROPERTY(int playerId READ playerId WRITE setPlayerId NOTIFY playerIdChanged FINAL)
	Q_PROPERTY(QSListModel *playersModel READ playersModel CONSTANT FINAL)
	Q_PROPERTY(QSListModel *enginesModel READ enginesModel CONSTANT FINAL)
	Q_PROPERTY(bool canAddEngine READ canAddEngine WRITE setCanAddEngine NOTIFY canAddEngineChanged FINAL)
	Q_PROPERTY(bool selectionCompleted READ selectionCompleted WRITE setSelectionCompleted NOTIFY selectionCompletedChanged FINAL)
	Q_PROPERTY(int maxPlayers READ maxPlayers WRITE setMaxPlayers NOTIFY maxPlayersChanged FINAL)
	Q_PROPERTY(bool locked READ locked WRITE setLocked NOTIFY lockedChanged FINAL)
	Q_PROPERTY(QString readableEngineId READ readableEngineId NOTIFY readableEngineIdChanged FINAL)

public:
	explicit ActionRpgMultiplayerGame(GameMapMissionLevel *missionLevel, Client *client);
	virtual ~ActionRpgMultiplayerGame();

	virtual void setRpgGame(RpgGame *newRpgGame) override;
	Q_INVOKABLE virtual void rpgGameActivated() override;
	Q_INVOKABLE virtual void gamePrepared() override;
	Q_INVOKABLE virtual void gameAbort() override;

	Q_INVOKABLE void selectTerrain(const QString &terrain);
	Q_INVOKABLE void selectCharacter(const QString &character);
	Q_INVOKABLE void banOutPlayer(const int &playerId);
	Q_INVOKABLE void lockEngine();

	Q_INVOKABLE void connectToEngine(const int &id);

	void setFinalData(const QJsonObject &data);
	const QJsonObject &finalData() const;

	void disconnectFromHost();

	int playerId() const;
	void setPlayerId(int newPlayerId);

	bool selectionCompleted() const;
	void setSelectionCompleted(bool newSelectionCompleted);

	QSListModel *playersModel() const;
	QSListModel *enginesModel() const;

	virtual bool isReconnecting() const override;

	void setConnectionToken(const QByteArray &newConnectionToken);

	bool canAddEngine() const;
	void setCanAddEngine(bool newCanAddEngine);

	int maxPlayers() const;
	void setMaxPlayers(int newMaxPlayers);

	bool locked() const;
	void setLocked(bool newLocked);

	QString readableEngineId() const;
	void setReadableEngineId(int newReadableEngineId);

	Q_INVOKABLE static QString toReadableEngineId(const int &id);

signals:
	void playerIdChanged();
	void selectionCompletedChanged();
	void canAddEngineChanged();
	void maxPlayersChanged();
	void lockedChanged();
	void readableEngineIdChanged();

protected:
	void onConfigChanged() override;
	virtual void timerEvent(QTimerEvent *) override;
	virtual void questSuccess(RpgQuest *quest) override;

	void changeGameState(const RpgConfig::GameState &state);

private:
	int m_readableEngineId = -1;
	int m_playerId = -1;
	bool m_selectionCompleted = false;
	bool m_tiledGameLoaded = false;
	bool m_gamePrepared = false;
	bool m_enemiesSynced = false;
	bool m_playersSynced = false;
	bool m_randomizerSynced = false;
	bool m_fullyPrepared = false;
	bool m_othersPrepared = false;
	int m_maxPlayers = 0;
	bool m_locked = false;
	bool m_isAborting = false;


	std::unique_ptr<QSListModel> m_playersModel;
	std::unique_ptr<QSListModel> m_enginesModel;
	QBasicTimer m_keepAliveTimer;


	void worldTerrainSelect(QString map, const bool forced);
	void updatePlayersModel();
	void updateEnginesModel(const RpgGameData::EngineSelector &selector);

	void syncEnemyList(const ClientStorage &storage);
	void syncPlayerList(const ClientStorage &storage);
	void syncBulletList(const ClientStorage &storage);
	void syncCollectionList(const ClientStorage &storage);
	void syncPickableList(const ClientStorage &storage);
	void updateLastObjectId(RpgPlayer *player);

	void onGameTimeout();
	void onGameSuccess();
	void onGameFailed();

	RpgPlayer *createPlayer(TiledScene *scene, const RpgGameData::PlayerBaseData &config, const RpgGameData::Player &playerData);

	void onTimeStepPrepare() override;
	void onTimeStepped() override;
	void onTimeBeforeWorldStep(const qint64 &tick) override;
	void onTimeAfterWorldStep(const qint64 &tick) override;
	bool onBodyStep(TiledObjectBody *body) override;
	void onWorldStep() override;
	bool onPlayerAttackEnemy(RpgPlayer *player, RpgEnemy *enemy,
							 const RpgGameData::Weapon::WeaponType &weaponType, const int &weaponSubtype) override;
	bool onPlayerUseControl(RpgPlayer *player, RpgActiveIface *control) override;
	bool onPlayerUseCast(RpgPlayer *player) override;
	bool onPlayerCastTimeout(RpgPlayer *player) override;
	bool onPlayerFinishCast(RpgPlayer *player) override;
	bool onPlayerHit(RpgPlayer *player, RpgEnemy *enemy, RpgWeapon *weapon) override;
	bool onPlayerShot(RpgPlayer *player, RpgWeapon *weapon, const qreal &angle) override;

	void onQuestionSuccess(RpgPlayer *player, RpgActiveIface *control, int xp) override;
	void onQuestionFailed(RpgPlayer *player, RpgActiveIface *control) override;

	void onPlayerWeaponChanged();

	bool onEnemyHit(RpgEnemy *enemy, RpgPlayer *player, RpgWeapon *weapon) override;
	bool onEnemyShot(RpgEnemy *enemy, RpgWeapon *weapon, const qreal &angle) override;
	bool onEnemyAttackPlayer(RpgEnemy *enemy, RpgPlayer *player,
							 const RpgGameData::Weapon::WeaponType &weaponType, const int &weaponSubtype) override;
	void onEnemyDead(RpgEnemy *enemy) override;

	bool onBulletImpact(RpgBullet *bullet, TiledObjectBody *other) override;
	//void onBulletDelete(IsometricBullet *bullet) override;

	void onRpgGameActivated();
	void onConnected();
	void onConnectionLost();
	void onDisconnected();

	void sendData(const QSerializer &data, const bool &reliable);
	void sendData(const QByteArray &data, const bool &reliable);

	void sendDataChrSel(const int &ban = -1, const bool &lock = false);
	void sendDataPrepare();
	void sendDataConnect();

	void setTickTimer(const qint64 &tick);
	void addLatency(const qint64 &latency);
	void overrideCurrentFrame(const qint64 &tick);

	RpgUdpEngine *m_engine = nullptr;
	ActionRpgMultiplayerGamePrivate *q = nullptr;

	friend class RpgUdpEngine;
	friend class ActionRpgMultiplayerGamePrivate;
};

#endif // ACTIONRPGMULTIPLAYERGAME_H
