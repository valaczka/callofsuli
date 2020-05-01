import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.3
import "Style"
import "JScript.js" as JS
import "."

QToolButton {
	id: button

	icon.source: CosStyle.iconClose

	ToolTip.text: qsTr("Bezárás")
}
