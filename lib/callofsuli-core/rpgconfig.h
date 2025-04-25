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

#include "qcborarray.h"
#include "qmutex.h"
#include <QSerializer>
#include <QIODevice>



#define RPG_UDP_DELTA_TICK		6				// Jitter buffer



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


#define EQUAL_OPERATOR(cname)	\
	bool operator==(const cname &r) const { return isEqual(r); } \
	bool operator!=(const cname &r) const { return !isEqual(r); }


namespace RpgGameData {



/**
 * @brief The LifeCycle class
 */

class LifeCycle
{
public:
	enum Stage {
		StageInvalid,
		StageCreate,
		StageLive,
		StageDead,
		StageDestroy
	};

	LifeCycle(const Stage &stage)
		: m_stage(stage)
	{}

	LifeCycle()
		: LifeCycle(StageInvalid)
	{}

	Stage stage() const { return m_stage; }
	void setStage(const Stage &newStage) { m_stage = newStage; }

protected:
	Stage m_stage = StageInvalid;

};



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
 * @brief The Weapon class
 */

class Weapon : public QSerializer
{
	Q_GADGET

public:
	enum WeaponType {
		WeaponInvalid = 0,
		WeaponHand,
		WeaponDagger,
		WeaponLongsword,
		WeaponShortbow,
		WeaponLongbow,
		WeaponBroadsword,
		WeaponAxe,
		WeaponHammer,
		WeaponMace,
		WeaponGreatHand,
		WeaponMageStaff [[deprecated]],
		WeaponLightningWeapon,
		WeaponFireFogWeapon,
		WeaponShield
	};

	Q_ENUM (WeaponType);


	Weapon(const WeaponType &_t, const int &_b = -1)
		: QSerializer()
		, t(_t)
		, b(_b)
	{}

	Weapon()
		: Weapon(WeaponInvalid)
	{}

	bool isEqual(const Weapon &other) const  {
		return other.t == t && other.b == b;
	}

	EQUAL_OPERATOR(Weapon);

	static const QHash<WeaponType, int> &damageValue() { return m_damageValue; }
	static const QHash<WeaponType, int> &protectValue() { return m_protectValue; }

private:
	static const QHash<WeaponType, int> m_damageValue;
	static const QHash<WeaponType, int> m_protectValue;

	QS_SERIALIZABLE

	QS_FIELD(WeaponType, t)			// type
	QS_FIELD(int, b)				// bullet count


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
		return other.f == f && other.sc == sc;
	}

	bool canMerge(const Body &other) const {
		return other.sc == sc;
	}

	EQUAL_OPERATOR(Body)

	QS_SERIALIZABLE

	QS_FIELD(qint64, f)					// frame
	QS_FIELD(int, sc)					// current scene
};




/**
 * @brief The Armory class
 */

class Armory : public QSerializer
{
	Q_GADGET

public:
	Armory()
		: QSerializer()
		, cw(Weapon::WeaponInvalid)
	{}

	bool isEqual(const Armory &other) const {
		return other.wl == wl && other.cw == cw;
	}

	QList<Weapon>::const_iterator find(const Weapon::WeaponType &type) const {
		return std::find_if(wl.cbegin(), wl.cend(),
							[&type](const Weapon &w) {
			return w.t == type;
		});
	}

	QList<Weapon>::iterator find(const Weapon::WeaponType &type) {
		return std::find_if(wl.begin(), wl.end(),
							[&type](const Weapon &w) {
			return w.t == type;
		});
	}


	EQUAL_OPERATOR(Armory)

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, Weapon, wl)			// weapon list
	QS_FIELD(Weapon::WeaponType, cw)					// current weapon
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
		return Body::isEqual(other) && other.p == p && other.a == a && other.hp == hp && other.mhp == mhp;
	}

	bool canMerge(const Entity &other) const {
		return Body::canMerge(other) && other.hp == hp && other.mhp == mhp;
	}

	EQUAL_OPERATOR(Entity)

	QS_SERIALIZABLE

	QS_COLLECTION(QList, float, p)		// position
	QS_COLLECTION(QList, float, cv)		// current linear velocity
	QS_FIELD(float, a)			// angle
	QS_FIELD(int, hp)			// HP
	QS_FIELD(int, mhp)			// max HP
};




