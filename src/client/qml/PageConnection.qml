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
		Label {
			text: "NÉV:"+cosClient.userFirstName+" "+cosClient.userLastName
		}

		QButton {
			id: button1
			label: "Teacher MAPS"

			onClicked: {
				JS.createPage("TeacherMaps", {}, page)
			}
		}


		QButton {
			label: "SEND LOGIN USERNAME"

			onClicked: {
				cosClient.login("admin", "", "dadmin")
			}
		}


		QButton {
			label: "SEND LOGIN"

			onClicked: {
				cosClient.login("admin", "", "admin")
			}
		}


		QButton {
			label: "logout"
			onClicked: cosClient.logout()
		}

		QButton {
			label: "GET USER INFO"
			onClicked: {
				cosClient.socketSend({"class": "userinfo", "func": "getUser"})
			}
		}

		QButton {
			label: "GET USER ALL"
			onClicked: {
				cosClient.socketSend({"class": "userinfo", "func": "getAllUser"})
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

	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.title = cosClient.serverName
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
