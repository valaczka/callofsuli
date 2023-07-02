import QtQuick 2.15
import QtQuick.Controls 2.15
//import QtQuick.Window 2.15
import QtWebView 1.15

QPage {
	id: control

	property alias url: web.url

	appBar.backButtonVisible: true

	WebView {
		id: web
		width: parent.width
		height: parent.height /*+ (Qt.inputMethod && Qt.inputMethod.visible ?
									 Qt.inputMethod.keyboardRectangle.height > 0 ?
										 (Qt.inputMethod.keyboardRectangle.height / Screen.devicePixelRatio) : control.height*0.8
								 : 0)*/
		httpUserAgent: "Mozilla/5.0 Google"

		onUrlChanged: console.debug("Load WebView URL:", url)

		onLoadingChanged: {
			console.debug("WebView status:", loadRequest.status, loadRequest.errorString)
		}
	}
}
