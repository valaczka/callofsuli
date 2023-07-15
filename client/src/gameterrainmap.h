/*
 * ---- Call of Suli ----
 *
 * gameterrainmap.h
 *
 * Created on: 2022. 12. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameTerrainMap
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

#ifndef GAMETERRAINMAP_H
#define GAMETERRAINMAP_H

#include "gameterrain.h"

#include "libtiled/map.h"


class GameTerrainMap : public GameTerrain
{
public:
	GameTerrainMap(const QString &filename = "");
	~GameTerrainMap();

	bool loadMapFromFile(QString filename);
	bool loadMap(const QString &terrain, const int &level);

	bool loadObjectLayers();

	int width() const;
	int height() const;

	Tiled::Map* map() const;

protected:
	void readEnemyLayer(Tiled::ObjectGroup *layer);
	void readObjectLayer(Tiled::ObjectGroup *layer);
	void readPlayerLayer(Tiled::ObjectGroup *layer);
	void readPickableLayer(Tiled::ObjectGroup *layer);

	std::unique_ptr<Tiled::Map> m_map;
};

#endif // GAMETERRAINMAP_H
