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
#include <Logger.h>
#include <ColorConsoleAppender.h>
#include <sqlimage.h>
#include <fontimage.h>
#include "qrimage.h"

#include "../Bacon2D-static/qml-box2d/box2dplugin.h"
#include "../Bacon2D-static/src/plugins.h"

#define QZXING_QML
#include <QZXing.h>

#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_WIN32)
#include <qsingleinstance.h>
#endif

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include "AndroidAppender.h"
#endif

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(Q_OS_LINUX)
#include <QtWebView/QtWebView>
#endif


#include "cosclient.h"
#include "gamemap.h"
#include "cosdb.h"

#include "modules/staticmodules.h"


int main(int argc, char *argv[])
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(Q_OS_LINUX)
	QtWebView::initialize();
#endif

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
	qputenv("QT_QUICK_CONTROLS_MATERIAL_VARIANT", "Normal");
#else
	qputenv("QT_QUICK_CONTROLS_MATERIAL_VARIANT", "Dense");
#endif


	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QGuiApplication app(argc, argv);

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


#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_WIN32)
	QSingleInstance instance;

	if (!instance.process()) {
		qInfo() << QObject::tr("Már fut az alkalmazás egy példánya");
		return 0;
	}
#endif

	srand(time(NULL));

	Box2DPlugin box2dplugin;
	box2dplugin.registerTypes("Box2D");

	Plugins plugin;
	plugin.registerTypes("Bacon2D");

	Q_INIT_RESOURCE(Bacon2D_static);


	QZXing::registerQMLTypes();

	Client::initialize();
	Client::loadModules();

	Client* client = Client::clientInstance();

	if (!client->commandLineParse(app))
		return 0;

	Client::standardPathCreate();
	Client::registerTypes();
	Client::registerResources();
	Client::reloadGameResources();

	QQmlApplicationEngine engine;
	QQmlContext *context = engine.rootContext();
	context->setContextProperty("cosClient", client);
	client->setRootContext(context);

	engine.addImageProvider("font", new FontImage());
	engine.addImageProvider("qrcode", new QrImage());

#ifdef QT_NO_DEBUG
	context->setContextProperty("DEBUG_MODE", QVariant(false));
#else
	context->setContextProperty("DEBUG_MODE", QVariant(true));
#endif

	const QUrl url(QStringLiteral("qrc:/main.qml"));
	QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
					 &app, [url](QObject *obj, const QUrl &objUrl) {
		if (!obj && url == objUrl)
			QCoreApplication::exit(-1);

#ifdef Q_OS_ANDROID
		QtAndroid::hideSplashScreen();
#endif
	}, Qt::QueuedConnection);
	engine.load(url);

#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_WIN32)
	instance.setNotifyWindow(QGuiApplication::topLevelWindows().at(0));
	client->setSingleInstance(&instance);
#endif

	return app.exec();
}



