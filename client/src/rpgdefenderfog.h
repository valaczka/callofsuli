/*
 * ---- Call of Suli ----
 *
 * rpgdefenderfog.h
 *
 * Created on: 2026. 07. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgDefenderFog
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

#ifndef RPGDEFENDERFOG_H
#define RPGDEFENDERFOG_H

#include "rpgdefender.h"
#include <QQmlEngine>


/**
 * @brief The RpgDefenderFog class
 */

class RpgDefenderFog : public RpgDefender
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgDefenderFog(RpgGameItem *gameItem, const Rpg::DefenderObject &config);

	virtual void initialize() override;

};

#endif // RPGDEFENDERFOG_H
