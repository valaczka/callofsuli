/*
 * ---- Call of Suli ----
 *
 * tiledpickableiface.h
 *
 * Created on: 2024. 03. 19.
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

#ifndef TILEDPICKABLEIFACE_H
#define TILEDPICKABLEIFACE_H

#include "qtmetamacros.h"


class TiledPickableIface
{
public:
	TiledPickableIface()
	{}


public:
	bool isActive() const;
	void setIsActive(bool newIsActive);


public:
	virtual void isActiveChanged() = 0;

protected:
	virtual void onActivated() = 0;
	virtual void onDeactivated() = 0;

protected:
	bool m_isActive = false;
};




/**
 * @brief TiledPickableIface::isActive
 * @return
 */

inline bool TiledPickableIface::isActive() const
{
	return m_isActive;
}


/**
 * @brief TiledPickableIface::setIsActive
 * @param newIsActive
 */

inline void TiledPickableIface::setIsActive(bool newIsActive)
{
	if (m_isActive == newIsActive)
		return;

	m_isActive = newIsActive;
	emit isActiveChanged();

	if (m_isActive)
		onActivated();
	else
		onDeactivated();
}

#endif // TILEDPICKABLEIFACE_H
