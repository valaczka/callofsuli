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


/**
 * @brief The ConquestWorldInfo class
 */

class ConquestWorldInfo : public QSerializer
{
	Q_GADGET

public:
	ConquestWorldInfo()
		: playerCount(0)
	{}

	QS_SERIALIZABLE
	QS_FIELD(int, playerCount)
	QS_COLLECTION(QList, QString, landIdList)
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

	friend bool operator==(const ConquestWorld &w1, const ConquestWorld &w2) {
		return w1.name == w2.name &&
				w1.playerCount == w2.playerCount
				;
	}

	QS_SERIALIZABLE
	QS_FIELD(QString, name)
	QS_COLLECTION(QList, QString, landIdList)
	QS_FIELD(int, playerCount)

	QS_COLLECTION_OBJECTS(QList, ConquestWorldInfo, infoList)
};



/**
 * @brief The ConquestWordListHelper class
 */

class ConquestWordListHelper : public QSerializer
{
	Q_GADGET

public:
	QS_SERIALIZABLE
	QS_COLLECTION_OBJECTS(QList, ConquestWorld, worldList);
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


	friend bool operator==(const ConquestConfig &c1, const ConquestConfig &c2) {
		return c1.gameState == c2.gameState &&
				c1.world == c2.world &&
				c1.mapUuid == c2.mapUuid &&
				c1.missionUuid == c2.missionUuid &&
				c1.missionLevel == c2.missionLevel
				;
	}

	QS_SERIALIZABLE
	QS_FIELD(GameState, gameState)
	QS_FIELD(QString, mapUuid)
	QS_FIELD(QString, missionUuid)
	QS_FIELD(int, missionLevel)
	QS_OBJECT(ConquestWorld, world)
};




#endif // CONQUESTCONFIG_H
