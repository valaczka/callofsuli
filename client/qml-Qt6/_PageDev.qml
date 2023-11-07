import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


Page {
	id: root

	Image {
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/internal/img/villa.png"

		cache: true
	}

	Rectangle {
		color: "black"
		anchors.centerIn: parent
		width: 400
		height: 400

		Column {
			spacing: 10
			width: parent.width

			TextField {
				width: parent.width
				activeFocusOnPress: true
			}

			TextField {
				width: parent.width
			}

			Qaterial.TextField {
				id: tf1
				width: parent.width
			}
		}
	}

	Component.onCompleted: tf1.forceActiveFocus()
}


