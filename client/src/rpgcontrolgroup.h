/*
 * ---- Call of Suli ----
 *
 * rpgcontrolgorup.h
 *
 * Created on: 2024. 03. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlGorup
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

#ifndef RPGCONTROLGROUP_H
#define RPGCONTROLGROUP_H

#include "tiledscene.h"

class RpgGame;

class RpgControlGroup
{
public:
	enum Type {
		ControlGroupInvalid = 0,
		ControlGroupOverlay
	};

	RpgControlGroup(const Type &type, RpgGame *game);
	virtual ~RpgControlGroup() = default;

	static RpgControlGroup* fromGroupLayer(RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer);

	Type type() const;

protected:
	RpgGame *const m_game;

private:
	const Type m_type;
};

#endif // RPGCONTROLGROUP_H
