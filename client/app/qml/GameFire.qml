import Bacon2D 1.0
import QtQuick 2.15
import COS.Client 1.0
import "Style"

PhysicsEntity {
	id: root
	sleepingAllowed: true

	z: 6

	width: 150
	height: 128

	readonly property point operatingPointLeft: Qt.point(45,0)
	readonly property point operatingPointRight: Qt.point(width-45,0)

	opacity: 0.9

	bodyType: Body.Static

	transformOrigin: Item.Center

	fixtures: [
		Box {
			id: boxDie
			width: 15
			height: root.height
			x: (root.width-width)/2
			y: 0
			sensor: false
			//collidesWith: Box.Category3
			categories: Box.Category4

			readonly property PhysicsEntity targetObject: root
			readonly property var targetData: {"fireDie": true}
		},
		Box {
			width: root.width
			height: root.height
			x: 0
			y: 0
			sensor: false
			//collidesWith: Box.Category3
			categories: Box.Category4

			readonly property PhysicsEntity targetObject: root
			readonly property var targetData: {"fire": true}
		}

	]


	SpriteSequence {
		id: spriteSequence

		anchors.fill: parent

		sprites: [
			Sprite {
				name: "idle"
				source: "qrc:/internal/game/fire.png"
				frameCount: 25
				frameDuration: 30
				frameWidth: 128
				frameHeight: 128
				randomStart: true
			}
		]
	}

	SequentialAnimation {
		id: dieAnimation
		running: false

		PauseAnimation {
			duration: 800
		}
		PropertyAnimation {
			target: root
			property: "opacity"
			to: 0
			duration: 1000
		}
		ScriptAction {
			script: root.destroy()
		}
	}

	function operate() {
		dieAnimation.start()
	}

}
