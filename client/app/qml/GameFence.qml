import Bacon2D 1.0
import QtQuick 2.15
import COS.Client 1.0
import QtGraphicalEffects 1.0
import "Style"

PhysicsEntity {
	id: root
	sleepingAllowed: true

	z: 6

	width: 120			//299
	height: 95			//486

	readonly property point operatingPointLeft: Qt.point(45,0)
	readonly property point operatingPointRight: Qt.point(width-45,0)

	property CosGame cosGame: null

	property bool glowEnabled: cosGame && cosGame.player && cosGame.player.entityPrivate.fence === root

	bodyType: Body.Static

	transformOrigin: Item.Center

	fixtures: [
		Box {
			id: box1
			width: 15
			height: root.height
			x: (root.width-width)/2+5
			y: 0
			sensor: false
			//collidesWith: Box.Category3
			categories: Box.Category1
			property bool invisible: true
		},
		Box {
			id: box2
			width: root.width
			height: root.height
			x: 0
			y: 0
			sensor: false
			collidesWith: Box.Category3
			categories: Box.Category4

			readonly property PhysicsEntity targetObject: root
			readonly property var targetData: {"fence": true}
		}

	]




	Glow {
		id: glow
		opacity: glowEnabled ? 1.0 : 0.0
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

	Image {
		id: img

		anchors.fill: parent

		source: "qrc:/internal/game/fence.png"
		fillMode: Image.PreserveAspectFit
		visible: false
	}



	Row {
		id: rectRow
		visible: false
		spacing: 0
		Rectangle {
			width: rectWhite.width
			height: root.height
			color: "transparent"
		}
		Rectangle {
			width: root.width-rectWhite.width
			height: root.height
			color: "white"
		}
	}

	OpacityMask {
		anchors.fill: img
		source: img
		maskSource: rectRow
	}





	Row {
		id: rectRowOver
		visible: false
		spacing: 0
		Rectangle {
			id: rectWhite
			width: root.width*0.6
			height: root.height
			color: "white"
		}
		Rectangle {
			width: root.width-rectWhite.width
			height: root.height
			color: "transparent"
		}
	}

	OpacityMask {
		id: maskOver

		parent: root.parent

		x: root.x+img.x
		y: root.y+img.y
		width: img.width
		height: img.height
		z: 11

		source: img
		maskSource: rectRowOver
	}




	SequentialAnimation {
		id: dieAnimation
		running: false

		PauseAnimation {
			duration: 800
		}
		PropertyAnimation {
			targets: [root, maskOver]
			property: "opacity"
			to: 0
			duration: 1000
		}
		ScriptAction {
			script: {
				if (cosGame)
					cosGame.fixturesReadyToDestroy([box1, box2])

				maskOver.destroy()
				root.destroy()
			}
		}
	}

	function operate() {
		dieAnimation.start()
	}

}
