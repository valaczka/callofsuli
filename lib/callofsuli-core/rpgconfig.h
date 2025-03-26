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

#ifndef RPGCONFIG_H
#define RPGCONFIG_H

#include "qmutex.h"
#include "qpoint.h"
#include <QSerializer>
#include <QIODevice>



#define RPG_UDP_DELTA_MSEC	1000



/**
 * @brief The ConquestConfigBase class
 */

class RpgConfigBase : public QSerializer
{
	Q_GADGET

public:
	RpgConfigBase()
		: missionLevel(-1)
		, campaign(-1)
		, duration(0)
	{}


	QS_SERIALIZABLE
	QS_FIELD(QString, mapUuid)
	QS_FIELD(QString, missionUuid)
	QS_FIELD(int, missionLevel)
	QS_FIELD(int, campaign)
	QS_FIELD(int, duration)
};





/**
 * @brief The ConquestConfig class
 */

class RpgConfig : public RpgConfigBase
{
	Q_GADGET

public:
	RpgConfig()
		: RpgConfigBase()
		, gameState(StateInvalid)
	{}


	enum GameState {
		StateInvalid = 0,
		StateDownloadStatic,
		StateConnect,
		StateCharacterSelect,
		StateDownloadContent,
		StatePrepare,
		StatePlay,
		StateFinished,
		StateError
	};

	Q_ENUM(GameState);


	void reset() {
		gameState = StateInvalid;
	}

	QJsonObject toBaseJson() const { return RpgConfigBase::toJson(); }

	friend bool operator==(const RpgConfig &c1, const RpgConfig &c2) {
		return c1.gameState == c2.gameState &&
				c1.mapUuid == c2.mapUuid &&
				c1.missionUuid == c2.missionUuid &&
				c1.missionLevel == c2.missionLevel &&
				c1.campaign == c2.campaign &&
				c1.duration == c2.duration
				;
	}

	QS_SERIALIZABLE
	QS_FIELD(GameState, gameState)
};


Q_DECLARE_METATYPE(RpgConfig)





/**
 * @brief The ConquestPlayer class
 */

class RpgPlayerConfig : public QSerializer
{
	Q_GADGET

public:
	RpgPlayerConfig(const int &_id, const QString &_user)
		: playerId(_id)
		, username(_user)
	{}
	RpgPlayerConfig(const int &_id) : RpgPlayerConfig(_id, QString()) {}
	RpgPlayerConfig() : RpgPlayerConfig(-1) {}

	QS_SERIALIZABLE

	QS_FIELD(int, playerId)
	QS_FIELD(QString, username)
	QS_FIELD(QString, nickname)

	QS_FIELD(QString, terrain)
	QS_FIELD(QString, character)
	QS_FIELD(QStringList, weapons)
};


Q_DECLARE_METATYPE(RpgPlayerConfig)





/**
 * @brief The RpgWalletExtendedInfo class
 */

class RpgMarketExtendedInfo : public QSerializer
{
	Q_GADGET

public:
	RpgMarketExtendedInfo(const QString &_icon, const QString &_text, const QString &_value,
						  const QString &_image = {}, const int &_imageSize = 0)
		: QSerializer()
		, icon(_icon)
		, text(_text)
		, value(_value)
		, image(_image)
		, imageSize(_imageSize)
	{}

	RpgMarketExtendedInfo(const QString &_value)
		: RpgMarketExtendedInfo(QString{}, QString{}, _value)
	{}

	RpgMarketExtendedInfo(const QString &_icon, const QString &_text)
		: RpgMarketExtendedInfo(_icon, _text, QString{})
	{}

	RpgMarketExtendedInfo()
		: RpgMarketExtendedInfo(QString{})
	{}

	QS_SERIALIZABLE

	QS_FIELD(QString, icon);
	QS_FIELD(QString, text);
	QS_FIELD(QString, value);
	QS_FIELD(QString, image);
	QS_FIELD(int, imageSize);

	friend bool operator==(const RpgMarketExtendedInfo &e1, const RpgMarketExtendedInfo &e2) {
		return e1.icon == e2.icon &&
				e1.text == e2.text &&
				e1.value == e2.value &&
				e1.image == e2.image &&
				e1.imageSize == e2.imageSize
				;
	}
};


