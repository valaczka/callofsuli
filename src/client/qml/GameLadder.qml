import Bacon2D 1.0
import QtQuick 2.14
import COS.Client 1.0
import QtGraphicalEffects 1.0
import "Style"

PhysicsEntity {
	id: root
	sleepingAllowed: false
	width: ladder ? ladder.boundRect.width : 10
	height: 0
	x: ladder ? ladder.boundRect.x : 0
	y: ladder ? ladder.boundRect.y-fixtureHeight : 0

	bodyType: Body.Static

	readonly property bool ladderActive: ladder && ladder.active
	property int _collision: 0 //Box.Category3

	readonly property int fixtureHeight: 20
	readonly property int ladderWidth: 30

	property bool glowEnabled: false
	property bool _glowForced: false

	property GameLadderPrivate ladder: null

	fixtures: [
		Box {
			width: ladderWidth-6
			x: (root.width-ladderWidth)/2+3
			y: 0
			height: fixtureHeight
			sensor: true
			collidesWith: _collision
			categories: Box.Category4

			readonly property QtObject targetObject: ladder
			readonly property var targetData: {"direction": "down" }

			onBeginContact: glowEnabled = true
			onEndContact: glowEnabled = false
		},
		Box {
			width: ladderWidth-6
			x: (root.width-ladderWidth)/2+3
			y: root.height-fixtureHeight
			height: fixtureHeight
			sensor: true
			collidesWith: _collision
			categories: Box.Category4

			readonly property QtObject targetObject: ladder
			readonly property var targetData: {"direction": "up" }

			onBeginContact: glowEnabled = true
			onEndContact: glowEnabled = false
		}
	]

	Glow {
		id: glow
		opacity: ladderActive && (glowEnabled || _glowForced) ? 1.0 : 0.0
		visible: opacity != 0

		color: CosStyle.colorGlowItem
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
		x: (root.width-ladderWidth)/2
		y: fixtureHeight
		width: ladderWidth
		height: root.height-fixtureHeight
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
			when: ladderActive
		}
	]

	transitions: [
		Transition {
			from: "*"
			to: "active"

			SequentialAnimation {
				PropertyAction {
					target: root
					property: "_glowForced"
					value: true
				}

				PropertyAnimation {
					target: root
					property: "height"
					from: 0
					to: ladder.boundRect.height+fixtureHeight
					duration: 3000
				}

				PropertyAction {
					target: root
					property: "_collision"
					value: Box.Category3
				}

				PauseAnimation {
					duration: 1750
				}

				PropertyAction {
					target: root
					property: "_glowForced"
					value: false
				}

			}
		}
	]
}
