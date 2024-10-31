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
#include <QApplication>
#include <QtQml>
#include "../modules/interfaces.h"


/**
 * @brief The Application class
 */

class Application
{
public:
	Application(QApplication *app);
	virtual ~Application();

	enum CommandLine {
		Normal,
		License,
		Editor,
		Map,
		Play,
		Demo,
		DevPage,
		Adjacency
	};

	int run();

	static const QVersionNumber version();
	static bool debug();

	static void initialize();

	QApplication *application() const;
	QQmlApplicationEngine *engine() const;
	Client *client() const;

	static Application *instance();

	void messageInfo(const QString &text, const QString &title = "") const;
	void messageWarning(const QString &text, const QString &title = "") const;
	void messageError(const QString &text, const QString &title = "") const;

	const QHash<QString, ModuleInterface *> &objectiveModules() const;
	const QHash<QString, ModuleInterface *> &storageModules() const;

	const CommandLine &commandLine() const;
	const QString &commandLineData() const;

	void selectUrl(const QUrl &url);

	static const QString &userAgent();
	static const QByteArray userAgentSign(const QByteArray &content);

protected:
	virtual bool loadMainQml();
	virtual bool loadResources();
	virtual Client *createClient() = 0;

	void registerQmlTypes();
	void loadFonts();
	void loadQaterial();
	void loadBox2D();
	void loadModules();

	friend class Client;
	friend class OnlineClient;

	CommandLine m_commandLine = Normal;
	QString m_commandLineData;
	QApplication *const m_application;
	std::unique_ptr<QQmlApplicationEngine> m_engine;
	std::unique_ptr<Client> m_client;
	static Application *m_instance;
	static const bool m_debug;

	QHash<QString, ModuleInterface*> m_objectiveModules;
	QHash<QString, ModuleInterface*> m_storageModules;

	static const QString m_userAgent;
};

#endif // APPLICATION_H
