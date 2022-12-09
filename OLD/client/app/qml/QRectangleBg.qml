import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style"


Rectangle {
	id: control

	implicitHeight: 10
	implicitWidth: 10

	property alias acceptedButtons: area.acceptedButtons

	property alias mouseArea: area
	default property alias areaData: area.data

	color: "transparent"

	states: [
		State {
			name: "HOVERED"
			when: area.containsMouse || area.pressed
			PropertyChanges {
				target: control
				color: CosStyle.colorBg
			}
		}
	]

	transitions: [
		Transition {
			from: "*"
			to: "HOVERED"

			ColorAnimation {
				duration: 200
			}
		},
		Transition {
			from: "HOVERED"
			to: "*"

			ColorAnimation {
				duration: 275
				easing.type: Easing.OutQuad
			}
		}
	]

	MouseArea {
		id: area
		anchors.fill: parent
		hoverEnabled: acceptedButtons != Qt.NoButton
		acceptedButtons: Qt.NoButton
	}
}
