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
	property real pixelSize: (Qt.platform.os === "android" ? 18 : 24)
	property bool marked: false
	property real horizontalPadding: 5

	implicitHeight: Math.max(txtRow.height, progressBar.implicitHeight)
	implicitWidth: txtRow.width+progressBar.width+txtRow.anchors.rightMargin+2*horizontalPadding

	property real _realSize: marked ? pixelSize*1.6 : pixelSize


	Behavior on _realSize {
		PropertyAnimation { duration: 450 }
	}

	Timer {
		id: resizeTimer
		interval: 2500
		running: marked
		triggeredOnStart: false
		repeat: false
		onTriggered: marked = false
	}

	Rectangle {
		anchors.fill: parent
		color: control.marked ? control.color : "transparent"
		radius: 5
		opacity: 0.7
		Behavior on color {
			ColorAnimation { duration: 350 }
		}
	}

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
			size: control._realSize
		}

		QLabel {
			id: label
			anchors.verticalCenter: parent.verticalCenter
			font.pixelSize: control._realSize*0.85
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
		anchors.rightMargin: control.horizontalPadding

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
