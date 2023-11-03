import QtQuick
import QtQuick.Controls
import CallOfSuli
import Box2D 2.0
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


GameObject {
	id: control

	z: 6

	width: 120			//299
	height: 95			//486

	readonly property point operatingPointLeft: Qt.point(45,0)
	readonly property point operatingPointRight: Qt.point(width-45,0)

	property bool glowEnabled: game && game.player && playerFence == control
	property GameObject playerFence: null

	objectType: "fence"

	transformOrigin: Item.Center

	Connections {
		target: game

		function onPlayerChanged() {
			if (!game.player)
				playerFence = null
		}
	}


	Connections {
		target: game ? game.player : null

		function onTerrainObjectChanged(type, object) {
			if (type === "fence")
				playerFence = object
		}
	}



	body.fixtures: [
		Box {
			id: box1
			width: 15
			height: control.height
			x: (control.width-width)/2+5
			y: 0
			sensor: false
			categories: Box.Category1
			property bool invisible: true
		},
		Box {
			id: box2
			width: control.width
			height: control.height
			x: 0
			y: 0
			sensor: true
			categories: Box.Category4
		}

	]




	Glow {
		id: glow
		opacity: glowEnabled ? 1.0 : 0.0
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
			height: control.height
			color: "transparent"
		}
		Rectangle {
			width: control.width-rectWhite.width
			height: control.height
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
			width: control.width*0.6
			height: control.height
			color: "white"
		}
		Rectangle {
			width: control.width-rectWhite.width
			height: control.height
			color: "transparent"
		}
	}

	OpacityMask {
		id: maskOver

		parent: control.parent

		x: control.x+img.x
		y: control.y+img.y
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
			targets: [control, maskOver]
			property: "opacity"
			to: 0
			duration: 1000
		}
		ScriptAction {
			script: {
				maskOver.destroy()
				control.deleteSelf()
			}
		}
	}

	function operate() {
		dieAnimation.start()
	}

}
