import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.3
import "Style"
import "JScript.js" as JS

Menu {
	id: menu

	Material.foreground: CosStyle.colorPrimaryLighter
	Material.background: JS.setColorAlpha(Qt.darker(CosStyle.colorPrimaryDark,2.5), 0.95)

	font.pixelSize: CosStyle.pixelSize

	Component {
		id: separatorItem
		MenuSeparator { }
	}

	function addSeparator() {
		var m = separatorItem.createObject(menu)
		if (m)
			menu.addItem(m)
	}
}
