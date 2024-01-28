/*
 * ---- Call of Suli ----
 *
 * conquestconfig.h
 *
 * Created on: 2024. 01. 21.
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

#ifndef CONQUESTCONFIG_H
#define CONQUESTCONFIG_H

#include <QSerializer>
#include <QString>
#include "utils_.h"


/**
 * @brief The ConquestWorldData class
 */

class ConquestWorldData : public QSerializer
{
	Q_GADGET

public:
	ConquestWorldData()
		: proprietor(-1)
		, xp(0)
		, xpOnce(0)
		, fortress(-1)
	{}

	friend bool operator==(const ConquestWorldData &w1, const ConquestWorldData &w2) {
		return w1.proprietor == w2.proprietor &&
				w1.xp == w2.xp &&
				w1.xpOnce == w2.xpOnce &&
				w1.fortress == w2.fortress
				;
	}

	QS_SERIALIZABLE
	QS_FIELD(QString, id)
	QS_FIELD(int, proprietor)
	QS_FIELD(int, xp)
	QS_FIELD(int, xpOnce)
	QS_FIELD(int, fortress)
};


/**
 * @brief The ConquestWorld class
 */

class ConquestWorld : public QSerializer
{
	Q_GADGET

public:
	ConquestWorld()
		: playerCount(0)
	{}


	/**
	 * @brief landFind
	 * @param id
	 * @return
	 */

	int landFind(const QString &id) const {
		const auto &it = std::find_if(landList.constBegin(), landList.constEnd(), [id](const ConquestWorldData &d){
			return d.id == id;
		});

		if (it == landList.constEnd())
			return -1;
		else
			return it - landList.constBegin();
	}


	friend bool operator==(const ConquestWorld &w1, const ConquestWorld &w2) {
		return w1.name == w2.name &&
				w1.playerCount == w2.playerCount
				;
	}

	QS_SERIALIZABLE
	QS_FIELD(QString, name)
	QS_COLLECTION_OBJECTS(QList, ConquestWorldData, landList)
	QS_FIELD(int, playerCount)
};



/**
 * @brief The ConquestAnswer class
 */

class ConquestAnswer : public QSerializer
{
	Q_GADGET

public:
	ConquestAnswer()
		: player(-1)
	{}

	friend bool operator==(const ConquestAnswer &c1, const ConquestAnswer &c2) {
		return c1.answer == c2.answer &&
				c1.player == c2.player
				;
	}

	/**
	 * @brief addDetails
	 * @param json
	 * @param success
	 * @param elapsed
	 * @return
	 */

	static void addDetails(QJsonObject *json, const bool &success, const qint64 &elapsed) {
		Q_ASSERT(json);
		json->insert(QStringLiteral("_success"), success);
		json->insert(QStringLiteral("_elapsed"), elapsed);
	}

	/**
	 * @brief loadDetails
	 * @param json
	 */

	void loadDetails(const QJsonObject &json) {
		success = json.value(QStringLiteral("_success")).toBool(false);
		elapsed = JSON_TO_INTEGER(json.value(QStringLiteral("_elapsed")));
	}

	bool success = false;
	qint64 elapsed = 0;

	QS_SERIALIZABLE
	QS_FIELD(int, player)
	QS_FIELD(QJsonObject, answer)
};





/**
 * @brief The ConquestTurn class
 */

class ConquestTurn : public QSerializer
{
	Q_GADGET

public:
	ConquestTurn()
		: player(-1)
		, subStage(SubStageInvalid)
		, subStageStart(0)
		, subStageEnd(0)
	{}

	enum Stage {
		StageInvalid = 0,
		StagePick,
		StageBattle,
		StageLastRound
	};

	Q_ENUM(Stage)


	enum SubStage {
		SubStageInvalid = 0,
		SubStageUserSelect,
		SubStageUserAnswer,
		SubStageFinished
	};

	Q_ENUM(SubStage)



	/**
	 * @brief answerGet
	 * @param playerId
	 * @return
	 */

	std::optional<QJsonObject> answerGet(const int &playerId) const {
		auto it = std::find_if(answerList.constBegin(), answerList.constEnd(), [playerId](const ConquestAnswer &a){
			return a.player == playerId;
		});

		if (it == answerList.constEnd())
			return std::nullopt;
		else
			return it->answer;
	}



