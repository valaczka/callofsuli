/*
 * ---- Call of Suli ----
 *
 * desktoputils.cpp
 *
 * Created on: 2026. 01. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * DesktopUtils
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

#include "desktoputils.h"
#include <QFile>
#include <sodium.h>

DesktopUtils::DesktopUtils()
{

}



/**
 * @brief DesktopUtils::getExeHash
 * @param exePath
 * @param err
 * @param chunkSize
 * @return
 */

std::optional<QByteArray> DesktopUtils::getExeHash(const QString &exePath, QString *err, const qint64 &chunkSize)
{
	QFile f(exePath);

	if (!f.open(QIODevice::ReadOnly)) {
		if (err) *err = QStringLiteral("cannot open file for reading");
		return std::nullopt;
	}

	crypto_generichash_state stFile{};

	if (crypto_generichash_init(&stFile, nullptr, 0, crypto_generichash_BYTES) != 0) {
		if (err) *err = QStringLiteral("crypto_generichash_init failed");
		return std::nullopt;
	}

	QByteArray chunk(std::max(4096ll, chunkSize), Qt::Uninitialized);

	while (true) {
		const qint64 got = f.read(chunk.data(), chunk.size());

		if (got <= 0)
			break;

		crypto_generichash_update(&stFile,
								  reinterpret_cast<const unsigned char*>(chunk.constData()),
								  (unsigned long long) got);
	}


	QByteArray out(crypto_generichash_BYTES, Qt::Uninitialized);

	crypto_generichash_final(&stFile,
							 reinterpret_cast<unsigned char*>(out.data()),
							 out.size());

	return out;


}
