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
		StateDownloadContent,
		StateCharacterSelect,
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
				c1.userHost == c2.userHost &&
				c1.campaign == c2.campaign &&
				c1.duration == c2.duration
				;
	}

	QS_SERIALIZABLE
	QS_FIELD(GameState, gameState)
	QS_FIELD(QString, userHost)
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

	QS_FIELD(QString, terrain)
	QS_FIELD(QString, character)
	QS_FIELD(QStringList, weapons)
};


Q_DECLARE_METATYPE(RpgPlayerConfig)





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
		Bullet		= 4,
		Hp			= 5,
		Time		= 6,
		Xp			= 7,
		Pickable	= 8,
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

	friend bool operator==(const RpgMarket &c1, const RpgMarket &c2) {
		return c1.type == c2.type &&
				c1.name == c2.name &&
				c1.cost == c2.cost &&
				c1.rank == c2.rank &&
				c1.amount == c2.amount &&
				c1.rollover == c2.rollover &&
				c1.num == c2.num
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
		WinnerDefault	= 2
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



#endif // RPGCONFIG_H
