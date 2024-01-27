import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qt5Compat.GraphicalEffects
import QtCharts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


Page {
	id: root

	property real _scale: width > height ? Math.min(1.0, width/2600) : Math.min(1.0, height/1200)

	Image {
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/internal/img/villa.png"

		cache: true
	}




	ConquestTurnChart {
		id: _chart
	}




	property int num: -1

	Row {
		QButton {
			text: "Next"
			onClicked: {
				++num
			}
		}

		QButton {
			text: "New"
			onClicked: {

			}
		}

		QButton {
			text: "++"
			onClicked: {
				_series.append("PX", 1)
			}
		}

	}


	/*Image {
		anchors.centerIn: parent
		source: "qrc:/internal/game/drop.png"

		opacity: rightCloudArea.containsMouse ? 0.3 : 1.0

		MaskedMouseArea {
			id: rightCloudArea
			anchors.fill: parent
			alphaThreshold: 0.4
			maskSource: parent.source
		}
	}*/
}


