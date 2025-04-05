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

#include "ColorConsoleAppender.h"
#include "mobileapplication.h"
#include "qsingleinstance.h"

class DesktopApplication : public MobileApplication
{
public:
	DesktopApplication(QApplication *app);
	virtual ~DesktopApplication();

	virtual void initialize() override;

	void commandLineParse();
	bool performCommandLine();

	void performInstanceArguments(const QStringList &arguments);

	int runSingleInstance();

	bool hasLocalSocket() const;
	void writeToSocket(const QCborValue &cbor);

protected:
	virtual Client *createClient() override;

private:
	void onNewSocketConnection();

	QStringList m_arguments;
	ColorConsoleAppender *m_appender = nullptr;
	QSingleInstance m_singleInstance;
	QString m_localServerName;
	QLocalServer m_localServer;
	QList<QLocalSocket*> m_localSockets;
};

#endif // DESKTOPAPPLICATION_H
