import Bacon2D 1.0
import QtQuick 2.14
import COS.Client 1.0

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


	readonly property bool isInverse: entityPrivate && entityPrivate.qrcData ?
										  ((facingLeft && !entityPrivate.qrcData.facingLeft) || (!facingLeft && entityPrivate.qrcData.facingLeft)) :
										  false

	readonly property bool isWalking: spriteSequence.currentSprite == "walk" || spriteSequence.currentSprite == "walk2" || spriteSequence.currentSprite == "walk3"
									  || spriteSequence.currentSprite == "walk4" || spriteSequence.currentSprite == "walk5"

	readonly property bool isRunning: spriteSequence.currentSprite == "run" || spriteSequence.currentSprite == "run2" || spriteSequence.currentSprite == "run3"
									  || spriteSequence.currentSprite == "run4" || spriteSequence.currentSprite == "run5"
									  || spriteSequence.currentSprite == "runend"

	readonly property bool isFalling: spriteSequence.currentSprite == "fall" || spriteSequence.currentSprite == "fall2" || spriteSequence.currentSprite == "fall3"
									  || spriteSequence.currentSprite == "fall4" || spriteSequence.currentSprite == "fall5"
									  || spriteSequence.currentSprite == "fallend"

	readonly property bool isDead: spriteSequence.currentSprite == "dead" || spriteSequence.currentSprite == "dead2" || spriteSequence.currentSprite == "dead3"
								   || spriteSequence.currentSprite == "dead4" || spriteSequence.currentSprite == "dead5"
								   || spriteSequence.currentSprite == "falldeath" || spriteSequence.currentSprite == "falldead"







	SpriteSequence {
		id: spriteSequence
		width: entityPrivate && currentSprite ? entityPrivate.qrcData.sprites[currentSprite].frameWidth : 10
		height: entityPrivate && currentSprite ? entityPrivate.qrcData.sprites[currentSprite].frameHeight : 10

		running: entityPrivate && entityPrivate.qrcData

		onCurrentSpriteChanged: if (entityPrivate.bodyPolygon)
									entityPrivate.setFixtureVertices(currentSprite, isInverse)

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

	Component {
		id: spriteComponent

		Sprite {  }
	}

	Behavior on x {
		enabled: isWalking
		NumberAnimation {
			duration: 50
			easing.type: Easing.InOutQuart
		}
	}

	Behavior on x {
		enabled: isRunning
		NumberAnimation {
			duration: 50
			easing.type: Easing.Linear
		}
	}


	Connections {
		target: entityPrivate
		onQrcDataChanged: {
			loadSprites()
		}
	}


	function loadSprites() {
		var searchSprites = [ "idle", "idle2", "idle3", "idle4", "idle5",
							 "walk", "walk2", "walk3", "walk4", "walk5",
							 "run", "run2", "run3", "run4", "run5", "runend",
							 "fall", "fall2", "fall3", "fall4", "fall5", "fallend", "falldeath", "falldeath2",
							 "jump",
							 "dead", "dead2", "dead3", "dead4", "falldead"
				]

		for (var i=0; i<searchSprites.length; i++) {
			var s = searchSprites[i]
			var sdata = entityPrivate.qrcData.sprites[s]
			if (!sdata)
				continue

			var obj = spriteComponent.createObject(spriteSequence)
			obj.name = s
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

