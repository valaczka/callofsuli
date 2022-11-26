/*
 * ---- Call of Suli ----
 *
 * main.cpp
 *
 * Created on: 2020. 03. 22.
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


#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtDebug>
#include <QQuickView>
#include <QStandardPaths>
#include <QDir>
#include <QObject>
#include <QQmlContext>
#include <QtWebView/QtWebView>
#include <Logger.h>
#include <ColorConsoleAppender.h>
#include <sqlimage.h>
#include <fontimage.h>
#include "qrimage.h"

#ifndef QZXING_QML
#define QZXING_QML
#endif

#include <QZXing.h>

#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_WIN32)
#include <qsingleinstance.h>
#endif

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include "AndroidAppender.h"
#endif


#include "cosclient.h"
#include "modules/staticmodules.h"


int main(int argc, char *argv[])
{
	QtWebView::initialize();

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
	qputenv("QT_QUICK_CONTROLS_MATERIAL_VARIANT", "Normal");
#else
	qputenv("QT_QUICK_CONTROLS_MATERIAL_VARIANT", "Dense");
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif


	QGuiApplication app(argc, argv);

	Client::initialize();
/*
#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_WIN32)
	QSingleInstance instance;

	if (!instance.process()) {
		qInfo() << QObject::tr("Már fut az alkalmazás egy példánya");
		return 0;
	}
#endif
*/
	QString cmdLine = Client::commandLineParse(app);

	if (cmdLine == "terrain")
		return 0;

#ifdef Q_OS_WIN32
	FILE *streamO = NULL;
	FILE *streamE = NULL;

	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
		streamO = freopen("CONOUT$", "w", stdout);
		streamE = freopen("CONOUT$", "w", stderr);
	}
#endif

	if (cmdLine == "license")
	{
		QTextStream out(stdout);
		out << Client::fileContent(":/common/license.txt") << Qt::endl;

#ifdef Q_OS_WIN32
		if (streamO != NULL)
			fclose(streamO);
		if (streamE != NULL)
			fclose(streamE);
#endif

		return 0;
	}




	ColorConsoleAppender* consoleAppender = new ColorConsoleAppender;
#ifdef Q_OS_ANDROID
	AndroidAppender* androidAppender = new AndroidAppender;
#endif


#ifndef QT_NO_DEBUG
	consoleAppender->setFormat("%{time}{hh:mm:ss} [%{TypeOne}] %{message} <%{function}>\n");
#ifdef Q_OS_ANDROID
	androidAppender->setFormat("%{time}{hh:mm:ss} [%{TypeOne}] %{message} <%{function}>\n");
#endif
#else
	consoleAppender->setFormat("%{time}{hh:mm:ss} [%{TypeOne}] %{message}\n");
#ifdef Q_OS_ANDROID
	androidAppender->setFormat("%{time}{hh:mm:ss} [%{TypeOne}] %{message}\n");
#endif
#endif

	cuteLogger->registerAppender(consoleAppender);
#ifdef Q_OS_ANDROID
	cuteLogger->registerAppender(androidAppender);
#endif





	QZXing::registerQMLTypes();

	Client::loadModules();
	Client::standardPathCreate();
	Client::registerTypes();
	Client::registerResources();
	Client::reloadGameResources();

	Client* client = Client::clientInstance();

	QQmlApplicationEngine *engine = new QQmlApplicationEngine();
	QQmlContext *context = engine->rootContext();
	context->setContextProperty("cosClient", client);
	client->setRootContext(context);

	engine->addImageProvider("font", new FontImage());
	engine->addImageProvider("qrcode", new QrImage());

#ifdef QT_NO_DEBUG
	context->setContextProperty("DEBUG_MODE", QVariant(false));
#else
	context->setContextProperty("DEBUG_MODE", QVariant(true));
#endif

	const QUrl url(QStringLiteral("qrc:/main.qml"));
	QObject::connect(engine, &QQmlApplicationEngine::objectCreated,
					 &app, [url](QObject *obj, const QUrl &objUrl) {
		if (!obj && url == objUrl)
			QCoreApplication::exit(-1);

#ifdef Q_OS_ANDROID
		QtAndroid::hideSplashScreen();
#endif
	}, Qt::QueuedConnection);
	engine->load(url);
/*
#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_WIN32)
	instance.setNotifyWindow(QGuiApplication::topLevelWindows().at(0));
	client->setSingleInstance(&instance);
#endif
*/
	int ret = app.exec();

	delete engine;

	qInfo() << "Exit with code" << ret;

#ifdef Q_OS_WIN32
	if (streamO != NULL)
		fclose(streamO);
	if (streamE != NULL)
		fclose(streamE);
#endif


	return ret;
}



