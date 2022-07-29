/*
 * ---- Call of Suli ----
 *
 * mapimage.cpp
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

#include "mapimage.h"

MapImage::MapImage(GameMapReaderIface *map)
	: QQuickImageProvider(QQuickImageProvider::Pixmap)
	, m_map(map)
{


}


/**
 * @brief MapImage::requestPixmap
 * @param id
 * @param size
 * @param requestedSize
 * @return
 */

QPixmap MapImage::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
	if (m_map) {
		QPixmap outPixmap;

		bool isOk = false;
		qint32 idInt = id.toInt(&isOk);	/* TODO */

		if (isOk) {
			foreach (GameMapImageIface *i, m_map->ifaceImages()) {
				if (i->m_id == idInt) {
					if (!outPixmap.loadFromData(i->m_data)) {
						qWarning() << "Invalid image data" << i->m_id;
					}
					break;
				}
			}
		}


		if (!outPixmap.isNull()) {
			if (requestedSize.isValid())
				outPixmap = outPixmap.scaled(requestedSize, Qt::KeepAspectRatio);

			if (size)
				*size = outPixmap.size();
			return outPixmap;
		}
	}

	int width = 100;
	int height = 50;

	if (size)
		*size = QSize(width, height);

	QPixmap pixmap(requestedSize.width() > 0 ? requestedSize.width() : width,
				   requestedSize.height() > 0 ? requestedSize.height() : height);
	pixmap.fill(QColor("red").rgba());
	return pixmap;
}






