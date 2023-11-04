/*
 * ---- Call of Suli ----
 *
 * mobileapplication.h
 *
 * Created on: 2023. 01. 13.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MobileApplication
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

#ifndef MOBILEAPPLICATION_H
#define MOBILEAPPLICATION_H

#include "application.h"

class MobileApplication : public Application
{
public:
	MobileApplication(QApplication *app);
	virtual ~MobileApplication();

	virtual void initialize();
	virtual void createStandardPath();

protected:
	virtual Client *createClient();

};

#endif // MOBILEAPPLICATION_H
