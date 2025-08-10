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

#include "credential.h"
#include "qcborarray.h"
#include "qpoint.h"
#include <QSerializer>
#include <QIODevice>
#include <QColor>



#define RPG_UDP_DELTA_TICK		6				// Jitter buffer


/**
 * @brief The RpgConfigBase class
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

	bool operator==(const RpgConfigBase &other) const {
		return other.mapUuid == mapUuid &&
				other.missionUuid == missionUuid &&
				other.missionLevel == missionLevel &&
				other.campaign == campaign;
	}

	QS_SERIALIZABLE
	QS_FIELD(QString, mapUuid)
	QS_FIELD(QString, missionUuid)
	QS_FIELD(int, missionLevel)
	QS_FIELD(int, campaign)
	QS_FIELD(int, duration)
};





/**
 * @brief The RpgConfig class
 */

class RpgConfig : public RpgConfigBase
{
	Q_GADGET

public:
	RpgConfig()
		: RpgConfigBase()
		, gameState(StateInvalid)
	{}

	RpgConfig(const RpgConfigBase &base)
		: RpgConfigBase(base)
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


	enum ControlType {
		ControlInvalid = 0,
		ControlContainer,
		ControlGate,
		ControlLight,
		ControlCollection,
		ControlPickable,
		ControlRandomizer,
		ControlTeleport,
		ControlExit
	};

	Q_ENUM(ControlType)


	void reset() {
		gameState = StateInvalid;
	}

	QJsonObject toBaseJson() const { return RpgConfigBase::toJson(); }

	bool operator==(const RpgConfig &other) const {
		return gameState == other.gameState &&
				static_cast<const RpgConfigBase &>(*this) == static_cast<const RpgConfigBase &>(other)
				;
	}

	bool operator==(const RpgConfigBase &other) const {
		return static_cast<const RpgConfigBase &>(*this) == other;
	}

	static const QHash<ControlType, int> &controlDamageValue() { return m_controlDamageValue; }

private:
	static const QHash<ControlType, int> m_controlDamageValue;

	QS_SERIALIZABLE
	QS_FIELD(GameState, gameState)
};


Q_DECLARE_METATYPE(RpgConfig)





/**
 * @brief The RpgPlayerConfig class
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
		, belongsValue(0)
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
	QS_FIELD(QString, belongs)				// belongs to a character (with base of)
	QS_FIELD(int, belongsValue)				// the biggest value will be activated

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


	bool isEqual(const RpgMarket &market) const;
	void setFromMarket(const RpgMarket &market);

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
		SuddenDeath		= 3,
		NoKill			= 4
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

	LifeCycle() = default;
	~LifeCycle() = default;

	virtual Stage stage() const = 0;
	virtual void setStage(const Stage &newStage) = 0;

	void destroy(const qint64 &tick);
	const qint64 &destroyTick() const { return m_destroyTick; }

protected:
	qint64 m_destroyTick = -1;

};




/**
 * @brief The Message class
 */

class Message : public QSerializer
{
	Q_GADGET

public:
	Message(const QString &msg = {}, const QColor &color = QColor(), const bool &priority = false)
		: QSerializer()
		, m(msg)
		, p(priority)
	{
		if (color.isValid())
			c = color.name();
	}

	Message(const QString &msg, const bool &priority)
		: Message(msg, QColor(), priority)
	{}

	QColor color() const { return c.isEmpty() ? QColor() : QColor::fromString(c); }
	void setColor(const QColor &color) {
		if (color.isValid())
			c = color.name();
		else
			c.clear();
	}

	QS_SERIALIZABLE

	QS_FIELD(QString, m)				// message text
	QS_FIELD(bool, p)					// priority
	QS_FIELD(QString, c)				// color
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
 * @brief The CollectionPlace class
 */

class CollectionPlace : public QSerializer
{
	Q_GADGET

public:

	CollectionPlace(const float &_x, const float &_y, const bool &_done = false)
		: QSerializer()
		, x(_x)
		, y(_y)
		, done(_done)
	{}


	CollectionPlace(const QPointF &pos, const bool &_done = false)
		: CollectionPlace(pos.x(), pos.y(), _done)
	{}


	CollectionPlace()
		: RpgGameData::CollectionPlace(0, 0)
	{}


	QS_SERIALIZABLE

	QS_FIELD(float, x)
	QS_FIELD(float, y)
	QS_FIELD(bool, done)						// once successful collected
};





/**
 * @brief The CollectionGroup class
 */

class CollectionGroup : public QSerializer
{
	Q_GADGET

public:

	CollectionGroup(const int &_scene, const int &_id)
		: QSerializer()
		, scene(_scene)
		, id(_id)
	{}


	CollectionGroup()
		: CollectionGroup(-1, -1)
	{}


	QS_SERIALIZABLE

	QS_FIELD(int, scene)
	QS_FIELD(int, id)
	QS_COLLECTION_OBJECTS(QList, CollectionPlace, pos)
};






/**
 * @brief The Collection class
 */

class Collection : public QSerializer
{
	Q_GADGET

public:

	Collection()
		: QSerializer()
	{}

	QHash<int, QList<int> > allocate(const int &num, int *dst = nullptr);
	QList<CollectionPlace> getFree(const int &gid) const;

	QList<CollectionGroup>::iterator find(const int &id);
	QList<CollectionGroup>::const_iterator find(const int &id) const;

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, CollectionGroup, groups)
	QS_COLLECTION(QList, int, images)							// A lehetséges képek (image layer) azonosítói
	QS_FIELD(QString, quest)									// A játék elején megjelenő quest
};







/**
 * @brief The RandomizerGroup class
 */

class RandomizerGroup : public QSerializer
{
	Q_GADGET

public:

	RandomizerGroup(const int &_scene, const int &_id)
		: QSerializer()
		, scene(_scene)
		, gid(_id)
		, current(-1)
	{}


	RandomizerGroup()
		: RandomizerGroup(-1, -1)
	{}


	void randomize();

	QS_SERIALIZABLE

	QS_FIELD(int, scene)
	QS_FIELD(int, gid)
	QS_FIELD(int, current)
	QS_COLLECTION(QList, int, idList)
};




/**
 * @brief The Randomizer class
 */

class Randomizer : public QSerializer
{
	Q_GADGET

public:

