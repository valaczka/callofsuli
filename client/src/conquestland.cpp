/*
 * ---- Call of Suli ----
 *
 * conqueststate.cpp
 *
 * Created on: 2024. 01. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ConquestLand
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

#include "conquestland.h"
#include "utils_.h"

ConquestLand::ConquestLand()
	: m_color(Qt::green)
{

}

QString ConquestLand::world() const
{
	return m_world;
}

void ConquestLand::setWorld(const QString &newWorld)
{
	if (m_world == newWorld)
		return;
	m_world = newWorld;
	emit worldChanged();
	reload();
}

int ConquestLand::stateId() const
{
	return m_stateId;
}

void ConquestLand::setStateId(int newStateId)
{
	if (m_stateId == newStateId)
		return;
	m_stateId = newStateId;
	emit stateIdChanged();
	reload();
}

QColor ConquestLand::color() const
{
	return m_color;
}

void ConquestLand::setColor(const QColor &newColor)
{
	if (m_color == newColor)
		return;
	m_color = newColor;
	emit colorChanged();
}



/**
 * @brief ConquestLand::reload
 */

void ConquestLand::reload()
{
	if (m_world.isEmpty() || m_stateId < 0)
		return setIsValid(false);

	const auto &data = Utils::fileToJsonObject(QStringLiteral(":/conquest/%1/data.json").arg(m_world));

	if (!data || !data->contains(QStringLiteral("states"))) {
		return setIsValid(false);
	}

	const QJsonObject &stateData = data->value(QStringLiteral("states")).toObject().value(QString::number(m_stateId)).toObject();

	if (stateData.isEmpty()) {
		return setIsValid(false);
	}

	setIsValid(true);
	setBaseX(stateData.value(QStringLiteral("x")).toDouble());
	setBaseY(stateData.value(QStringLiteral("y")).toDouble());
}


/**
 * @brief ConquestLand::setIsValid
 * @param valid
 */

void ConquestLand::setIsValid(const bool &valid)
{
	if (m_isValid == valid)
		return;
	m_isValid = valid;
	emit isValidChanged();
}

ConquestGame *ConquestLand::game() const
{
	return m_game;
}

void ConquestLand::setGame(ConquestGame *newGame)
{
	if (m_game == newGame)
		return;
	m_game = newGame;
	emit gameChanged();

	if (m_game) {
		connect(m_game, &ConquestGame::testImage, this, [this](const int id, const bool v) {
			if (id == m_stateId)
				setIsActive(v);
		});
	}
}

bool ConquestLand::isActive() const
{
	return m_isActive;
}

void ConquestLand::setIsActive(bool newIsActive)
{
	if (m_isActive == newIsActive)
		return;
	m_isActive = newIsActive;
	emit isActiveChanged();
}

qreal ConquestLand::baseY() const
{
	return m_baseY;
}

void ConquestLand::setBaseY(qreal newBaseY)
{
	if (qFuzzyCompare(m_baseY, newBaseY))
		return;
	m_baseY = newBaseY;
	emit baseYChanged();
}

qreal ConquestLand::baseX() const
{
	return m_baseX;
}

void ConquestLand::setBaseX(qreal newBaseX)
{
	if (qFuzzyCompare(m_baseX, newBaseX))
		return;
	m_baseX = newBaseX;
	emit baseXChanged();
}


/**
 * @brief ConquestLand::isValid
 * @return
 */

bool ConquestLand::isValid() const
{
	return m_isValid;
}

