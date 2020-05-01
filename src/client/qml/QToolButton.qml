import QtQuick 2.7
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.3
import "Style"
import "JScript.js" as JS

ToolButton {
	id: control

	Material.foreground: CosStyle.colorPrimaryLight

	ToolTip.text: text
	ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
	ToolTip.visible: hovered && ToolTip.text.length
}
