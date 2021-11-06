import QtQuick 2.15
import QtQuick.Controls 2.15
import QtWebView 1.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QSimpleContainer {
	id: control

	title: qsTr("Bejelentkezés")
	icon: CosStyle.iconLogin

	property url url;

	WebView {
		id: web
		anchors.fill: parent
		httpUserAgent: "Mozilla/5.0 Google"
	}

	onPopulated: web.url = url
}

