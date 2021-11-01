import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import QtQuick.Controls.Material 2.3
import QtMultimedia 5.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


ApplicationWindow {
	id: mainWindow
	visible: true
	width: 640
	height: 480

	title: (cosClient.connectionState != Client.Standby && cosClient.serverName.length ? cosClient.serverName+" - " : "") + "Call of Suli"

	minimumWidth: 640
	minimumHeight: 480

	FontLoader { source: "qrc:/internal/font/ariblk.ttf" }
	FontLoader { source: "qrc:/internal/font/Books.ttf" }
	FontLoader { source: "qrc:/internal/font/Material.ttf" }
	FontLoader { source: "qrc:/internal/font/School.ttf" }
	FontLoader { source: "qrc:/internal/font/Academic.ttf" }
	FontLoader { source: "qrc:/internal/font/AcademicI.ttf" }

	FontLoader { source: "qrc:/internal/font/rajdhani-bold.ttf" }
	FontLoader { source: "qrc:/internal/font/rajdhani-light.ttf" }
	FontLoader { source: "qrc:/internal/font/rajdhani-regular.ttf" }
	FontLoader { source: "qrc:/internal/font/rajdhani-medium.ttf" }
	FontLoader { source: "qrc:/internal/font/rajdhani-semibold.ttf" }

	FontLoader { source: "qrc:/internal/font/SpecialElite.ttf" }
	FontLoader { source: "qrc:/internal/font/HVD_Peace.ttf" }
	FontLoader { source: "qrc:/internal/font/RenegadeMaster.ttf" }



	background: Rectangle {
		color: "black"
	}

	Action {
		id: actionBack
		shortcut: "Esc"
		onTriggered: if (mainStack.depth > 1)
						 mainStack.get(1).stackBack()
	}


	StackView {
		id: mainStack
		objectName: "mainStack"
		anchors.fill: parent

		focus: true

		Keys.onReleased: {
			if (event.key === Qt.Key_Back) {
				back()
				event.accepted=true;
			}
		}


		initialItem: QCosImage {
			maxWidth: Math.min(mainWindow.width*0.7, 800)
			glowRadius: 6
		}


		Transition {
			id: transitionEnter

			PropertyAnimation {
				property: "opacity"
				from: 0.0
				to: 1.0
			}


		}



		Transition {
			id: transitionExit

			PropertyAnimation {
				property: "opacity"
				from: 1.0
				to: 0.0
			}
		}


		pushEnter: transitionEnter
		pushExit: transitionExit
		popEnter: transitionEnter
		popExit: transitionExit

		function back() {
			if (depth > 1) {
				if (!get(1).stackBack())
					mainWindow.close()
			} else {
				mainWindow.close()
			}
		}

		function loginRequest() {
			if ((cosClient.connectionState === Client.Connected || cosClient.connectionState === Client.Reconnected)
					&& (cosClient.userRoles & Client.RoleGuest)) {
				JS.createPage("Login", {})
			}
		}

	}


	MouseArea {
		anchors.fill: parent
		acceptedButtons: Qt.NoButton
		onWheel: {
			if (wheel.modifiers & Qt.ControlModifier) {
				var i = wheel.angleDelta.y/120
				if (i>0)
					fontPlus.trigger()
				else if (i<0)
					fontMinus.trigger()

				wheel.accepted = true
			} else {
				wheel.accepted = false
			}
		}
	}


	Action {
		id: fontPlus
		shortcut: "Ctrl++"
		onTriggered: if (CosStyle.pixelSize < 36)
						 CosStyle.pixelSize++
	}

	Action {
		id: fontMinus
		shortcut: "Ctrl+-"
		onTriggered: if (CosStyle.pixelSize > 10)
						 CosStyle.pixelSize--
	}

	Action {
		id: fontNormal
		shortcut: "Ctrl+0"
		onTriggered: CosStyle.pixelSize = 18
	}


	onClosing: {
		if (mainStack.depth > 2) {
			if (!mainStack.get(mainStack.depth-1).windowClose()) {
				close.accepted = true
				Qt.quit()
			} else {
				close.accepted = false
			}

		}

		cosClient.windowSaveGeometry(mainWindow, CosStyle.pixelSize)
	}


	Component.onCompleted: {
		cosClient.windowSetIcon(mainWindow)
		var fs = cosClient.windowRestoreGeometry(mainWindow)
		if (fs > 0)
			CosStyle.pixelSize = fs
		cosClient.messageSent.connect(JS.dialogMessage)
		JS.createPage("Start", {})
	}
}