	Randomizer()
		: QSerializer()
	{}

	QList<RandomizerGroup>::iterator find(const int &id);
	QList<RandomizerGroup>::const_iterator find(const int &id) const;

	void randomize();

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, RandomizerGroup, groups)
};







/**
 * @brief The ConnectToken class
 */

class ConnectionToken : public UdpToken
{
	Q_GADGET

public:
	ConnectionToken()
		: UdpToken(Rpg)
	{}

	ConnectionToken(const QString &_user, const quint32 &_peer, const qint64 &_exp)
		: UdpToken(Rpg, _user, _peer, _exp)
	{}

	QS_SERIALIZABLE

	QS_OBJECT(RpgConfigBase, config)		// mission level data
};






/**
 * @brief The EnginePlayer class
 */

class EnginePlayer : public QSerializer
{
	Q_GADGET

public:
	EnginePlayer()
		: QSerializer()
	{}

	EnginePlayer(const QString &_username, const QString &_nickname)
		: QSerializer()
		, username(_username)
		, nickname(_nickname)
	{}

	QS_SERIALIZABLE

	QS_FIELD(QString, username)
	QS_FIELD(QString, nickname)
};





/**
 * @brief The Engine class
 */

class Engine : public QSerializer
{
	Q_GADGET

public:
	Engine()
		: QSerializer()
		, id(0)
		, readableId(0)
		, count(0)
	{}

	QVariantMap toVariantMap() const {
		QVariantMap m = this->toJson().toVariantMap();
		m.remove(QStringLiteral("players"));
		QVariantList list;
		for (const EnginePlayer &p : players)
			list << p.toJson().toVariantMap();
		m.insert(QStringLiteral("players"), list);
		return m;
	}

	QS_SERIALIZABLE

	QS_FIELD(int, id)
	QS_FIELD(int, readableId)
	QS_OBJECT(EnginePlayer, owner)
	QS_COLLECTION_OBJECTS(QList, EnginePlayer, players)
	QS_FIELD(int, count)				// max. players

};






/**
 * @brief The EngineSelector class
 */

class EngineSelector : public QSerializer
{
	Q_GADGET

public:
	enum Operation {
		Invalid,
		List,				// list available engines (-> engines)
		Connect,			// connect to selected engine (engine ->)
		Create,				// create new engine
		Reset				// reset my engine
	};

	Q_ENUM(Operation)

	EngineSelector(const Operation &_op, const int &_engine = 0)
		: QSerializer()
		, operation(_op)
		, engine(_engine)
		, add(false)
	{}

	EngineSelector()
		: EngineSelector(Invalid)
	{}

	QS_SERIALIZABLE

	QS_FIELD(Operation, operation)
	QS_COLLECTION_OBJECTS(QList, Engine, engines)
	QS_FIELD(int, engine)
	QS_FIELD(bool, add)			// can create new engine
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
		, duration(0)
	{}

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, PlayerPosition, positionList)
	QS_FIELD(QString, terrain)
	QS_OBJECT(Collection, collection)
	QS_OBJECT(Randomizer, randomizer)
	QS_FIELD(int, duration)
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
		WeaponLightningWeapon,
		WeaponFireFogWeapon,
		WeaponShield
	};

	Q_ENUM (WeaponType);


	Weapon(const WeaponType &_t, const int &_s = 0, const int &_b = -1)
		: QSerializer()
		, t(_t)
		, s(_s)
		, b(_b)
	{}

	Weapon()
		: Weapon(WeaponInvalid)
	{}

	bool isEqual(const Weapon &other) const  {
		return other.t == t && other.b == b && other.s == s;
	}

	EQUAL_OPERATOR(Weapon);

	static const QHash<QPair<WeaponType, int>, int> &damageValue() { return m_damageValue; }
	static const QHash<QPair<WeaponType, int>, int> &protectValue() { return m_protectValue; }

private:
	static const QHash<QPair<WeaponType, int>, int> m_damageValue;
	static const QHash<QPair<WeaponType, int>, int> m_protectValue;

	QS_SERIALIZABLE

	QS_FIELD(WeaponType, t)			// type
	QS_FIELD(int, s)				// subtype
	QS_FIELD(int, b)				// bullet count


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
		, s(0)
	{}

	bool isEqual(const Armory &other) const {
		return other.wl == wl && other.cw == cw && other.s == s;
	}

	QList<Weapon>::const_iterator find(const Weapon::WeaponType &type, const int &subType) const {
		return std::find_if(wl.cbegin(), wl.cend(),
							[&type, &subType](const Weapon &w) {
			return w.t == type && w.s == subType;
		});
	}

	QList<Weapon>::iterator find(const Weapon::WeaponType &type, const int &subType) {
		return std::find_if(wl.begin(), wl.end(),
							[&type, &subType](const Weapon &w) {
			return w.t == type && w.s == subType;
		});
	}

	const Weapon &add(const Weapon::WeaponType &type, const int &subType, const int &bullet = 1) {
		auto it = find(type, subType);
		if (it != wl.end()) {
			it->b += bullet;
			return *it;
		} else {
			return wl.emplace_back(type, subType, bullet);
		}
	}

	bool addBullet(const int &bullet);


	EQUAL_OPERATOR(Armory)

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, Weapon, wl)			// weapon list
	QS_FIELD(Weapon::WeaponType, cw)					// current weapon
	QS_FIELD(int, s)									// current weapon subtype
};





/**
 * @brief The CharacterSelect class
 */

class CharacterSelect : public QSerializer
{
	Q_GADGET

public:
	CharacterSelect()
		: QSerializer()
		, playerId(-1)
		, completed(false)
		, locked(false)
		, maxHp(0)
		, maxMp(0)
		, mp(0)
		, lastObjectId(-1)
		, xp(0)
		, cur(0)
		, kill(0)
	{}

	CharacterSelect(const RpgPlayerConfig &config)
		: CharacterSelect()
	{
		username = config.username;
		nickname = config.nickname;
		character = config.character;
	}

	QS_SERIALIZABLE

	QS_FIELD(int, playerId)
	QS_FIELD(QString, username)
	QS_FIELD(QString, nickname)

	QS_FIELD(QString, character)
	QS_FIELD(bool, completed)
	QS_FIELD(bool, locked)											// lock the engine

