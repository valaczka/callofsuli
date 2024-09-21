/*
 * ---- Call of Suli ----
 *
 * isometricobjectiface.h
 *
 * Created on: 2024. 03. 03.
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

#ifndef ISOMETRICOBJECTIFACE_H
#define ISOMETRICOBJECTIFACE_H

#include "qobjectdefs.h"
#include "qtypes.h"


/**
 * @brief The IsometricObjectIface class
 */

class IsometricObjectIface
{
	Q_GADGET

public:
	IsometricObjectIface();

	qreal defaultZ() const;
	void setDefaultZ(qreal newDefaultZ);

	bool useDynamicZ() const;
	void setUseDynamicZ(bool newUseDynamicZ);

	qreal subZ() const;
	void setSubZ(qreal newSubZ);

public:
	virtual void defaultZChanged() = 0;
	virtual void useDynamicZChanged() = 0;
	virtual void subZChanged() = 0;

protected:
	qreal m_defaultZ = 0;
	qreal m_subZ = 0;
	bool m_useDynamicZ = true;
};




#endif // ISOMETRICOBJECTIFACE_H
