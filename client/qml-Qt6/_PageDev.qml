import QtQuick
import QtQuick.Controls
import CallOfSuli
import Tiled as Tiled
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


	Image {
		anchors.centerIn: parent
		source: "qrc:/internal/game/drop.png"

		opacity: rightCloudArea.containsMouse ? 0.3 : 1.0

		MaskedMouseArea {
			id: rightCloudArea
			anchors.fill: parent
			alphaThreshold: 0.4
			maskSource: parent.source
		}
	}
}