Q_DECLARE_METATYPE(RpgMarketExtendedInfo)




/**
 * @brief The RpgMarket class
 */

class RpgMarket : public QSerializer
{
	Q_GADGET

public:
	enum Type {
		Invalid		= 0,
		Map			= 1,
		Skin		= 2,
		Weapon		= 3,
		/*Bullet [[deprecated]]		= 4,*/
		Hp			= 5,
		Time		= 6,
		Xp			= 7,
		Mp			= 8,
		Pickable	= 9,
		Other		= 999,
	};

	Q_ENUM(Type);

	enum Rollover {
		None		= 0,
		Game		= 1,
		Day			= 2
	};

	Q_ENUM(Rollover);

	RpgMarket()
		: QSerializer()
		, type(Invalid)
		, cost(0)
		, rank(0)
		, amount(1)
		, rollover(None)
		, num(0)
		, exp(0)
	{}


	QS_SERIALIZABLE

	QS_FIELD(Type, type)
	QS_FIELD(QString, name)

	QS_FIELD(int, cost)
	QS_FIELD(int, rank)

	QS_FIELD(int, amount)

	// Maximize

	QS_FIELD(Rollover, rollover)
	QS_FIELD(int, num)

	// Expiry

	QS_FIELD(int, exp)			// minutes

	// Extended info

	QS_FIELD(QJsonObject, info)


	friend bool operator==(const RpgMarket &c1, const RpgMarket &c2) {
		return c1.type == c2.type &&
				c1.name == c2.name &&
				c1.cost == c2.cost &&
				c1.rank == c2.rank &&
				c1.amount == c2.amount &&
				c1.rollover == c2.rollover &&
				c1.num == c2.num &&
				c1.exp == c2.exp
				;
	}
};

Q_DECLARE_METATYPE(RpgMarket)


/**
 * @brief The RpgMarketList class
 */

class RpgMarketList : public QSerializer
{
	Q_GADGET

public:
	RpgMarketList()
		: QSerializer()
	{}

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QVector, RpgMarket, list)
};

Q_DECLARE_METATYPE(RpgMarketList)





/**
 * @brief The RpgWallet class
 */

class RpgWallet : public QSerializer
{
	Q_GADGET

public:
	RpgWallet()
		: QSerializer()
		, type(RpgMarket::Invalid)
		, amount(0)
		, expiry(0)
	{}


	QS_SERIALIZABLE

	QS_FIELD(RpgMarket::Type, type)
	QS_FIELD(QString, name)
	QS_FIELD(int, amount)
	QS_FIELD(qint64, expiry)
};




/**
 * @brief The RpgQuest class
 */

class RpgQuest : public QSerializer
{
	Q_GADGET

	Q_PROPERTY(int success MEMBER success)

public:
	enum Type {
		Invalid			= 0,
		EnemyDefault	= 1,
		WinnerDefault	= 2,
		SuddenDeath		= 3
	};

	Q_ENUM(Type);

	RpgQuest(const Type &_type = Invalid, const int &_amount = 0, const int &_currency = 0,
			 const QJsonObject &_data = {})
		: QSerializer()
		, type(_type)
		, amount(_amount)
		, currency(_currency)
		, data(_data)
	{}


	int success = 0;

	QS_SERIALIZABLE

	QS_FIELD(Type, type)
	QS_FIELD(int, amount)
	QS_FIELD(int, currency)
	QS_FIELD(QJsonObject, data)

};

Q_DECLARE_METATYPE(RpgQuest)







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




/// ---------------------------------------------------------------
/// IN GAME serialization
/// ---------------------------------------------------------------


#define EQUAL_OPERATOR(cname)	bool operator==(const cname &r) const { return isEqual(r); }


namespace RpgGameData {

class CharacterSelect : public QSerializer
{
	Q_GADGET

public:
	CharacterSelect()
		: QSerializer()
		, playerId(-1)
		, completed(false)
	{}

	CharacterSelect(const RpgPlayerConfig &config)
		: QSerializer()
		, playerId(-1)
		, completed(false)
	{
		username = config.username;
		nickname = config.nickname;
		character = config.character;
		weapons = config.weapons;
	}