/**
 * @brief The ArmoredEntityBaseData class
 */

class ArmoredEntityBaseData : public BaseData
{
	Q_GADGET

public:
	ArmoredEntityBaseData(const float &_df, const float &_pf, const int &_o, const int &_s, const int &_id)
		: BaseData(_o, _s, _id)
		, df(_df)
		, pf(_pf)
	{}

	ArmoredEntityBaseData(const float &_df, const float &_pf)
		: ArmoredEntityBaseData(_df, _pf, -1, -1, -1)
	{}

	ArmoredEntityBaseData()
		: ArmoredEntityBaseData(1., 1.)
	{}

	bool isEqual(const ArmoredEntityBaseData &other) const {
		return BaseData::isEqual(other) && other.df == df && other.pf == pf;
	}

	EQUAL_OPERATOR(ArmoredEntityBaseData)

	QS_SERIALIZABLE

	QS_FIELD(float, df)				// damage factor
	QS_FIELD(float, pf)				// protect factor
};






/**
 * @brief The ArmoredEntity class
 */

class ArmoredEntity : public Entity
{
	Q_GADGET

public:
	ArmoredEntity()
		: Entity()
	{}

	bool isEqual(const ArmoredEntity &other) const {
		return Entity::isEqual(other) && other.arm == arm;
	}

	bool canMerge(const ArmoredEntity &other) const {
		return Entity::canMerge(other) && other.arm == arm;
	}

	static void attacked(const ArmoredEntityBaseData &dstBase, ArmoredEntity &dst,
					   const Weapon::WeaponType &weapon, const ArmoredEntityBaseData &other);

	void attacked(const ArmoredEntityBaseData &dstBase,
				const Weapon::WeaponType &weapon, const ArmoredEntityBaseData &other)
	{
		attacked(dstBase, *this, weapon, other);
	}

	EQUAL_OPERATOR(ArmoredEntity)

	QS_SERIALIZABLE

	QS_OBJECT(Armory, arm)			// armory
};






/**
 * @brief The PlayerBaseData class
 */

class PlayerBaseData : public ArmoredEntityBaseData
{
	Q_GADGET

public:
	PlayerBaseData(const float &_df, const float &_pf, const int &_o, const int &_s, const int &_id)
		: ArmoredEntityBaseData(_df, _pf, _o, _s, _id)
	{}

	PlayerBaseData(const int &_o, const int &_s, const int &_id)
		: PlayerBaseData(1., 1., _o, _s, _id)
	{}

	PlayerBaseData()
		: PlayerBaseData(-1, -1, -1)
	{}

	bool isEqual(const PlayerBaseData &other) const {
		return ArmoredEntityBaseData::isEqual(other);
	}

	EQUAL_OPERATOR(PlayerBaseData)

	QS_SERIALIZABLE
};





/**
 * @brief The Player class
 */

class Player : public ArmoredEntity
{
	Q_GADGET

public:
	Player()
		: ArmoredEntity()
		, st(PlayerInvalid)
	{}

	enum PlayerState {
		PlayerInvalid = 0,
		PlayerIdle,
		PlayerMoving,
		PlayerHit,
		PlayerShot,
		PlayerCast,
		PlayerAttack
	};

	Q_ENUM(PlayerState);


	bool isEqual(const Player &other) const  {
		return ArmoredEntity::isEqual(other) && other.st == st && other.tg == tg;
	}

	bool canMerge(const Player &other) const {
		return ArmoredEntity::canMerge(other) && other.st == st && other.tg == tg;
	}

	EQUAL_OPERATOR(Player);

	QS_SERIALIZABLE

	QS_FIELD(PlayerState, st)			// state
	QS_OBJECT(BaseData, tg)				// target (enemy)
};






/**
 * @brief The EnemyBaseData class
 */

