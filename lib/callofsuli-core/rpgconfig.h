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
		, prepared(false)
		, xp(0)
		, hp(0)
		, streak(0)
		, online(false)
	{}
	RpgPlayerConfig(const int &_id) : RpgPlayerConfig(_id, QStringLiteral("")) {}
	RpgPlayerConfig() : RpgPlayerConfig(-1) {}

	friend bool operator==(const RpgPlayerConfig &c1, const RpgPlayerConfig &c2) {
		return c1.playerId == c2.playerId &&
				c1.username == c2.username &&
				c1.prepared == c2.prepared &&
				c1.xp == c2.xp &&
				c1.character == c2.character &&
				c1.fullNickName == c2.fullNickName &&
				c1.hp == c2.hp &&
				c1.streak == c2.streak &&
				c1.online == c2.online
				;
	}

	void reset() {
		prepared = false;
		xp = 0;
		hp = 0;
		streak = 0;
	}

	QS_SERIALIZABLE

	QS_FIELD(int, playerId)
	QS_FIELD(QString, username)
	QS_FIELD(bool, prepared)
	QS_FIELD(int, xp)
	QS_FIELD(QString, character)
	QS_FIELD(QString, fullNickName)
	QS_FIELD(int, hp)
	QS_FIELD(int, streak)
	QS_FIELD(bool, online)
};


Q_DECLARE_METATYPE(RpgPlayerConfig)

#endif // RPGCONFIG_H
