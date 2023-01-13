import QtQuick 2.12
import QtQuick.Controls 2.12
import QtWebView 1.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


QPage {
	id: control

	//title: "Call of Suli"
	property url url;

	appBar.backButtonVisible: true


	WebView {
		id: web
		width: parent.width
		height: parent.height + (Qt.inputMethod && Qt.inputMethod.visible ? (Qt.inputMethod.keyboardRectangle.height / Screen.devicePixelRatio) : 0)
		httpUserAgent: "Mozilla/5.0 Google"

		Component.onCompleted: {
			url = control.url
		}
	}

}
