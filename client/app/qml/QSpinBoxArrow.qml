import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "Style"
import "JScript.js" as JS


SpinBox {
	id: control

	property color textColor: CosStyle.colorAccent
	property color buttonColor: CosStyle.colorPrimaryLighter

	ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
	ToolTip.visible: hovered && ToolTip.text.length

	contentItem: QLabel {
		z: 2
		text: control.textFromValue(control.value, control.locale)

		font.pixelSize: CosStyle.pixelSize
		font.weight: Font.DemiBold
		font.capitalization: Font.AllUppercase
		opacity: enabled ? 1 : 0.5
		color: control.enabled ?
				   control.textColor :
				   "white"
		verticalAlignment: Qt.AlignVCenter
		horizontalAlignment: Qt.AlignHCenter
	}

	up.indicator: Rectangle {
		id: up
		x: control.mirrored ? 0 : parent.width - width
		height: width
		implicitWidth: CosStyle.pixelSize*2
		implicitHeight: CosStyle.pixelSize*2
		anchors.verticalCenter: parent.verticalCenter
		color: control.up.pressed || control.up.hovered ? CosStyle.colorBg : "transparent"

		QLabel {
			anchors.centerIn: parent
			text: "»"
			font.family: "HVD Peace"
			color: up.enabled ? control.buttonColor : CosStyle.colorBg
			font.pixelSize: CosStyle.pixelSize*1.6
		}
	}

	down.indicator: Rectangle {
		id: down

		x: control.mirrored ? parent.width - width : 0
		height: width
		implicitWidth: CosStyle.pixelSize*2
		implicitHeight: CosStyle.pixelSize*2
		anchors.verticalCenter: parent.verticalCenter
		color: control.down.pressed || control.down.hovered ? CosStyle.colorBg : "transparent"

		QLabel {
			anchors.centerIn: parent
			text: "«"
			font.family: "HVD Peace"
			color: down.enabled ? control.buttonColor : CosStyle.colorBg
			font.pixelSize: CosStyle.pixelSize*1.6
		}
	}
}
