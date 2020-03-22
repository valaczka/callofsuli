import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.3
import "Style"
import "JScript.js" as JS
import "."

QToolButton {
	id: button

	icon.source: CosStyle.iconMenu
	property QMenu menu: null

	visible: menu

	onClicked: {
		menu.parent = button
		menu.y = button.height
		menu.open()
	}
}
