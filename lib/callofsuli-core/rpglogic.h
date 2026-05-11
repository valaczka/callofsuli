/*
 * ---- Call of Suli ----
 *
 * rpgconfig.h
 *
 * Created on: 2024. 03. 24.
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

#ifndef RPGLOGIC_H
#define RPGLOGIC_H

#include "qcborarray.h"
#include <QSerializer>
#include <QIODevice>
#include <QColor>
#include <entt/entt.hpp>
#include "Logger.h"
#include "qpaintdevice.h"
#include "qpoint.h"
#include "rpgstream.h"





/**************************************************************
 * RPG WORLD
 **************************************************************/


/**
 * @brief The RpgWorldLandGeometry class
 */


class RpgWorldLandGeometry : public QSerializer
{
	Q_GADGET

public:
	RpgWorldLandGeometry()
		: QSerializer()
		, x(0.)
		, y(0.)
		, textX(0.)
		, textY(0.)
		, rotate(0.)
	{}

	QS_SERIALIZABLE

	QS_FIELD(qreal, x)
	QS_FIELD(qreal, y)
	QS_FIELD(qreal, textX)
	QS_FIELD(qreal, textY)
	QS_FIELD(qreal, rotate)
};



/**
 * @brief The RpgWorldOrig class
 */

class RpgWorldOrig : public QSerializer
{
	Q_GADGET

public:
	RpgWorldOrig()
		: QSerializer()
		, width(0.)
		, height(0.)
	{}

	QS_SERIALIZABLE

	QS_FIELD(qreal, width)
	QS_FIELD(qreal, height)
	QS_FIELD(QString, description)
	QS_FIELD(QString, background)
	QS_FIELD(QString, over)
	QS_QT_DICT(QMap, QString, QJsonArray, adjacency)

};





/**
 * @brief The RpgWorldLandMap class
 */

class RpgWorldMapBinding : public QSerializer
{
	Q_GADGET

public:
	RpgWorldMapBinding()
		: QSerializer()
		, free(false)
	{}

	QS_SERIALIZABLE

	QS_FIELD(QString, map)
	QS_FIELD(bool, free)					// Szabadon játszható
};



/**
 * @brief The RpgWorld class
 */


class RpgWorld : public QSerializer
{
	Q_GADGET

public:
	RpgWorld()
		: QSerializer()
	{}

	QS_SERIALIZABLE

	QS_OBJECT(RpgWorldOrig, orig)
	QS_QT_DICT_OBJECTS(QMap, QString, RpgWorldLandGeometry, lands)
	QS_QT_DICT_OBJECTS(QMap, QString, RpgWorldMapBinding, binding)
};






/**************************************************************
 * RPG LOGIC
 **************************************************************/

namespace Rpg {

// Minden objektum közös azonosítója (packId, unpackId)
// Amelyik entity-nek van a kliensen, azt már szinkronizáltuk (létrehoztuk), tehát az exclued-del le tudjuk kérni, amit meg kell csinálni

struct IdTag
{
	quint32 id = 0;
};


// Csapatjelzés

struct TeamTag
{
	enum Team {
		TeamNone = 0,
		TeamA = 1,
		TeamB = 2
	};

	Team team = TeamNone;
};



// Játékos

struct Player
{
	RpgStream::PlayerData playerData;

	quint32 lastObjectId = 0;

	quint32 idTag() const;
};



// Chunkgrid data

struct ChunkGrid
{
	QRectF viewport;
	QSet<QPair<quint32, quint32> > excludeSet;
	QSizeF chunkSize;

	static ChunkGrid fromRpgStream(const RpgStream::ChunkGrid &grid);
	RpgStream::ChunkGrid toRpgStream() const;

	QPair<quint32, quint32> getChunk(const float &x, const float &y) const;
	QPair<quint32, quint32> getChunk(const QPointF &pos) const {
		return getChunk(pos.x(), pos.y());
	}

	bool isAccessible(const float &x, const float &y) const {
		return excludeSet.contains(getChunk(x, y));
	}
	bool isAccessible(const QPointF &pos) const {
		return isAccessible(pos.x(), pos.y());
	}
};











/**
 * @brief The RpgLogic class
 */

class RpgLogic
{
public:
	RpgLogic();
	~RpgLogic() = default;

	const entt::registry &registry() const { return m_registry; }
	entt::registry &registry() { return m_registry; }

	// EnTT Context

	template <typename T>
	void registerCtx();

	template<typename T>
	[[nodiscard]] const T *getCtx() const;

	template<typename T>
	[[nodiscard]] T *getCtx();


	// EnTT object id

	static quint32 packId(const quint32 &scene, const quint32 &owner, const quint32 &id);
	static void unpackId(const quint32 &from, quint32 &scene, quint32 &owner, quint32 &id);

	void entitySetIdTag(entt::entity &entity, const quint32 &tag);

	// Chunk grid

	ChunkGrid &loadChunkGrid(ChunkGrid &&grid);
	ChunkGrid &loadChunkGrid(const RpgStream::ChunkGrid &grid);


	// Player

	void playerPositionAdd(const QPointF &pos, const TeamTag::Team &team = TeamTag::TeamNone);
	void playerPositionListSet(RpgStream::PlayerPositionList &&list);

	entt::entity playerAdd(const TeamTag::Team &team = TeamTag::TeamNone);

	bool emplacePlayers();
	bool initializePlayers();


private:
	entt::registry m_registry;
};







inline quint32 Player::idTag() const { return RpgLogic::packId(0, playerData.playerId(), 0); }


/**
 * @brief RpgLogic::registerCtx
 */

template<typename T>
inline void RpgLogic::registerCtx()
{
	if (!m_registry.ctx().contains<T>()) {
		LOG_CDEBUG("game") << "Register to ctx";
		m_registry.ctx().emplace<T>();
	}
}


/**
 * @brief RpgLogic::getCtx
 * @return
 */

template<typename T>
inline T *RpgLogic::getCtx()
{
	return m_registry.ctx().find<T>();
}


/**
 * @brief RpgLogic::getCtx
 * @return
 */

template<typename T>
inline const T *RpgLogic::getCtx() const
{
	return m_registry.ctx().find<T>();
}



}			// end namespace

#endif // RPGLOGIC_H
