import QtQuick 2.12
import QtQuick.Controls 2.12
import QtWebEngine 1.10
import QtQuick.Window 2.12
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


QPage {
	id: control

	property alias url: web.url

	appBar.backButtonVisible: true

	WebEngineView {
		id: web
		width: parent.width
		height: parent.height + (Qt.inputMethod && Qt.inputMethod.visible ? (Qt.inputMethod.keyboardRectangle.height / Screen.devicePixelRatio) : 0)

		onCertificateError: if (error.error == -202)			// CertificateAuthorityInvalid
								error.ignoreCertificateError()
	}
}
