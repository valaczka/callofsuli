import QtQuick 2.7
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.3
import "Style"
import "JScript.js" as JS

ToolButton {
	id: control

	Material.foreground: color
	Material.accent: accentColor

	property color color: CosStyle.colorPrimaryLight
	property color accentColor: CosStyle.colorAccentDarker

	ToolTip.text: text
	ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
	ToolTip.visible: hovered && ToolTip.text.length

	font.pixelSize: CosStyle.pixelSize

	icon.height: CosStyle.pixelSize*1.4
	icon.width: CosStyle.pixelSize*1.4
}