	QS_SERIALIZABLE

	QS_FIELD(int, playerId)
	QS_FIELD(QString, username)
	QS_FIELD(QString, nickname)

	QS_FIELD(QString, character)
	QS_FIELD(QStringList, weapons)
	QS_FIELD(bool, completed)
};




/**
 * @brief The Prepare class
 */

class Prepare : public QSerializer
{
	Q_GADGET

public:
	Prepare()
		: QSerializer()
		, prepared(false)
	{}

	QS_SERIALIZABLE

	QS_FIELD(bool, prepared)
};



/**
 * @brief The PlayerPosition class
 */

class PlayerPosition : public QSerializer
{
	Q_GADGET

public:
	PlayerPosition(const int &_s, const float &_x, const float &_y)
		: QSerializer()
		, scene(_s)
		, x(_x)
		, y(_y)
	{ }

	PlayerPosition()
		: PlayerPosition(-1, 0., 0.)
	{ }

	QS_SERIALIZABLE

	QS_FIELD(int, scene)
	QS_FIELD(float, x)
	QS_FIELD(float, y)
};





/**
 * @brief The GameConfig class
 */

class GameConfig : public QSerializer
{
	Q_GADGET

public:
	GameConfig()
		: QSerializer()
	{}

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, PlayerPosition, positionList)
	QS_FIELD(QString, terrain)
};





/**
 * @brief The BaseData class
 */

class BaseData : public QSerializer
{
	Q_GADGET

public:
	BaseData(const int &_o, const int &_s, const int &_id)
		: QSerializer()
		, o(_o)
		, s(_s)
		, id(_id)
	{ }

	BaseData()
		: BaseData(-1, -1, -1)
	{ }

	bool isEqual(const BaseData &other) const {
		return other.o == o && other.s == s && other.id == id;
	}

	bool isValid() const {
		return o >= 0 && s >= 0 && id >= 0;
	}

	EQUAL_OPERATOR(BaseData)

	QS_SERIALIZABLE

	QS_FIELD(int, o)					// ownerId
	QS_FIELD(int, s)					// sceneId
	QS_FIELD(int, id)					// objectId

};



/**
 * @brief The Body class
 */

class Body : public QSerializer
{
	Q_GADGET

public:
	Body()
		: QSerializer()
		, f(-1)
		, sc(-1)
	{ }

	bool isEqual(const Body &other) const {
		return other.f == f;
	}

	EQUAL_OPERATOR(Body)

	QS_SERIALIZABLE

	QS_FIELD(qint64, f)					// frame

	QS_FIELD(int, sc)					// current scene
	QS_COLLECTION(QList, float, p)		// position
};




/**
 * @brief The Entity class
 */

class Entity : public Body
{
	Q_GADGET

public:
	Entity()
		: Body()
		, a(0.)
		, hp(0)
		, mhp(0)
	{}


	bool isEqual(const Entity &other) const {
		return Body::isEqual(other) && other.a == a && other.hp == hp && other.mhp == mhp;
	}

	EQUAL_OPERATOR(Entity)

	QS_SERIALIZABLE

	QS_FIELD(float, a)			// angle
	QS_FIELD(int, hp)			// HP
	QS_FIELD(int, mhp)			// max HP
};





/**
 * @brief The Player class
 */

class Player : public Entity
{
	Q_GADGET

public:
	Player()
		: Entity()
		, st(PlayerInvalid)
	{}

	enum PlayerState {
		PlayerInvalid = 0,
		PlayerIdle,
		PlayerMoving
	};

	Q_ENUM(PlayerState);


	bool isEqual(const Player &other) const  {
		return Entity::isEqual(other) && other.st == st;
	}

	EQUAL_OPERATOR(Player);

	QS_SERIALIZABLE

	QS_FIELD(PlayerState, st)
};






/**
 * @brief The EnemyBaseData class
 */

