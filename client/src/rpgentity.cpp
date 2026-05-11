/*
 * ---- Call of Suli ----
 *
 * rpgentity.cpp
 *
 * Created on: 2026. 05. 11.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgEntity
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

#include "rpgentity.h"



RpgEntity::RpgEntity(RpgGameItem *gameItem, const QPointF &center, const qreal &radius, const cpBodyType &type)
	: RpgObject(gameItem, center, radius, type)
{

}

int RpgEntity::hp() const
{
	return m_hp;
}

void RpgEntity::setHp(int newHp)
{
	if (m_hp == newHp)
		return;
	m_hp = newHp;
	emit hpChanged();
}

int RpgEntity::maxHp() const
{
	return m_maxHp;
}

void RpgEntity::setMaxHp(int newMaxHp)
{
	if (m_maxHp == newMaxHp)
		return;
	m_maxHp = newMaxHp;
	emit maxHpChanged();
}
