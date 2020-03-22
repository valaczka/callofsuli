/*
 * ---- Call of Suli ----
 *
 * ColorConsoleAppender.h
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

#ifndef COLORCONSOLEAPPENDER_H
#define COLORCONSOLEAPPENDER_H

#include <ConsoleAppender.h>

class CUTELOGGERSHARED_EXPORT ColorConsoleAppender : public ConsoleAppender
{
public:
	ColorConsoleAppender();

protected:
  virtual void append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line,
					  const char* function, const QString& category, const QString& message) override;
};

#endif // COLORCONSOLEAPPENDER_H
