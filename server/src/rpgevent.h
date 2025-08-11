/*
 * ---- Call of Suli ----
 *
 * rpgevent.h
 *
 * Created on: 2025. 07. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgEvent
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

#ifndef RPGEVENT_H
#define RPGEVENT_H


#include "rpgengine.h"



/**
 * @brief The RpgEvent class
 */

template <typename T>
class RpgEvent : public RpgEventBase
{
public:
	RpgEvent(RpgEngine *engine, const qint64 &tick, const T &data, const bool &unique = true)
		: RpgEventBase(engine, tick, unique)
		, m_data(data)
	{ }

	virtual ~RpgEvent() = default;

	const T &baseData() const { return m_data; }

	bool isBaseEqual(RpgEventBase *other) const {
		if (RpgEvent<T> *d = dynamic_cast<RpgEvent<T>*>(other); d &&
				d->m_tick == m_tick &&
				d->m_data == m_data
				)
			return true;

		return false;
	}


protected:
	const T m_data;
};



#define ADD_EQUAL(T)		virtual bool isEqual(RpgEventBase *other) const override { \
	return dynamic_cast<T*>(other) && isBaseEqual(other); \
	}


/**
 * @brief The RpgEventEnemyDied class
 */

class RpgEventEnemyDied : public RpgEvent<RpgGameData::EnemyBaseData>
{
public:
	RpgEventEnemyDied(RpgEngine *engine, const qint64 &tick, const RpgGameData::EnemyBaseData &data,
					  const QList<RpgGameData::PickableBaseData> &pickables)
		: RpgEvent<RpgGameData::EnemyBaseData>(engine, tick, data)
		, m_pickables(pickables)
	{
		ELOG_DEBUG << "Enemy died" << m_data.o << m_data.id << m_pickables;
	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	ADD_EQUAL(RpgEventEnemyDied);

private:
	const QList<RpgGameData::PickableBaseData> m_pickables;
};





/**
 * @brief The RpgEventEnemyResurrect class
 */

class RpgEventEnemyResurrect : public RpgEvent<bool>
{
public:
	RpgEventEnemyResurrect(RpgEngine *engine, const qint64 &tick)
		: RpgEvent<bool>(engine, tick, false)
	{

	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	ADD_EQUAL(RpgEventEnemyResurrect);
};







/**
 * @brief The RpgEventPlayerDied class
 */

class RpgEventPlayerDied : public RpgEvent<RpgGameData::PlayerBaseData>
{
public:
	RpgEventPlayerDied(RpgEngine *engine, const qint64 &tick, const RpgGameData::PlayerBaseData &data)
		: RpgEvent<RpgGameData::PlayerBaseData>(engine, tick, data)
	{

	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	ADD_EQUAL(RpgEventPlayerDied);
};





/**
 * @brief The RpgEventPlayerResurrect class
 */

class RpgEventPlayerResurrect : public RpgEvent<RpgGameData::PlayerBaseData>
{
public:
	RpgEventPlayerResurrect(RpgEngine *engine, const qint64 &tick, const RpgGameData::PlayerBaseData &data)
		: RpgEvent<RpgGameData::PlayerBaseData>(engine, tick, data)
	{

	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	virtual bool isEqual(RpgEventBase *other) const override;
};





/**
 * @brief The RpgEventContainerUnlock class
 */

class RpgEventControlUnlock : public RpgEvent<RpgGameData::PlayerBaseData>
{
public:
	RpgEventControlUnlock(RpgEngine *engine, const qint64 &tick, const RpgGameData::PlayerBaseData &data)
		: RpgEvent<RpgGameData::PlayerBaseData>(engine, tick, data, true)
	{

	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	ADD_EQUAL(RpgEventControlUnlock);
};





/**
 * @brief The RpgEventPlayerLost class
 */

class RpgEventPlayerLost : public RpgEvent<RpgGameData::PlayerBaseData>
{
public:
	RpgEventPlayerLost(RpgEngine *engine, const qint64 &tick, const RpgGameData::PlayerBaseData &data)
		: RpgEvent<RpgGameData::PlayerBaseData>(engine, tick, data, true)
	{

	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	virtual bool isEqual(RpgEventBase *other) const override;
};




/**
 * @brief The RpgEventCollectionUsed class
 */

class RpgEventCollectionUsed : public RpgEvent<RpgGameData::ControlCollectionBaseData>
{
public:
	RpgEventCollectionUsed(RpgEngine *engine, const qint64 &tick, const RpgGameData::ControlCollectionBaseData &data,
						   const bool &success, const RpgGameData::PlayerBaseData &player)
		: RpgEvent<RpgGameData::ControlCollectionBaseData>(engine, tick, data, true)
		, m_success(success)
		, m_player(player)
	{

	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	ADD_EQUAL(RpgEventCollectionUsed);

private:
	bool m_success = false;
	const RpgGameData::PlayerBaseData m_player;
};





/**
 * @brief The RpgEventCollectionPost class
 */

class RpgEventCollectionPost : public RpgEvent<RpgGameData::ControlCollectionBaseData>
{
public:
	RpgEventCollectionPost(RpgEngine *engine, const qint64 &tick, const RpgGameData::ControlCollectionBaseData &data,
						   const bool &success)
		: RpgEvent<RpgGameData::ControlCollectionBaseData>(engine, tick, data, true)
		, m_success(success)
	{

	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	ADD_EQUAL(RpgEventCollectionPost);

private:
	bool m_success = false;
};




/**
 * @brief The RpgEventContainerUsed class
 */

class RpgEventContainerUsed : public RpgEvent<RpgGameData::ControlContainerBaseData>
{
public:
	RpgEventContainerUsed(RpgEngine *engine, const qint64 &tick, const RpgGameData::ControlContainerBaseData &data,
						  const RpgGameData::PlayerBaseData &player, const QList<RpgGameData::PickableBaseData> &pickables,
						  const bool &success)
		: RpgEvent<RpgGameData::ControlContainerBaseData>(engine, tick, data, true)
		, m_player(player)
		, m_pickables(pickables)
		, m_success(success)
	{

	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	ADD_EQUAL(RpgEventContainerUsed);

private:
	const RpgGameData::PlayerBaseData m_player;
	const QList<RpgGameData::PickableBaseData> m_pickables;
	bool m_success = false;
};






/**
 * @brief The RpgEventPickablePicked class
 */

class RpgEventPickablePicked : public RpgEvent<RpgGameData::PickableBaseData>
{
public:
	RpgEventPickablePicked(RpgEngine *engine, const qint64 &tick, const RpgGameData::PickableBaseData &data,
						   const RpgGameData::PlayerBaseData &player)
		: RpgEvent<RpgGameData::PickableBaseData>(engine, tick, data, true)
		, m_player(player)
	{

	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	ADD_EQUAL(RpgEventPickablePicked);

private:
	RpgGameData::PlayerBaseData const m_player;
};







/**
 * @brief The RpgEventTeleportUsed class
 */

class RpgEventTeleportUsed : public RpgEvent<RpgGameData::ControlTeleportBaseData>
{
public:
	RpgEventTeleportUsed(RpgEngine *engine, const qint64 &tick, const RpgGameData::ControlTeleportBaseData &data,
						 const RpgGameData::PlayerBaseData &player)
		: RpgEvent<RpgGameData::ControlTeleportBaseData>(engine, tick, data, true)
		, m_player(player)
	{

	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	ADD_EQUAL(RpgEventTeleportUsed);

private:
	RpgGameData::PlayerBaseData const m_player;
};



#endif // RPGEVENT_H
