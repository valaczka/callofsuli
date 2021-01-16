import Bacon2D 1.0
import QtQuick 2.15
import COS.Client 1.0
import QtGraphicalEffects 1.0
import "Style"

PhysicsEntity {
	id: root
	sleepingAllowed: false
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

	property real horizontalPadding: 10
	property real verticalPadding: 5

	property bool glowEnabled: false
	property bool _glowForced: false

	property CosGame cosGame: null
	property GamePickablePrivate targetObject: null

	default property alias contentItem: itemLoader.sourceComponent

	Connections {
		target: targetObject
		function onPicked() {
			cosClient.playSound("qrc:/sound/sfx/pick.ogg", CosSound.GameSound)
			state = "picked"
		}
	}

	fixtures: [
		Box {
			width: root.width
			height: root.height
			x: 0
			y: 0
			sensor: true
			collidesWith: _collision
			categories: Box.Category4

			readonly property GamePickablePrivate targetObject: root.targetObject
			readonly property var targetData: null

			onBeginContact: glowEnabled = true
			onEndContact: glowEnabled = false
		}
	]

	Glow {
		id: glow
		opacity: (glowEnabled || _glowForced) ? 1.0 : 0.0
		visible: opacity != 0

		color: CosStyle.colorGlowItem
		source: itemLoader.status == Loader.Ready ? itemLoader.item : null
		anchors.fill: itemLoader

		radius: 2
		samples: 5

		Behavior on opacity {
			NumberAnimation { duration: 200 }
		}
	}

	Loader {
		id: itemLoader
		anchors.centerIn: parent
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
					script: root.destroy()
				}
			}
		}
	]
}
