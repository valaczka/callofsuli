/*
 * ---- Call of Suli ----
 *
 * rpgmp.h
 *
 * Created on: 2026. 05. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgMp
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

#ifndef RPGMP_H
#define RPGMP_H

#include "rpgobject.h"
#include <QQmlEngine>

class RpgMp : public RpgObject
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgMp(RpgGameItem *gameItem, const cpVect &emitterCenter, const cpVect &pos, const quint32 &duration = 60);

	virtual void initialize() override;

protected:
	virtual void onMotorStepped() override;

private:
	bool m_activated = false;

};

#endif // RPGMP_H
