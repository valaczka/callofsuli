/*
 * ---- Call of Suli ----
 *
 * qrimage.cpp
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

#include "qrimage.h"
#include "Logger.h"

/**
 * @brief QrImage::QrImage
 */

QrImage::QrImage()
	: QQuickImageProvider(QQuickImageProvider::Image)
{

}


/**
 * @brief QrImage::requestImage
 * @param id
 * @param size
 * @param requestedSize
 * @return
 */

QImage QrImage::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
	QSize s = requestedSize;
	if (s == QSize(-1,-1))
		s = QSize(250, 250);

	QString t = QString::fromUtf8(QByteArray::fromBase64(id.toLocal8Bit()));

	LOG_CERROR("client") << "Missin implementation in" << __PRETTY_FUNCTION__;

	LOG_CTRACE("client") << "Encode QR:" << t;

	QImage result; //= QZXing::encodeData(t, QZXing::EncoderFormat_QR_CODE, s, QZXing::EncodeErrorCorrectionLevel_L, true, false);

	*size = result.size();

	return result;
}
