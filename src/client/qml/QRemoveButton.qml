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

	height: label.height
	width: 48

	Label {
		id: label

		anchors.centerIn: parent

		color: area.containsMouse ? CosStyle.colorErrorLighter : CosStyle.colorError

		Component.onCompleted: JS.setIconFont(label, "M\ue872")

		ToolTip.text: tooltip
		ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
		ToolTip.visible: area.containsMouse && ToolTip.text.length

		font.pixelSize: CosStyle.pixelSize*1.2


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
		enabled: label.visible
		anchors.fill: label
		hoverEnabled: true
		acceptedButtons: Qt.LeftButton

		onClicked: control.clicked
	}
}
