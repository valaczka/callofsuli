import QtQuick 2.7
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.3
import "Style"
import "JScript.js" as JS

ToolButton {
	id: control

	property color color: CosStyle.colorPrimaryLighter

	Material.foreground: color

	ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
	ToolTip.visible: hovered && ToolTip.text.length

	font.pixelSize: CosStyle.pixelSize*1.6
	font.weight: Font.Thin

	icon.height: CosStyle.pixelSize*4
	icon.width: CosStyle.pixelSize*4

	display: AbstractButton.TextUnderIcon
}
