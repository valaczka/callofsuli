import QtQuick 2.12
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: page

	header: QToolBar {
		id: toolbar

		backButton.visible: true
		backButton.onClicked: mainStack.back()
	}

	Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/img/villa.png"
	}

	QButton {
		id: button1
		label: "SEND DATA"

		onClicked: {
			cosClient.sendData()
		}
	}

	QButton {
		id: button2
		label: "CLOSE CONNECTION"

		anchors.top: button1.bottom

		onClicked: {
			cosClient.closeConnection()
		}
	}

	/* CONTENT */
	BusyIndicator {
		id: busy
		anchors.centerIn: parent
		running: false
	}
	/* CONTENT */

	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.title = qsTr("Page")
			/* LOAD */
	}

	StackView.onDeactivated: {
			/* UNLOAD */
	}


	function stackBack() {
		if (mainStack.depth > page.StackView.index+1) {
			if (!mainStack.get(page.StackView.index+1).stackBack()) {
				if (mainStack.depth > page.StackView.index+1) {
					mainStack.pop(page)
				}
			}
			return true
		}

		/* BACK */

		return false
	}
}
