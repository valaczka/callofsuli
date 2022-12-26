import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Item {
	id: control

	property alias iconLabel: iconLabel
	property color color: "blue"
	property real pixelSize: Qaterial.Style.textTheme.body2.pixelSize
	property string text: "%1"
	property bool marked: false
	property real horizontalPadding: 10
	property bool iconVisible: true

	implicitHeight: iconLabel.height
	implicitWidth: iconLabel.width+2*horizontalPadding

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


	Qaterial.IconLabel {
		anchors.centerIn: parent
		id: iconLabel
		color: control.color
		icon.width: control._realSize*1.1
		icon.height: control._realSize*1.1
		font.family: "Rajdhani"
		font.pixelSize: control._realSize
		font.weight: Font.Bold
		text: control.text.arg(control.value)
		display: control.iconVisible ? Qaterial.IconLabel.Display.TextBesideIcon : Qaterial.IconLabel.Display.TextOnly
	}



	Glow {
		anchors.fill: iconLabel
		source: iconLabel
		color: "black"
		radius: 2
		spread: 0.5
		samples: 5
	}
}
