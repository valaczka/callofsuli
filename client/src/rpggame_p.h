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
	void playerPositionAdd(const QPointF &pos, const Rpg::TeamTag::Team &team);
	void mpEmitterAdd(const QPointF &pos, const quint32 &tagId);


	void connectJoysticks();

	Q_INVOKABLE void joystickClickedA();
	Q_INVOKABLE void joystickClickedB();
	Q_INVOKABLE void joystickClickedC();
	Q_INVOKABLE void joystickClickedD();


	// Play

	void startGame();
	void onBeforeWorldStep();
	void onAfterWorldStep(const RpgStream::FullState &full);


	// Synchronize

	void syncObjects();
	void syncPlayers();
	void syncMp();
	void syncDeleted();

	void onTimeStepped();

	void syncChunkMarker();



	/// RPG LOGIC LOCAL

	quint32 logicRegisterObject(RpgObject *object);
	void logicAddPlayer(const Rpg::TeamTag::Team &team = Rpg::TeamTag::TeamNone, const int &count = 1);

private:
	RpgGame *const q;
	std::unique_ptr<Rpg::RpgLogicClient> m_logic;
	quint32 m_deadlineTick = 0;

	QPointF m_chunkMarkerBaseOffset;										// A 0,0 tile kerüljön a bal felső sarokba
	QPointer<TiledQuick::TileLayerItem> m_chunkMarkerLayer;

	RpgStream::MapData m_mapData;

	inline static RpgStream::HashFnv1A64 m_terrainHash = {};
	inline static RpgStream::HashFnv1A64 m_characterHash = {};


	friend class RpgGame;
	friend class RpgGameItem;
};






#endif // RPGGAME_P_H
