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

#include "qpoint.h"
#include <QSerializer>


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
	RpgPlayerConfig(const int &_id) : RpgPlayerConfig(_id, QStringLiteral("")) {}
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


#define EQUAL_OPERATOR(cname)	friend bool operator==(const cname &l, const cname &r) { return l.isEqual(r); }


namespace RpgGameData {

class CharacterSelect : public RpgPlayerConfig
{
	Q_GADGET

public:
	CharacterSelect()
		: RpgPlayerConfig()
		, completed(false)
	{}

	CharacterSelect(const RpgPlayerConfig &config)
		: RpgPlayerConfig(config)
		, completed(false)
	{}

	QS_SERIALIZABLE

	QS_FIELD(bool, completed)
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
 * @brief The Body class
 */

class Body : public QSerializer
{
	Q_GADGET

public:
	Body(const int &_sceneId, const int &_id)
		: QSerializer()
		, o(-1)
		, s(_sceneId)
		, id(_id)
	{ }

	Body() :
		Body(-1, -1)
	{}

	bool isEqual(const Body &other) const {
		return other.o == o && other.s == s && other.id == id;
	}

	EQUAL_OPERATOR(Body)

	QS_SERIALIZABLE

	QS_FIELD(int, o)					// ownerId
	QS_FIELD(int, s)					// sceneId
	QS_FIELD(int, id)					// objectId

	QS_COLLECTION(QList, float, p)		// position
};



class Entity : public Body
{
	Q_GADGET

public:
	Entity(const int &_sceneId, const int &_id)
		: Body(_sceneId, _id)
		, a(0.)
		, hp(0)
		, mhp(0)
	{}

	Entity()
		: Entity(-1, -1)
	{}

	bool isEqual(const Entity &other) const {
		return Body::isEqual(other) && other.a == a && other.hp == hp && other.mhp == mhp;
	}

	EQUAL_OPERATOR(Entity)

	QS_SERIALIZABLE

	QS_COLLECTION(QList, float, mov)		// moveTowards

	QS_FIELD(float, a)			// angle
	QS_FIELD(int, hp)			// HP
	QS_FIELD(int, mhp)			// max HP


};


class Player : public Entity
{
	Q_GADGET

public:
	Player(const int &_sceneId, const int &_id)
		: Entity(_sceneId, _id)
	{}

	Player()
		: Player(-1, -1)
	{}

	bool isEqual(const Player &other) const  {
		return Entity::isEqual(other);
	}

	EQUAL_OPERATOR(Player);

	QS_SERIALIZABLE

};




class Enemy : public Entity
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

	Enemy(const EnemyType &_type, const int &_sceneId, const int &_id)
		: Entity(_sceneId, _id)
		, t(_type)
	{}

	Enemy() :
		Enemy(EnemyInvalid, -1, -1)
	{}

	bool isEqual(const Enemy &other) const {
		return Entity::isEqual(other) && other.t == t;
	}

	EQUAL_OPERATOR(Enemy)

	QS_SERIALIZABLE

	QS_FIELD(EnemyType, t)
};



};



#endif // RPGCONFIG_H
