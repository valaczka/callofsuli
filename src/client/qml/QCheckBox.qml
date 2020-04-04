import QtQuick 2.12
import QtQuick.Controls 2.14
import "Style"

CheckBox {
	id: control

	property alias textColor: content.color

	ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
	ToolTip.visible: hovered && ToolTip.text.length

	contentItem: Text {
		id: content
		leftPadding: control.indicator.width + control.spacing
		opacity: enabled ? 1.0 : 0.3
		verticalAlignment: Text.AlignVCenter

		text: control.text
		font: control.font

		color: CosStyle.colorPrimary
	}
}
