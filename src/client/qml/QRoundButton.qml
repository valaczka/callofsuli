import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.3
import QtQuick.Layouts 1.3
import "Style"
import "JScript.js" as JS


RoundButton {
	id: button

	anchors.right: parent.right
	anchors.bottom: parent.bottom
	anchors.margins: 10

	width: 72
	height: 72

	font.pixelSize: CosStyle.pixelSize * 1.2
	font.weight: Font.DemiBold

	Material.background: CosStyle.colorPrimaryDark
	Material.foreground: CosStyle.colorPrimaryLight
	Material.elevation: 6

	function setIcon(i) {
		JS.setIconFont(button, i)
	}
}
