#include "myapp.h"
#include "qdebug.h"
#include "qurl.h"
#include <QDesktopServices>

#include "../Bacon2D-static/qml-box2d/box2dplugin.h"
#include "../Bacon2D-static/src/plugins.h"

MyApp::MyApp(QObject *parent)
	: QObject{parent}
{
	Box2DPlugin box2dplugin;
	box2dplugin.registerTypes("Box2D");

	Plugins plugin;
	plugin.registerTypes("Bacon2D");

	Q_INIT_RESOURCE(Bacon2D_static);
}


/**
 * @brief MyApp::testUrl
 */

void MyApp::testUrl()
{
	qDebug() << "TEST URL";
	QDesktopServices::openUrl(QUrl("http://piarista.hu"));
}
