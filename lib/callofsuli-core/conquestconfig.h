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

#define MAX_PLAYERS_COUNT		4

#define LAND_XP					50
#define LAND_XP_ONCE			100

#define MSEC_SELECT				9000
#define MSEC_PREPARE			1250
#define MSEC_ANSWER				12000
#define MSEC_WAIT				2000
#define MSEC_GAME_TIMEOUT		20*60*1000

#define INITIAL_FORTRESS_COUNT	3
#define STREAK_SIZE				4



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
	ConquestWorld() = default;

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
		return w1.name == w2.name
				;
	}

	QHash<QString, QStringList> adjacencyMatrix;

	QS_SERIALIZABLE
	QS_FIELD(QString, name)
	QS_COLLECTION_OBJECTS(QList, ConquestWorldData, landList)
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
		, success(false)
		, elapsed(-1)
	{}

	friend bool operator==(const ConquestAnswer &c1, const ConquestAnswer &c2) {
		return c1.answer == c2.answer &&
				c1.player == c2.player &&
				c1.success == c2.success &&
				c1.elapsed == c2.elapsed
				;
	}


	QS_SERIALIZABLE
	QS_FIELD(int, player)
	QS_FIELD(QJsonObject, answer)
	QS_FIELD(bool, success)
	QS_FIELD(qint64, elapsed)
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
		, answerState(AnswerPending)
	{}

	enum Stage {
		StageInvalid = 0,
		StagePrepare,
		StagePick,
		StageBattle,
		StageLastRound
	};

	Q_ENUM(Stage)


	enum SubStage {
		SubStageInvalid = 0,
		SubStageUserSelect,
		SubStageUserAnswer,
		SubStageWait,
		SubStagePrepareBattle,
		SubStageFinished
	};

	Q_ENUM(SubStage)


	enum AnswerState {
		AnswerPending = 0,
		AnswerPlayerWin,
		AnswerPlayerLost
	};

	Q_ENUM(AnswerState)


	/**
	 * @brief answerGet
	 * @param playerId
	 * @return
	 */

	std::optional<ConquestAnswer> answerGet(const int &playerId) const {
		auto it = std::find_if(answerList.constBegin(), answerList.constEnd(), [playerId](const ConquestAnswer &a){
			return a.player == playerId;
		});

		if (it == answerList.constEnd())
			return std::nullopt;
		else
			return *it;
	}



	/**
	 * @brief answerGetSuccess
	 * @param playerId
	 * @return
	 */

	bool answerIsSuccess(const int &playerId, const Stage &gameStage) const {
		auto it = std::find_if(answerList.constBegin(), answerList.constEnd(), [playerId](const ConquestAnswer &a){
			return a.player == playerId;
		});

		if (it == answerList.constEnd())
			return false;

		if (!it->success)
			return false;

		if (gameStage == StageBattle) {
			for (const ConquestAnswer &a : answerList) {
				if (a.player != playerId && a.success && a.elapsed < it->elapsed)
					return false;
			}
		}

		return true;
	}


	/**
	 * @brief getSuccess
	 * @param playerId
	 * @return
	 */

	Q_INVOKABLE bool getSuccess(const int &playerId) const {
		auto it = std::find_if(answerList.constBegin(), answerList.constEnd(), [playerId](const ConquestAnswer &a){
			return a.player == playerId;
		});

		if (it == answerList.constEnd())
			return false;

		return it->success;
	}


	/**
	 * @brief getElapsed
	 * @param playerId
	 * @return
	 */

	Q_INVOKABLE qint64 getElapsed(const int &playerId) const {
		auto it = std::find_if(answerList.constBegin(), answerList.constEnd(), [playerId](const ConquestAnswer &a){
			return a.player == playerId;
		});

		if (it == answerList.constEnd())
			return -1;

		return it->elapsed;
	}


	void clear() {
		subStageStart = 0;
		subStageEnd = 0;
		pickedId.clear();
		answerList.clear();
		canPick.clear();
		answerState = AnswerPending;
	}

	friend bool operator==(const ConquestTurn &c1, const ConquestTurn &c2) {
		return c1.player == c2.player &&
				c1.subStage == c2.subStage &&
				c1.subStageStart == c2.subStageStart &&
				c1.subStageEnd == c2.subStageEnd &&
				c1.pickedId == c2.pickedId &&
				c1.answerList == c2.answerList &&
				c1.canPick == c2.canPick &&
				c1.answerState == c2.answerState
				;
	}

	QS_SERIALIZABLE
	QS_FIELD(int, player)
	QS_FIELD(SubStage, subStage)
	QS_FIELD(qint64, subStageStart)
	QS_FIELD(qint64, subStageEnd)
	QS_FIELD(QString, pickedId)
	QS_FIELD(AnswerState, answerState)
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
		, elapsed(0)
		, winner(false)
		, hp(0)
		, streak(0)
		, success(false)
	{}
	ConquestPlayer(const int &_id) : ConquestPlayer(_id, QStringLiteral("")) {}
	ConquestPlayer() : ConquestPlayer(-1) {}

	friend bool operator==(const ConquestPlayer &c1, const ConquestPlayer &c2) {
		return c1.playerId == c2.playerId &&
				c1.username == c2.username &&
				c1.prepared == c2.prepared &&
				c1.xp == c2.xp &&
				c1.character == c2.character &&
				c1.elapsed == c2.elapsed &&
				c1.winner == c2.winner &&
				c1.fullNickName == c2.fullNickName &&
				c1.hp == c2.hp &&
				c1.streak == c2.streak &&
				c1.success == c2.success
				;
	}

	void reset() {
		prepared = false;
		xp = 0;
		elapsed = 0;
		winner = false;
		hp = 0;
		streak = 0;
		success = false;
	}

	QS_SERIALIZABLE

	QS_FIELD(int, playerId)
	QS_FIELD(QString, username)
	QS_FIELD(bool, prepared)
	QS_FIELD(int, xp)
	QS_FIELD(QString, character)
	QS_FIELD(qint64, elapsed)
	QS_FIELD(bool, winner)
	QS_FIELD(QString, fullNickName)
	QS_FIELD(int, hp)
	QS_FIELD(int, streak)
	QS_FIELD(bool, success)
};



