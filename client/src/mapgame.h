/*
 * ---- Call of Suli ----
 *
 * mapgame.h
 *
 * Created on: 2023. 04. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapGame
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

#ifndef MAPGAME_H
#define MAPGAME_H

#include <QObject>
#include "qjsonobject.h"
#include "user.h"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"

class MapGame;
using MapGameList = qolm::QOlm<MapGame>;
Q_DECLARE_METATYPE(MapGameList*)


class MapGame : public QObject
{
	Q_OBJECT

	Q_PROPERTY(User *user READ user CONSTANT)
	Q_PROPERTY(QString username READ username NOTIFY usernameChanged)
	Q_PROPERTY(int solved READ solved WRITE setSolved NOTIFY solvedChanged)
	Q_PROPERTY(int durationMin READ durationMin WRITE setDurationMin NOTIFY durationMinChanged)
	Q_PROPERTY(int durationMax READ durationMax WRITE setDurationMax NOTIFY durationMaxChanged)
	Q_PROPERTY(int posDuration READ posDuration WRITE setPosDuration NOTIFY posDurationChanged)
	Q_PROPERTY(int posSolved READ posSolved WRITE setPosSolved NOTIFY posSolvedChanged)

public:
	explicit MapGame(QObject *parent = nullptr);
	virtual ~MapGame();

	void loadFromJson(const QJsonObject &object, const bool &allField = true);

	User *user() const;

	int solved() const;
	void setSolved(int newSolved);

	int durationMin() const;
	void setDurationMin(int newDurationMin);

	int durationMax() const;
	void setDurationMax(int newDurationMax);

	int posDuration() const;
	void setPosDuration(int newPosDuration);

	int posSolved() const;
	void setPosSolved(int newPosSolved);

	const QString &username() const;

signals:
	void solvedChanged();
	void durationMinChanged();
	void durationMaxChanged();
	void posDurationChanged();
	void posSolvedChanged();
	void usernameChanged();

private:
	std::unique_ptr<User> m_user;
	int m_solved = 0;
	int m_durationMin = 0;
	int m_durationMax = 0;
	int m_posDuration = 0;
	int m_posSolved = 0;
};

#endif // MAPGAME_H
