import QtQuick 2.12
import Bacon2D 1.0

PhysicsEntity {
	id: root
	width: 100
	height: 62
	sleepingAllowed: false
	bodyType: Body.Static
	transformOrigin: Item.Center

	property bool picked: false

	fixtures: [
		Box {
			width: target.width
			height: target.height
			sensor: true
			collidesWith: Box.Category1

			readonly property int direction: -target.height-10

			onBeginContact: {
				rect.opacity = 0
			}

			onEndContact: rect.opacity = 0.2
		},

		Box {
			width: target.width
			height: 50
			y: -height
			sensor: true
			collidesWith: Box.Category1

			readonly property int direction: target.height

			onBeginContact: {
				rect.opacity = 0.8
			}

			onEndContact: rect.opacity = 0.2
		}

	]

	/*Sprite {
		animation: "spin"
		width: parent.width
		height: parent.height
		animations: SpriteAnimation {
			name: "spin"
			source: "images/coin.png"
			frames: 10
			duration: 1000
			loops: Animation.Infinite
		}
	}*/

	Rectangle {
		id: rect
		anchors.fill: parent
		anchors.margins: 3
		color: "yellow"
		opacity: 0.2
	}

	/*onOpacityChanged: {
		if(opacity == 0)
			destroy()
	}

	Behavior on opacity { NumberAnimation {} }
	Behavior on scale { NumberAnimation {} } */
}

