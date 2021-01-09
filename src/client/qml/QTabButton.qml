import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import "Style"
import "JScript.js" as JS

TabButton {
	id: control

	font.weight: Font.DemiBold
	font.pixelSize: CosStyle.pixelSize*0.8

	display: AbstractButton.IconOnly

	property color iconColor: CosStyle.colorPrimaryLighter

	Material.foreground: iconColor
}