class EnemyBaseData : public ArmoredEntityBaseData
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

	EnemyBaseData(const EnemyType &_type, const float &_df, const float &_pf, const int &_o, const int &_s, const int &_id)
		: ArmoredEntityBaseData(_df, _pf, _o, _s, _id)
		, t(_type)
	{}

	EnemyBaseData(const EnemyType &_type, const int &_o, const int &_s, const int &_id)
		: EnemyBaseData(_type, 1., 1., _o, _s, _id)
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


class Enemy : public ArmoredEntity
{
	Q_GADGET

public:
	enum EnemyState {
		EnemyInvalid = 0,
		EnemyIdle,
		EnemyMoving,
		EnemyHit,
		EnemyShot,
		EnemyCast,
		EnemyAttack
	};

	Q_ENUM(EnemyState);

	Enemy(const EnemyState &_st)
		: ArmoredEntity()
		, st(_st)
	{}

	Enemy()
		: Enemy(EnemyInvalid)
	{}


	bool isEqual(const Enemy &other) const {
		return ArmoredEntity::isEqual(other) && other.st == st && other.tg == tg;
	}

	bool canMerge(const Enemy &other) const {
		return ArmoredEntity::canMerge(other) && other.st == st && other.tg == tg;
	}

	EQUAL_OPERATOR(Enemy)

	QS_SERIALIZABLE

	QS_FIELD(EnemyState, st)			// enemy state
	QS_OBJECT(BaseData, tg)				// target (player)

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
	Pickable() = default;

	bool isEqual(const Pickable &other) const {
		return Body::isEqual(other);
	}

	EQUAL_OPERATOR(Pickable)

	QS_SERIALIZABLE

};






/**
 * @brief The BulletBaseData class
 */

class BulletBaseData : public BaseData
{
	Q_GADGET

public:
	enum Owner {
		OwnerNone = 0,
		OwnerPlayer = 1,
		OnwerEnemy
	};

	Q_ENUM(Owner)

	enum Target {
		TargetNone = 0,
		TargetEnemy = 1,
		TargetPlayer = 1 << 1,
		TargetGround = 1 << 2,

		TargetAll = TargetEnemy|TargetPlayer|TargetGround
	};

	Q_ENUM(Target)
	Q_DECLARE_FLAGS(Targets, Target)
	Q_FLAG(Targets)



	BulletBaseData(const Weapon::WeaponType &_type,
				   const int &_o, const int &_s, const int &_id,
				   const Owner &_own, const BaseData &_ownId,
				   const Targets &_tar)
		: BaseData(_o, _s, _id)
		, t(_type)
		, own(_own)
		, tar(_tar)
		, ownId(_ownId)
	{}

	BulletBaseData(const Weapon::WeaponType &_type,
				   const Owner &_own, const BaseData &_ownId,
				   const Targets &_tar)
		: BulletBaseData(_type, -1, -1, -1, _own, _ownId, _tar)
	{}

	BulletBaseData(const Weapon::WeaponType &_type, const Owner &_own, const Targets &_tar)
		: BulletBaseData(_type, -1, -1, -1, _own, BaseData(-1, -1, -1), _tar)
	{}

	BulletBaseData(const Weapon::WeaponType &_type)
		: BulletBaseData(_type, OwnerNone, TargetNone)
	{}

	BulletBaseData()
		: BulletBaseData(Weapon::WeaponInvalid)
	{}


	bool isEqual(const BulletBaseData &other) const {
		return BaseData::isEqual(other) && other.t == t && other.own == own &&
				other.tar == tar && other.ownId == ownId && other.pth == pth;
	}

	EQUAL_OPERATOR(BulletBaseData)

	QS_SERIALIZABLE

	QS_FIELD(Weapon::WeaponType, t)		// weapon
	QS_FIELD(Owner, own)				// owner
	QS_FIELD(Targets, tar)				// targets
	QS_OBJECT(BaseData, ownId)			// ownerId
	QS_COLLECTION(QList, float, pth)	// path (x1, y1, x2, y2, ...)
};


Q_DECLARE_OPERATORS_FOR_FLAGS(BulletBaseData::Targets);



/**
 * @brief The Bullet class
 */

