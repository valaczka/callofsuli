/*
 * ---- Call of Suli ----
 *
 * rpgcontrolteleport.h
 *
 * Created on: 2025. 06. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlTeleport
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

#ifndef RPGCONTROLTELEPORT_H
#define RPGCONTROLTELEPORT_H

#include "rpgcontrol.h"



enum RpgControlTeleportState {
	Inactive = 0,
	Active,
	Operating
};


/**
 * @brief The RpgControlTeleport class
 */

class RpgControlTeleport : public RpgActiveControl<RpgGameData::ControlTeleport,
		RpgGameData::ControlTeleportBaseData,
		RpgControlTeleportState>
{
public:
	RpgControlTeleport(RpgGame *game, TiledScene *scene,
					   Tiled::GroupLayer *group, const bool &isHideout,
					   Tiled::MapRenderer *renderer = nullptr);

	virtual void updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::ControlTeleport> &snapshot) override;
	virtual void updateFromSnapshot(const RpgGameData::ControlTeleport &snap) override;

protected:
	virtual bool loadFromLayer(RpgGame *game, TiledScene *scene,
							   Tiled::Layer *layer, Tiled::MapRenderer *renderer = nullptr) override;

	virtual RpgGameData::ControlTeleport serializeThis() const override;

	virtual void onShapeContactBegin(cpShape *self, cpShape *other) override;
	virtual void onShapeContactEnd(cpShape *self, cpShape *other) override;
};

#endif // RPGCONTROLTELEPORT_H
