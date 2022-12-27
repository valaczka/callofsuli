/*
 * ---- Call of Suli ----
 *
 * %{Cpp:License:FileName}
 *
 * Created on: 2020. 03. 21.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  Call of Suli is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <signal.h>
#include <QCoreApplication>
#include <QtDebug>
#include <Logger.h>
#include <ColorConsoleAppender.h>

#include "server.h"


/**
 * @brief signalHandler
 * @param sig
 */

void signalHandler(int sig)
{
	qInfo("Signal %d caught, exit...", sig);

	QCoreApplication::instance()->quit();
}




/**
 * @brief main
 * @param argc
 * @param argv
 * @return
 */

int main(int argc, char *argv[])
{
	signal(SIGABRT, &signalHandler);
	signal(SIGTERM, &signalHandler);
	signal(SIGINT, &signalHandler);

	QCoreApplication app(argc, argv);

	ColorConsoleAppender* consoleAppender = new ColorConsoleAppender;
	consoleAppender->setFormat("%{time}{hh:mm:ss} [%{TypeOne}] %{message}\n");
	cuteLogger->registerAppender(consoleAppender);
	cuteLogger->logToGlobalInstance("sql", true);

	Server s;

	if (!s.commandLineParse(app))
		return 1;

	if (s.start())
		app.exec();

	s.stop();

	qDebug() << "Finished";
	return 0;
}