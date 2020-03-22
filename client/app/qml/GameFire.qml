import Bacon2D 1.0
import QtQuick 2.15
import COS.Client 1.0
import QtGraphicalEffects 1.0
import "Style"

PhysicsEntity {
	id: root
	sleepingAllowed: false

	width: 128
	height: 128

	opacity: 0.9

	bodyType: Body.Static

	transformOrigin: Item.Center

	fixtures: [
		Box {
			id: fixBox
			width: root.width
			height: root.height
			x: 0
			y: 0
			sensor: true
			//collidesWith: _collision
			categories: Box.Category4

			//readonly property GamePickablePrivate targetObject: root.targetObject
			//readonly property var targetData: null
		}
	]


	SpriteSequence {
		id: spriteSequence

		anchors.fill: parent

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


}
