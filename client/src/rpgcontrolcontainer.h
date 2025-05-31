/*
 * ---- Call of Suli ----
 *
 * rpgcontrolcontainer.h
 *
 * Created on: 2025. 05. 18.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlContainer
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

#ifndef RPGCONTROLCONTAINER_H
#define RPGCONTROLCONTAINER_H

#include <QObject>
#include "rpgcontrol.h"

class RpgControlContainer : public RpgActiveControl<RpgGameData::ControlContainer,
		RpgGameData::ControlContainerBaseData,
		RpgGameData::ControlContainer::State>
{
public:
	RpgControlContainer(RpgGame *game, TiledScene *scene,
						Tiled::GroupLayer *group, Tiled::MapRenderer *renderer = nullptr);


	virtual TiledObjectBody::ObjectId objectId() const override { return m_objectId; }

	virtual void updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::ControlContainer> &snapshot) override;
	virtual void updateFromSnapshot(const RpgGameData::ControlContainer &snap) override;

	virtual void use(RpgPlayer *player) override;

protected:
	virtual bool loadFromLayer(RpgGame *game, TiledScene *scene,
							   Tiled::Layer *layer, Tiled::MapRenderer *renderer = nullptr) override;

	virtual RpgGameData::ControlContainer serializeThis() const override;

	virtual void onShapeContactBegin(cpShape *self, cpShape *other) override;
	virtual void onShapeContactEnd(cpShape *self, cpShape *other) override;

private:
	void _updateGlow();

	TiledObjectBody::ObjectId m_objectId;
};

#endif // RPGCONTROLCONTAINER_H
