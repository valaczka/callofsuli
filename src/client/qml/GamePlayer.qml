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

		property int _fallStartY: -1

		onDie: {
			console.debug("DIE")
			timerPlace.start()
		}

		onIsOnGroundChanged: {
			if (isOnGround) {
				if (_fallStartY == -1) {
					spriteSequence.jumpTo("idle")
				} else {
					if (root.y-_fallStartY > ep.cosGame.gameData.level[ep.cosGame.level-1].deathlyFall || ep.isOnBaseGround) {
						spriteSequence.jumpTo("falldeath")
					} else {
						spriteSequence.jumpTo("idle")
					}
				}
			} else {
				_fallStartY = root.y
				spriteSequence.jumpTo("fall")
			}
		}
	}


	Timer {
		id: timerPlace
		interval: 2000
		running: false
		repeat: false
		triggeredOnStart: false

		onTriggered: {
			ep.cosGame.placePlayer()
			ep.isAlive = true
			spriteSequence.jumpTo("idle")
		}
	}


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
		if(scene.game.gameState != Bacon2D.Running || !ep.isAlive)
			return

		if (isFalling)
			return

		root.facingLeft = false

		if (!root.isWalking)
			spriteSequence.jumpTo("walk")

	}

	function walkLeft() {
		if(scene.game.gameState != Bacon2D.Running || !ep.isAlive)
			return


		if (isFalling)
			return

		root.facingLeft = true

		if (!root.isWalking)
			spriteSequence.jumpTo("walk")
	}


	function runRight() {
		if(scene.game.gameState != Bacon2D.Running || !ep.isAlive)
			return


		if (isFalling)
			return

		root.facingLeft = false

		if (!root.isRunning)
			spriteSequence.jumpTo("run")
	}

	function runLeft() {
		if(scene.game.gameState != Bacon2D.Running || !ep.isAlive)
			return


		if (isFalling)
			return

		root.facingLeft = true

		if (!root.isRunning)
			spriteSequence.jumpTo("run")
	}


	function jump() {
		if(scene.game.gameState != Bacon2D.Running || !ep.isAlive)

		if (isFalling)
			return
	}


}
