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
		var p = button.mapToItem(ApplicationWindow.contentItem, x, y)
		var mp = menu.parent.mapFromItem(ApplicationWindow.contentItem, Math.min(p.x, ApplicationWindow.contentItem.width-menu.width), p.y+button.height)
		menu.x = mp.x
		menu.y = mp.y
		menu.open()
	}
}
