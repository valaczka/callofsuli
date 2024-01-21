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

#include "qjsonobject.h"
#include <QString>

struct ConquestConfig
{
	Q_GADGET

	Q_PROPERTY(GameState gameState MEMBER state)
	Q_PROPERTY(QString world MEMBER world)
	Q_PROPERTY(QString mapUuid MEMBER mapUuid)
	Q_PROPERTY(QString missionUuid MEMBER missionUuid)
	Q_PROPERTY(int missionLevel MEMBER missionLevel)

public:
	ConquestConfig() = default;
	~ConquestConfig() = default;

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
	 * @brief toJson
	 * @return
	 */

	QJsonObject toJson() const {
		QJsonObject o;
		o[QStringLiteral("state")] = state;
		o[QStringLiteral("world")] = world;
		o[QStringLiteral("map")] = mapUuid;
		o[QStringLiteral("mission")] = missionUuid;
		o[QStringLiteral("level")] = missionLevel;
		return o;
	}

	/**
	 * @brief fromJson
	 * @param o
	 * @return
	 */

	static ConquestConfig fromJson(const QJsonObject &o) {
		ConquestConfig c;
		c.state = o.value(QStringLiteral("state")).toVariant().value<GameState>();
		c.world = o.value(QStringLiteral("world")).toString();
		c.mapUuid = o.value(QStringLiteral("map")).toString();
		c.missionUuid = o.value(QStringLiteral("mission")).toString();
		c.missionLevel = o.value(QStringLiteral("level")).toInt();
		return c;
	}


	friend bool operator==(const ConquestConfig &c1, const ConquestConfig &c2) {
		return c1.state == c2.state &&
				c1.world == c2.world &&
				c1.mapUuid == c2.mapUuid &&
				c1.missionUuid == c2.missionUuid &&
				c1.missionLevel == c2.missionLevel
				;
	}

	// Members

	GameState state = StateInvalid;
	QString world;
	QString mapUuid;
	QString missionUuid;
	int missionLevel = 0;
};


#endif // CONQUESTCONFIG_H
