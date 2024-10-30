/*
 * ---- Call of Suli ----
 *
 * rpgcontrolgorup.cpp
 *
 * Created on: 2024. 03. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlGroup
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

#include "rpgcontrolgroup.h"
#include "rpgcontrolgroupcontainer.h"
#include "rpgcontrolgroupdoor.h"
#include "rpgcontrolgroupoverlay.h"
#include "rpgcontrolgroupsave.h"
#include <libtiled/grouplayer.h>


RpgControlGroup::RpgControlGroup(const Type &type, RpgGame *game, TiledScene *scene)
	: m_game(game)
	, m_scene(scene)
	, m_type(type)
{
	Q_ASSERT(game);
}



/**
 * @brief RpgControlGroup::fromGroupLayer
 * @param scene
 * @param group
 * @param renderer
 * @return
 */

RpgControlGroup *RpgControlGroup::fromGroupLayer(RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer)
{
	const QString &cname = group->className();

	if (cname == QStringLiteral("overlayTrigger"))
		return new RpgControlGroupOverlay(game, scene, group, renderer);
	else if (cname == QStringLiteral("container"))
		return new RpgControlGroupContainer(game, scene, group, renderer);
	else if (cname == QStringLiteral("save"))
		return new RpgControlGroupSave(game, scene, group, renderer);
	else if (cname == QStringLiteral("door"))
		return new RpgControlGroupDoor(game, scene, group, renderer);


	LOG_CWARNING("game") << "Invalid group layer:" << cname;
	return nullptr;
}


/**
 * @brief RpgControlGroup::type
 * @return
 */

RpgControlGroup::Type RpgControlGroup::type() const
{
	return m_type;
}
