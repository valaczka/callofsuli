import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.3
import "Style"
import "JScript.js" as JS
import "."

QToolButton {
	id: button

	icon.source: CosStyle.iconMenu
	property alias menu: menu

	default property alias menuItems: menu.contentData

	onClicked: menu.open()

	QMenu {
		id: menu
		y: button.height
	}
}
