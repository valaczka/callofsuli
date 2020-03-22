import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.3
import "Style"
import "JScript.js" as JS
import "."

ToolButton {
	id: button

	Material.foreground: CosStyle.colorPrimaryLight
	Component.onCompleted: JS.setIconFont(button, "M\ue5d4")

	default property alias menuItems: menu.contentData

	onClicked: menu.open()

	QMenu {
		id: menu
		y: button.height
	}
}
