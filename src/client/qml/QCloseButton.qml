import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.3
import "Style"
import "JScript.js" as JS
import "."

ToolButton {
	id: button

	Material.foreground: CosStyle.colorPrimaryLight
	Component.onCompleted: JS.setIconFont(button, "M\ue14c")

	ToolTip.text: qsTr("Bezárás")
	ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
	ToolTip.visible: hovered && ToolTip.text.length
}
