/*
 * ---- Call of Suli ----
 *
 * main.cpp
 *
 * Created on: 2022. 12. 09.
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

#include <QtGlobal>

#ifdef Q_OS_WASM
#include "onlineapplication.h"
#elif defined (Q_OS_ANDROID) || defined (Q_OS_IOS)
#include "mobileapplication.h"
#include "mobileutils.h"
#else
#include "desktopapplication.h"
#endif

int main(int argc, char *argv[])
{
	Application::initialize();
	QApplication qapp(argc, argv);

#ifdef Q_OS_WASM
	OnlineApplication app(&qapp);
	return app.run();
#else

#if defined (Q_OS_ANDROID) || defined (Q_OS_IOS)
	MobileApplication app (&qapp);
	app.initialize();
	return app.run();
#else

	DesktopApplication app(&qapp);

	app.commandLineParse();
	app.initialize();

	if (!app.performCommandLine())
		return 0;

	return app.runSingleInstance();

#endif

#endif

}