	// Character specification

	QS_OBJECT(Armory, armory)
	QS_FIELD(int, maxHp)
	QS_FIELD(int, maxMp)
	QS_FIELD(int, mp)

	QS_OBJECT(GameConfig, gameConfig)

	QS_FIELD(int, lastObjectId)

	QS_FIELD(bool, finished)										// finished
	QS_FIELD(int, xp)												// XP
	QS_FIELD(int, cur)												// currency
	QS_FIELD(int, kill)												// killed enemies
};





/**
 * @brief The CharacterSelectServer class
 */

class CharacterSelectServer : public QSerializer
{
	Q_GADGET

public:
	CharacterSelectServer()
		: QSerializer()
		, locked(false)
		, max(0)
		, engineReadableId(0)
	{}


	QS_SERIALIZABLE

	QS_OBJECT(GameConfig, gameConfig)
	QS_COLLECTION_OBJECTS(QList, CharacterSelect, players)
	QS_FIELD(bool, locked)
	QS_FIELD(int, max)
	QS_FIELD(int, engineReadableId)
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
		, count(0)
		, avg(0.)
		, loaded(false)
	{}

	QS_SERIALIZABLE

	QS_FIELD(bool, prepared)
	QS_OBJECT(GameConfig, gameConfig)
	QS_FIELD(int, count)						// Question count
	QS_FIELD(float, avg)						// Average question duration
	QS_FIELD(bool, loaded)						// A host már betöltött mindent
	QS_COLLECTION_OBJECTS(QList, CharacterSelect, players)
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

	bool isBaseEqual(const BaseData &other) const {
		return other.o == o && other.s == s && other.id == id;
	}

	bool isEqual(const BaseData &other) const {
		return other.o == o && other.s == s && other.id == id;
	}

	bool isValid() const {
		return o >= 0 || (s >= 0 && id >= 0);
	}

	friend QDebug operator<<(QDebug debug, const BaseData &c) {
		QDebugStateSaver saver(debug);
		debug.nospace() << '(' << c.o << ',' << c.s << ',' << c.id << ')';
		return debug;
	};

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

	bool canInterpolateFrom(const Body &other) const {
		return other.sc == sc && f > other.f;
	}

	EQUAL_OPERATOR(Body)

	QS_SERIALIZABLE

	QS_FIELD(qint64, f)					// frame
	QS_FIELD(int, sc)					// current scene
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
	{}


	bool isEqual(const Entity &other) const {
		return Body::isEqual(other) && other.p == p && other.a == a && other.hp == hp;
	}

	bool canMerge(const Entity &other) const {
		return Body::canMerge(other) && other.hp == hp && other.cv == cv && other.a == a;
	}

	bool canInterpolateFrom(const Entity &other) const;

	EQUAL_OPERATOR(Entity)

	QS_SERIALIZABLE

	QS_COLLECTION(QList, float, p)		// position
	QS_COLLECTION(QList, float, cv)		// current linear velocity
	QS_FIELD(float, a)			// angle
	QS_FIELD(int, hp)			// HP
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
 * @brief The BulletBaseData class
 */

class BulletBaseData : public BaseData
{
	Q_GADGET

public:
	enum Owner {
		OwnerNone = 0,
		OwnerPlayer = 1,
		OwnerEnemy
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



	BulletBaseData(const Weapon::WeaponType &_type, const int &_st,
				   const int &_o, const int &_s, const int &_id,
				   const Owner &_own, const BaseData &_ownId,
				   const Targets &_tar)
		: BaseData(_o, _s, _id)
		, t(_type)
		, ts(_st)
		, own(_own)
		, tar(_tar)
		, ownId(_ownId)
	{}

	BulletBaseData(const Weapon::WeaponType &_type, const int &_st,
				   const Owner &_own, const BaseData &_ownId,
				   const Targets &_tar)
		: BulletBaseData(_type, _st, -1, -1, -1, _own, _ownId, _tar)
	{}

	BulletBaseData(const Weapon::WeaponType &_type, const int &_st, const Owner &_own, const Targets &_tar)
		: BulletBaseData(_type, _st, -1, -1, -1, _own, BaseData(-1, -1, -1), _tar)
	{}

	BulletBaseData(const Weapon::WeaponType &_type, const int &_st = 0)
		: BulletBaseData(_type, _st, OwnerNone, TargetNone)
	{}

	BulletBaseData()
		: BulletBaseData(Weapon::WeaponInvalid)
	{}


	bool isEqual(const BulletBaseData &other) const {
		return BaseData::isEqual(other) && other.t == t && other.ts == ts && other.own == own &&
				other.tar == tar && other.ownId == ownId && other.pth == pth;
	}

	EQUAL_OPERATOR(BulletBaseData)

	QS_SERIALIZABLE

	QS_FIELD(Weapon::WeaponType, t)		// weapon
	QS_FIELD(int, ts)					// weapon subtype
	QS_FIELD(Owner, own)				// owner
	QS_FIELD(Targets, tar)				// targets
	QS_OBJECT(BaseData, ownId)			// ownerId
	QS_COLLECTION(QList, float, pth)	// path (x1, y1, x2, y2, ...)
};


Q_DECLARE_OPERATORS_FOR_FLAGS(BulletBaseData::Targets);



/**
 * @brief The Bullet class
 */

class Bullet : public Body, public LifeCycle
{
	Q_GADGET

public:
	Bullet(const int &_sc)
		: Body()
		, LifeCycle()
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

	bool canInterpolateFrom(const Bullet &other) const {
		return isEqual(other);
	}

	virtual Stage stage() const { return st; }
	virtual void setStage(const Stage &newStage) { st = newStage; }


	EQUAL_OPERATOR(Bullet)

	QS_SERIALIZABLE

	QS_FIELD(float, p)					// progress on path
	QS_FIELD(LifeCycle::Stage, st)		// stage
	QS_OBJECT(BaseData, tg)				// impacted target id

};




/**
 * @brief The RpgControlBase class
 */

class ControlBaseData : public BaseData
{
	Q_GADGET

public:
	ControlBaseData(const RpgConfig::ControlType &_t,
					const int &_o, const int &_s, const int &_id)
		: BaseData(_o, _s, _id)
		, t(_t)
	{}

