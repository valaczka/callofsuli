/*
 * ---- Call of Suli ----
 *
 * mapimage.h
 *
 * Created on: 2022. 01. 05.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapImage
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

#ifndef MAPIMAGE_H
#define MAPIMAGE_H

#include <QQuickImageProvider>
#include <QPixmap>
#include "gamemapreaderiface.h"

class MapImage : public QQuickImageProvider
{
public:
	MapImage(GameMapReaderIface *map = nullptr);

	QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);

	GameMapReaderIface *map() const { return m_map; }
	void setMap(GameMapReaderIface *newMap) { m_map = newMap; }

private:
	GameMapReaderIface *m_map;
};

#endif // MAPIMAGE_H
