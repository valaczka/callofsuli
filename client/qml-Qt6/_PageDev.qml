import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qt5Compat.GraphicalEffects
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


	Image {
		id: _bgImg
		source: "qrc:/conquest/sample/bg.png"
		width: 2600*_scale
		height: 1200*_scale
		fillMode: Image.PreserveAspectFit
	}

	Repeater {
		model: 16
		delegate: ConquestState {
			world: "sample"
			stateId: index+1
			mapScale: _scale
		}
	}


	Image {
		source: "qrc:/conquest/sample/over.png"
		width: 2600*_scale
		height: 1200*_scale
		fillMode: Image.PreserveAspectFit
	}


	property int num: 3

	QButton {
		text: "Next"
		onClicked: ++num
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


