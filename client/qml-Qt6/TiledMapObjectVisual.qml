import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli


Item {
	id: root

	property TiledMapObjectImpl baseObject: null

	anchors.fill: parent

	property alias spriteSequence: spriteSequence

	SpriteSequence {
		id: spriteSequence

		x: 0
		y: 0
		width: root.width
		height: root.height

		running: true //baseObject && baseObject.scene && baseObject.scene.running

		/*transform: Rotation {
			id: rotation
			origin.x: spriteSequence.width/2
			origin.y: spriteSequence.height/2
			axis.x: 0; axis.y: 1; axis.z: 0
			angle: 0
		}

		state: ""

		states: State {
			name: "inverse"
			//*when: entity && entity.facingLeft != entity.
			PropertyChanges { target: rotation; angle: 180 }
		}

		transitions: Transition {
			NumberAnimation { target: rotation; property: "angle"; duration: 150 }
		}*/


		sprites: []
	}

	/*Glow {
		id: glow
		opacity: _glowForced || (entity && entity.glowEnabled) ? 1.0 : 0.0
		visible: opacity != 0
		color: entity ? entity.glowColor : "white"

		source: spriteSequence
		anchors.fill: spriteSequence

		radius: 4
		samples: 9

		Behavior on opacity {
			NumberAnimation { duration: 200 }
		}

		transform: rotation
	}


	ColorOverlay {
		id: overlay
		source: spriteSequence
		anchors.fill: spriteSequence
		opacity: entity && entity.overlayEnabled ? 1.0 : 0.0
		visible: opacity != 0
		color: entity ? entity.overlayColor : "white"

		Behavior on opacity {
			NumberAnimation { duration: 300 }
		}

		transform: rotation
	}*/


/*
	Timer {
		interval: 1500
		repeat: false
		running: true
		triggeredOnStart: false
		onTriggered: _glowForced = false
	}
*/

	/*Connections {
		target: entity

		function onRayCastPerformed(rect) {
			if (!entity.scene.debugView)
				return

			var k = mapFromItem(entity.scene, rect.x, rect.y)

			rayRect.x = k.x
			rayRect.y = k.y
			rayRect.width = rect.width
			rayRect.height = Math.max(rect.height, 1)
			rayRect.visible = true
			timerOff.start()
		}
	}



	Rectangle {
		id: rayRect
		color: "blue"
		visible: false
		border.width: 1
		border.color: "blue"

		Timer {
			id: timerOff
			interval: 25
			triggeredOnStart: false
			running: false
			repeat: false
			onTriggered: rayRect.visible = false
		}
	}*/



	Component {
		id: componentSprite

		Sprite {  }
	}

	function appendSprite(_data) {
		let obj = componentSprite.createObject(spriteSequence, _data)
		spriteSequence.sprites.push(obj)
	}


}
