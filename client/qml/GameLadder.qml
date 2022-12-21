import QtQuick 2.15
import CallOfSuli 1.0
import Box2D 2.0
import QtGraphicalEffects 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


GameLadderPrivate {
	id: control
	width: boundRect.width
	height: 0
	x: boundRect.x
	y: boundRect.y-fixtureHeight
	z: 5

	property int _collision: 0 //Box.Category3

	readonly property int fixtureHeight: 20
	readonly property int ladderWidth: 30

	property bool glowEnabled: scene && (scene.showObjects || (scene.game.player && scene.game.player.ladder == control))
	property bool _glowForced: false

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


	Glow {
		id: glow
		opacity: control.active && (glowEnabled || _glowForced) ? 1.0 : 0.0
		visible: opacity != 0

		color: Qaterial.Style.colorItemGlow
		source: img
		anchors.fill: img

		radius: 2
		samples: 5

		Behavior on opacity {
			NumberAnimation { duration: 200 }
		}
	}

	BorderImage {
		id: img
		source: "qrc:/terrain/tileset/ladder1.png"
		x: (control.width-ladderWidth)/2
		y: fixtureHeight
		width: ladderWidth
		height: control.height-fixtureHeight
		border.left: 9
		border.top: 9
		border.right: 7
		border.bottom: 12
		horizontalTileMode: BorderImage.Repeat
		verticalTileMode: BorderImage.Repeat
	}


	states: [
		State {
			name: "active"
			when: control.active
		}
	]

	transitions: [
		Transition {
			from: "*"
			to: "active"

			SequentialAnimation {
				PropertyAction {
					target: control
					property: "_glowForced"
					value: true
				}

				PropertyAnimation {
					target: control
					property: "height"
					from: 0
					to: control.boundRect.height+fixtureHeight
					duration: 3000
				}

				PropertyAction {
					target: control
					property: "_collision"
					value: Box.Category2
				}

				PauseAnimation {
					duration: 1750
				}

				PropertyAction {
					target: control
					property: "_glowForced"
					value: false
				}

			}
		}
	]

}
