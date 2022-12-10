/*
 * ---- Call of Suli ----
 *
 * ColorConsoleAppender.cpp
 *
 * Created on: 2021. 11. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ColorConsoleAppender
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

#include "ColorConsoleAppender.h"
#include <iostream>

ColorConsoleAppender::ColorConsoleAppender()
	: ConsoleAppender()
{

}

/**
 * @brief ColorConsoleAppender::append
 * @param timeStamp
 * @param logLevel
 * @param file
 * @param line
 * @param function
 * @param category
 * @param message
 */

void ColorConsoleAppender::append(const QDateTime &timeStamp, Logger::LogLevel logLevel, const char *file, int line, const char *function, const QString &category, const QString &message)
{
#ifdef Q_OS_LINUX
	if (category == "sql") {
		std::cerr << "\e[95m";
	} else switch (logLevel) {
		case Logger::Warning:
			std::cerr << "\e[91m";
			break;
		case Logger::Error:
		case Logger::Fatal:
			std::cerr << "\e[91m";
			break;
		case Logger::Info:
			std::cerr << "\e[96m";
			break;
		default:
			break;
	}
#endif

	std::cerr << qPrintable(formattedString(timeStamp, logLevel, file, line, function, category, message));

#ifdef Q_OS_LINUX
	std::cerr << "\e[39m";
#endif
}