class EnemyBaseData : public BaseData
{
	Q_GADGET

public:
	enum EnemyType {
		EnemyInvalid = 0,
		EnemyWerebear,
		EnemySoldier,
		EnemyArcher,
		EnemySoldierFix,
		EnemyArcherFix,
		EnemySkeleton,
		EnemySmith,
		EnemySmithFix,
		EnemyBarbarian,
		EnemyBarbarianFix,
		EnemyButcher,
		EnemyButcherFix,
	};

	Q_ENUM(EnemyType);

	EnemyBaseData(const EnemyType &_type, const int &_o, const int &_s, const int &_id)
		: BaseData(_o, _s, _id)
		, t(_type)
	{}

	EnemyBaseData(const EnemyType &_type)
		: EnemyBaseData(_type, -1, -1, -1)
	{}

	EnemyBaseData()
		: EnemyBaseData(EnemyInvalid)
	{}

	bool isEqual(const EnemyBaseData &other) const {
		return BaseData::isEqual(other);
	}

	EQUAL_OPERATOR(EnemyBaseData)

	QS_SERIALIZABLE

	QS_FIELD(EnemyType, t)
};





/**
 * @brief The Enemy class
 */


class Enemy : public Entity
{
	Q_GADGET

public:

	Enemy()
		: Entity()
	{}


	bool isEqual(const Enemy &other) const {
		return Entity::isEqual(other);
	}

	EQUAL_OPERATOR(Enemy)

	QS_SERIALIZABLE

};






/**
 * @brief The PickableBaseData class
 */

class PickableBaseData : public BaseData
{
	Q_GADGET

public:
	enum PickableType {
		PickableInvalid = 0,
		PickableHp,
		PickableShortbow,
		PickableLongbow,
		/*PickableArrow [[deprecated]],
		PickableFireball [[deprecated]],
		PickableLightning [[deprecated]],*/
		PickableLongsword,
		PickableDagger,
		PickableShield,
		PickableTime,
		PickableMp,
		PickableCoin,
		PickableKey,
	};

	Q_ENUM(PickableType);

	PickableBaseData (const PickableType &_type, const int &_o, const int &_s, const int &_id)
		: BaseData(_o, _s, _id)
		, t(_type)
	{}

	PickableBaseData(const PickableType &_type)
		: PickableBaseData(_type, -1, -1, -1)
	{}

	PickableBaseData()
		: PickableBaseData(PickableInvalid)
	{}

	bool isEqual(const PickableBaseData &other) const {
		return BaseData::isEqual(other);
	}

	EQUAL_OPERATOR(PickableBaseData)

	QS_SERIALIZABLE

	QS_FIELD(PickableType, t)
};






/**
 * @brief The Pickable class
 */

class Pickable : public Body
{
	Q_GADGET

public:

	bool isEqual(const Pickable &other) const {
		return Body::isEqual(other);
	}

	EQUAL_OPERATOR(Pickable)

	QS_SERIALIZABLE

};










/**
 * @brief The SnapshotInterpolation class
 */

template <typename T, typename = std::enable_if<std::is_base_of<Body, T>::value>::type>
struct SnapshotInterpolation {
	T s1;
	T s2;
	qint64 current = -1;

	void clear() {
		s1 = {};
		s2 = {};
		current = -1;
	}
};


template <typename T2, typename T,
		  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
		  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
using Snapshot = std::vector<std::pair<T2, SnapshotInterpolation<T> > >;


template <typename T2, typename T,
		  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
		  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
using CurrentSnapshotList = std::vector<std::pair<T2, T> >;


/**
 * @brief The FullSnapshot class
 */

struct FullSnapshot {
	Snapshot<BaseData, Player> players;
	Snapshot<EnemyBaseData, Enemy> enemies;

	void clear() {
		players.clear();
		enemies.clear();
	}

	template <typename T2, typename T,
			  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
	std::optional<SnapshotInterpolation<T> > getSnapshot(const T2 &data, const Snapshot<T2, T> &list) const;


	std::optional<SnapshotInterpolation<Player> > getPlayer(const BaseData &data) const {
		return getSnapshot(data, players);
	}
	std::optional<SnapshotInterpolation<Enemy> > getEnemy(const EnemyBaseData &data) const {
		return getSnapshot(data, enemies);
	}
};




