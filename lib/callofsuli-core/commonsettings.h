/*
 * ---- Call of Suli ----
 *
 * commonsettings.h
 *
 * Created on: 2024. 07. 19.
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

#ifndef COMMONSETTINGS_H
#define COMMONSETTINGS_H

#include <QMetaObject>

namespace CallOfSuli {

Q_NAMESPACE

enum NotificationType {
	NotificationInvalid = 0,
	NotificationStarted,
	NotificationHour24,
	NotificationHour48,
	NotificationWeek1
};

Q_ENUM_NS(NotificationType);

};

#endif // COMMONSETTINGS_H
