/*
 * ---- Call of Suli ----
 *
 * rpgtimepickable.h
 *
 * Created on: 2024. 04. 05.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgTimePickable
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

#ifndef RPGTIMEPICKABLE_H
#define RPGTIMEPICKABLE_H

#include "rpgpickableobject.h"
#include <QQmlEngine>

class RpgTimePickable : public RpgPickableObject
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgTimePickable(TiledScene *scene = nullptr);

	bool playerPick(RpgPlayer *) override final;

protected:
	void load() override final;
};

#endif // RPGTIMEPICKABLE_H
