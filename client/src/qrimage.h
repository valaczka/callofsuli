/*
 * ---- Call of Suli ----
 *
 * qrimage.h
 *
 * Created on: 2023. 03. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * QrImage
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

#ifndef QRIMAGE_H
#define QRIMAGE_H

#include <QQuickImageProvider>

class QrImage : public QQuickImageProvider
{
public:
	QrImage();

	QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
};

#endif // QRIMAGE_H
