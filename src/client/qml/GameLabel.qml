import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.12
import "."
import "Style"
import "JScript.js" as JS

Item {
	id: control

	property alias label: label
	property alias image: fontImage
	property color color: CosStyle.colorAccent
	property real pixelSize: 24
	property string text: "%1"

	implicitHeight: txtRow.height
	implicitWidth: txtRow.width

	property int value: 0

	Behavior on value {
		NumberAnimation { duration: 225; easing.type: Easing.OutQuart }
	}

	Row {
		id: txtRow
		visible: false

		anchors.centerIn: parent

		spacing: 5

		QFontImage {
			id: fontImage
			anchors.verticalCenter: parent.verticalCenter
			color: control.color
			size: control.pixelSize*1.1
		}

		QLabel {
			id: label
			anchors.verticalCenter: parent.verticalCenter
			font.pixelSize: control.pixelSize
			font.weight: Font.Bold
			color: control.color
			text: control.text.arg(control.value)
		}
	}

	Glow {
		anchors.fill: txtRow
		source: txtRow
		color: "black"
		radius: 2
		spread: 0.5
		samples: 5
	}
}
