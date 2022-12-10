import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


Qaterial.Page {
	id: control

	background: Item {
		anchors.fill: parent
		Image {
			id: bgImage
			anchors.fill: parent
			fillMode: Image.PreserveAspectCrop
			source: "qrc:/internal/img/villa.png"
			//visible: !compact
		}
		/*ColorOverlay {
			anchors.fill: bgImage
			source: bgImage
			color: backgroundImageColor
			visible: compact
		}*/
	}

}
