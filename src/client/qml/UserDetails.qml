import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Popup {
	id: popupItem

	closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
	modal: false
	focus: false

	x: parent.width-width
	y: 0

	property int openedWidth: Math.min(500, parent.width)
	property int openedHeight: Math.min(400, parent.height)
	readonly property bool running: transEnter.running || transExit.running

	background: Rectangle {
		id: bgRect
		color: "grey"
		opacity: 0.8
	}

	Column {
		anchors.centerIn: parent
		Label {
			text: (cosClient.userRoles & Client.RoleGuest) ? qsTr("Vendég") :
															 cosClient.userFirstName+" "+cosClient.userLastName
		}

		Label {
			text: cosClient.userRankName
			visible: !(cosClient.userRoles & Client.RoleGuest)
		}

		Label {
			text: cosClient.userXP+" XP"
			visible: !(cosClient.userRoles & Client.RoleGuest)
		}

		QButton {
			text: (cosClient.userRoles & Client.RoleGuest) ? qsTr("Bejelentkezés") : qsTr("Kijelentkezés")

			onClicked: {
				popupItem.close()
				if (cosClient.userRoles & Client.RoleGuest)
					mainStack.loginRequest()
				else
					cosClient.logout()
			}
		}

		QButton {
			text: qsTr("Regisztráció")
			enabled: cosClient.registrationEnabled
			visible: cosClient.registrationEnabled

			onClicked: {
				popupItem.close()
				cosClient.registrationRequest()
			}
		}
	}

	enter: Transition {
		id: transEnter
		SequentialAnimation {
			ParallelAnimation {
				NumberAnimation {
					property: "opacity"
					duration: 75
					from: 0.0
					to: 1.0
					easing.type: Easing.InOutQuad
				}
				NumberAnimation {
					property: "width"
					duration: 150
					from: 0.0
					to: openedWidth
					easing.type: Easing.InOutQuad
				}
				NumberAnimation {
					property: "height"
					duration: 150
					from: 0.0
					to: openedHeight
					easing.type: Easing.InOutQuad
				}
			}


			NumberAnimation {
				target: contentItem
				property: "opacity"
				duration: 150
				from: 0.0
				to: 1.0
				easing.type: Easing.InOutQuad
			}
		}
	}

	exit: Transition {
		id: transExit
		SequentialAnimation {
			NumberAnimation {
				target: contentItem
				property: "opacity"
				duration: 120
				to: 0.0
				easing.type: Easing.InOutQuad
			}

			NumberAnimation {
				properties: "opacity,width,height"
				duration: 150
				to: 0.0
				easing.type: Easing.InOutQuad
			}
		}
	}

}
