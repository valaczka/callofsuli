/*
 * ---- Call of Suli ----
 *
 * rpgcontrolgroupoverlay.h
 *
 * Created on: 2024. 03. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlGroupOverlay
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

#ifndef RPGCONTROLGROUPOVERLAY_H
#define RPGCONTROLGROUPOVERLAY_H

#include "rpgcontrolgroup.h"
#include <libtiledquick/tilelayeritem.h>


/**
 * @brief The RpgControlGroupOverlay class
 */

class RpgControlGroupOverlay : public RpgControlGroup
{
public:
	RpgControlGroupOverlay(RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer);

private:
	void onFixtureBeginContact(Box2DFixture *other);
	void onFixtureEndContact(Box2DFixture *other);
	void onControlledPlayerChanged();
	void connectFixture(Box2DFixture *fixture);
	void updateLayers();

	QVector<QPointer<TiledQuick::TileLayerItem>> m_tileLayers;
	QVector<QPointer<Box2DFixture>> m_contactedFixtures;

};

#endif // RPGCONTROLGROUPOVERLAY_H
