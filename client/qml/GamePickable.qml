import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import QtGraphicalEffects 1.0
import Box2D 2.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


GamePickablePrivate {
	id: control

	width: itemLoader.status == Loader.Ready ? itemLoader.item.width+(2*horizontalPadding) : 10
	height: itemLoader.status == Loader.Ready ? itemLoader.item.height+(2*verticalPadding) : 10
	z: 7

	x: bottomPoint.x-(width/2)
	y: bottomPoint.y-elevation-(height/2)

	transformOrigin: Item.Center

	property real elevation: 25
	property var pickableData: null


	property int _collision: 0 //Box.Category3

	property real horizontalPadding: 25
	property real verticalPadding: 5

	property bool glowEnabled: false
	property bool _glowForced: false
	property bool overlayEnabled: false

	property CosGame cosGame: null

	default property alias contentItem: itemLoader.sourceComponent

	body.fixtures: [
		Box {
			id: fixBox
			width: control.width
			height: control.height
			x: 0
			y: 0
			sensor: true
			//collidesWith: _collision
			categories: Box.Category4

			readonly property var targetData: null
		}
	]


	/*Connections {
		target: targetObject

		function onPicked() {
			_collision = 0
			fixBox.collidesWith = 0
			fixBox.categories = 0
			cosClient.playSound("qrc:/sound/sfx/pick.mp3", CosSound.GameSound)
			state = "picked"
		}
	}*/

	Connections {
		target: cosGame

		function onPickableChanged(pickable) {
			glowEnabled = (targetObject && pickable === targetObject)
		}
	}


	Loader {
		id: itemLoader
		anchors.centerIn: parent
	}

	Glow {
		id: glow
		opacity: (glowEnabled || _glowForced) ? 1.0 : 0.0
		visible: opacity != 0

		color: CosStyle.colorGlowItem
		source: itemLoader
		anchors.fill: itemLoader

		radius: 2
		samples: 5

		Behavior on opacity {
			NumberAnimation { duration: 200 }
		}
	}


	ColorOverlay {
		id: overlay
		source: itemLoader
		anchors.fill: itemLoader
		opacity: overlayEnabled ? 1.0 : 0.0
		visible: opacity != 0

		Behavior on opacity {
			NumberAnimation { duration: 300 }
		}
	}


	states: [
		State {
			name: "active"
			when: itemLoader.status == Loader.Ready
		},
		State {
			name: "picked"
		}
	]

	transitions: [
		Transition {
			from: "*"
			to: "active"

			SequentialAnimation {
				ParallelAnimation {
					PropertyAnimation {
						target: root
						property: "scale"
						from: 0.0
						to: 1.0
						duration: 700
						easing.type: Easing.OutBack
					}

					SequentialAnimation {
						PropertyAnimation {
							target: root
							property: "y"
							from: root.y+root.elevation
							to: root.y-100
							duration: 450
							easing.type: Easing.OutQuad
						}


						PropertyAnimation {
							target: root
							property: "y"
							to: root.y
							duration: 300
							easing.type: Easing.InSine
						}
					}
				}

				PropertyAction {
					target: root
					property: "_collision"
					value: Box.Category3
				}
			}
		},

		Transition {
			from: "*"
			to: "picked"

			SequentialAnimation {
				ParallelAnimation {
					PropertyAnimation {
						target: root
						property: "scale"
						to: 7.0
						duration: 250
					}
					PropertyAnimation {
						target: root
						property: "opacity"
						to: 0.0
						duration: 260
					}
				}
				ScriptAction {
					script: {
						if (cosGame && targetObject)
							cosGame.removePickable(targetObject)
						root.destroy()
					}
				}
			}
		}
	]














	z: 6

	width: 150
	height: 128

	readonly property point operatingPointLeft: Qt.point(45,0)
	readonly property point operatingPointRight: Qt.point(width-45,0)

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
			script: control.destroy()
		}
	}

	function operate() {
		dieAnimation.start()
	}

}
