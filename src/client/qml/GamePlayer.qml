import Bacon2D 1.0
import QtQuick 2.14
import COS.Client 1.0
import "Style"

GameEntity {
	id: root
	sleepingAllowed: false
	width: spriteSequence.width
	height: spriteSequence.height

	z: 10

	entityPrivate: ep

	glowColor: CosStyle.colorPrimaryLighter

	GamePlayerPrivate {
		id: ep

		property bool wantToMove: false
		property bool gunOn: false
		readonly property bool gunPreparing: Array("gunon", "idlegun", "shot").includes(spriteSequence.currentSprite)

		hasGun: Array("idlegun", "shot").includes(spriteSequence.currentSprite)

		onGunOnChanged: {
			console.debug("GUN ON", gunOn)
			if (spriteSequence.currentSprite == "idle" && ep.gunOn && !ep.wantToMove)
				spriteSequence.jumpTo("gunon")
			else if (gunPreparing && !ep.gunOn)
				spriteSequence.jumpTo("gunoff")
		}

		rayCastEnabled: ladderMode != GamePlayerPrivate.LadderClimb && ladderMode != GamePlayerPrivate.LadderClimbFinish

		property int _fallStartY: -1

		onKilledByEnemy: {
			spriteSequence.jumpTo("dead")
		}

		onAttack: {
			spriteSequence.jumpTo("shot")
		}

		onIsOnGroundChanged: {
			if (isOnGround) {
				if (_fallStartY == -1) {
					ep.wantToMove = false
					spriteSequence.jumpTo("idle")
				} else {
					ep.wantToMove = false
					if (root.y-_fallStartY > ep.cosGame.levelData.player.deathlyFall || ep.isOnBaseGround) {
						spriteSequence.jumpTo("falldeath")
					} else {
						spriteSequence.jumpTo("idle")
					}
				}
				_fallStartY = -1
			} else if (ladderMode != GamePlayerPrivate.LadderClimb && ladderMode != GamePlayerPrivate.LadderClimbFinish) {
				_fallStartY = root.y
				spriteSequence.jumpTo("fall")
				ep.wantToMove = false
			}
		}

		onLadderModeChanged: {
			if (ladderMode == GamePlayerPrivate.LadderClimb) {
				root.bodyType = Body.Kinematic
			} else if (ladderMode == GamePlayerPrivate.LadderUnavaliable) {
				root.bodyType = Body.Dynamic
			}
		}

		//onRayCastPerformed: setray(rect)
	}

	function setray(rect) {
		var k = mapFromItem(scene, rect.x, rect.y)

		rayRect.x = k.x
		rayRect.y = k.y
		rayRect.width = rect.width
		rayRect.height = Math.max(rect.height, 1)
		rayRect.visible = true
		timerOff.start()

	}

	Rectangle {
		id: rayRect
		color: "blue"
		visible: false
		border.width: 1
		border.color: "blue"

		Timer {
			id: timerOff
			interval: 200
			triggeredOnStart: false
			running: false
			repeat: false
			onTriggered: rayRect.visible = false
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
			if(readyToStop || !ep.cosGame.running) {
				spriteSequence.jumpTo("idle")
			}

			if (ep.cosGame.running) {
				if (root.facingLeft)
					root.x -= ep.qrcData.walk
				else
					root.x += ep.qrcData.walk
			}
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
			if(readyToStop || !ep.cosGame.running) {
				spriteSequence.jumpTo("runend")
			}

			if (ep.cosGame.running) {
				if (root.facingLeft)
					root.x -= ep.qrcData.run
				else
					root.x += ep.qrcData.run
			}
		}

		onRunningChanged: if (running)
							  readyToStop = false

	}

	Timer {
		id: timerClimb
		interval: 60
		repeat: true

		property string nextSprite: ""

		triggeredOnStart: true

		running: ep.cosGame.running &&
				 Array("climbup", "climbup2", "climbup3", "climbupend",
					   "climbdown", "climbdown2", "climbdown3", "climbdownend").includes(spriteSequence.currentSprite)

		onTriggered: {
			if (Array("climbdown", "climbdown2", "climbdown3", "climbdownend").includes(spriteSequence.currentSprite)) {
				ep.ladderClimbDown()
			} else {
				ep.ladderClimbUp()
			}

			if (ep.ladderMode == GamePlayerPrivate.LadderClimb && nextSprite != "" &&
					spriteSequence.currentSprite != "climbup" && spriteSequence.currentSprite != "climbdown") {
				spriteSequence.jumpTo(nextSprite)
				nextSprite = ""
			}
		}
	}



	spriteSequence.onCurrentSpriteChanged: {
		if (ep.ladderMode == GamePlayerPrivate.LadderClimbFinish && spriteSequence.currentSprite == "idle") {
			ep.ladderClimbFinish()
		}

		console.debug("currentSprite changed", spriteSequence.currentSprite, ep.gunOn, ep.hasGun, ep.wantToMove)

		if (spriteSequence.currentSprite == "idle") {
			if (ep.gunOn && !ep.wantToMove)
				spriteSequence.jumpTo("gunon")
		}
	}

	function stopMoving() {
		timerWalk.readyToStop = true
		timerRun.readyToStop = true
		timerClimb.nextSprite = "climbpause"
		ep.wantToMove = false
	}


	function turnRight() {
		if(!ep.cosGame.running || !ep.isAlive)
			return

		if (isFalling ||
				ep.ladderMode == GamePlayerPrivate.LadderClimb ||
				ep.ladderMode == GamePlayerPrivate.LadderClimbFinish)
			return

		root.facingLeft = false
	}


	function turnLeft() {
		if(!ep.cosGame.running || !ep.isAlive)
			return


		if (isFalling ||
				ep.ladderMode == GamePlayerPrivate.LadderClimb ||
				ep.ladderMode == GamePlayerPrivate.LadderClimbFinish)
			return

		root.facingLeft = true
	}


	function walkRight() {
		if(!ep.cosGame.running || !ep.isAlive)
			return

		if (isFalling ||
				ep.ladderMode == GamePlayerPrivate.LadderClimb ||
				ep.ladderMode == GamePlayerPrivate.LadderClimbFinish)
			return

		root.facingLeft = false

		ep.wantToMove = true

		if (!root.isWalking)
			spriteSequence.jumpTo("walk")

	}

	function walkLeft() {
		if(!ep.cosGame.running || !ep.isAlive)
			return


		if (isFalling ||
				ep.ladderMode == GamePlayerPrivate.LadderClimb ||
				ep.ladderMode == GamePlayerPrivate.LadderClimbFinish)
			return

		root.facingLeft = true

		ep.wantToMove = true

		if (!root.isWalking)
			spriteSequence.jumpTo("walk")
	}


	function runRight() {
		if(!ep.cosGame.running || !ep.isAlive)
			return

		if (isFalling ||
				ep.ladderMode == GamePlayerPrivate.LadderClimb ||
				ep.ladderMode == GamePlayerPrivate.LadderClimbFinish)
			return

		root.facingLeft = false

		if (!root.isRunning)
			spriteSequence.jumpTo("run")
	}

	function runLeft() {
		if(!ep.cosGame.running || !ep.isAlive)
			return


		if (isFalling ||
				ep.ladderMode == GamePlayerPrivate.LadderClimb ||
				ep.ladderMode == GamePlayerPrivate.LadderClimbFinish)
			return

		root.facingLeft = true

		if (!root.isRunning)
			spriteSequence.jumpTo("run")
	}


	function moveUp() {
		if(!ep.cosGame.running || !ep.isAlive)
			return

		if (isFalling)
			return

		if (ep.ladderMode == GamePlayerPrivate.LadderUpAvailable) {
			timerClimb.nextSprite = ""
			ep.ladderClimbUp()
		} else if (ep.ladderMode == GamePlayerPrivate.LadderClimb) {
			timerClimb.nextSprite = ""
			if (spriteSequence.currentSprite != "climbup2" && spriteSequence.currentSprite != "climbup3" && spriteSequence.currentSprite != "climbupend")
				spriteSequence.jumpTo("climbup2")
		}

	}


	function moveDown() {
		if(!ep.cosGame.running || !ep.isAlive)
			return

		if (isFalling)
			return

		if (ep.ladderMode == GamePlayerPrivate.LadderDownAvailable) {
			timerClimb.nextSprite = ""
			ep.ladderClimbDown()
		} else if (ep.ladderMode == GamePlayerPrivate.LadderClimb) {
			timerClimb.nextSprite = ""
			if (spriteSequence.currentSprite != "climbdown2" && spriteSequence.currentSprite != "climbdown3" && spriteSequence.currentSprite != "climbdownend" )
				spriteSequence.jumpTo("climbdown2")
		}
	}


}
