/*
 * ---- Call of Suli ----
 *
 * conqueststate.cpp
 *
 * Created on: 2024. 01. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ConquestState
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

#include "conqueststate.h"
#include "utils_.h"

ConquestState::ConquestState()
	: m_color(Qt::green)
{

}

QString ConquestState::world() const
{
	return m_world;
}

void ConquestState::setWorld(const QString &newWorld)
{
	if (m_world == newWorld)
		return;
	m_world = newWorld;
	emit worldChanged();
	reload();
}

int ConquestState::stateId() const
{
	return m_stateId;
}

void ConquestState::setStateId(int newStateId)
{
	if (m_stateId == newStateId)
		return;
	m_stateId = newStateId;
	emit stateIdChanged();
	reload();
}

QColor ConquestState::color() const
{
	return m_color;
}

void ConquestState::setColor(const QColor &newColor)
{
	if (m_color == newColor)
		return;
	m_color = newColor;
	emit colorChanged();
}



/**
 * @brief ConquestState::reload
 */

void ConquestState::reload()
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
 * @brief ConquestState::setIsValid
 * @param valid
 */

void ConquestState::setIsValid(const bool &valid)
{
	if (m_isValid == valid)
		return;
	m_isValid = valid;
	emit isValidChanged();
}

bool ConquestState::isActive() const
{
	return m_isActive;
}

void ConquestState::setIsActive(bool newIsActive)
{
	if (m_isActive == newIsActive)
		return;
	m_isActive = newIsActive;
	emit isActiveChanged();
}

qreal ConquestState::baseY() const
{
	return m_baseY;
}

void ConquestState::setBaseY(qreal newBaseY)
{
	if (qFuzzyCompare(m_baseY, newBaseY))
		return;
	m_baseY = newBaseY;
	emit baseYChanged();
}

qreal ConquestState::baseX() const
{
	return m_baseX;
}

void ConquestState::setBaseX(qreal newBaseX)
{
	if (qFuzzyCompare(m_baseX, newBaseX))
		return;
	m_baseX = newBaseX;
	emit baseXChanged();
}


/**
 * @brief ConquestState::isValid
 * @return
 */

bool ConquestState::isValid() const
{
	return m_isValid;
}

