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
#include "conquestgame.h"

ConquestLand::ConquestLand()
	: m_baseColor(Qt::transparent)
	, m_ownColor(Qt::yellow)
{

}

ConquestLandData *ConquestLand::landData() const
{
	return m_landData;
}

void ConquestLand::setLandData(ConquestLandData *newLandData)
{
	if (m_landData == newLandData)
		return;
	m_landData = newLandData;
	emit landDataChanged();

	if (m_landData) {
		connect(m_landData, &ConquestLandData::baseXChanged, this, &ConquestLand::setX);
		connect(m_landData, &ConquestLandData::baseYChanged, this, &ConquestLand::setY);
		connect(m_landData, &ConquestLandData::proprietorChanged, this, &ConquestLand::onProprietorChanged);

		setX(m_landData->baseX());
		setY(m_landData->baseY());
		onProprietorChanged();
	}
}

QColor ConquestLand::baseColor() const
{
	return m_baseColor;
}

void ConquestLand::setBaseColor(const QColor &newBaseColor)
{
	if (m_baseColor == newBaseColor)
		return;
	m_baseColor = newBaseColor;
	emit baseColorChanged();
}


/**
 * @brief ConquestLand::onProprietorChanged
 */

void ConquestLand::onProprietorChanged()
{
	if (m_landData && m_landData->game() && m_landData->game()->playerId() != -1) {
		ConquestGame *game = m_landData->game();

		if (m_landData->proprietor() != -1) {
			setBaseColor(game->getPlayerColor(m_landData->proprietor()));
			setActive(true);
			return;
		}
	}

	setBaseColor(Qt::transparent);
	setActive(false);
}

QColor ConquestLand::ownColor() const
{
	return m_ownColor;
}

void ConquestLand::setOwnColor(const QColor &newOwnColor)
{
	if (m_ownColor == newOwnColor)
		return;
	m_ownColor = newOwnColor;
	emit ownColorChanged();
}

bool ConquestLand::active() const
{
	return m_active;
}

void ConquestLand::setActive(bool newActive)
{
	if (m_active == newActive)
		return;
	m_active = newActive;
	emit activeChanged();
}


