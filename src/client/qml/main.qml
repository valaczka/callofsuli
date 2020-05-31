import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.3
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


ApplicationWindow {
	id: mainWindow
	visible: true
	width: 640
	height: 480

	title: "Call of Suli"

	minimumHeight: 600
	minimumWidth: 600

	FontLoader { source: "qrc:/font/Books.ttf" }
	FontLoader { source: "qrc:/font/Material.ttf" }
	FontLoader { source: "qrc:/font/School.ttf" }
	FontLoader { source: "qrc:/font/Academic.ttf" }
	FontLoader { source: "qrc:/font/AcademicI.ttf" }

	FontLoader { source: "qrc:/font/rajdhani-bold.ttf" }
	FontLoader { source: "qrc:/font/rajdhani-light.ttf" }
	FontLoader { source: "qrc:/font/rajdhani-regular.ttf" }
	FontLoader { source: "qrc:/font/rajdhani-medium.ttf" }
	FontLoader { source: "qrc:/font/rajdhani-semibold.ttf" }

	FontLoader { source: "qrc:/font/SpecialElite.ttf" }
	FontLoader { source: "qrc:/font/HVD_Peace.ttf" }



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

		readonly property int animDuration: 175

		focus: true

		Keys.onBackPressed: back()

		initialItem: QCosImage {
			maxWidth: Math.min(mainWindow.width*0.7, 800)
			glowRadius: 6
		}

		pushEnter: Transition {
			SequentialAnimation {
				PropertyAction {
					property: "opacity"
					value: 0.0
				}

				PauseAnimation {
					duration: mainStack.animDuration
				}

				PropertyAnimation {
					property: "opacity"
					from: 0.0
					to: 1.0
					duration: mainStack.animDuration
				}

				PropertyAnimation {
					target: blur
					property: "radius"
					from: 100
					to: 0
					duration: mainStack.animDuration
				}
			}


		}

		pushExit: Transition {
			SequentialAnimation {
				PropertyAnimation {
					target: blur
					property: "radius"
					from: 0
					to: 100
					duration: mainStack.animDuration
				}

				PropertyAnimation {
					property: "opacity"
					from: 1.0
					to: 0.0
					duration: mainStack.animDuration
				}
				PauseAnimation {
					duration: mainStack.animDuration
				}
			}
		}


		popEnter: Transition {
			SequentialAnimation {
				PropertyAction {
					property: "opacity"
					value: 0.0
				}

				PauseAnimation {
					duration: mainStack.animDuration
				}

				PropertyAnimation {
					property: "opacity"
					from: 0.0
					to: 1.0
					duration: mainStack.animDuration
				}

				PropertyAnimation {
					target: blur
					property: "radius"
					from: 100
					to: 0
					duration: mainStack.animDuration
				}
			}


		}

		popExit: Transition {
			SequentialAnimation {
				PropertyAnimation {
					target: blur
					property: "radius"
					from: 0
					to: 100
					duration: mainStack.animDuration
				}

				PropertyAnimation {
					property: "opacity"
					from: 1.0
					to: 0.0
					duration: mainStack.animDuration
				}
				PauseAnimation {
					duration: mainStack.animDuration
				}
			}
		}

		function back() {
			if (depth > 1)
				get(1).stackBack()
			else
				mainWindow.close()
		}

		function loginRequest() {
			if ((cosClient.connectionState === Client.Connected || cosClient.connectionState === Client.Reconnected)
					&& (cosClient.userRoles & Client.RoleGuest)) {
				JS.createPage("Login", {})
			}
		}

	}

	FastBlur {
		id: blur
		anchors.fill: mainStack

		radius: 0
		visible: mainStack.busy

		source: mainStack
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
				close.accepted = false
				return
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