	ControlBaseData(const RpgConfig::ControlType &_t)
		: ControlBaseData(_t, -1, -1, -1)
	{}

	ControlBaseData()
		: ControlBaseData(RpgConfig::ControlInvalid)
	{}

	bool isEqual(const ControlBaseData &other) const {
		return BaseData::isEqual(other) && other.t == t;
	}

	EQUAL_OPERATOR(ControlBaseData)

	QS_SERIALIZABLE

	QS_FIELD(RpgConfig::ControlType, t)
};






/**
 * @brief The ControlActiveBaseData class
 */

class ControlActiveBaseData : public ControlBaseData
{
	Q_GADGET

public:

	ControlActiveBaseData(const RpgConfig::ControlType &_t,
						  const int &_o, const int &_s, const int &_id)
		: ControlBaseData(_t, _o, _s, _id)
	{}

	ControlActiveBaseData(const RpgConfig::ControlType &_t)
		: ControlBaseData(_t, -1, -1, -1)
	{}

	ControlActiveBaseData()
		: ControlBaseData(RpgConfig::ControlInvalid)
	{}

	bool isEqual(const ControlActiveBaseData &other) const {
		return ControlBaseData::isEqual(other) && other.lck == lck;
	}

	EQUAL_OPERATOR(ControlActiveBaseData)

	QS_SERIALIZABLE

	QS_FIELD(QString, lck)				// keylock
};





/**
 * @brief The Control class
 */

class Control : public Body
{
	Q_GADGET

public:
	Control() : Body() {}

	bool isEqual(const Control &other) const {
		return Body::isEqual(other);
	}

	bool canMerge(const Control &other) const {
		return Body::canMerge(other) ;
	}

	bool canInterpolateFrom(const Control &other) const {
		return isEqual(other);
	}

	EQUAL_OPERATOR(Control)

	QS_SERIALIZABLE

	QS_OBJECT(BaseData, u)				// held by user
};




/**
 * @brief The ControlLight class
 */

class ControlLight : public Control
{
	Q_GADGET

public:
	ControlLight()
		: Control()
		, st(LightOff)
	{}

	enum State {
		LightOff = 0,
		LightOn
	};

	Q_ENUM(State)

	bool isEqual(const ControlLight &other) const {
		return Control::isEqual(other) && st == other.st;
	}

	bool canMerge(const ControlLight &other) const {
		return Control::canMerge(other) && st == other.st;
	}

	bool canInterpolateFrom(const ControlLight &other) const {
		return isEqual(other);
	}

	EQUAL_OPERATOR(ControlLight)

	QS_SERIALIZABLE

	QS_FIELD(State, st)
};




class Inventory;

/**
 * @brief The ControlActive class
 */


class ControlActive : public Control
{
	Q_GADGET

public:
	ControlActive()
		: Control()
		, lck(false)
		, a(false)
	{}

	bool isEqual(const ControlActive &other) const {
		return Control::isEqual(other) && a == other.a && lck == other.lck;
	}

	bool canMerge(const ControlActive &other) const {
		return Control::canMerge(other) && a == other.a && lck == other.lck ;
	}

	bool canInterpolateFrom(const ControlActive &other) const {
		return isEqual(other);
	}

	bool unlock(const ControlActiveBaseData &ownData, const Inventory &inventory, const bool &toLocked = false);

	static bool unlock(ControlActive &dest, const ControlActiveBaseData &destData,
					   const Inventory &inventory, const bool &toLocked = false) {
		return dest.unlock(destData, inventory, toLocked);
	}

	EQUAL_OPERATOR(ControlActive)

	QS_SERIALIZABLE

	QS_FIELD(bool, lck)			// locked
	QS_FIELD(bool, a)			// active
};









/**
 * @brief The PickableBaseData class
 */

class PickableBaseData : public ControlActiveBaseData
{
	Q_GADGET

public:
	enum PickableType {
		PickableInvalid = 0,
		PickableHp,
		PickableShield,
		PickableBullet,
		PickableTime,
		PickableMp,
		PickableCoin,
		PickableKey,
	};

	Q_ENUM(PickableType);

	PickableBaseData (const PickableType &_type, const int &_o, const int &_s, const int &_id)
		: ControlActiveBaseData(RpgConfig::ControlPickable, _o, _s, _id)
		, pt(_type)
	{}

	PickableBaseData(const PickableType &_type)
		: PickableBaseData(_type, -1, -1, -1)
	{}

	PickableBaseData()
		: PickableBaseData(PickableInvalid)
	{}

	bool isEqual(const PickableBaseData &other) const {
		return ControlActiveBaseData::isEqual(other) && other.pt == pt && other.p == p;
	}

	EQUAL_OPERATOR(PickableBaseData)

	QS_SERIALIZABLE

	QS_FIELD(PickableType, pt)
	QS_COLLECTION(QList, float, p)				// position
};






/**
 * @brief The Pickable class
 */

class Pickable : public ControlActive, public LifeCycle
{
	Q_GADGET

public:
	Pickable()
		: ControlActive()
		, LifeCycle()
		, st(LifeCycle::StageInvalid)
	{}

	bool isEqual(const Pickable &other) const {
		return ControlActive::isEqual(other) && other.st == st && other.own == own;
	}

	bool canMerge(const Pickable &other) const {
		return ControlActive::canMerge(other) && other.st == st && other.own == own;
	}

	bool canInterpolateFrom(const Pickable &other) const {
		return isEqual(other);
	}

	virtual Stage stage() const { return st; }
	virtual void setStage(const Stage &newStage) { st = newStage; }


	EQUAL_OPERATOR(Pickable)

	QS_SERIALIZABLE

	QS_FIELD(LifeCycle::Stage, st)				// stage
	QS_OBJECT(BaseData, own)					// owner (collected by)

};










/**
 * @brief The ControlTeleportBaseData class
 */

class ControlTeleportBaseData : public ControlActiveBaseData
{
	Q_GADGET

public:
	ControlTeleportBaseData(const int &_o, const int &_s, const int &_id)
		: ControlActiveBaseData(RpgConfig::ControlTeleport, _o, _s, _id)
		, x(0)
		, y(0)
		, a(0)
		, hd(false)
	{}

