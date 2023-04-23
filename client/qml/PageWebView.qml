import QtQuick 2.15
import QtQuick.Controls 2.15
import QtWebView 1.15
import QtQuick.Window 2.12
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

QPage {
	id: control

	property alias url: web.url

	appBar.backButtonVisible: true

	WebView {
		id: web
		width: parent.width
		height: parent.height //+ (Qt.inputMethod && Qt.inputMethod.visible ? (Qt.inputMethod.keyboardRectangle.height / Screen.devicePixelRatio) : 0)
		httpUserAgent: "Mozilla/5.0 Google"
	}
}
