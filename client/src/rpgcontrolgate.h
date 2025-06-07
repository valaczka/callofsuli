/*
 * ---- Call of Suli ----
 *
 * rpgcontrolgate.h
 *
 * Created on: 2025. 06. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlGate
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

#ifndef RPGCONTROLGATE_H
#define RPGCONTROLGATE_H

#include "rpgcontrol.h"

class RpgControlGate : public RpgActiveControl<RpgGameData::ControlGate,
		RpgGameData::ControlGateBaseData,
		RpgGameData::ControlGate::State>
{
public:
	RpgControlGate(RpgGame *game, TiledScene *scene,
				   Tiled::GroupLayer *group, Tiled::MapRenderer *renderer = nullptr);

	virtual void updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::ControlGate> &snapshot) override;
	virtual void updateFromSnapshot(const RpgGameData::ControlGate &snap) override;

protected:
	virtual bool loadFromLayer(RpgGame *game, TiledScene *scene,
							   Tiled::Layer *layer, Tiled::MapRenderer *renderer = nullptr) override;

	virtual RpgGameData::ControlGate serializeThis() const override;

	virtual void onShapeContactBegin(cpShape *self, cpShape *other) override;
	virtual void onShapeContactEnd(cpShape *self, cpShape *other) override;

	virtual void onCurrentStateChanged() override;

private:
	void _updateGlow();

	TiledScene *const m_scene;
	QHash<RpgGameData::ControlGate::State, QList<TiledObjectBody*> > m_groundList;

};


#endif // RPGCONTROLGATE_H
