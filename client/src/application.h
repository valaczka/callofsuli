/*
 * ---- Call of Suli ----
 *
 * baseapplication.h
 *
 * Created on: 2022. 12. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * BaseApplication
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

#ifndef APPLICATION_H
#define APPLICATION_H

#include "client.h"
#include <QGuiApplication>
#include <QtQml>

class Application
{
public:
	Application(int &argc, char **argv);
	virtual ~Application();

	int run();

	static int versionMajor();
	static int versionMinor();
	static int versionBuild();
	const char *version() const;

	QGuiApplication *application() const;
	QQmlApplicationEngine *engine() const;
	Client *client() const;

protected:
	virtual bool loadMainQml();
	virtual bool loadResources();
	virtual Client *createClient();

	void registerQmlTypes();
	void loadFonts();
	void loadQaterial();
	void loadBox2D();

	static const int m_versionMajor;
	static const int m_versionMinor;
	static const int m_versionBuild;
	static const char* m_version;

	QGuiApplication *m_application = nullptr;
	QQmlApplicationEngine *m_engine = nullptr;
	Client *m_client = nullptr;
};

Q_DECLARE_LOGGING_CATEGORY(lcApp)

#endif // APPLICATION_H
