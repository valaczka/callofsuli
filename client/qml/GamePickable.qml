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

	property real horizontalPadding: 25
	property real verticalPadding: 5

	property bool glowEnabled: (scene && scene.showObjects) || (game && game.player && game.pickable == control)
	property bool _glowForced: false


	body.fixtures: [
		Box {
			id: fixBox
			width: control.width
			height: control.height
			x: 0
			y: 0
			sensor: true
			categories: Box.Category4

			readonly property var targetData: {"pickable": true}
		}
	]


	Component {
		id: cmpAnimated

		AnimatedImage {
			source: control.image
			width: 25
			height: 25
			speed: 0.75
			fillMode: Image.PreserveAspectFit
		}
	}



	Component {
		id: cmpPixmap

		Image {
			id: lbl
			source: control.image
			width: 25 //control.imageWidth
			height: 25 //control.imageHeight
			fillMode: Image.PreserveAspectFit
		}
	}

	Loader {
		id: itemLoader
		anchors.centerIn: parent

		sourceComponent: if (control.type == GamePickablePrivate.PickableInvalid)
							 undefined
						 else if (control.format == GamePickablePrivate.FormatAnimated)
							 cmpAnimated
						 else
							 cmpPixmap

	}

	SequentialAnimation {
		running: itemLoader.status == Loader.Ready
		loops: Animation.Infinite
		ParallelAnimation {
			PropertyAnimation {
				targets: [itemLoader, glow]
				property: "scale"
				to: 1.2
				duration: 500
			}
		}
		ParallelAnimation {
			PropertyAnimation {
				targets: [itemLoader, glow]
				property: "scale"
				to: 1.0
				duration: 500
			}
		}
	}

	Glow {
		id: glow
		opacity: (glowEnabled || _glowForced) ? 1.0 : 0.0
		visible: opacity != 0

		color: Qaterial.Style.colorItemGlow
		source: itemLoader
		anchors.fill: itemLoader

		radius: 2
		samples: 5

		clip: false

		Behavior on opacity {
			NumberAnimation { duration: 200 }
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
						target: control
						property: "scale"
						from: 0.0
						to: 1.0
						duration: 700
						easing.type: Easing.OutBack
					}

					SequentialAnimation {
						PropertyAnimation {
							target: control
							property: "y"
							from: control.y+control.elevation
							to: control.y-100
							duration: 450
							easing.type: Easing.OutQuad
						}


						PropertyAnimation {
							target: control
							property: "y"
							to: control.y
							duration: 300
							easing.type: Easing.InSine
						}
					}
				}

			}
		},

		Transition {
			from: "*"
			to: "picked"

			SequentialAnimation {
				ParallelAnimation {
					PropertyAnimation {
						target: control
						property: "scale"
						to: 7.0
						duration: 250
					}
					PropertyAnimation {
						target: control
						property: "opacity"
						to: 0.0
						duration: 260
					}
				}
				ScriptAction {
					script: {
						control.pickFinished()
					}
				}
			}
		}
	]

}
