import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style"


Rectangle {
	id: control

	property alias text: label.text
	property alias textColor: label.color
	color: CosStyle.colorPrimaryDark

	height: Math.max(label.implicitHeight+2, 6)
	radius: height/2
	width: Math.max(label.implicitWidth+(height/2), height)

	Label {
		id: label
		font.pixelSize: CosStyle.pixelSize*0.7
		font.weight: Font.Bold
		color: "white"

		anchors.centerIn: parent
	}

}
