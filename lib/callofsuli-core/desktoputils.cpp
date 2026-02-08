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
#include <QCoreApplication>
#include <sodium.h>

DesktopUtils::DesktopUtils()
{

}




#ifdef Q_OS_LINUX

#include <time.h>

static quint64 timespecToMs(const timespec& ts) {
	return (quint64)ts.tv_sec * 1000ull + (quint64)ts.tv_nsec / 1000000ull;
}

static QString currentExePath()
{
	char buf[PATH_MAX + 1];
	ssize_t n = readlink("/proc/self/exe", buf, PATH_MAX);
	if (n <= 0) return {};
	buf[n] = '\0';
	return QString::fromLocal8Bit(buf);
}


static quint64 platformMsec()
{
	timespec ts{};

#if defined(CLOCK_BOOTTIME)
	if (clock_gettime(CLOCK_BOOTTIME, &ts) == 0)
		return timespecToMs(ts);
#endif

	// Fallback
	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
		return timespecToMs(ts);

	return 0;
}
#endif


#ifdef Q_OS_WIN
#include <windows.h>

static QString currentExePath()
{
	wchar_t buf[32768];
	DWORD len = GetModuleFileNameW(nullptr, buf, (DWORD)(sizeof(buf)/sizeof(buf[0])));
	if (len == 0 || len >= (DWORD)(sizeof(buf)/sizeof(buf[0]))) return {};
	return QString::fromWCharArray(buf, len);
}


static quint64 platformMsec()
{
	return (quint64) GetTickCount64();
}

#endif





/**
 * @brief DesktopUtils::getExeHash
 * @param exePath
 * @param err
 * @param chunkSize
 * @return
 */

std::optional<QByteArray> DesktopUtils::getExeHash(const QString &path, QString *err, const qint64 &chunkSize)
{
	QString p = path;

	if (path.isEmpty()) {
		p = currentExePath();

		if (p.isEmpty())
			p = QCoreApplication::applicationFilePath();
	};


	QFile f(p);

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



/**
 * @brief DesktopUtils::msecSinceBoot
 * @return
 */

quint64 DesktopUtils::msecSinceBoot()
{
	return platformMsec();
}