	ControlTeleportBaseData()
		: RpgGameData::ControlTeleportBaseData(-1, -1, -1)
	{}

	bool isEqual(const ControlTeleportBaseData &other) const {
		return ControlActiveBaseData::isEqual(other) && other.x == x && other.y == y && other.a == a &&
				other.dst == dst && other.hd == hd;
	}

	EQUAL_OPERATOR(ControlTeleportBaseData)

	QS_SERIALIZABLE

	QS_FIELD(float, x)					// exit position
	QS_FIELD(float, y)
	QS_FIELD(float, a)					// exit angle
	QS_FIELD(bool, hd)					// simple hideout
	QS_OBJECT(ControlBaseData, dst)		// destination teleport (invalid = final teleport)
};






/**
 * @brief The ControlTeleport class
 */

class ControlTeleport : public ControlActive
{
	Q_GADGET

public:
	ControlTeleport()
		: ControlActive()
		, op(false)
	{}

	bool isEqual(const ControlTeleport &other) const {
		return Control::isEqual(other) && other.op == op;
	}

	bool canMerge(const ControlTeleport &other) const {
		return Control::canMerge(other) && other.op == op;
	}

	bool canInterpolateFrom(const ControlTeleport &other) const {
		return isEqual(other) && other.op == op;
	}

	EQUAL_OPERATOR(ControlTeleport)

	QS_SERIALIZABLE

	QS_FIELD(bool, op)				// operating
};









/**
 * @brief The InventoryItem class
 */

class InventoryItem : public QSerializer
{
	Q_GADGET

public:
	InventoryItem(const PickableBaseData::PickableType &_type, const QString &_name, const int &_count = 1)
		: QSerializer()
		, t(_type)
		, n(_name)
		, c(_count)
	{}

	InventoryItem(const PickableBaseData::PickableType &_type, const int &_count = 1)
		: InventoryItem(_type, QString(), _count)
	{}

	InventoryItem()
		: InventoryItem(PickableBaseData::PickableInvalid)
	{}


	bool isEqual(const InventoryItem &other) const {
		return other.t == t && other.n == n && other.c == c;
	}

	EQUAL_OPERATOR(InventoryItem)

	QS_SERIALIZABLE

	QS_FIELD(PickableBaseData::PickableType, t)				// type
	QS_FIELD(QString, n)									// name
	QS_FIELD(int, c)										// count
};






/**
 * @brief The Inventory class
 */

class Inventory : public QSerializer
{
	Q_GADGET

public:
	Inventory()
		: QSerializer()
	{}

	bool isEqual(const Inventory &other) const {
		return other.l == l;
	}

	QList<InventoryItem>::const_iterator find(const PickableBaseData::PickableType &type, const QString &name = {}) const {
		return std::find_if(l.cbegin(), l.cend(),
							[&type, &name](const InventoryItem &w) {
			return w.t == type && w.n == name;
		});
	}

	QList<InventoryItem>::iterator find(const PickableBaseData::PickableType &type, const QString &name = {}) {
		return std::find_if(l.begin(), l.end(),
							[&type, &name](const InventoryItem &w) {
			return w.t == type && w.n == name;
		});
	}

	bool contains(const PickableBaseData::PickableType &type, const QString &name = {}) const {
		return find(type, name) != l.cend();
	}

	const InventoryItem &add(const PickableBaseData::PickableType &type, const int &count = 1, const QString &name = {}) {
		auto it = find(type, name);
		if (it != l.end()) {
			it->c += count;
			return *it;
		} else {
			return l.emplace_back(type, name, count);
		}
	}

	friend QDebug operator<<(QDebug debug, const Inventory &c) {
		QDebugStateSaver saver(debug);
		debug.nospace() << '[';
		bool isFirst = true;
		for (const InventoryItem &i : std::as_const(c.l)) {
			if (!isFirst)
				debug.nospace() << ',' << ' ';

			debug.nospace() << i.t;

			if (!i.n.isEmpty())
				debug.nospace() << ':' << qPrintable(i.n);

			isFirst = false;
		}

		debug.nospace() << ']';

		return debug;
	};


	EQUAL_OPERATOR(Inventory)

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, InventoryItem, l)		// inventory list
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

	bool canInterpolateFrom(const ArmoredEntity &other) const {
		return Entity::canInterpolateFrom(other) && other.arm == arm;
	}

	static void attacked(const ArmoredEntityBaseData &dstBase, ArmoredEntity &dst,
						 const Weapon::WeaponType &weapon, const int &weaponSubType, const ArmoredEntityBaseData &other);

	void attacked(const ArmoredEntityBaseData &dstBase,
				  const Weapon::WeaponType &weapon, const int &weaponSubType, const ArmoredEntityBaseData &other)
	{
		attacked(dstBase, *this, weapon, weaponSubType, other);
	}

	EQUAL_OPERATOR(ArmoredEntity)

	QS_SERIALIZABLE

	QS_OBJECT(Armory, arm)			// armory
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
		, mhp(0)
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
		return BaseData::isEqual(other) && other.mhp == mhp;
	}

	EQUAL_OPERATOR(EnemyBaseData)

	QS_SERIALIZABLE

	QS_FIELD(EnemyType, t)
	QS_FIELD(int, mhp)
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
		EnemyAttack,