class Bullet : public Body
{
	Q_GADGET

public:
	Bullet(const int &_sc)
		: Body()
		, st(LifeCycle::StageInvalid)
	{
		sc = _sc;
	}

	Bullet()
		: Bullet(-1)
	{ }


	bool isEqual(const Bullet &other) const {
		return Body::isEqual(other) && other.p == p && other.st == st;
	}

	bool canMerge(const Bullet &other) const {
		return Body::canMerge(other) && other.st == st && other.tg == tg;
	}

	EQUAL_OPERATOR(Bullet)

	QS_SERIALIZABLE

	QS_FIELD(float, p)					// progress on path
	QS_FIELD(LifeCycle::Stage, st)		// stage
	QS_OBJECT(BaseData, tg)				// impacted target id

};






/**
 * @brief The SnapshotInterpolation class
 */

template <typename T, typename = std::enable_if<std::is_base_of<Body, T>::value>::type>
struct SnapshotInterpolation {
	T s1;
	T s2;
	T last;
	qint64 current = -1;

	void clear() {
		s1 = {};
		s2 = {};
		last = {};
		current = -1;
	}
};


template <typename T2, typename T,
		  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
		  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
using SnapshotInterpolationList = std::vector<std::pair<T2, SnapshotInterpolation<T> > >;



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
 * @brief The FullSnapshot class
 */

struct FullSnapshot {
	SnapshotInterpolationList<PlayerBaseData, Player> players;
	SnapshotInterpolationList<EnemyBaseData, Enemy> enemies;
	SnapshotInterpolationList<BulletBaseData, Bullet> bullets;

	void clear() {
		players.clear();
		enemies.clear();
		bullets.clear();
	}

	template <typename T2, typename T,
			  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
	std::optional<SnapshotInterpolation<T> > getSnapshot(const T2 &data, const SnapshotInterpolationList<T2, T> &list) const;


	std::optional<SnapshotInterpolation<Player> > getPlayer(const PlayerBaseData &data) const {
		return getSnapshot(data, players);
	}
	std::optional<SnapshotInterpolation<Enemy> > getEnemy(const EnemyBaseData &data) const {
		return getSnapshot(data, enemies);
	}
	std::optional<SnapshotInterpolation<Bullet> > getBullet(const BulletBaseData &data) const {
		return getSnapshot(data, bullets);
	}
};





/**
 * @brief The CurrentSnapshot class
 */

struct CurrentSnapshot {
	SnapshotList<Player, PlayerBaseData> players;
	SnapshotList<Enemy, EnemyBaseData> enemies;
	SnapshotList<Bullet, BulletBaseData> bullets;

	void clear() {
		players.clear();
		enemies.clear();
		bullets.clear();
	}


	template <typename T, typename T2>
	QCborArray toCborArray(const SnapshotList<T, T2> &list, const QString &keyBase, const QString &keyData,
						   const std::function<void(QCborMap*)> &func) const;

	template <typename T, typename T2>
	static int fromCborArray(SnapshotList<T, T2> &dest, const QCborArray &src, const QString &keyBase, const QString &keyData,
							 const std::function<void(QCborMap*)> &func);

	template <typename T>
	static int fromCborArray(std::map<qint64, T> &dest, const QCborArray &src,
							 const std::function<void(QCborMap*)> &func);

	QCborMap toCbor() const;
	QCborMap toProtectedCbor() const;

	int fromCbor(const QCborMap &map);
	int fromProtectedCbor(const QCborMap &map);

	template <typename T, typename T2>
	static SnapshotList<T, T2>::iterator find(SnapshotList<T, T2> &list, const T2 &src);

	template <typename T, typename T2>
	static SnapshotList<T, T2>::const_iterator find(const SnapshotList<T, T2> &list, const T2 &src);

	template <typename T>
	static void copy(std::map<qint64, T> &dest, const std::map<qint64, T> &src);

	static void removeEntityProtectedFields(QCborMap *map);
	static void removeArmoredEntityProtectedFields(QCborMap *map);
	static void removeEnemyProtectedFields(QCborMap *map);
	static void removePlayerProtectedFields(QCborMap *map);
	static void removeBulletProtectedFields(QCborMap *map);
};





