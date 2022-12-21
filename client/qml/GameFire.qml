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

	opacity: 0.9
	/*
	body.fixtures: [
		Box {
			width: ladderWidth-6
			x: (control.width-ladderWidth)/2+3
			y: 0
			height: fixtureHeight
			sensor: true
			collidesWith: _collision
			categories: Box.Category4

			readonly property QtObject targetObject: control
			readonly property var targetData: {"direction": "down" }

			onBeginContact: _glowForced = true
			onEndContact: _glowForced = false
		},
		Box {
			width: ladderWidth-6
			x: (control.width-ladderWidth)/2+3
			y: control.height-fixtureHeight
			height: fixtureHeight
			sensor: true
			collidesWith: _collision
			categories: Box.Category4

			readonly property QtObject targetObject: control
			readonly property var targetData: {"direction": "up" }

			onBeginContact: _glowForced = true
			onEndContact: _glowForced = false
		}
	]


	body.bodyType: Body.Static
*/

	transformOrigin: Item.Center

	body.fixtures: [
		Box {
			id: boxDie
			width: 15
			height: control.height
			x: (control.width-width)/2
			y: 0
			sensor: true
			//collidesWith: Box.Category3
			categories: Box.Category4

			readonly property QtObject targetObject: control
			readonly property var targetData: {"fireDie": true}
		},
		Box {
			width: control.width
			height: control.height
			x: 0
			y: 0
			sensor: true
			//collidesWith: Box.Category3
			categories: Box.Category4

			readonly property QtObject targetObject: control
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
			target: control
			property: "opacity"
			to: 0
			duration: 1000
		}
		ScriptAction {
			script: control.destroy()
		}
	}

	function operate() {
		dieAnimation.start()
	}

}
