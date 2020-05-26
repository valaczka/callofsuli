import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.12
import "Style"
import "JScript.js" as JS

TabButton {
	id: control

	font.weight: Font.DemiBold
	font.pixelSize: CosStyle.pixelSize*0.8

	Material.accent: "red"
}