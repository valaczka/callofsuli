import Bacon2D 1.0
import QtQuick 2.14

PhysicsEntity {
	id: root
	sleepingAllowed: false
	width: 100
	height: 100
	bodyType: Body.Dynamic
	fixedRotation: true

	enum CharState {
		Idle,
		Spring
	}

	gravityScale: 5

	property int walkElapsed: 0

	readonly property Scene scene: parent
	property bool facingLeft: false
	property bool airborne: false
	property bool running: false
	property bool jumping: false
	readonly property int groundSensorRoom: -7

	property int characterState: TMXplayer.CharState.Idle

	property int hasLadder: 0

	/*var r = cosClient.readJsonFile(":/character1/data.json")
	console.debug(r, r.ablak, r.teszt, r.masik)*/

	property string characterDirName: "qrc:/character/"+"character1"
	property var characterData: cosClient.readJsonFile(characterDirName+"/data.json")

	fixtures: [
		Polygon {
			id: boxBody

			vertices: [
				Qt.point(0,0),
				Qt.point(0,10),
				Qt.point(10,10),
				Qt.point(10,0)
			]

			restitution: 0
			friction: .7
			categories: Box.Category1

			onBeginContact: {
				console.info("LADDER begin", other, other.x, other.y, other.height, other.direction)
				root.hasLadder = other.direction ? other.direction : 0
			}

		},

		// ground sensor
		Box {
			sensor: true
			x: 0
			y: target.height - groundSensorRoom
			width: target.width
			height: 2

			onBeginContact: {
				root.airborne = false
			}

			onEndContact: {
				root.airborne = true
			}
		}

	]


	onRunningChanged: if (running && (rMoveLeftTimer.running || rMoveRightTimer.running))
						  spriteseq.goalSprite="run"
					  else if (!running && (rMoveLeftTimer.running || rMoveRightTimer.running))
						  spriteseq.goalSprite="walk"


	SpriteSequence {
		id: spriteseq
		width: currentSprite ? characterData.sprites[currentSprite].frameWidth : 10
		height: currentSprite ? characterData.sprites[currentSprite].frameHeight : 10

		onCurrentSpriteChanged: {
			if (!currentSprite)
				return

			if (goalSprite == currentSprite)
				goalSprite = ""

			var vertices = characterData.sprites[currentSprite].bodyVertices

			var vlist = []

			for (var i=0; i<vertices.length; i++) {
				var v = vertices[i]
				vlist.push(Qt.point(v.x, v.y))
			}

			if (root.facingLeft)
				boxBody.vertices  = cosClient.rotatePolygon(vlist, 180, Qt.YAxis)
			else
				boxBody.vertices = vlist

		}

		transform: Rotation {
			id: rotation
			origin.x: spriteseq.width/2
			origin.y: spriteseq.height/2
			axis.x: 0; axis.y: 1; axis.z: 0     // set axis.y to 1 to rotate around y-axis
			angle: 0    // the default angle
		}

		states: State {
			name: "back"
			PropertyChanges { target: rotation; angle: 180 }
			when: root.facingLeft
		}

		transitions: Transition {
			NumberAnimation { target: rotation; property: "angle"; duration: 150 }
		}


		Sprite {
			name: "idle"
			source: characterDirName+"/"+characterData.sprites.idle.source
			frameCount: characterData.sprites.idle.frameCount
			frameWidth: characterData.sprites.idle.frameWidth
			frameHeight: characterData.sprites.idle.frameHeight
			frameDuration: characterData.sprites.idle.frameDuration
			frameDurationVariation: 5
			to: {"idle": 1, "walk": 0, "run": 0}
		}

		Sprite {
			name: "walk"
			source: characterDirName+"/"+characterData.sprites.walk.source
			frameCount: characterData.sprites.walk.frameCount
			frameWidth: characterData.sprites.walk.frameWidth
			frameHeight: characterData.sprites.walk.frameHeight
			frameDuration: characterData.sprites.walk.frameDuration
			to: {"idle": 0, "walk": 1, "run": 0}
		}

		Sprite {
			name: "run"
			source: characterDirName+"/"+characterData.sprites.run.source
			frameCount: characterData.sprites.run.frameCount
			frameWidth: characterData.sprites.run.frameWidth
			frameHeight: characterData.sprites.run.frameHeight
			frameDuration: characterData.sprites.walk.frameDuration
			to: {"idle": 0, "walk": 0, "run": 1}
		}

	}


	/*Sprite {
		id: sprite
		animation: "idle"
		anchors.centerIn: parent

		animations: [
			SpriteAnimation {
				name: "idle"
				source: characterDirName+"/"+characterData.idle.source
				frames: characterData.idle.frames
				duration: characterData.idle.duration
				loops: Animation.Infinite
			}

		]
	}*/

	/*
	fixtures: [
		Box {
			x: 15
			width: target.width - 40
			height: target.height - groundSensorRoom
			density: .3
			restitution: 0
			friction: .7
			categories: Box.Category1
		},

		// ground sensor
		Box {
			sensor: true
			x: 305.14.2/gcc_64/qml/Bacon2D.1.0
			y: target.height - groundSensorRoom
			width: 30
			height: 2

			onBeginContact: {
				root.airborne = false
				sprite.animation = "idle"
			}

			onEndContact: {
				root.airborne = true
			}
		}

	]


	Sprite {
		id: sprite
		animation: "idle"
		horizontalMirror: root.facingLeft

		animations: [
			SpriteAnimation {
				name: "idle"
				source: "images/dog_idle.png"
				frames: 10
				duration: 500
				loops: Animation.Infinite
			},

			SpriteAnimation {
				name: "run"
				source: "images/dog_run.png"
				frames: 8
				duration: 500
				loops: Animation.Infinite
				inverse: root.facingLeft
			},

			SpriteAnimation {
				name: "jump"
				source: "images/dog_jump.png"
				frames: 8
				duration: 1000
				loops: 1

				onFinished: {
					root.jumping = false

					if(root.airborne && sprite.animation == name)
						sprite.animation = "freefall"
				}
			},

			SpriteAnimation {
				name: "freefall"
				source: "images/dog_freefall.png"
				frames: 2
				duration: 2000
				loops: Animation.Infinite
				inverse: root.facingLeft
			}
		]
	}
*/

	Timer {
		id: rMoveLeftTimer
		interval: 50
		repeat: true
		triggeredOnStart: true

		onRunningChanged: if (running) {
							  spriteseq.jumpTo(root.running ? "run" : "walk")
						  } else {
							  walkElapsed = 0
							  spriteseq.goalSprite=""
							  spriteseq.jumpTo("idle")
						  }

		onTriggered: {
			walkElapsed += interval
			root.rMoveLeft()
		}
	}

	Timer {
		id: rMoveRightTimer
		interval: 50
		repeat: true
		triggeredOnStart: true

		onRunningChanged: if (running) {
							  spriteseq.jumpTo(root.running ? "run" : "walk")
						  } else {
							  walkElapsed = 0
							  spriteseq.goalSprite=""
							  spriteseq.jumpTo("idle")
						  }

		onTriggered: {
			walkElapsed += interval
			root.rMoveRight()
		}
	}


	function moveLeft() {
		if(scene.game.gameState != Bacon2D.Running)
			return

		rMoveLeftTimer.start()
		rMoveRightTimer.stop()
		root.facingLeft = true

		/*if(root.airborne && sprite.animation != "jump")
			sprite.animation = "freefall"
		else
			sprite.animation = "run" */
	}

	// Repeat move left
	function rMoveLeft() {
		if(scene.game.gameState != Bacon2D.Running)
			return

		root.x -= root.running ? root.characterData.run : root.characterData.walk
		root.facingLeft = true

		/*if(root.airborne && sprite.animation != "jump")
			sprite.animation = "freefall"
		else
			sprite.animation = "run" */
	}

	function stopMovingLeft() {
		if(scene.game.gameState != Bacon2D.Running)
			return

		rMoveLeftTimer.stop()
		root.facingLeft = true
		root.running = false

		/*if(root.airborne && sprite.animation != "jump")
			sprite.animation = "freefall"
		else
			sprite.animation = "idle" */
	}

	function moveRight() {
		root.bodyType = Body.Dynamic
		if(scene.game.gameState != Bacon2D.Running)
			return

		rMoveLeftTimer.stop()
		rMoveRightTimer.start()
		root.facingLeft = false

		/*if(root.airborne && sprite.animation != "jump")
			sprite.animation = "freefall"
		else
			sprite.animation = "run" */
	}

	// Repeat move right
	function rMoveRight() {
		if(scene.game.gameState != Bacon2D.Running)
			return

		root.x += root.running ? root.characterData.run : root.characterData.walk
		root.facingLeft = false

		/*if(root.airborne && sprite.animation != "jump")
			sprite.animation = "freefall"
		else
			sprite.animation = "run" */
	}

	function stopMovingRight() {
		if(scene.game.gameState != Bacon2D.Running)
			return

		rMoveRightTimer.stop()
		root.facingLeft = false
		root.running = false

		/*if(root.airborne && sprite.animation != "jump")
			sprite.animation = "freefall"
		else
			sprite.animation = "idle" */
	}

	function jump() {
		if(scene.game.gameState != Bacon2D.Running)
			return

		if (root.hasLadder != 0)
			root.y += hasLadder

		/*root.bodyType = Body.Static

		root.y-=50 */

		/*		if(root.airborne)
			return

		if(sprite.animation == "idle" || sprite.animation == "run")
			sprite.animation = "jump"
		else
			return

		root.applyLinearImpulse(Qt.point(0, -root.getMass() * 10), root.getWorldCenter()); */
	}


}

