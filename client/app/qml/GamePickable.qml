import Bacon2D 1.0
import QtQuick 2.15
import COS.Client 1.0
import QtGraphicalEffects 1.0
import "Style"

PhysicsEntity {
	id: root
	sleepingAllowed: true
	width: itemLoader.status == Loader.Ready ? itemLoader.item.width+(2*horizontalPadding) : 10
	height: itemLoader.status == Loader.Ready ? itemLoader.item.height+(2*verticalPadding) : 10
	z: 6

	x: bottomPoint.x-(width/2)
	y: bottomPoint.y-elevation-(height/2)

	bodyType: Body.Static

	transformOrigin: Item.Center

	property real elevation: 25
	property var pickableData: null

	property point bottomPoint: pickableData ? pickableData.bottomPoint : Qt.point(0,0)

	property int _collision: 0 //Box.Category3

	property real horizontalPadding: 25
	property real verticalPadding: 5

	property bool glowEnabled: false
	property bool _glowForced: false
	property bool overlayEnabled: false

	property CosGame cosGame: null
	property GamePickablePrivate targetObject: null

	default property alias contentItem: itemLoader.sourceComponent

	fixtures: [
		Box {
			id: fixBox
			width: root.width
			height: root.height
			x: 0
			y: 0
			sensor: true
			collidesWith: _collision
			categories: Box.Category4

			readonly property GamePickablePrivate targetObject: root.targetObject
			readonly property var targetData: null
		}
	]


	Connections {
		target: targetObject

		function onPicked() {
			_collision = 0
			fixBox.collidesWith = 0
			fixBox.categories = 0
			cosClient.playSound("qrc:/sound/sfx/pick.mp3", CosSound.GameSound)
			state = "picked"
		}
	}

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
}
