import Bacon2D 1.0
import QtQuick 2.14

PhysicsEntity {
	id: root
	sleepingAllowed: false
	width: spriteseq.width
	height: spriteseq.height
	bodyType: Body.Dynamic
	fixedRotation: true

	gravityScale: 5

	readonly property Scene scene: parent
	property bool facingLeft: false

	property string characterDirName: "qrc:/character/"+"character2"
	readonly property var characterData: cosClient.readJsonFile(characterDirName+"/data.json")
	readonly property var gameData: cosClient.readJsonFile("qrc:/internal/game/parameters.json")

	property int gameLevel: 1

	property int boundLeft: 0
	property int boundRight: 0


	onCharacterDataChanged: loadSprites()

	readonly property bool isInverse: (facingLeft && !characterData.facingLeft) || (!facingLeft && characterData.facingLeft)


	fixtures: [

		Box {
			id: boundBox

			width: characterData.width
			height: characterData.height
			x: root.isInverse ? root.width-width-characterData.x : characterData.x
			y: characterData.y


			property int _fallStartY: -1

			restitution: 0
			friction: 1
			categories: Box.Category1

			collidesWith: Box.Category1

		},

		Polygon {
			id: bodyBox

			vertices: [
				Qt.point(boundBox.x, boundBox.y),
				Qt.point(boundBox.x+boundBox.width, boundBox.y),
				Qt.point(boundBox.x+boundBox.width, boundBox.y+boundBox.height),
				Qt.point(boundBox.x,boundBox.y+boundBox.height)
			]

			//restitution: 0
			//friction: .7
			categories: Box.Category2

			onBeginContact: {
			}

			onEndContact: {
			}

		}

	]



	SpriteSequence {
		id: spriteseq
		width: currentSprite ? characterData.sprites[currentSprite].frameWidth : 10
		height: currentSprite ? characterData.sprites[currentSprite].frameHeight : 10

		onCurrentSpriteChanged: {
			if (!currentSprite)
				return

			var vertices = characterData.sprites[currentSprite].bodyVertices

			if (!vertices)
				return

			var vlist = []

			for (var i=0; i<vertices.length; i++) {
				var v = vertices[i]
				vlist.push(Qt.point(v.x, v.y))
			}

			if (isInverse)
				bodyBox.vertices  = cosClient.rotatePolygon(vlist, 180, Qt.YAxis)
			else
				bodyBox.vertices = vlist

		}

		transform: Rotation {
			id: rotation
			origin.x: spriteseq.width/2
			origin.y: spriteseq.height/2
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


	Timer {
		id: timerWalk
		interval: 60
		repeat: true

		triggeredOnStart: true
		onTriggered: {
			if(scene.game.gameState != Bacon2D.Running)
				return

			var newx = root.x

			if (root.facingLeft)
				newx -= root.characterData.walk
			else
				newx += root.characterData.walk

			if (newx < boundLeft) {
				root.x = boundLeft
				stop()
				timerTurn.start()
			} else if (newx > boundRight)  {
				root.x = boundRight
				stop()
				timerTurn.start()
			} else {

				root.x = newx
			}

		}
	}


	Timer {
		id: timerTurn
		interval: 2000
		repeat: false

		triggeredOnStart: false

		onTriggered: {
			if(scene.game.gameState != Bacon2D.Running)
				return

			root.facingLeft = !root.facingLeft
			timerWalk.start()
		}
	}






	Behavior on x {
		enabled: timerWalk.running
		NumberAnimation {
			duration: 50
			easing.type: Easing.InOutQuart
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
			var sdata = characterData.sprites[s]
			if (!sdata)
				continue

			var obj = spriteComponent.createObject(spriteseq)
			obj.name = s
			obj.source = characterDirName+"/"+sdata.source
			obj.frameCount = sdata.frameCount ? sdata.frameCount : 1
			obj.frameWidth = sdata.frameWidth ? sdata.frameWidth : 1
			obj.frameHeight = sdata.frameHeight ? sdata.frameHeight : 1
			obj.frameX = sdata.frameX ? sdata.frameX : 0
			obj.frameY = sdata.frameY ? sdata.frameY : 0
			obj.frameDuration = sdata.frameDuration ? sdata.frameDuration : 100
			obj.frameDurationVariation = sdata.frameDurationVariation ? sdata.frameDurationVariation : 0
			obj.to = sdata.to ? sdata.to : {}
			spriteseq.sprites.push(obj)
		}
	}


	Component.onCompleted: timerWalk.start()

}

