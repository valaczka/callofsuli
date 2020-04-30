import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.3
import "Style"
import "JScript.js" as JS
import "."

ToolButton {
	id: button

	property string buttonLabel: "M\ue5d4"
	property alias menu: menu

	Material.foreground: CosStyle.colorPrimaryLight
	Component.onCompleted: JS.setIconFont(button, buttonLabel)
	onButtonLabelChanged: JS.setIconFont(button, buttonLabel)

	default property alias menuItems: menu.contentData

	onClicked: menu.open()

	QMenu {
		id: menu
		y: button.height
	}
}
