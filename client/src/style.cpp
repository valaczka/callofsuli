/*
 * ---- Call of Suli ----
 *
 * style.cpp
 *
 * Created on: 2022. 12. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Style
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

#include "style.h"

Style::Style(QObject *parent)
	: QObject{parent}
	, m_colorGlow("white")
	, m_colorEnemyGlow("red")
{

}


Style::~Style()
{

}

const QColor &Style::colorGlow() const
{
	return m_colorGlow;
}

void Style::setColorGlow(const QColor &newColorGlow)
{
	if (m_colorGlow == newColorGlow)
		return;
	m_colorGlow = newColorGlow;
	emit colorGlowChanged();
}

const QColor &Style::colorEnemyGlow() const
{
	return m_colorEnemyGlow;
}

void Style::setColorEnemyGlow(const QColor &newColorEnemyGlow)
{
	if (m_colorEnemyGlow == newColorEnemyGlow)
		return;
	m_colorEnemyGlow = newColorEnemyGlow;
	emit colorEnemyGlowChanged();
}