/**
 * @brief The SnapshotStorage class
 */

class SnapshotStorage {

public:
	SnapshotStorage() = default;


	SnapshotList<Player, PlayerBaseData> players() { QMutexLocker locker(&m_mutex); return m_players; }
	SnapshotList<Enemy, EnemyBaseData> enemies() { QMutexLocker locker(&m_mutex); return m_enemies; }
	SnapshotList<Bullet, BulletBaseData> bullets() { QMutexLocker locker(&m_mutex); return m_bullets; }


	SnapshotInterpolation<Player> getSnapshot(const PlayerBaseData &id, const qint64 &tick) {
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
	static void zapSnapshots(std::map<qint64, T> &map, const qint64 &minTick);


	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
	void dumpSnapshots(QIODevice *device, const SnapshotList<T, T2> &snapshots) const;


	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
	SnapshotList<T, T2> convertToSnapshotList(SnapshotList<T, T2> &list);



	QRecursiveMutex m_mutex;

	SnapshotList<Player, PlayerBaseData> m_players;
	SnapshotList<Enemy, EnemyBaseData> m_enemies;
	SnapshotList<Bullet, BulletBaseData> m_bullets;
};





/**
 * @brief CurrentSnapshot::copy
 * @param dest
 * @param src
 */

template<typename T>
inline void CurrentSnapshot::copy(std::map<qint64, T> &dest, const std::map<qint64, T> &src)
{
	for (const auto &ptr : src) {
		dest.insert_or_assign(ptr.first, ptr.second);
	}
}



/**
 * @brief CurrentSnapshot::find
 * @param list
 * @param src
 * @return
 */

template<typename T, typename T2>
inline SnapshotList<T, T2>::const_iterator CurrentSnapshot::find(const SnapshotList<T, T2> &list, const T2 &src)
{
	return std::find_if(list.cbegin(),
						list.cend(),
						[&src](const auto &ptr) {
		return ptr.data == src;
	});
}



/**
 * @brief CurrentSnapshot::find
 * @param list
 * @param src
 * @return
 */

template<typename T, typename T2>
inline SnapshotList<T, T2>::iterator CurrentSnapshot::find(SnapshotList<T, T2> &list, const T2 &src)
{
	return std::find_if(list.begin(),
						list.end(),
						[&src](const auto &ptr) {
		return ptr.data == src;
	});
}






/**
 * @brief FullSnapshot::getSnapshot
 * @param data
 * @param list
 * @return
 */

template<typename T2, typename T, typename T3, typename T4>
inline std::optional<SnapshotInterpolation<T> > FullSnapshot::getSnapshot(const T2 &data,
																		  const SnapshotInterpolationList<T2, T> &list) const
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

	const qint64 time = currentTick - RPG_UDP_DELTA_TICK;

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

	qint64 time = currentTick - RPG_UDP_DELTA_TICK;

	sip.current = time;

	if (!map.empty())
		sip.last = std::prev(map.cend())->second;

	if (time < 0)
		return sip;

	QMutexLocker locker(&m_mutex);

	const auto it = map.lower_bound(time);					// lower_bound = greater or equal

	if (it != map.cend() && it != map.cbegin() && it->first > time)
		sip.s1 = std::prev(it)->second;
	else if (it != map.cend())
		sip.s1 = it->second;

	auto uit = map.upper_bound(time);					// upper_bound = greater

