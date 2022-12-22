/*
 * ---- Call of Suli ----
 *
 * desktopapplication.h
 *
 * Created on: 2022. 12. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * DesktopApplication
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

#ifndef DESKTOPAPPLICATION_H
#define DESKTOPAPPLICATION_H


#ifdef Q_OS_ANDROID
#include <AndroidAppender.h>
#else
#include <ColorConsoleAppender.h>
#endif


#include "application.h"

class DesktopApplication : public Application
{
	enum CommandLine {
		Normal,
		License,
		Editor,
		Map,
		Play
	};

public:
	DesktopApplication(int &argc, char **argv);
	virtual ~DesktopApplication();

	void commandLineParse();
	void initialize();
	void shutdown();
	bool performCommandLine();

protected:
	virtual Client *createClient();

private:
	CommandLine m_commandLine = Normal;
	QString m_loadMap;
	QStringList m_arguments;

#ifdef Q_OS_WIN32
	FILE *m_streamO = NULL;
	FILE *m_streamE = NULL;
#endif
};

#endif // DESKTOPAPPLICATION_H