/**
 * @brief The CurrentSnapshot class
 */

struct CurrentSnapshot {
	CurrentSnapshotList<BaseData, Player> players;
	CurrentSnapshotList<EnemyBaseData, Enemy> enemies;

	void clear() {
		players.clear();
		enemies.clear();
	}

	template <typename T2, typename T,
			  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
	std::optional<T> getSnapshot(const T2 &data, const CurrentSnapshotList<T2, T> &list) const;


	std::optional<Player> getPlayer(const BaseData &data) const {
		return getSnapshot(data, players);
	}
	std::optional<Enemy> getEnemy(const EnemyBaseData &data) const {
		return getSnapshot(data, enemies);
	}
};




// Snapshots

template <typename T, typename T2,
		  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
		  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
struct SnapshotData {
	T2 data;
	std::map<qint64, T> list;
};


template <typename T, typename T2,
		  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
		  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
using SnapshotList = std::vector<SnapshotData<T, T2> >;






/**
 * @brief The SnapshotStorage class
 */

class SnapshotStorage {

public:
	SnapshotStorage() = default;


	SnapshotList<Player, BaseData> players() { QMutexLocker locker(&m_mutex); return m_players; }
	SnapshotList<Enemy, EnemyBaseData> enemies() { QMutexLocker locker(&m_mutex); return m_enemies; }


	SnapshotInterpolation<Player> getSnapshot(const BaseData &id, const qint64 &tick) {
		return getSnapshotInterpolation(m_players, id, tick);
	}

	SnapshotInterpolation<Enemy> getSnapshot(const EnemyBaseData &id, const qint64 &tick) {
		return getSnapshotInterpolation(m_enemies, id, tick);
	}

	FullSnapshot getFullSnapshot(const qint64 &tick);
	CurrentSnapshot getCurrentSnapshot();

	void zapSnapshots(const qint64 &tick);



protected:

	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
	SnapshotInterpolation<T> getSnapshotInterpolation(const SnapshotList<T, T2> &snapshots,
													  const T2 &id,
													  const qint64 &currentTick);


	template <typename T, typename = std::enable_if<std::is_base_of<Body, T>::value>::type>
	SnapshotInterpolation<T> getSnapshotInterpolation(const std::map<qint64, T> &map,
													  const qint64 &currentTick);


	template <typename T, typename = std::enable_if<std::is_base_of<Body, T>::value>::type>
	void zapSnapshots(std::map<qint64, T> &map, const qint64 &minTick);


	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
	void dumpSnapshots(QIODevice *device, const SnapshotList<T, T2> &snapshots) const;



	QRecursiveMutex m_mutex;

	SnapshotList<Player, BaseData> m_players;
	SnapshotList<Enemy, EnemyBaseData> m_enemies;
};







/**
 * @brief FullSnapshot::getSnapshot
 * @param data
 * @param list
 * @return
 */

template<typename T2, typename T, typename T3, typename T4>
inline std::optional<SnapshotInterpolation<T> > FullSnapshot::getSnapshot(const T2 &data,
																		  const Snapshot<T2, T> &list) const
{
	const auto it = std::find_if(list.cbegin(),
								 list.cend(),
								 [&data](const auto &ptr) {
		return ptr.first == data;
	});


	if (it != list.cend())
		return it->second;
	else
		return std::nullopt;
}





/**
 * @brief SnapshotStorage::getSnapshotInterpolation
 * @param snapshots
 * @param id
 * @param currentTick
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
inline SnapshotInterpolation<T> SnapshotStorage::getSnapshotInterpolation(const SnapshotList<T, T2> &snapshots,
																		  const T2 &id,
																		  const qint64 &currentTick)
{
	SnapshotInterpolation<T> sip;

	const qint64 time = currentTick - RPG_UDP_DELTA_MSEC;

	sip.current = time;

	if (time < 0)
		return sip;

	QMutexLocker locker(&m_mutex);

	const auto mapIt = std::find_if(snapshots.cbegin(), snapshots.cend(), [&id](const auto &b){
		return b.data == id;
	});

	if (mapIt == snapshots.cend()) {
		return sip;
	}

	return getSnapshotInterpolation<T>(mapIt->list, currentTick);
}




/**
 * @brief SnapshotStorage::getSnapshotInterpolation
 * @param map
 * @param currentTick
 * @return
 */

