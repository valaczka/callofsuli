import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
import "Style"
import "JScript.js" as JS


SpinBox {
	id: control

	property color textColor: CosStyle.colorAccent

	ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
	ToolTip.visible: hovered && ToolTip.text.length

	contentItem: TextInput {
		z: 2
		text: control.textFromValue(control.value, control.locale)

		font.family: "Special Elite"
		font.pixelSize: CosStyle.pixelSize
		font.weight: Font.Medium
		opacity: enabled ? 1 : 0.5
		color: control.enabled ?
				   control.textColor :
				   "white"
		selectionColor: JS.setColorAlpha(CosStyle.colorPrimary, 0.4)
		selectedTextColor: color
		verticalAlignment: Qt.AlignVCenter
		horizontalAlignment: Qt.AlignHCenter

		readOnly: !control.editable
		validator: control.validator

		inputMethodHints: Qt.ImhFormattedNumbersOnly
	}

}
