import Bacon2D 1.0
import QtQuick 2.14
import COS.Client 1.0
import QtGraphicalEffects 1.0

PhysicsEntity {
	id: root
	sleepingAllowed: false
	width: ladder ? ladder.boundRect.width : 10
	height: ladder ? ladder.boundRect.height+fixtureHeight : 10
	x: ladder ? ladder.boundRect.x : 0
	y: ladder ? ladder.boundRect.y-fixtureHeight : 0

	bodyType: Body.Static

	readonly property int fixtureHeight: 30
	readonly property int ladderWidth: 30


	property GameLadderPrivate ladder: null

	fixtures: [
		Box {
			width: ladderWidth-6
			x: (root.width-ladderWidth)/2+3
			y: 0
			height: fixtureHeight
			sensor: true
			collidesWith: Box.Category2
			categories: Box.Category3

			readonly property QtObject targetObject: ladder
			readonly property var targetData: {"direction": "down" }

			onBeginContact: glow.opacity = 1.0
			onEndContact: glow.opacity = 0
		},
		Box {
			width: ladderWidth-6
			x: (root.width-ladderWidth)/2+3
			y: root.height-fixtureHeight
			height: fixtureHeight
			sensor: true
			collidesWith: Box.Category2
			categories: Box.Category3

			readonly property QtObject targetObject: ladder
			readonly property var targetData: {"direction": "up" }

			onBeginContact: glow.opacity = 1.0
			onEndContact: glow.opacity = 0
		}
	]

	Glow {
		id: glow
		opacity: 0.0
		visible: opacity != 0

		color: "yellow"
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
}
