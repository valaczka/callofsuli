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
#include <QObject>


class RpgUdpEngine;


/**
 * @brief The ActionRpgMultiplayerGame class
 */

class ActionRpgMultiplayerGame : public ActionRpgGame
{
	Q_OBJECT

	Q_PROPERTY(int playerId READ playerId WRITE setPlayerId NOTIFY playerIdChanged FINAL)
	Q_PROPERTY(QSListModel *playersModel READ playersModel CONSTANT FINAL)
	Q_PROPERTY(bool selectionCompleted READ selectionCompleted WRITE setSelectionCompleted NOTIFY selectionCompletedChanged FINAL)

public:
	explicit ActionRpgMultiplayerGame(GameMapMissionLevel *missionLevel, Client *client);
	virtual ~ActionRpgMultiplayerGame();

	virtual void setRpgGame(RpgGame *newRpgGame) override;
	Q_INVOKABLE virtual void rpgGameActivated() override;
	Q_INVOKABLE virtual void gamePrepared() override;
	Q_INVOKABLE virtual void gameAbort() override;

	Q_INVOKABLE void selectTerrain(const QString &terrain);
	Q_INVOKABLE void selectCharacter(const QString &character);
	Q_INVOKABLE void selectWeapons(const QStringList &weaponList);

	void disconnectFromHost();

	int playerId() const;
	void setPlayerId(int newPlayerId);

	bool selectionCompleted() const;
	void setSelectionCompleted(bool newSelectionCompleted);

	QSListModel *playersModel() const;

signals:
	void playerIdChanged();
	void selectionCompletedChanged();

protected:
	void onConfigChanged() override;
	virtual void timerEvent(QTimerEvent *) override;

	void changeGameState(const RpgConfig::GameState &state);

private:
	int m_playerId = -1;
	bool m_selectionCompleted = false;
	bool m_gamePrepared = false;
	bool m_enemiesSynced = false;
	bool m_playersSynced = false;

	std::unique_ptr<QSListModel> m_playersModel;
	QBasicTimer m_keepAliveTimer;


	void worldTerrainSelect(QString map, const bool forced);
	void updatePlayersModel(const QVariantList &list);
	void syncEnemyList();
	void syncPlayerList();

	RpgPlayer *createPlayer(TiledScene *scene, const RpgGameData::Player &config);

	bool onPlayerPick(RpgPlayer *player, RpgPickableObject *pickable);
	bool onPlayerAttackEnemy(RpgPlayer *player, IsometricEnemy *enemy, const TiledWeapon::WeaponType &weaponType);
	bool onPlayerUseContainer(RpgPlayer *player, RpgContainer *container);
	bool onPlayerUseCast(RpgPlayer *player);
	bool onPlayerCastTimeout(RpgPlayer *player);
	bool onPlayerFinishCast(RpgPlayer *player);
	bool onEnemyAttackPlayer(IsometricEnemy *enemy, RpgPlayer *player, const TiledWeapon::WeaponType &weaponType);

	void beforeWorldStep();
	void afterWorldStep();
	void worldStep(const TiledGame::Body &body);

	void onRpgGameActivated();

	void sendData(const QByteArray &data, const bool &reliable);

	void sendDataChrSel();
	void sendDataPrepare();
	void sendDataPlay();

	void setTickTimer(const qint64 &tick);

	RpgUdpEngine *m_engine = nullptr;

	friend class RpgUdpEngine;
};

#endif // ACTIONRPGMULTIPLAYERGAME_H
