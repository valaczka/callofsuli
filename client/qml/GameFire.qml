import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Box2D 2.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


GameObject {
	id: control

	z: 6

	width: 150
	height: 128

	readonly property point operatingPointLeft: Qt.point(45,0)
	readonly property point operatingPointRight: Qt.point(width-45,0)

	objectType: "fire"

	opacity: 0.9

	transformOrigin: Item.Center

	body.fixtures: [
		Box {
			id: boxDie
			width: 15
			height: control.height
			x: (control.width-width)/2
			y: 0
			sensor: true
			categories: Box.Category4

			property var targetData: {"fireDie": true}
		},
		Box {
			width: control.width
			height: control.height
			x: 0
			y: 0
			sensor: true
			categories: Box.Category4
		}

	]


	SpriteSequence {
		id: spriteSequence

		anchors.fill: parent

		running: control.game && control.game.running

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
			target: control
			property: "opacity"
			to: 0
			duration: 1000
		}
		ScriptAction {
			script: control.deleteSelf()
		}
	}

	function operate() {
		dieAnimation.start()
	}

}
