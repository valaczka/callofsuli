import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QBasePage {
	id: control

	defaultTitle: qsTr("Test")

	QStackComponent {
		id: stackComponent
		anchors.fill: parent
		basePage: control

		initialItem: QSimpleContainer {
			id: panel

			title: "Test"
			icon: CosStyle.iconSetup



		}

	}



	function windowClose() {
		return false
	}

	function pageStackBack() {
		if (stackComponent.layoutBack())
			return true

		return false
	}
}
