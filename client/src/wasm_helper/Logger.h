/*
 * ---- Call of Suli ----
 *
 * Logger.h
 *
 * Created on: 2023. 01. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#ifndef LOGGER_H
#define LOGGER_H

#include <QDebug>

#define LOG_CTRACE(x)	qDebug() << x
#define LOG_CDEBUG(x)	qDebug() << x
#define LOG_CINFO(x)	qInfo() << x
#define LOG_CWARNING(x)	qWarning() << x
#define LOG_CERROR(x)	qCritical() << x
#define LOG_CFATAL(x)	qCritical() << x

#endif // LOGGER_H
