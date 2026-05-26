/*
 * ---- Call of Suli ----
 *
 * rpggame_p.h
 *
 * Created on: 2026. 05. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#ifndef RPGGAME_P_H
#define RPGGAME_P_H

#include <QObject>
#include <libtiledquick/tilelayeritem.h>
#include "rpggame.h"
#include "rpglogic.h"
#include "rpglogicclient.h"
#include "rpgobject.h"
#include "rpgplayer.h"
#include "rpgtower.h"



#ifdef WITH_GAMEPAD
#include <QtGamepadLegacy/QGamepad>
#endif


/**
 * @brief The RpgGamePrivate class
 */

class RpgGamePrivate : public QObject
{
	Q_OBJECT

private:
	RpgGamePrivate(RpgGame *game, const bool &multi);

	void clearSharedTextures();


	// Connect

	void connectionPrepare();
	void connectionCheck();

	void onContentDownloaded();
	void onContentError();


	// Prepare

	void prepareGameItem();
	void onGameItemPrepared();
	void loadChunkGrid();
	void playerPositionAdd(const QPointF &pos, const RpgStream::Team &team);
	void mpEmitterAdd(const QPointF &pos, const quint32 &tagId);
	void towerAdd(RpgTower *tower);


	void connectJoysticks();
	void setJoystickState(RpgMotorPlayerControlled *motor, const TiledGame::Joystick &joystick, const TiledGame::JoystickState &state);
	bool setFromGamepad(RpgMotorPlayerControlled *motor);

	Q_INVOKABLE void joystickClickedA(const bool &clicked);
	Q_INVOKABLE void joystickClickedB(const bool &clicked);
	Q_INVOKABLE void joystickClickedC(const bool &clicked);
	Q_INVOKABLE void joystickClickedD(const bool &clicked);


	// Play

	void startGame();
	void onBeforeWorldStep(const qint64 &tick);
	void onAfterWorldStep(const RpgStream::FullState &full);


	// Synchronize

	void syncGameState();

	void syncObjects();
	void syncPlayers();
	void syncMp();
	void syncDefenders();

	struct ObjectSet
	{
		QSet<quint32> player;
		QSet<quint32> mp;
		QSet<quint32> defender;
	};

	ObjectSet extractObjects(const RpgStream::FullState &full) const;

	void deleteMissingObjects(const ObjectSet &objects);

	void processEvents(const qint64 &tick);

	void onTimeStepped();



	/// RPG LOGIC LOCAL

	quint32 logicRegisterObject(RpgObject *object);
	void logicAddPlayer(const RpgStream::Team &team = RpgStream::TeamNone, const int &count = 1);		// deprecated

	void changeControlledPlayer();					// deprecated

private:
	RpgGame *const q;
	std::unique_ptr<Rpg::RpgLogicClient> m_logic;
	quint32 m_deadlineTick = 0;

	RpgStream::MapData m_mapData;
	QList<QPointer<RpgTower> > m_towerList;

	inline static RpgStream::HashFnv1A64 m_terrainHash = {};
	inline static RpgStream::HashFnv1A64 m_characterHash = {};


#ifdef WITH_GAMEPAD
	void onGamePadChanged(const double &);
	void onGamePadButtonL3Changed(const bool &pressed);
	void onGamePadButtonR3Changed(const bool &pressed);
	void onGamePadButtonL1Changed(const bool &pressed);
	void onGamePadButtonR1Changed(const bool &pressed);
	std::unique_ptr<QGamepad> m_gamePad;
#endif


	friend class RpgGame;
	friend class RpgGameItem;
};







#endif // RPGGAME_P_H
