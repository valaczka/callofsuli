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
#include <sqlimage.h>
#include <fontimage.h>


#include "../3rdparty/Bacon2D-static/qml-box2d/box2dplugin.h"
#include "../3rdparty/Bacon2D-static/src/plugins.h"

#include "cosclient.h"

class CustomClass {

public:
	CustomClass(int i) : m_int(i) {
		b = QString("lksjféaá dléákfáafdka ").toLatin1();
	}

	friend QDataStream &operator<<(QDataStream &out, const CustomClass &cc) {
		out << cc.m_int;
		out << "szia";
		return out;
	}

	friend QDebug operator<<(QDebug out, const CustomClass &cc) {
		out << (quint16) 0x434F53;
		out << cc.m_int;
		out << "szia";
		out << cc.b;
		return out;
	}

private:
	int m_int;
	QByteArray b;
};

int main(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
	qputenv("QT_QUICK_CONTROLS_MATERIAL_VARIANT", "Normal");
#else
	qputenv("QT_QUICK_CONTROLS_MATERIAL_VARIANT", "Dense");
#endif

	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QGuiApplication app(argc, argv);

	srand(time(NULL));

	Box2DPlugin box2dplugin;
	box2dplugin.registerTypes("Box2D");

	Plugins plugin;
	plugin.registerTypes("Bacon2D");

	Client client;

	client.initialize();
	client.standardPathCreate();
	client.registerTypes();
	client.registerResources();

	/*CustomClass c(5544);

	qDebug() << c;

	exit(1); */

	QQmlApplicationEngine engine;
	QQmlContext *context = engine.rootContext();
	context->setContextProperty("cosClient", &client);

	engine.addImageProvider("sql", new SqlImage(&client));
	engine.addImageProvider("font", new FontImage());

	const QUrl url(QStringLiteral("qrc:/main.qml"));
	QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
					 &app, [url](QObject *obj, const QUrl &objUrl) {
		if (!obj && url == objUrl)
			QCoreApplication::exit(-1);
	}, Qt::QueuedConnection);
	engine.load(url);

	return app.exec();
}
