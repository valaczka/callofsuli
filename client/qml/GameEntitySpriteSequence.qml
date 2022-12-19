import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import CallOfSuli 1.0


Item {
	id: control

	property GameEntityPrivate entity: null

	property bool _glowForced: true

	anchors.fill: parent

	property alias spriteSequence: spriteSequence

	SpriteSequence {
		id: spriteSequence
		anchors.centerIn: parent

		transform: Rotation {
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
		}


		sprites: []
	}

	Glow {
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
		source: control
		anchors.fill: control
		opacity: entity && entity.overlayEnabled ? 1.0 : 0.0
		visible: opacity != 0
		color: entity ? entity.overlayColor : "white"

		Behavior on opacity {
			NumberAnimation { duration: 300 }
		}

		transform: rotation
	}



	ProgressBar {
		id: hpProgress
		visible: entity && entity.hpProgressEnabled && !entity.scene.zoomOverview

		width: entity ? entity.bodyRect.width+6 : 30
		x: entity ? entity.bodyRect.x-3 : 0
		y: entity ? entity.bodyRect.y-10 : -5
		height: 2

		from: 0
		to: entity ? Math.max(entity.maxHp, entity.hp, 1) : 0
		value: entity ? entity.hp : 0

		Behavior on value {
			NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
		}

		background: Rectangle {
			implicitWidth: 200
			implicitHeight: 2
			color: entity ? entity.hpProgressColor : "red"
			radius: 0
			opacity: 0.3
		}

		contentItem: Item {
			implicitWidth: 200
			implicitHeight: 2

			Rectangle {
				width: hpProgress.visualPosition * parent.width
				height: parent.height
				radius: 0
				color: entity ? entity.hpProgressColor : "red"
			}
		}
	}


	Timer {
		interval: 1500
		repeat: false
		running: true
		triggeredOnStart: false
		onTriggered: _glowForced = false
	}


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

	function addToSprites(sdata) {
		var obj = componentSprite.createObject(spriteSequence, sdata)

		spriteSequence.sprites.push(obj)
	}


}