	if (uit != map.cend()) {
		const T s2 = uit->second;
		sip.s2 = uit->second;

		// Merge the similar snapshots

		for (++uit; uit != map.cend(); ++uit) {
			if (s2.canMerge(uit->second))
				sip.s2 = uit->second;
			else
				break;
		}

	}

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
 * @brief CurrentSnapshot::toCborArray
 * @param list
 * @return
 */

template<typename T, typename T2>
inline QCborArray CurrentSnapshot::toCborArray(const SnapshotList<T, T2> &list,
											   const QString &keyBase, const QString &keyData,
											   const std::function<void(QCborMap*)> &func) const
{
	QCborArray array;

	if (keyBase.isEmpty() && keyData.isEmpty())
		return array;

	for (const auto &ptr : list) {
		QCborMap m;

		if (!keyBase.isEmpty())
			m.insert(keyBase, ptr.data.toCborMap(true));

		if (!keyData.isEmpty()) {
			QCborArray a;

			std::optional<QCborMap> prev;

			for (const auto &it : ptr.list) {
				QCborMap fullmap = it.second.toCborMap(true);

				if (func)
					func(&fullmap);

				if (prev) {
					QCborMap map = it.second.toCborMap(prev.value(), true);

					if (func)
						func(&map);

					QCborMap delta = map;
					delta.remove(QStringLiteral("f"));

					if (delta.isEmpty())
						continue;

					a.append(map);
				} else {
					a.append(fullmap);
				}

				prev = fullmap;
			}

			m.insert(keyData, a);
		}

		array.append(m);
	}

	return array;
}





/**
 * @brief CurrentSnapshot::fromCborArray
 * @param dest
 * @param src
 * @param keyBase
 * @param keyData
 * @return
 */

template<typename T, typename T2>
inline int CurrentSnapshot::fromCborArray(SnapshotList<T, T2> &dest, const QCborArray &src,
										  const QString &keyBase, const QString &keyData,
										  const std::function<void(QCborMap*)> &func)
{
	if (keyBase.isEmpty() || keyData.isEmpty())
		return -1;

	if (src.isEmpty())
		return 0;

	int r = 0;

	for (const QCborValue &v : src) {
		const QCborMap &m = v.toMap();

		T2 base;

		if (const QCborMap::const_iterator &it = m.constFind(keyBase); it != m.constEnd()) {
			base.fromCbor(it.value());
		} else {
			continue;
		}

		const QCborArray &list = m.value(keyData).toArray();

		auto it = std::find_if(dest.begin(),
							   dest.end(),
							   [&base](const auto &ptr) {
			return ptr.data == base;
		});

		if (it == dest.end()) {
			std::map<qint64, T> map;

			r += fromCborArray(map, list, func);

			dest.emplace_back(base, map);
		} else {
			r += fromCborArray(it->list, list, func);
		}
	}

	return r;
}




/**
 * @brief CurrentSnapshot::fromCborArray
 * @param dest
 * @param src
 * @return
 */

template<typename T>
inline int CurrentSnapshot::fromCborArray(std::map<qint64, T> &dest, const QCborArray &src,
										  const std::function<void(QCborMap*)> &func)
{
	T prev;

	for (const QCborValue &v : src) {
		QCborMap m = v.toMap();

		if (func)
			func(&m);

		prev.fromCbor(m);
		dest.insert_or_assign(prev.f, prev);
	}

	return src.size();
}







/**
 * @brief SnapshotStorage::toCurrentSnapshotList
 * @param list
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
inline SnapshotList<T, T2> SnapshotStorage::convertToSnapshotList(SnapshotList<T, T2> &list)
{
	SnapshotList<T, T2> ret;

	for (SnapshotData<T, T2> &ptr : list) {
		if (ptr.list.empty()) {
			ret.emplace_back(ptr.data, std::map<qint64, T>());
			continue;
		}

		std::map<qint64, T> array;

		// Merge similar snapshots

		typename std::map<qint64, T>::const_iterator base = ptr.list.cend();
		typename std::map<qint64, T>::const_iterator last = ptr.list.cend();

		for (auto it = ptr.list.cbegin(); it != ptr.list.cend(); ++it) {
			if (base != ptr.list.cend()) {
				if (base->second.canMerge(it->second)) {
					last = it;
					continue;
				}
			}

			if (last != ptr.list.cend()) {
				array.insert_or_assign(last->second.f, last->second);
				last = ptr.list.cend();
			}

			array.insert_or_assign(it->second.f, it->second);
			///ptr.lastOut = it->first;

			base = it;
		}

		if (last != ptr.list.cend()) {
			array.insert_or_assign(last->second.f, last->second);
			///ptr.lastOut = last->first;
		}

		ret.emplace_back(ptr.data, array);
	}

	return ret;
}




};	// namespace RpgGameData



#endif // RPGCONFIG_H
