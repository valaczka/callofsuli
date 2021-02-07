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

	property alias progressBar: progressBar
	property alias label: label
	property alias image: fontImage
	property color color: CosStyle.colorAccent

	implicitHeight: Math.max(txtRow.height, progressBar.implicitHeight)
	implicitWidth: txtRow.width+progressBar.width+txtRow.anchors.rightMargin

	/*DropShadow {
		id: shadow
		anchors.fill: txtRow
		source: txtRow
		color: "black"
		radius: 2
		samples: 5
		horizontalOffset: 1
		verticalOffset: 1
	}*/



	Row {
		id: txtRow
		visible: false

		anchors.verticalCenter: parent.verticalCenter
		anchors.right: progressBar.left
		anchors.rightMargin: 10

		spacing: 5

		QFontImage {
			id: fontImage
			anchors.verticalCenter: parent.verticalCenter
			color: control.color
			size: CosStyle.pixelSize
		}

		QLabel {
			id: label
			anchors.verticalCenter: parent.verticalCenter
			font.pixelSize: CosStyle.pixelSize*0.85
			font.weight: Font.Bold
			color: control.color
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

	ProgressBar {
		id: progressBar
		visible: true
		anchors.verticalCenter: parent.verticalCenter
		anchors.right: parent.right
		width: 85
		from: 0
		to: 0
		value: 0

		Material.accent: color

		property color color: control.color

		Behavior on value {
			NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
		}
	}
}
