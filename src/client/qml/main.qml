import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.3
import COS.Client 1.0
import "."
import "JScript.js" as JS


ApplicationWindow {
	id: mainWindow
	visible: true
	width: 640
	height: 480

	title: "Call of Suli"

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


	StackView {
		id: mainStack
		objectName: "mainStack"
		anchors.fill: parent

		focus: true

		Keys.onEscapePressed: {
			if (depth > 2)
				get(1).stackBack()
		}

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
					duration: 250
				}

				PropertyAnimation {
					property: "opacity"
					from: 0.0
					to: 1.0
					duration: 250
				}

				PropertyAnimation {
					target: blur
					property: "radius"
					from: 100
					to: 0
					duration: 250
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
					duration: 250
				}

				PropertyAnimation {
					property: "opacity"
					from: 1.0
					to: 0.0
					duration: 250
				}
				PauseAnimation {
					duration: 250
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
					duration: 250
				}

				PropertyAnimation {
					property: "opacity"
					from: 0.0
					to: 1.0
					duration: 250
				}

				PropertyAnimation {
					target: blur
					property: "radius"
					from: 100
					to: 0
					duration: 250
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
					duration: 250
				}

				PropertyAnimation {
					property: "opacity"
					from: 1.0
					to: 0.0
					duration: 250
				}
				PauseAnimation {
					duration: 250
				}
			}
		}

		function back() {
			if (depth > 2)
				get(1).stackBack()
			else
				mainWindow.close()
		}

	}

	FastBlur {
		id: blur
		anchors.fill: mainStack

		radius: 0
		visible: mainStack.busy

		source: mainStack
	}


	Client {
		id: cosClient
	}


	onClosing: {
		if (mainStack.depth > 2) {
			if (!mainStack.get(mainStack.depth-1).windowClose()) {
				close.accepted = false
				return
			}
		}

		cosClient.windowSaveGeometry(mainWindow)
	}


	Component.onCompleted: {
		cosClient.windowSetIcon(mainWindow)
		cosClient.windowRestoreGeometry(mainWindow)
		cosClient.messageSent.connect(JS.dialogMessage)

		JS.createPage("Start", {}, mainWindow)
	}
}