template<typename T, typename T2>
inline SnapshotInterpolation<T> SnapshotStorage::getSnapshotInterpolation(const std::map<qint64, T> &map,
																		  const qint64 &currentTick)
{
	SnapshotInterpolation<T> sip;

	const qint64 time = currentTick - RPG_UDP_DELTA_MSEC;

	sip.current = time;

	if (time < 0)
		return sip;

	QMutexLocker locker(&m_mutex);

	const auto it = map.lower_bound(time);					// lower_bound = greater or equal

	if (it != map.cend() && it != map.cbegin() && it->first > time)
		sip.s1 = std::prev(it)->second;
	else if (it != map.cend())
		sip.s1 = it->second;

	const auto uit = map.upper_bound(time);					// upper_bound = greater

	if (uit != map.cend())
		sip.s2 = uit->second;

	return sip;
}



/**
 * @brief SnapshotStorage::zapSnapshots
 * @param map
 * @param minTick
 */


template<typename T, typename T2>
inline void SnapshotStorage::zapSnapshots(std::map<qint64, T> &map, const qint64 &minTick)
{
	if (map.size() < 2)
		return;

	auto max = map.lower_bound(minTick);					// lower_bound = greater or equal

	if (max == map.end())
		max = std::prev(max);

	if (max == map.begin())
		return;

	map.erase(map.begin(), std::prev(max));
}






/**
 * @brief SnapshotStorage::dumpSnapshots
 * @param device
 * @param snapshots
 */

template<typename T, typename T2, typename T3, typename T4>
inline void SnapshotStorage::dumpSnapshots(QIODevice *device, const SnapshotList<T, T2> &snapshots) const
{
	Q_ASSERT(device);

	for (const SnapshotData<T, T2> &ptr : snapshots) {
		QJsonDocument d(ptr.data.toJson());
		device->write("------------------------------------\n");
		device->write(d.toJson());
		device->write("------------------------------------\n");

		for (const auto &[tick, body] : ptr.list) {
			device->write("*** ");
			device->write(QByteArray::number(tick));
			device->write(" ***\n");
			QJsonDocument d(body.toJson());
			device->write(d.toJson());
			device->write("\n\n");
		}
	}

	device->write("=====================================\n\n");
}



/**
 * @brief SnapshotStorage::getFullSnapshot
 * @param tick
 * @return
 */

inline FullSnapshot SnapshotStorage::getFullSnapshot(const qint64 &tick)
{
	QMutexLocker locker(&m_mutex);

	FullSnapshot s;

	for (const auto &[data, list] : m_players) {
		s.players.emplace_back(data, getSnapshotInterpolation(list, tick));
	}

	for (const auto &[data, list] : m_enemies) {
		s.enemies.emplace_back(data, getSnapshotInterpolation(list, tick));
	}

	return s;
}





/**
 * @brief SnapshotStorage::getCurrentSnapshot
 * @return
 */

inline CurrentSnapshot SnapshotStorage::getCurrentSnapshot()
{
	QMutexLocker locker(&m_mutex);

	CurrentSnapshot s;

	for (const auto &[data, list] : m_players) {
		if (list.empty())
			s.players.emplace_back(data, Player());
		else
			s.players.emplace_back(data, list.cend()->second);
	}

	for (const auto &[data, list] : m_enemies) {
		if (list.empty())
			s.enemies.emplace_back(data, Enemy());
		else
			s.enemies.emplace_back(data, list.cend()->second);
	}

	return s;
}



/**
 * @brief SnapshotStorage::zapSnapshots
 * @param tick
 */

inline void SnapshotStorage::zapSnapshots(const qint64 &tick)
{
	QMutexLocker locker(&m_mutex);

	if (tick <= 0)
		return;

	for (auto &[data, list] : m_players) {
		zapSnapshots(list, tick);
	}

	for (auto &[data, list] : m_enemies) {
		zapSnapshots(list, tick);
	}
}




};	// namespace RpgGameData



#endif // RPGCONFIG_H
