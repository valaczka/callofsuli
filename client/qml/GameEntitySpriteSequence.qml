import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0


SpriteSequence {
	id: control


	//running: entityPrivate && entityPrivate.cosGame.running

	/*onCurrentSpriteChanged: if (entityPrivate) {
								var d = entityPrivate.qrcData.sprites[currentSprite]
								spriteSequence.width = d.frameWidth
								spriteSequence.height = d.frameHeight
								entityPrivate.updateFixtures(currentSprite, isInverse)

								if (Array("dead", "dead2", "dead3", "dead4", "falldeath", "falldeath2", "falldead", "burn", "burndead").includes(currentSprite))
									entityPrivate.isAlive = false
							} else {
								spriteSequence.width = 35
								spriteSequence.height = 35
							}*/


	transform: Rotation {
		id: rotation
		origin.x: control.width/2
		origin.y: control.height/2
		axis.x: 0; axis.y: 1; axis.z: 0
		angle: 0
	}

	state: ""

	states: State {
		name: "back"
		PropertyChanges { target: rotation; angle: 180 }
	}

	transitions: Transition {
		NumberAnimation { target: rotation; property: "angle"; duration: 150 }
	}

	sprites: []

	/*

	Glow {
		id: glow
		opacity: glowEnabled || _glowForced ? 1.0 : 0.0
		visible: opacity != 0

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
		opacity: overlayEnabled ? 1.0 : 0.0
		visible: opacity != 0

		Behavior on opacity {
			NumberAnimation { duration: 300 }
		}

		transform: rotation
	}




	Timer {
		id: glowForcedDelay
		interval: 1500
		repeat: false
		triggeredOnStart: false
		onTriggered: _glowForced = false
	}

*/
	Component.onCompleted: {
		console.debug("**** CREATED", control)
		//glowForcedDelay.start()
	}


	Component.onDestruction: {
		console.debug("**** DESTROY", control)
	}



}
