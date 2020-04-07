import QtQuick 2.12
import QtQuick.Controls 2.14
import "Style"

ComboBox {
	id: control

	ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
	ToolTip.visible: hovered && ToolTip.text.length


}