/**
 * @brief The ConquestConfigBase class
 */

class ConquestConfigBase : public QSerializer
{
	Q_GADGET

public:
	ConquestConfigBase()
		: missionLevel(-1)
		, startHp(0)
		, campaign(-1)
	{}


	QS_SERIALIZABLE
	QS_FIELD(QString, mapUuid)
	QS_FIELD(QString, missionUuid)
	QS_FIELD(int, missionLevel)
	QS_FIELD(int, startHp)
	QS_FIELD(int, campaign)
};





/**
 * @brief The ConquestConfig class
 */

class ConquestConfig : public ConquestConfigBase
{
	Q_GADGET

public:
	ConquestConfig()
		: ConquestConfigBase()
		, gameState(StateInvalid)
		, currentTurn(-1)
		, currentStage(ConquestTurn::StageInvalid)
	{}


	enum GameState {
		StateInvalid = 0,
		StateConnect,
		StateWorldSelect,
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
	bool landSwapPlayer(const QString &landId, ConquestPlayer *playerNew, ConquestPlayer *playerOld,
						const bool &oppositeCorrect);
	bool allLandSwapPlayer(ConquestPlayer *playerNew, ConquestPlayer *playerOld);
	bool landDefended(const QString &landId, ConquestPlayer *playerOld);
	int getPickedLandProprietor(const int &turn) const;
	bool checkPlayerLands(const int &playerId) const;
	void removePlayerFromNextTurns(const int &playerId);
	bool playerResult(const ConquestPlayer &player) const;

	void reset() {
		gameState = StateInvalid;
		world = {};
		order.clear();
		turnList.clear();
		currentTurn = -1;
		currentStage = ConquestTurn::StageInvalid;
		currentQuestion = {};
	}

	QJsonObject toBaseJson() const { return ConquestConfigBase::toJson(); }

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
				c1.currentQuestion == c2.currentQuestion &&
				c1.startHp == c2.startHp &&
				c1.userHost == c2.userHost &&
				c1.campaign == c2.campaign
				;
	}

	QS_SERIALIZABLE
	QS_FIELD(GameState, gameState)
	QS_OBJECT(ConquestWorld, world)
	QS_COLLECTION(QList, int, order)
	QS_COLLECTION_OBJECTS(QList, ConquestTurn, turnList)
	QS_FIELD(int, currentTurn)
	QS_FIELD(ConquestTurn::Stage, currentStage)
	QS_FIELD(QJsonObject, currentQuestion)
	QS_FIELD(QString, userHost)
};




/// ---------------- HELPERS ----------------------


/**
 * @brief The ConquestWorldHelper class
 */

class ConquestWorldHelper : public QSerializer
{
	Q_GADGET

public:
	ConquestWorldHelper() :
		playerCount(0)
	{}

	static QJsonObject adjacencyToJson(const QHash<QString, QStringList> &matrix);
	static void adjacencyToMatrix(const QJsonObject &data, QHash<QString, QStringList> *dst);
	QHash<QString, QStringList> adjacencyToMatrix() const {
		QHash<QString, QStringList> matrix;
		adjacencyToMatrix(adjacency, &matrix);
		return matrix;
	}

	QS_SERIALIZABLE
	QS_FIELD(QString, name)
	QS_COLLECTION(QList, QString, landIdList)
	QS_FIELD(int, playerCount)
	QS_FIELD(QJsonObject, adjacency)
};


/**
 * @brief ConquestWorldHelper::adjacencyToJson
 * @param matrix
 * @return
 */

inline QJsonObject ConquestWorldHelper::adjacencyToJson(const QHash<QString, QStringList> &matrix)
{
	QJsonObject obj;

	for (auto it = matrix.constBegin(); it != matrix.constEnd(); ++it) {
		obj.insert(it.key(), QJsonArray::fromStringList(it.value()));
	}

	return obj;
}

