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

	Column {
		QButton {
			id: button1
			label: "SEND JSON DATA"

			onClicked: {
				cosClient.socketSendJson({"test": "szia", "vissza": 3})
			}
		}


		QButton {
			label: "SEND LOGIN USERNAME"

			onClicked: {
				cosClient.socketSendJson({"auth":{"username": "admin"}, "test": "szia", "vissza": 3})
			}
		}


		QButton {
			label: "SEND LOGIN"

			onClicked: {
				cosClient.login("admin", "", "admin")
			}
		}

		QButton {
			label: "GET USER INFO"
			onClicked: {
				cosClient.socketSendJson({"class": "userinfo", "func": "getUser"})
			}
		}

		QButton {
			label: "GET USER ALL"
			onClicked: {
				cosClient.socketSendJson({"class": "userinfo", "func": "getAllUser"})
			}
		}

		QButton {
			id: button2
			label: "CLOSE CONNECTION"

			onClicked: {
				cosClient.closeConnection()
			}
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

		cosClient.closeConnection()

		return true
	}
}
