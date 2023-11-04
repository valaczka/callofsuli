/*
 * ---- Call of Suli ----
 *
 * mobileutils.h
 *
 * Created on: 2023. 07. 05.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MobileUtils
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

#ifndef MOBILEUTILS_H
#define MOBILEUTILS_H

#include "qurl.h"
#include <string>

class MobileUtils
{

public:
	MobileUtils();

	static MobileUtils* instance() {
		if (!m_instance)
			m_instance = new MobileUtils();
		return m_instance;
	}

	static void vibrate(const int &milliseconds = 400);

	static void openUrl(const std::string &url);
	static QString checkPendingIntents();

private:
	static MobileUtils *m_instance;

	static QString m_pendingArg;
};

#endif // MOBILEUTILS_H