/**
 * @brief ConquestWorldHelper::adjacencyToMatrix
 * @param adjacency
 * @param dst
 * @return
 */

inline void ConquestWorldHelper::adjacencyToMatrix(const QJsonObject &data, QHash<QString, QStringList> *dst)
{
	Q_ASSERT(dst);
	dst->clear();

	for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
		const auto &array = it.value().toArray();
		QStringList list;
		for (const QJsonValue &v : array)
			list.append(v.toString());
		dst->insert(it.key(), list);
	}
}




/**
 * @brief The ConquestWordListHelper class
 */

class ConquestWordListHelper : public QSerializer
{
	Q_GADGET

public:
	QS_SERIALIZABLE
	QS_COLLECTION_OBJECTS(QList, ConquestWorldHelper, worldList);
	QS_COLLECTION(QList, QString, characterList);
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
	} else if (currentStage == ConquestTurn::StageBattle || currentStage == ConquestTurn::StageLastRound) {
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

inline bool ConquestConfig::landSwapPlayer(const QString &landId, ConquestPlayer *playerNew,
										   ConquestPlayer *playerOld, const bool &oppositeCorrect)
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

	const int old = land.proprietor;

	land.proprietor = playerNew->playerId;

	if (land.xpOnce > 0) {
		playerNew->xp += land.xpOnce;

		if (playerOld && playerOld->playerId == old) {
			playerOld->xp -= oppositeCorrect ? land.xpOnce/2 : land.xpOnce;
			if (playerOld->xp < 0)
				playerOld->xp = 0;
		}

		land.xpOnce = 0;
	} else {
		playerNew->xp += land.xp;

		if (playerOld && playerOld->playerId == old) {
			playerOld->xp -= oppositeCorrect ? land.xp/2 : land.xp;
			if (playerOld->xp < 0)
				playerOld->xp = 0;
		}
	}

	return true;
}



/**
 * @brief ConquestConfig::allLandSwapPlayer
 * @param playerNew
 * @param playerOld
 * @return
 */

inline bool ConquestConfig::allLandSwapPlayer(ConquestPlayer *playerNew, ConquestPlayer *playerOld)
{
	if (!playerNew || !playerOld)
		return false;

	for (ConquestWorldData &land : world.landList) {
		if (land.proprietor != playerOld->playerId)
			continue;

		if (land.xpOnce > 0) {
			playerNew->xp += land.xpOnce;
			playerOld->xp -= land.xpOnce;
			if (playerOld->xp < 0)
				playerOld->xp = 0;
			land.xpOnce = 0;
		} else {
			playerNew->xp += land.xp;
			playerOld->xp -= land.xp;
			if (playerOld->xp < 0)
				playerOld->xp = 0;
		}

		land.proprietor = playerNew->playerId;
	}

	return true;
}




/**
 * @brief ConquestConfig::landDefended
 * @param landId
 * @param playerOld
 * @return
 */

inline bool ConquestConfig::landDefended(const QString &landId, ConquestPlayer *playerOld)
{
	if (!playerOld)
		return false;

	if (currentTurn < 0 || currentTurn >= turnList.size())
		return false;

	ConquestTurn &turn = turnList[currentTurn];

	if (turn.subStage != ConquestTurn::SubStageUserAnswer)
		return false;

	const int &idx = world.landFind(landId);

	if (idx == -1)
		return false;

	ConquestWorldData &land = world.landList[idx];

	if (land.proprietor != playerOld->playerId)
		return false;

	if (land.xpOnce > 0)
		playerOld->xp += land.xpOnce;
	else
		playerOld->xp += land.xp;

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

	return world.landList.at(idx).proprietor;
}



/**
 * @brief ConquestConfig::checkPlayerLands
 * @param playerId
 * @return
 */

inline bool ConquestConfig::checkPlayerLands(const int &playerId) const
{
	for (const ConquestWorldData &land : world.landList) {
		if (land.proprietor == playerId)
			return true;
	}

	return false;
}



/**
 * @brief ConquestConfig::removePlayerFromNextTurns
 * @param playerId
 */

inline void ConquestConfig::removePlayerFromNextTurns(const int &playerId)
{
	if (currentTurn < 0 || currentTurn >= turnList.size())
		return;

	auto it = turnList.begin();

	for (it += currentTurn+1; it != turnList.end(); ++it) {
		if (it->player == playerId)
			it->player = -1;
	}
}


/**
 * @brief ConquestConfig::playerResult
 * @param player
 * @return
 */

inline bool ConquestConfig::playerResult(const ConquestPlayer &player) const
{
	if (player.hp <= 0)
		return false;

	if (gameState != ConquestConfig::StateFinished)
		return false;

	if (!checkPlayerLands(player.playerId))
		return false;

	return true;
}




#endif // CONQUESTCONFIG_H