		EnemySpecial = 99
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
		return ArmoredEntity::isEqual(other) && other.st == st && other.tg == tg && other.inv == inv;
	}

	bool canMerge(const Enemy &other) const {
		return ArmoredEntity::canMerge(other) && other.st == st && other.tg == tg && other.inv == inv;
	}

	bool canInterpolateFrom(const Enemy &other) const {
		return ArmoredEntity::canInterpolateFrom(other) && other.st == st && other.tg == tg;
	}

	EQUAL_OPERATOR(Enemy)

	QS_SERIALIZABLE

	QS_FIELD(EnemyState, st)			// enemy state
	QS_FIELD(QString, sp)				// special state
	QS_OBJECT(BaseData, tg)				// target (player)
	QS_OBJECT(Inventory, inv)			// inventory

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
		, rq(0)
	{}

	PlayerBaseData(const int &_o, const int &_s, const int &_id)
		: PlayerBaseData(1., 1., _o, _s, _id)
	{}

	PlayerBaseData()
		: PlayerBaseData(-1, -1, -1)
	{}

	bool isEqual(const PlayerBaseData &other) const {
		return ArmoredEntityBaseData::isEqual(other) && other.rq == rq;
	}

	static void assign(const QList<PlayerBaseData*> &dst, const int &num);
	static int getXpForAttack(const int &diffHp, const int &currHp);

	EQUAL_OPERATOR(PlayerBaseData)

	QS_SERIALIZABLE

	QS_FIELD(int, rq)						// required collection item
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
		, l(false)
		, c(0)
		, x(0)
		, ft(FeatureInvalid)
		, mp(0)
	{}

	enum PlayerState {
		PlayerInvalid = 0,
		PlayerIdle,
		PlayerMoving,
		PlayerHit,
		PlayerShot,
		PlayerCast,
		PlayerAttack,
		PlayerWeaponChange,
		PlayerLockControl,					// előbb a feladat
		PlayerUnlockControl,				// sikertelen feladat után
		PlayerUseControl,					// nincs feladat vagy sikeres feladat után
		PlayerExit,							// kilép a teleportból, búvóhelyről,...stb.

		PlayerSpecial = 99
	};

	Q_ENUM(PlayerState);


	enum Feature {
		FeatureInvalid		= 0,
		FeatureCamouflage	= 1,
		FeatureFreeWalk		= 1 << 1,
		FeatureLockEnemy	= 1 << 2,
	};

	Q_ENUM(Feature)

	Q_DECLARE_FLAGS(Features, Feature);
	Q_FLAGS(Features);


	bool isEqual(const Player &other) const  {
		return ArmoredEntity::isEqual(other) && other.st == st && other.tg == tg && other.l == l &&
				other.c == c && other.x == x &&
				other.pck == pck && other.ft == ft &&
				other.mp == mp;
	}

	bool canMerge(const Player &other) const {
		return ArmoredEntity::canMerge(other) && other.st == st && other.tg == tg && other.l == l &&
				other.c == c && other.x == x &&
				other.pck == pck && other.ft == ft &&
				other.mp == mp;
	}

	bool canInterpolateFrom(const Player &other) const {
		return ArmoredEntity::canInterpolateFrom(other) && other.st == st && other.tg == tg &&
				other.pck == pck && other.ft == ft &&
				other.mp == mp;
	}

	static void controlFailed(Player &dst, const RpgConfig::ControlType &control);

	void controlFailed(const RpgConfig::ControlType &control) {
		controlFailed(*this, control);
	}

	static int pick(Player &dst, const PickableBaseData::PickableType &type, const QString &name = {});

	int pick(const PickableBaseData::PickableType &type, const QString &name = {}) {
		return pick(*this, type, name);
	}


	static bool useTeleport(Player &dst, const ControlTeleportBaseData &base, const PlayerBaseData &playerBase);

	bool useTeleport(const ControlTeleportBaseData &base, const PlayerBaseData &playerBase) {
		return useTeleport(*this, base, playerBase);
	}

	static bool threshold(const QList<float> &p1, const QList<float> &p2);

	static bool threshold(const Player &p1, const Player &p2) {
		return threshold(p1.p, p2.p);
	}

	bool threshold(const Player &other) const {
		return threshold(*this, other);
	}

	bool threshold(const QList<float> &other) const {
		return threshold(this->p, other);
	}

	EQUAL_OPERATOR(Player);

	QS_SERIALIZABLE

	QS_FIELD(PlayerState, st)			// state
	QS_FIELD(QString, sp)				// special state
	QS_OBJECT(BaseData, tg)				// target (enemy, control)
	QS_OBJECT(BaseData, pck)			// packed (hiding place, teleport,...)
	QS_FIELD(bool, l)					// locked
	QS_OBJECT(Inventory, inv)			// inventory
	QS_FIELD(int, c)					// collected items
	QS_FIELD(int, x)					// extra xp on specified states (PlayerUseControl)
	QS_FIELD(Features, ft)				// active features
	QS_FIELD(int, mp)					// MP
};


Q_DECLARE_OPERATORS_FOR_FLAGS(Player::Features);







/**
 * @brief The ControlActiveBaseData class
 */

class ControlContainerBaseData : public ControlActiveBaseData
{
	Q_GADGET

public:

	ControlContainerBaseData(const int &_o, const int &_s, const int &_id)
		: ControlActiveBaseData(RpgConfig::ControlContainer, _o, _s, _id)
		, x(0)
		, y(0)
	{}

	ControlContainerBaseData()
		: ControlContainerBaseData(-1, -1, -1)
	{}

	bool isEqual(const ControlContainerBaseData &other) const {
		return ControlActiveBaseData::isEqual(other) && other.inv == inv;
	}

	EQUAL_OPERATOR(ControlContainerBaseData)

	QS_SERIALIZABLE

	QS_OBJECT(Inventory, inv)			// inventory
	QS_FIELD(float, x)					// position
	QS_FIELD(float, y)
};








/**
 * @brief The ControlContainer class
 */

class ControlContainer : public ControlActive
{
	Q_GADGET

public:
	ControlContainer()
		: ControlActive()
		, st(ContainerClose)
	{}

	enum State {
		ContainerClose = 0,
		ContainerOpen
	};

	Q_ENUM(State)

	bool isEqual(const ControlContainer &other) const {
		return ControlActive::isEqual(other) && st == other.st;
	}

	bool canMerge(const ControlContainer &other) const {
		return ControlActive::canMerge(other) && st == other.st;
	}

	bool canInterpolateFrom(const ControlContainer &other) const {
		return isEqual(other);
	}

	EQUAL_OPERATOR(ControlContainer)

	QS_SERIALIZABLE

	QS_FIELD(State, st)
};







/**
 * @brief The ControlCollectionBaseData class
 */

class ControlCollectionBaseData : public ControlActiveBaseData
{
	Q_GADGET

public:

	ControlCollectionBaseData(const int &_s, const int &_id, const int &_gid = -1)
		: ControlActiveBaseData(RpgConfig::ControlCollection, -1, _s, _id)
		, gid(_gid)
		, img(-1)
	{}

	ControlCollectionBaseData()
		: ControlCollectionBaseData(-1, -1)
	{}

	bool isEqual(const ControlCollectionBaseData &other) const {
		return ControlActiveBaseData::isEqual(other) && gid == other.gid && img == other.img;
	}

