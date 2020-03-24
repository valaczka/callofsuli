import QtQuick 2.12
import QtQuick.Controls 2.12

CheckBox {
	id: control

	property alias textColor: content.color

	contentItem: Text {
		id: content
		leftPadding: control.indicator.width + control.spacing
		opacity: enabled ? 1.0 : 0.3
		verticalAlignment: Text.AlignVCenter

		text: control.text
		font: control.font
	}
}
