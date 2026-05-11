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
#include "rpggame.h"
#include "rpglogic.h"
#include "rpgobject.h"

/**
 * @brief The RpgGamePrivate class
 */

class RpgGamePrivate : public QObject
{
	Q_OBJECT

private:
	RpgGamePrivate(RpgGame *game);

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


	// Synchronize

	void syncObjects();
	void syncPlayers();
	void syncChunks();



	/// RPG LOGIC LOCAL

	quint32 logicRegisterObject(RpgObject *object);
	void logicAddPlayer(const Rpg::TeamTag::Team &team = Rpg::TeamTag::TeamNone, const int &count = 1);

private:
	RpgGame *const q;
	Rpg::RpgLogic m_logic;

	inline static RpgStream::HashFnv1A64 m_terrainHash = {};
	inline static RpgStream::HashFnv1A64 m_characterHash = {};


	friend class RpgGame;
	friend class RpgGameItem;
};




/**
 * @brief The RpgLogicObjectMapper class
 */

struct RpgLogicObjectMapper
{
	QHash<quint32, QPointer<RpgObject> > map;

	static quint32 getId(const TiledObjectBody::ObjectId &id);
	static quint32 getId(const RpgObject *object) { return object ? getId(object->objectId()) : 0; }
	static TiledObjectBody::ObjectId toObjectId(const quint32 &id);

	quint32 set(RpgObject *object);
	RpgObject *get(const quint32 &id) { return map.value(id); }
};


#endif // RPGGAME_P_H
