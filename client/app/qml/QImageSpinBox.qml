import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "Style"
import "JScript.js" as JS


SpinBox {
	id: control

	implicitWidth: implicitHeight*5
	implicitHeight: imageSize+10

	property color textColor: CosStyle.colorAccent
	property color buttonColor: CosStyle.colorPrimaryLighter

	property real imageSize: CosStyle.pixelSize*2
	property var imageFromValue: null
	property alias contentSpacing: rw.spacing

	ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
	ToolTip.visible: hovered && ToolTip.text.length

	contentItem: Row {
		id: rw
		spacing: 5

		anchors.verticalCenter: parent.verticalCenter

		Image {
			z: 2
			id: img
			anchors.verticalCenter: parent.verticalCenter
			width: imageSize
			height: imageSize
			fillMode: Image.PreserveAspectFit
			opacity: control.enabled ? 1 : 0.5
			source: imageFromValue ? control.imageFromValue(control.value) : ""
		}

		QLabel {
			id: label
			z:2
			text: control.textFromValue(control.value, control.locale)
			anchors.verticalCenter: parent.verticalCenter
			font.family: "Special Elite"
			font.pixelSize: CosStyle.pixelSize
			font.weight: Font.Medium
			opacity: control.enabled ? 1 : 0.5
			color: control.enabled ?
					   control.textColor :
					   "white"
			verticalAlignment: Qt.AlignVCenter
			horizontalAlignment: Qt.AlignHCenter
		}
	}


	up.indicator: Rectangle {
		id: up
		x: control.mirrored ? 0 : parent.width - width
		height: width
		implicitWidth: imageSize
		implicitHeight: imageSize
		anchors.verticalCenter: parent.verticalCenter
		color: control.up.pressed ? CosStyle.colorBg : "transparent"

		QLabel {
			anchors.centerIn: parent
			text: "»"
			font.family: "HVD Peace"
			color: up.enabled ? control.buttonColor : CosStyle.colorBg
			font.pixelSize: CosStyle.pixelSize*1.5
		}
	}

	down.indicator: Rectangle {
		id: down

		x: control.mirrored ? parent.width - width : 0
		height: width
		implicitWidth: imageSize
		implicitHeight: imageSize
		anchors.verticalCenter: parent.verticalCenter
		color: control.down.pressed ? CosStyle.colorBg : "transparent"

		QLabel {
			anchors.centerIn: parent
			text: "«"
			font.family: "HVD Peace"
			color: down.enabled ? control.buttonColor : CosStyle.colorBg
			font.pixelSize: CosStyle.pixelSize*1.5
		}
	}

	background: Item {}

}
