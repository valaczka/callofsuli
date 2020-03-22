import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Rectangle {
	id: control

	implicitHeight: 50
	implicitWidth: 50

	property color backgroundColor: "purple"

	default property alias cardChildren: rectBg.areaData

	color: JS.setColorAlpha(backgroundColor, 0.5)

	border.color: Qt.lighter(backgroundColor)
	border.width: 1

	signal clicked()

	QRectangleBg {
		id: rectBg
		anchors.fill: parent
		acceptedButtons: Qt.LeftButton

		mouseArea.onClicked: control.clicked()


	}

	states: [
		State {
			name: "Pressed"
			when: rectBg.mouseArea.pressed

			PropertyChanges {
				target: control
				scale: 0.85
			}
		}
	]

	transitions: [
		Transition {
			PropertyAnimation {
				target: control
				property: "scale"
				duration: 125
				easing.type: Easing.InOutCubic
			}
		}
	]
}