	/**
	 * @brief answerGetSuccess
	 * @param playerId
	 * @return
	 */

	std::optional<QJsonObject> answerGetSuccess(const int &playerId) const {
		auto it = std::find_if(answerList.constBegin(), answerList.constEnd(), [playerId](const ConquestAnswer &a){
			return a.player == playerId;
		});

		qWarning() << "---?" << (it == answerList.constEnd());

		if (it == answerList.constEnd())
			return std::nullopt;

		qWarning() << "---S" << it->success << it->elapsed;

		if (!it->success)
			return std::nullopt;

		for (const ConquestAnswer &a : answerList) {

			qWarning() << "---CH" << a.player << playerId << a.success;

			if (a.player != playerId && a.success && a.elapsed < it->elapsed)
				return std::nullopt;
		}

		return it->answer;
	}


	friend bool operator==(const ConquestTurn &c1, const ConquestTurn &c2) {
		return c1.player == c2.player &&
				c1.subStage == c2.subStage &&
				c1.subStageStart == c2.subStageStart &&
				c1.subStageEnd == c2.subStageEnd &&
				c1.pickedId == c2.pickedId &&
				c1.answerList == c2.answerList &&
				c1.canPick == c2.canPick
				;
	}

	QS_SERIALIZABLE
	QS_FIELD(int, player)
	QS_FIELD(SubStage, subStage)
	QS_FIELD(qint64, subStageStart)
	QS_FIELD(qint64, subStageEnd)
	QS_FIELD(QString, pickedId)
	QS_COLLECTION_OBJECTS(QList, ConquestAnswer, answerList)
	QS_COLLECTION(QList, QString, canPick)
};







/**
 * @brief The ConquestPlayer class
 */

class ConquestPlayer : public QSerializer
{
	Q_GADGET

public:
	ConquestPlayer(const int &_id, const QString &_user)
		: playerId(_id)
		, username(_user)
		, prepared(false)
		, xp(0)
	{}
	ConquestPlayer(const int &_id) : ConquestPlayer(_id, QStringLiteral("")) {}
	ConquestPlayer() : ConquestPlayer(-1) {}

	QS_SERIALIZABLE

	QS_FIELD(int, playerId);
	QS_FIELD(QString, username);
	QS_FIELD(bool, prepared)
	QS_FIELD(int, xp)
	QS_FIELD(QString, theme)
};






/**
 * @brief The ConquestConfig class
 */

class ConquestConfig : public QSerializer
{
	Q_GADGET

public:
	ConquestConfig()
		: gameState(StateInvalid)
		, missionLevel(-1)
		, currentTurn(-1)
		, currentStage(ConquestTurn::StageInvalid)
	{}


	enum GameState {
		StateInvalid = 0,
		StateConnect,
		StatePrepare,
		StatePlay,
		StateFinished,
		StateError
	};

	Q_ENUM(GameState);


	/**
	 * @brief landPick
	 * @param landId
	 * @param playerId
	 * @return
	 */

	bool landPick(const QString &landId, const int &playerId, ConquestPlayer *player);
	bool playerAnswer(const ConquestAnswer &answer);
	bool landSwapPlayer(const QString &landId, ConquestPlayer *playerNew, ConquestPlayer *playerOld);
	int getPickedLandProprietor(const int &turn) const;

	friend bool operator==(const ConquestConfig &c1, const ConquestConfig &c2) {
		return c1.gameState == c2.gameState &&
				c1.world == c2.world &&
				c1.mapUuid == c2.mapUuid &&
				c1.missionUuid == c2.missionUuid &&
				c1.missionLevel == c2.missionLevel &&
				c1.turnList == c2.turnList &&
				c1.currentTurn == c2.currentTurn &&
				c1.currentStage == c2.currentStage &&
				c1.order == c2.order &&
				c1.currentQuestion == c2.currentQuestion
				;
	}

	QS_SERIALIZABLE
	QS_FIELD(GameState, gameState)
	QS_FIELD(QString, mapUuid)
	QS_FIELD(QString, missionUuid)
	QS_FIELD(int, missionLevel)
	QS_OBJECT(ConquestWorld, world)
	QS_COLLECTION(QList, int, order)
	QS_COLLECTION_OBJECTS(QList, ConquestTurn, turnList)
	QS_FIELD(int, currentTurn)
	QS_FIELD(ConquestTurn::Stage, currentStage)
	QS_FIELD(QJsonObject, currentQuestion)
};




