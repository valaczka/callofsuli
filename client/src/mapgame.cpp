/*
 * ---- Call of Suli ----
 *
 * mapgame.cpp
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

#include "mapgame.h"

MapGame::MapGame(QObject *parent)
	: QObject{parent}
	, m_user(new User(this))
{

}


/**
 * @brief MapGame::~MapGame
 */

MapGame::~MapGame()
{
	delete m_user;
	m_user = nullptr;
}


/**
 * @brief MapGame::loadFromJson
 * @param object
 * @param allField
 */

void MapGame::loadFromJson(const QJsonObject &object, const bool &allField)
{
	if (object.contains(QStringLiteral("username")) || allField)
		m_user->loadFromJson(object, allField);

	if (object.contains(QStringLiteral("num")) || allField)
		setSolved(object.value(QStringLiteral("num")).toInt());

	if (object.contains(QStringLiteral("dMax")) || allField)
		setDurationMax(object.value(QStringLiteral("dMax")).toInt());

	if (object.contains(QStringLiteral("dMin")) || allField)
		setDurationMin(object.value(QStringLiteral("dMin")).toInt());

	if (object.contains(QStringLiteral("durationPos")) || allField)
		setPosDuration(object.value(QStringLiteral("durationPos")).toInt());

	if (object.contains(QStringLiteral("numPos")) || allField)
		setPosSolved(object.value(QStringLiteral("numPos")).toInt());
}




/**
 * @brief MapGame::user
 * @return
 */

User *MapGame::user() const
{
	return m_user;
}

int MapGame::solved() const
{
	return m_solved;
}

void MapGame::setSolved(int newSolved)
{
	if (m_solved == newSolved)
		return;
	m_solved = newSolved;
	emit solvedChanged();
}

int MapGame::durationMin() const
{
	return m_durationMin;
}

void MapGame::setDurationMin(int newDurationMin)
{
	if (m_durationMin == newDurationMin)
		return;
	m_durationMin = newDurationMin;
	emit durationMinChanged();
}

int MapGame::durationMax() const
{
	return m_durationMax;
}

void MapGame::setDurationMax(int newDurationMax)
{
	if (m_durationMax == newDurationMax)
		return;
	m_durationMax = newDurationMax;
	emit durationMaxChanged();
}

int MapGame::posDuration() const
{
	return m_posDuration;
}

void MapGame::setPosDuration(int newPosDuration)
{
	if (m_posDuration == newPosDuration)
		return;
	m_posDuration = newPosDuration;
	emit posDurationChanged();
}

int MapGame::posSolved() const
{
	return m_posSolved;
}

void MapGame::setPosSolved(int newPosSolved)
{
	if (m_posSolved == newPosSolved)
		return;
	m_posSolved = newPosSolved;
	emit posSolvedChanged();
}
