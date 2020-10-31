import Bacon2D 1.0
import QtQuick 2.14
import COS.Client 1.0
import QtGraphicalEffects 1.0

PhysicsEntity {
	id: root
	sleepingAllowed: false
	width: spriteSequence.width
	height: spriteSequence.height
	bodyType: Body.Dynamic
	fixedRotation: true

	gravityScale: 5

	property GameEntityPrivate entityPrivate: null
	property alias spriteSequence: spriteSequence

	property bool facingLeft: false

	property alias glowColor: glow.color
	property bool glowEnabled: false


	readonly property bool isInverse: entityPrivate && entityPrivate.qrcData ?
										  ((facingLeft && !entityPrivate.qrcData.facingLeft) || (!facingLeft && entityPrivate.qrcData.facingLeft)) :
										  false

	readonly property bool isWalking: Array("walk", "walk2", "walk3", "walk4", "walk5").includes(spriteSequence.currentSprite)
	readonly property bool isRunning: Array("run", "run2", "run3", "run4", "run5", "runend").includes(spriteSequence.currentSprite)
	readonly property bool isFalling: Array("fall", "fall2", "fall3", "fall4", "fall5", "fallend", "falldeath", "falldeath2").includes(spriteSequence.currentSprite)


	onWidthChanged: if (entityPrivate) entityPrivate.updateFixtures(spriteSequence.currentSprite, isInverse)
	onHeightChanged: if (entityPrivate) entityPrivate.updateFixtures(spriteSequence.currentSprite, isInverse)




	SpriteSequence {
		id: spriteSequence
		width: entityPrivate && currentSprite ? entityPrivate.qrcData.sprites[currentSprite].frameWidth : 10
		height: entityPrivate && currentSprite ? entityPrivate.qrcData.sprites[currentSprite].frameHeight : 10

		anchors.horizontalCenter: root.horizontalCenter
		anchors.bottom: root.bottom

		onCurrentSpriteChanged: if (entityPrivate) {
									entityPrivate.updateFixtures(currentSprite, isInverse)

									if (Array("dead", "dead2", "dead3", "dead4", "falldeath", "falldeath2", "falldead").includes(currentSprite))
										entityPrivate.isAlive = false
								}

		transform: Rotation {
			id: rotation
			origin.x: spriteSequence.width/2
			origin.y: spriteSequence.height/2
			axis.x: 0; axis.y: 1; axis.z: 0
			angle: 0
		}

		states: State {
			name: "back"
			PropertyChanges { target: rotation; angle: 180 }
			when: root.isInverse
		}

		transitions: Transition {
			NumberAnimation { target: rotation; property: "angle"; duration: 150 }
		}

		sprites: []

	}

	Glow {
		id: glow
		opacity: glowEnabled ? 1.0 : 0.0
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


	Component {
		id: spriteComponent

		Sprite {  }
	}


	Connections {
		target: entityPrivate ? entityPrivate : null
		onDie: {
			console.debug(root, "DIED")
			root.destroy()
		}
	}



	function loadSprites() {
		if (!entityPrivate || !entityPrivate.qrcData || !entityPrivate.qrcData.sprites)
			return

		var sprites = Object.keys(entityPrivate.qrcData.sprites)

		for (var i=-1; i<sprites.length; i++) {
			var sname = (i === -1) ? "idle" : sprites[i]

			if (sname === "idle" && i !== -1)
				continue

			var sdata = entityPrivate.qrcData.sprites[sname]
			if (!sdata)
				continue


			var obj = spriteComponent.createObject(spriteSequence)
			obj.name = sname
			obj.source = entityPrivate.qrcDirName+"/"+sdata.source
			obj.frameCount = sdata.frameCount ? sdata.frameCount : 1
			obj.frameWidth = sdata.frameWidth ? sdata.frameWidth : 1
			obj.frameHeight = sdata.frameHeight ? sdata.frameHeight : 1
			obj.frameX = sdata.frameX ? sdata.frameX : 0
			obj.frameY = sdata.frameY ? sdata.frameY : 0
			obj.frameDuration = sdata.frameDuration ? sdata.frameDuration : 100
			obj.frameDurationVariation = sdata.frameDurationVariation ? sdata.frameDurationVariation : 0
			obj.to = sdata.to ? sdata.to : {}
			spriteSequence.sprites.push(obj)
		}
	}


}

