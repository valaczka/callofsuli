/*
 * ---- Call of Suli ----
 *
 * rpgcontrolgroupsave.h
 *
 * Created on: 2024. 05. 08.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlGroupSave
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

#ifndef RPGCONTROLGROUPSAVE_H
#define RPGCONTROLGROUPSAVE_H

#include "qtimer.h"
#include "rpgcontrolgroup.h"
#include "rpgplayer.h"
#include <libtiledquick/tilelayeritem.h>


/**
 * @brief The RpgControlGroupSave class
 */

class RpgControlGroupSave : public RpgControlGroup
{
public:
	RpgControlGroupSave(RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer);

	int count() const;
	void setCount(int newCount);

private:
	void onFixtureBeginContact(Box2DFixture *other);
	void connectFixture(Box2DFixture *fixture);
	void updateLayers();
	void hide(RpgPlayer *player);
	void show();
	void deactivate();

	QPointer<Box2DFixture> m_fixture;
	QVector<QPointer<TiledQuick::TileLayerItem>> m_tileLayers;

	int m_count = -1;
	bool m_active = true;

	QTimer m_timer;
};



#endif // RPGCONTROLGROUPSAVE_H
