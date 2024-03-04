import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli


Item {
	id: root

	property TiledObjectImpl baseObject: null

	anchors.fill: parent

	property alias spriteSequence: spriteSequence

	Rectangle {
		visible: baseObject && baseObject.scene.debugView
		color: "transparent"
		border.color: "black"
		border.width: 2
		anchors.fill: parent
	}

	SpriteSequence {
		id: spriteSequence

		x: 0
		y: 0
		width: root.width
		height: root.height

		running: baseObject && baseObject.scene && baseObject.scene.running

		sprites: []
	}

	ThresholdMask {
		id: _threshold
		visible: false

		source: spriteSequence
		maskSource: spriteSequence
		anchors.fill: spriteSequence

		threshold: 0.7
	}

	Glow {
		id: glow
		opacity: baseObject && baseObject.glowEnabled ? 1.0 : 0.0
		visible: opacity != 0
		color: "yellow" /*entity ? entity.glowColor : "white"*/

		source: _threshold
		anchors.fill: _threshold

		radius: 4
		samples: 9

		Behavior on opacity {
			NumberAnimation { duration: 200 }
		}
	}


	ColorOverlay {
		id: overlay

		opacity: baseObject && baseObject.glowEnabled ? 0.5 : 0.0
		visible: opacity != 0
		color: "yellow" /*entity ? entity.glowColor : "white"*/

		source: _threshold
		anchors.fill: _threshold

		Behavior on opacity {
			NumberAnimation { duration: 300 }
		}
	}



	Component {
		id: componentSprite

		Sprite {  }
	}

	function appendSprite(_data) {
		let obj = componentSprite.createObject(spriteSequence, _data)
		spriteSequence.sprites.push(obj)
	}


}
