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
	property real pixelSize: 18
	property string text: "%1"
	property bool marked: false
	property real horizontalPadding: 5

	implicitHeight: txtRow.height
	implicitWidth: txtRow.width+2*horizontalPadding

	property int value: 0

	Behavior on value {
		NumberAnimation { duration: 225; easing.type: Easing.OutQuart }
	}



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

		anchors.centerIn: parent

		spacing: 5

		QFontImage {
			id: fontImage
			anchors.verticalCenter: parent.verticalCenter
			color: control.color
			size: control._realSize*1.1
		}

		QLabel {
			id: label
			anchors.verticalCenter: parent.verticalCenter
			font.pixelSize: control._realSize
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