	EQUAL_OPERATOR(ControlCollectionBaseData)

	QS_SERIALIZABLE

	QS_FIELD(int, gid)				// CollectionGroupId
	QS_FIELD(int, img)							// image id
};








/**
 * @brief The ControlCollection class
 */


class ControlCollection : public ControlActive
{
	Q_GADGET

public:
	ControlCollection(const int &_idx = -1)
		: ControlActive()
		, idx(_idx)
	{}

	bool isEqual(const ControlCollection &other) const {
		return ControlActive::isEqual(other) && own == other.own && idx == other.idx && p == other.p;
	}

	bool canMerge(const ControlCollection &other) const {
		return ControlActive::canMerge(other) && own == other.own && idx == other.idx && p == other.p;
	}

	bool canInterpolateFrom(const ControlCollection &other) const {
		return isEqual(other) && own == other.own;
	}

	EQUAL_OPERATOR(ControlCollection)

	QS_SERIALIZABLE

	QS_FIELD(int, idx)							// place index in CollectionGroup
	QS_COLLECTION(QList, float, p)				// current position
	QS_OBJECT(BaseData, own)					// owner (collected by)
};









/**
 * @brief The ControlGateBaseData class
 */

class ControlGateBaseData : public ControlActiveBaseData
{
	Q_GADGET

public:

	ControlGateBaseData(const int &_o, const int &_s, const int &_id)
		: ControlActiveBaseData(RpgConfig::ControlGate, _o, _s, _id)
	{}

	ControlGateBaseData()
		: ControlGateBaseData(-1, -1, -1)
	{}

	bool isEqual(const ControlGateBaseData &other) const {
		return ControlActiveBaseData::isEqual(other);
	}

	EQUAL_OPERATOR(ControlGateBaseData)

	QS_SERIALIZABLE
};







/**
 * @brief The ControlGate class
 */

class ControlGate : public ControlActive
{
	Q_GADGET

public:
	ControlGate()
		: ControlActive()
		, st(GateClose)
	{}

	enum State {
		GateClose = 0,
		GateOpen,
		GateDamaged
	};

	Q_ENUM(State)

	bool isEqual(const ControlGate &other) const {
		return ControlActive::isEqual(other) && st == other.st;
	}

	bool canMerge(const ControlGate &other) const {
		return ControlActive::canMerge(other) && st == other.st;
	}

	bool canInterpolateFrom(const ControlGate &other) const {
		return isEqual(other);
	}

	bool unlock(const ControlGateBaseData &ownData, const Inventory &inventory, const bool &toLocked = false);
	bool lock(const ControlGateBaseData &ownData, const Inventory &inventory) {
		return unlock(ownData, inventory, true);
	}

	bool open(const ControlGateBaseData &ownData, const Inventory &inventory) {
		if (!unlock(ownData, inventory))
			return false;
		st = GateOpen;
		return true;
	}

	void close() {
		st = GateClose;
	}

	void damage() {
		st = GateDamaged;
		lck = false;
	}

	EQUAL_OPERATOR(ControlGate)

	QS_SERIALIZABLE

	QS_FIELD(State, st)
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
using SnapshotInterpolationData = std::pair<T2, SnapshotInterpolation<T> >;

template <typename T2, typename T,
		  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
		  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
using SnapshotInterpolationList = std::vector<SnapshotInterpolationData<T2, T> >;



// Snapshots

template <typename T, typename T2,
		  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
		  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
struct SnapshotData {
	T2 data;
	std::map<qint64, T> list;
	qint64 lastFullSnap = -1;

	// Return the item not greater than tick, first item when all items are greater, nullopt on empty list

	std::optional<T> get(const qint64 &tick) const {
		if (list.empty())
			return std::nullopt;
		auto it = list.upper_bound(tick);
		if (it == list.cbegin())
			return it->second;
		else
			return std::prev(it)->second;
	}
};


template <typename T, typename T2,
		  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
		  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
using SnapshotList = std::vector<SnapshotData<T, T2> >;





/**
 * @brief The SnapshotInterpolationControls class
 */

struct SnapshotInterpolationControls
{
	SnapshotInterpolationList<ControlBaseData, ControlLight> lights;
	SnapshotInterpolationList<ControlContainerBaseData, ControlContainer> containers;
	SnapshotInterpolationList<ControlCollectionBaseData, ControlCollection> collections;
	SnapshotInterpolationList<PickableBaseData, Pickable> pickables;
	SnapshotInterpolationList<ControlGateBaseData, ControlGate> gates;
	SnapshotInterpolationList<ControlTeleportBaseData, ControlTeleport> teleports;
};



/**
 * @brief The SnapshotControls class
 */

struct SnapshotControls
{
	SnapshotList<ControlLight, ControlBaseData> lights;
	SnapshotList<ControlContainer, ControlContainerBaseData> containers;
	SnapshotList<ControlCollection, ControlCollectionBaseData> collections;
	SnapshotList<Pickable, PickableBaseData> pickables;
	SnapshotList<ControlGate, ControlGateBaseData> gates;
	SnapshotList<ControlTeleport, ControlTeleportBaseData> teleports;
};



/**
 * @brief The FullSnapshot class
 */

struct FullSnapshot {
	SnapshotInterpolationList<PlayerBaseData, Player> players;
	SnapshotInterpolationList<EnemyBaseData, Enemy> enemies;
	SnapshotInterpolationList<BulletBaseData, Bullet> bullets;
	SnapshotInterpolationControls controls;

	void clear() {
		players.clear();
		enemies.clear();
		bullets.clear();

		controls.lights.clear();
		controls.containers.clear();
		controls.collections.clear();
		controls.pickables.clear();
		controls.gates.clear();
		controls.teleports.clear();
	}

	template <typename T2, typename T,
			  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
	std::optional<SnapshotInterpolation<T> > getSnapshot(const T2 &data, const SnapshotInterpolationList<T2, T> &list) const;


	template <typename T2, typename T,
			  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
	std::optional<SnapshotInterpolation<T> > getSnapshot(const T2 &data) const {
		Q_UNUSED(data);
		qWarning() << "Missing specialization";
		return std::nullopt;
	}

