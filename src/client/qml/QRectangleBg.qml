import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style"


Rectangle {
	id: control

	property alias mouseArea: area
	default property alias children: area.data

	color: "transparent"

	states: [
		State {
			name: "HOVERED"
			when: area.containsMouse
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
				duration: 125
			}
		},
		Transition {
			from: "HOVERED"
			to: "*"

			ColorAnimation {
				duration: 275
				easing.type: Easing.InQuad
			}
		}
	]

	MouseArea {
		id: area
		anchors.fill: control
		hoverEnabled: true
		acceptedButtons: Qt.NoButton
	}
}
