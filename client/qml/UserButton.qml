import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Qaterial.SquareButton {
	id: control

	property real contentSize: (Math.min(width,height)*0.75)-(2*roundBorderWidth)
	property int roundBorderWidth: 1

	contentItem: Item {
		Rectangle {
			anchors.centerIn: parent
			width: img.width+2*roundBorderWidth
			height: width
			radius: width/2
			color: Qaterial.Style.iconColor()
			Qaterial.RoundImage {
				id: img
				anchors.centerIn: parent
				width: contentSize
				height: contentSize
				source: "qrc:/internal/img/metalbg.png"
			}
		}
	}
}
