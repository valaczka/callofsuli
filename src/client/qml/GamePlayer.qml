import Bacon2D 1.0
import QtQuick 2.14
import COS.Client 1.0

GameEntity {
	id: root
	sleepingAllowed: false
	width: spriteSequence.width
	height: spriteSequence.height

	entityPrivate: ep

	GamePlayerPrivate {
		id: ep
		bodyPolygon: bodyBox
	}

	property var contactToGround: []

	fixtures: [

		Box {
			id: boundBox

			width: ep.qrcData ? ep.qrcData.width : 0
			height: ep.qrcData ? ep.qrcData.height : 0
			x: ep.qrcData ?
				   (root.isInverse ? root.width-width-ep.qrcData.x : ep.qrcData.x) :
				   0
			y: ep.qrcData ? ep.qrcData.y : 0

			property int _fallStartY: -1

			restitution: 0
			friction: 1
			categories: Box.Category1

			collidesWith: Box.Category1

			onEndContact: {
				contactToGround.splice(contactToGround.indexOf(other), 1)
				if (contactToGround.length == 0) {
					_fallStartY = root.y
					spriteSequence.jumpTo("fall")
				}
			}

			onBeginContact: {
				contactToGround.push(other)
				if (_fallStartY == -1) {
					spriteSequence.jumpTo("idle")
				} else {
					if (root.y-_fallStartY > ep.cosGame.gameData.level[ep.cosGame.level-1].deathlyFall) {
						spriteSequence.jumpTo("falldeath")
					} else {
						spriteSequence.jumpTo("idle")
					}
				}
				_fallStartY = -1
			}
		},

		Polygon {
			id: bodyBox

			sensor: true

			vertices: [
				Qt.point(boundBox.x, boundBox.y),
				Qt.point(boundBox.x+boundBox.width, boundBox.y),
				Qt.point(boundBox.x+boundBox.width, boundBox.y+boundBox.height),
				Qt.point(boundBox.x,boundBox.y+boundBox.height)
			]

			//restitution: 0
			//friction: .7
			categories: Box.Category2
		}

	]


	Timer {
		id: timerWalk
		interval: 30
		repeat: true

		property bool readyToStop: false

		running: root.isWalking

		triggeredOnStart: true
		onTriggered: {
			if(readyToStop) {
				spriteSequence.jumpTo("idle")
			}

			if (root.facingLeft)
				root.x -= ep.qrcData.walk
			else
				root.x += ep.qrcData.walk
		}

		onRunningChanged: if (running) {
							  readyToStop = false
						  }

	}


	Timer {
		id: timerRun
		interval: 30
		repeat: true

		property bool readyToStop: false

		running: root.isRunning

		triggeredOnStart: true
		onTriggered: {
			if(readyToStop) {
				spriteSequence.jumpTo("runend")
			}

			if (root.facingLeft)
				root.x -= ep.qrcData.run
			else
				root.x += ep.qrcData.run
		}

		onRunningChanged: if (running)
							  readyToStop = false

	}

	function stopMoving() {
		timerWalk.readyToStop = true
		timerRun.readyToStop = true
	}



	function walkRight() {
		if(scene.game.gameState != Bacon2D.Running)
			return

		if (isDead)
			return

		root.facingLeft = false

		if (isFalling)
			return

		if (!root.isWalking)
			spriteSequence.jumpTo("walk")

	}

	function walkLeft() {
		if(scene.game.gameState != Bacon2D.Running)
			return

		if (isDead)
			return

		root.facingLeft = true

		if (isFalling)
			return

		if (!root.isWalking)
			spriteSequence.jumpTo("walk")
	}


	function runRight() {
		if(scene.game.gameState != Bacon2D.Running)
			return

		if (isDead)
			return

		root.facingLeft = false

		if (isFalling)
			return

		if (!root.isRunning)
			spriteSequence.jumpTo("run")
	}

	function runLeft() {
		if(scene.game.gameState != Bacon2D.Running)
			return

		if (isDead)
			return

		root.facingLeft = true

		if (isFalling)
			return

		if (!root.isRunning)
			spriteSequence.jumpTo("run")
	}


	function jump() {
		if(scene.game.gameState != Bacon2D.Running)
			return

		if (isDead)
			return

		if (isFalling)
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
