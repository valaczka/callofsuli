/*
 * ---- Call of Suli ----
 *
 * main.cpp
 *
 * Created on: 2022. 11. 26.
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

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlFileSelector>
#include <QSettings>
#include <QQuickStyle>
#include <QIcon>
#include <QtNetwork>
#include <QtNetworkAuth/QOAuthHttpServerReplyHandler>

#include "myapp.h"

class GLobalStatic
{
public:
	GLobalStatic() {
		qDebug() << "GLobalStatic constructor";
	}
	~GLobalStatic() {
		qDebug() << "GLobalStatic destructor";
	}
};
static GLobalStatic globalStatic;



int main(int argc, char **argv)
{
	QGuiApplication::setApplicationName("Gallery");
	QGuiApplication::setOrganizationName("QtProject");
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QGuiApplication app(argc, argv);


	QQmlApplicationEngine engine = QQmlApplicationEngine("qrc:/main.qml");
	QQmlContext *context = engine.rootContext();
	//context->setContextProperty("cosClient", client);
	//client->setRootContext(context);

	//engine->addImageProvider("font", new FontImage());
	//engine->addImageProvider("qrcode", new QrImage());

#ifdef QT_NO_DEBUG
	context->setContextProperty("DEBUG_MODE", QVariant(false));
#else
	context->setContextProperty("DEBUG_MODE", QVariant(true));
#endif

	uchar *internalRes = NULL;

	/*QQmlFileSelector *selector = QQmlFileSelector::get(&engine);

	qDebug() << selector->selector()->allSelectors();*/

	MyApp *myApp = new MyApp();

	context->setContextProperty("myApp", myApp);

	QUrl url2("internal.cres");
	QNetworkRequest request(url2);
	QNetworkAccessManager manager;
	QObject::connect(&manager, &QNetworkAccessManager::finished, [internalRes, &engine](QNetworkReply *reply) mutable {
		qDebug() << "download finished";
		if (reply->error()) {
			qDebug() << "error" << reply->errorString();
		} else {
			QByteArray payload = reply->readAll();
			qDebug() << "size" << payload.size();

			QByteArray hash = QCryptographicHash::hash(payload, QCryptographicHash::Sha256);
			qDebug() << "sha256" << hash.toHex();

			internalRes = new uchar[payload.size()];
			std::memcpy(internalRes, payload.constData(), payload.size());

			qDebug() << "COPIED";

			QResource::registerResource(internalRes);

			qDebug() << "REGISTERED";

			QFile f(":internal/img/callofsuli.svg");
			qDebug() << f.exists();

			engine.rootObjects().at(0)->setProperty("cresLoaded", true);
		}
	});

	qDebug() << "download begin";
	manager.get(request);



	//QOAuthHttpServerReplyHandler handler;

	//qDebug() << "Listen on port" << handler.port();

	int ret = app.exec();

	qInfo() << "Exit with code" << ret;
}