/// ---------------- HELPERS ----------------------


/**
 * @brief The ConquestWorldHelperInfo class
 */

class ConquestWorldHelperInfo : public QSerializer
{
	Q_GADGET

public:
	ConquestWorldHelperInfo()
		: playerCount(0)
	{}

	QS_SERIALIZABLE
	QS_FIELD(int, playerCount)
	QS_COLLECTION(QList, QString, landIdList)

	// TODO: adjacency list
};




/**
 * @brief The ConquestWorldHelper class
 */

class ConquestWorldHelper : public QSerializer
{
	Q_GADGET

public:
	ConquestWorldHelper() = default;

	QS_SERIALIZABLE
	QS_FIELD(QString, name)
	QS_COLLECTION_OBJECTS(QList, ConquestWorldHelperInfo, infoList)
};




/**
 * @brief The ConquestWordListHelper class
 */

class ConquestWordListHelper : public QSerializer
{
	Q_GADGET

public:
	QS_SERIALIZABLE
	QS_COLLECTION_OBJECTS(QList, ConquestWorldHelper, worldList);
};





/// ------------- DEFINITIONS -------------------------



/**
 * @brief ConquestConfig::landPick
 * @param landId
 * @param playerId
 * @return
 */

inline bool ConquestConfig::landPick(const QString &landId, const int &playerId, ConquestPlayer *player) {
	if (currentTurn < 0 || currentTurn >= turnList.size())
		return false;

	ConquestTurn &turn = turnList[currentTurn];

	if (turn.subStage != ConquestTurn::SubStageUserSelect || turn.player != playerId)
		return false;

	if (currentStage == ConquestTurn::StagePick) {
		const int &idx = world.landFind(landId);

		if (idx == -1)
			return false;

		ConquestWorldData &land = world.landList[idx];
		turn.pickedId = landId;

		if (land.proprietor != -1)
			return false;

		land.proprietor = playerId;

		if (player) {
			player->xp += land.xp;
		}

		return true;
	} else if (currentStage == ConquestTurn::StageBattle) {
		const int &idx = world.landFind(landId);

		if (idx == -1)
			return false;

		turn.pickedId = landId;

		return true;
	}

	return false;
}



/**
 * @brief ConquestConfig::playerAnswer
 * @param answer
 * @return
 */

inline bool ConquestConfig::playerAnswer(const ConquestAnswer &answer)
{
	if (currentTurn < 0 || currentTurn >= turnList.size())
		return false;

	ConquestTurn &turn = turnList[currentTurn];

	if (turn.subStage != ConquestTurn::SubStageUserAnswer)
		return false;

	if (turn.answerGet(answer.player))
		return false;

	turn.answerList.append(answer);

	return true;
}




/**
 * @brief ConquestConfig::landSwapPlayer
 * @param landId
 * @param playerNew
 * @param playerOld
 * @return
 */

inline bool ConquestConfig::landSwapPlayer(const QString &landId, ConquestPlayer *playerNew, ConquestPlayer *playerOld)
{
	if (!playerNew)
		return false;

	if (currentTurn < 0 || currentTurn >= turnList.size())
		return false;

	ConquestTurn &turn = turnList[currentTurn];

	if (turn.subStage != ConquestTurn::SubStageUserAnswer || turn.player != playerNew->playerId)
		return false;

	const int &idx = world.landFind(landId);

	if (idx == -1)
		return false;

	ConquestWorldData &land = world.landList[idx];

	if (land.proprietor == playerNew->playerId)
		return false;

	const int &old = land.proprietor;

	land.proprietor = playerNew->playerId;
	playerNew->xp += land.xp;
	playerNew->xp += land.xpOnce;
	land.xpOnce = 0;

	if (playerOld && playerOld->playerId == old) {
		playerOld->xp -= land.xp;
	}

	return true;
}





/**
 * @brief ConquestConfig::getPickedLandProprietor
 * @return
 */

inline int ConquestConfig::getPickedLandProprietor(const int &turn) const
{
	if (turn < 0 || turn >= turnList.size())
		return -1;

	const int &idx = world.landFind(turnList.at(turn).pickedId);

	if (idx == -1)
		return -1;

	return world.landList[idx].proprietor;
}



#endif // CONQUESTCONFIG_H
