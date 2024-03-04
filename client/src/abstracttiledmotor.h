/*
 * ---- Call of Suli ----
 *
 * abstracttiledmotor.h
 *
 * Created on: 2024. 03. 04.
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

#ifndef ABSTRACTTILEDMOTOR_H
#define ABSTRACTTILEDMOTOR_H

#include "qpoint.h"


class AbstractTiledMotor
{
public:
	enum Type {
		Invalid = 0,
		PathMotor,
		ReturnPathMotor
	};

	AbstractTiledMotor(const Type &type)
		: m_type(type)
	{}

	const Type &type() const { return m_type; }

	virtual QPointF currentPosition() const = 0;

protected:
	const Type m_type;
};

#endif // ABSTRACTTILEDMOTOR_H
