/*
 * ---- Call of Suli ----
 *
 * conquestlanddata.cpp
 *
 * Created on: 2024. 01. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ConquestLandData
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

#include "conquestlanddata.h"

ConquestLandData::ConquestLandData(QObject *parent)
	: QObject{parent}
{

}



/**
 * @brief ConquestLandData::loadFromConfig
 * @param data
 */

void ConquestLandData::loadFromConfig(const ConquestWorldData &data)
{
	if (data.id != m_landId)
		return;

	setProprietor(data.proprietor);
	setXp(data.xp + data.xpOnce);
}



/**
 * @brief ConquestLandData::landId
 * @return
 */

QString ConquestLandData::landId() const
{
	return m_landId;
}

void ConquestLandData::setLandId(const QString &newLandId)
{
	if (m_landId == newLandId)
		return;
	m_landId = newLandId;
	emit landIdChanged();
}

qreal ConquestLandData::baseX() const
{
	return m_baseX;
}

void ConquestLandData::setBaseX(qreal newBaseX)
{
	if (qFuzzyCompare(m_baseX, newBaseX))
		return;
	m_baseX = newBaseX;
	emit baseXChanged(m_baseX);
}

qreal ConquestLandData::baseY() const
{
	return m_baseY;
}

void ConquestLandData::setBaseY(qreal newBaseY)
{
	if (qFuzzyCompare(m_baseY, newBaseY))
		return;
	m_baseY = newBaseY;
	emit baseYChanged(m_baseY);
}

QUrl ConquestLandData::imgMap() const
{
	return m_imgMap;
}

void ConquestLandData::setImgMap(const QUrl &newImgMap)
{
	if (m_imgMap == newImgMap)
		return;
	m_imgMap = newImgMap;
	emit imgMapChanged();
}

QUrl ConquestLandData::imgBorder() const
{
	return m_imgBorder;
}

void ConquestLandData::setImgBorder(const QUrl &newImgBorder)
{
	if (m_imgBorder == newImgBorder)
		return;
	m_imgBorder = newImgBorder;
	emit imgBorderChanged();
}

int ConquestLandData::proprietor() const
{
	return m_proprietor;
}

void ConquestLandData::setProprietor(int newProprietor)
{
	if (m_proprietor == newProprietor)
		return;
	m_proprietor = newProprietor;
	emit proprietorChanged();
}



/**
 * @brief ConquestLandData::game
 * @return
 */

ConquestGame *ConquestLandData::game() const
{
	return m_game;
}

void ConquestLandData::setGame(ConquestGame *newGame)
{
	if (m_game == newGame)
		return;
	m_game = newGame;
	emit gameChanged();
}

int ConquestLandData::xp() const
{
	return m_xp;
}

void ConquestLandData::setXp(int newXp)
{
	if (m_xp == newXp)
		return;
	m_xp = newXp;
	emit xpChanged();
}
