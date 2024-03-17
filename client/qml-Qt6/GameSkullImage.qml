import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


Image {
	id: _skullImage
	opacity: 0.0
	visible: opacity
	source: "qrc:/internal/game/skull.svg"
	sourceSize.width: 200
	sourceSize.height: 200

	width: 200
	height: 200
	fillMode: Image.PreserveAspectFit

	ParallelAnimation {
		id: skullImageAnim

		SequentialAnimation {
			PropertyAnimation {
				target: _skullImage
				property: "opacity"
				from: 0.0
				to: 0.6
				duration: 350
				easing.type: Easing.InQuad
			}
			PropertyAnimation {
				target: _skullImage
				property: "opacity"
				from: 0.6
				to: 0.0
				duration: 600
				easing.type: Easing.InQuad
			}
		}
		PropertyAnimation {
			target: _skullImage
			property: "scale"
			from: 0.1
			to: 8.0
			duration: 950
			easing.type: Easing.OutInQuad
		}
	}

	function play() {
		skullImageAnim.start()
	}
}