	std::optional<SnapshotInterpolation<Player> > getSnapshot(const PlayerBaseData &data) const {
		return getSnapshot(data, players);
	}

	std::optional<SnapshotInterpolation<Enemy> > getSnapshot(const EnemyBaseData &data) const {
		return getSnapshot(data, enemies);
	}

	std::optional<SnapshotInterpolation<Bullet> > getSnapshot(const BulletBaseData &data) const {
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
	SnapshotControls controls;

	void clear() {
		players.clear();
		enemies.clear();
		bullets.clear();

		controls.lights.clear();
		controls.containers.clear();
		controls.collections.clear();
		controls.pickables.clear();
		controls.gates.clear();
		controls.teleports.clear();
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

	int fromCbor(const QCborMap &map);

	template <typename T, typename T2>
	static SnapshotList<T, T2>::iterator find(SnapshotList<T, T2> &list, const BaseData &src);

	template <typename T, typename T2>
	static SnapshotList<T, T2>::const_iterator find(const SnapshotList<T, T2> &list, const BaseData &src);

	template <typename T>
	static void copy(std::map<qint64, T> &dest, const std::map<qint64, T> &src);

	template <typename T, typename T2>
	static void assign(SnapshotList<T, T2> &dest, const T2 &data, const T &src);
};







/**
 * @brief The SnapshotStorage class
 */

class SnapshotStorage {

public:
	SnapshotStorage() = default;


	const SnapshotList<Player, PlayerBaseData> &players() const { return m_players; }
	const SnapshotList<Enemy, EnemyBaseData> &enemies() const { return m_enemies; }
	const SnapshotList<Bullet, BulletBaseData> &bullets() const { return m_bullets; }
	const SnapshotControls &controls() const { return m_controls; }

	FullSnapshot getFullSnapshot(const qint64 &tick, const bool &findLast = false);
	FullSnapshot getNextFullSnapshot(const qint64 &tick) { return getFullSnapshot(tick, true); }
	CurrentSnapshot getCurrentSnapshot();

	void zapSnapshots(const qint64 &tick);



protected:

	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
	SnapshotInterpolation<T> getSnapshotInterpolation(const SnapshotList<T, T2> &snapshots,
													  const T2 &id,
													  const qint64 &currentTick,
													  const qint64 &fromTick = -1);


	template <typename T, typename = std::enable_if<std::is_base_of<Body, T>::value>::type>
	SnapshotInterpolation<T> getSnapshotInterpolation(const std::map<qint64, T> &map,
													  const qint64 &currentTick,
													  const qint64 &fromTick = -1);

	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
	void saveLastTick(SnapshotData<T, T2> *dst, const SnapshotInterpolation<T> &sip);


	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<BaseData, T2>::value>::type>
	void addFullSnapshot(SnapshotInterpolationList<T2, T> *dst, SnapshotList<T, T2> &src,
						 const qint64 &currentTick, const bool &findLast);


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


	SnapshotList<Player, PlayerBaseData> m_players;
	SnapshotList<Enemy, EnemyBaseData> m_enemies;
	SnapshotList<Bullet, BulletBaseData> m_bullets;
	SnapshotControls m_controls;
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
inline SnapshotList<T, T2>::const_iterator CurrentSnapshot::find(const SnapshotList<T, T2> &list, const BaseData &src)
{
	return std::find_if(list.cbegin(),
						list.cend(),
						[&src](const auto &ptr) {
		return ptr.data.isBaseEqual(src);
	});
}



/**
 * @brief CurrentSnapshot::find
 * @param list
 * @param src
 * @return
 */

template<typename T, typename T2>
inline SnapshotList<T, T2>::iterator CurrentSnapshot::find(SnapshotList<T, T2> &list, const BaseData &src)
{
	return std::find_if(list.begin(),
						list.end(),
						[&src](const auto &ptr) {
		return ptr.data.isBaseEqual(src);
	});
}







/**
 * @brief CurrentSnapshot::assign
 * @param dest
 * @param data
 * @param src
 */

template<typename T, typename T2>
inline void CurrentSnapshot::assign(SnapshotList<T, T2> &dest, const T2 &data, const T &src)
{
	const auto &it = find(dest, data);

	if (it == dest.end()) {
		dest.emplace_back(data, std::map<qint64, T>{{src.f, src}}, -1);
	} else {
		it->list.insert_or_assign(src.f, src);
	}
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
		return ptr.first.isBaseEqual(data);
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
																		  const qint64 &currentTick,
																		  const qint64 &fromTick)
{
	SnapshotInterpolation<T> sip;

	const qint64 time = currentTick - RPG_UDP_DELTA_TICK;

	sip.current = time;

	if (time < 0)
		return sip;

	const auto mapIt = std::find_if(snapshots.cbegin(), snapshots.cend(), [&id](const auto &b){
		return b.data.isBaseEqual(id);
	});

	if (mapIt == snapshots.cend()) {
		return sip;
	}

	return getSnapshotInterpolation<T>(mapIt->list, currentTick, fromTick);
}




/**
 * @brief SnapshotStorage::getSnapshotInterpolation
 * @param map
 * @param currentTick
 * @return
 */

template<typename T, typename T2>
inline SnapshotInterpolation<T> SnapshotStorage::getSnapshotInterpolation(const std::map<qint64, T> &map,
																		  const qint64 &currentTick,
																		  const qint64 &fromTick)
{
	SnapshotInterpolation<T> sip;

	qint64 time = currentTick - RPG_UDP_DELTA_TICK;

	sip.current = time;

	if (!map.empty())
		sip.last = std::prev(map.cend())->second;

	if (time < 0)
		return sip;

	const auto it = map.lower_bound(fromTick < 0 ? time : std::min(fromTick, time));					// lower_bound = greater or equal

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

			for (auto it = ptr.list.cbegin(); it != ptr.list.cend(); ++it) {
				QCborMap fullmap = it->second.toCborMap(true);

				if (func)
					func(&fullmap);

				if (prev) {
					QCborMap map = it->second.toCborMap(prev.value(), true);

					if (func)
						func(&map);

					if (std::next(it) != ptr.list.cend()) {
						QCborMap delta = map;
						delta.remove(QStringLiteral("f"));

						if (delta.isEmpty())
							continue;
					}

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
			return ptr.data.isBaseEqual(base);
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
