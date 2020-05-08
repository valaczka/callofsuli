import QtQuick 2.12
import QtQuick.Controls 2.14
import QtQml 2.14
import "."
import "Style"
import "JScript.js" as JS

Item {
	id: control

	property string tooltip: qsTr("Törlés")
	property alias buttonVisible: label.visible

	signal clicked()

	implicitWidth: width
	implicitHeight: height
	height: label.height
	width: 48

	QFontImage {
		id: label

		anchors.centerIn: parent

		color: area.containsMouse ? CosStyle.colorErrorLighter : CosStyle.colorError

		icon: CosStyle.iconRemove

		ToolTip.text: tooltip
		ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
		ToolTip.visible: area.containsMouse && ToolTip.text.length

		size: CosStyle.pixelSize*1.5


		Binding on scale {
			restoreMode: Binding.RestoreBindingOrValue
			when: area.pressed
			value: 0.85
		}

		Behavior on color {  ColorAnimation { duration: 75 } }
		Behavior on scale {  NumberAnimation { duration: 75 } }
	}

	MouseArea {
		id: area
		enabled: buttonVisible
		anchors.fill: label
		hoverEnabled: true
		acceptedButtons: Qt.LeftButton

		onClicked: control.clicked()
	}
}
