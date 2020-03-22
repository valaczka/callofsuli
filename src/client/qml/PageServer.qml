import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.3
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: page

	property alias filePath: sqlConnection.filePath

	signal openFailed()

	SqlConnection {
		id: sqlConnection

		client: cosClient

		onSqlDbOpenFailed: {
			page.openFailed()
			var d = JS.dialogMessageError(text, info, details)
			d.closedAndDestroyed.connect(function() {
				mainStack.back()
			})
		}

		onSqlDbOpenSuccess: {
			toolbar.title = map.name
			socket.url = url
			socketPing()
		}

		onSocketTryConnect: console.debug("try connect")
		onSocketConnected: {
			console.debug("connected")
			cosClient.setSetting("autoConnect", filePath)
		}
		onSocketDisconnected: console.debug("disconnected")
		onSocketPingSent: console.debug("ping")

	}

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



	BusyIndicator {
		id: busy
		anchors.centerIn: parent
		running: false
	}




	StackView.onRemoved: {
		destroy()
	}

	StackView.onActivated: {
		sqlConnection.sqlDbOpen()
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
