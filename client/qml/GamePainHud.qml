import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0

Image {
	id: root
	source: "qrc:/internal/game/painhud.png"
	opacity: 0.0
	visible: opacity
	sourceSize.width: width
	sourceSize.height: height

	SequentialAnimation {
		id: _anim

		PropertyAnimation {
			target: root
			property: "opacity"
			from: 0.0
			to: 1.0
			duration: 100
			easing.type: Easing.OutQuad
		}
		PropertyAnimation {
			target: root
			property: "opacity"
			from: 1.0
			to: 0.0
			duration: 375
			easing.type: Easing.InQuad
		}
	}

	function play() {
		_anim.start()
	}
}
