import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.12
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Item {
	id: control

	property alias progressBar: progressBar
	property alias iconLabel: iconLabel
	property alias text: iconLabel.text
	property color color: Qaterial.Style.accentColor
	property real pixelSize: (Qt.platform.os === "android" || Qt.platform.os === "ios" ? 18 : 24)
	property bool marked: false
	property real horizontalPadding: 10

	implicitHeight: Math.max(iconLabel.height, progressBar.implicitHeight)
	implicitWidth: iconLabel.width+progressBar.width+iconLabel.anchors.rightMargin+2*horizontalPadding

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

	Qaterial.IconLabel {
		id: iconLabel

		anchors.verticalCenter: parent.verticalCenter
		anchors.right: progressBar.left
		anchors.rightMargin: 10

		color: control.color
		icon.width: control._realSize
		icon.height: control._realSize
		font.family: "Rajdhani"
		font.pixelSize: control._realSize*0.85
		font.weight: Font.Bold
	}



	Glow {
		anchors.fill: iconLabel
		source: iconLabel
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
