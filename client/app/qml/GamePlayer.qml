import Bacon2D 1.0
import QtQuick 2.15
import COS.Client 1.0
import QtMultimedia 5.15
import "Style"

GameEntity {
	id: root
	sleepingAllowed: false
	width: spriteSequence.width
	height: spriteSequence.height

	z: 10

	entityPrivate: ep

	glowColor: CosStyle.colorPrimaryLighter


	hpColor: ep.shield ? CosStyle.colorOK : CosStyle.colorErrorLighter
	hpVisible: ep.enemy || hpVisibleTimer.running
	hpValue: ep.shield ? ep.shield : ep.hp


	readonly property bool isWalking: Array("walk", "walk2", "walk3", "walk4", "walk5").includes(spriteSequence.currentSprite)
	readonly property bool isRunning: Array("run", "run2", "run3", "run4", "run5", "runend").includes(spriteSequence.currentSprite)
	readonly property bool isFalling: ep._fallStartY != -1 || Array("fall", "fall2", "fall3", "fall4", "fall5", "fallend", "falldeath", "falldeath2").includes(spriteSequence.currentSprite)
	readonly property bool isClimbing: Array("climbup", "climbup2", "climbup3",
											 "climbdown", "climbdown2", "climbdown3").includes(spriteSequence.currentSprite)

	readonly property bool isOperating: Array("operate", "operate2", "operate3").includes(spriteSequence.currentSprite)
	readonly property bool isBurning: Array("burn", "burndead").includes(spriteSequence.currentSprite)

	SoundEffect {
		id: shotEffect
		source: "qrc:/sound/sfx/shot.wav"
		volume: cosClient.sfxVolume
	}

	GamePlayerPrivate {
		id: ep

		rayCastEnabled: ladderMode != GamePlayerPrivate.LadderClimb && ladderMode != GamePlayerPrivate.LadderClimbFinish

		property int _fallStartY: -1

		onShieldChanged: {
			if (shield < 1) {
				root.hpProgress.to = Math.max(ep.defaultHp, ep.hp)
			}
		}

		onKilledByEnemy: {
			ep.moveToPoint = Qt.point(0,0)
			spriteSequence.jumpTo("dead")
		}

		onAttack: {
			ep.moveToPoint = Qt.point(0,0)
			shotEffect.play()
			spriteSequence.jumpTo("shot")
		}

		onUnderAttack: {
			hpVisibleTimer.restart()
		}

		onHurt: {
			playPainTimer.start()
		}

		onIsOnGroundChanged: {
			console.debug("ISONGROUND", isOnGround)
			if (!isBurning) {
				if (isOnGround) {
					if (_fallStartY == -1) {
						spriteToIdle()
					} else {
						if (root.y-_fallStartY > ep.cosGame.levelData.player.deathlyFall || ep.isOnBaseGround) {
							spriteSequence.jumpTo("falldeath")
							ep.diedByFall()
						} else if (root.y-_fallStartY > ep.cosGame.levelData.player.hurtFall) {
							hurtByEnemy(null)
							spriteToIdle()
						} else {
							spriteToIdle()
						}
					}
					_fallStartY = -1
				} else if (ladderMode != GamePlayerPrivate.LadderClimb && ladderMode != GamePlayerPrivate.LadderClimbFinish) {
					_fallStartY = root.y
					spriteSequence.jumpTo("fall")
				}
			}
		}

		onLadderModeChanged: {
			if (ladderMode == GamePlayerPrivate.LadderClimb) {
				root.bodyType = Body.Kinematic
			} else if (ladderMode == GamePlayerPrivate.LadderUnavaliable) {
				root.bodyType = Body.Dynamic
			}
		}

		/*onRayCastPerformed: {
			if (cosGame.gameScene.debug)
				setray(rect)
		}*/

		onDiedByBurn: {
			ep.moveToPoint = Qt.point(0,0)
			spriteSequence.jumpTo("burn")
		}

		onAutoMoveWalkRequest: {
			root.facingLeft = moveLeft

			if (!root.isWalking) {
				spriteSequence.jumpTo("walk")
			}
		}

		onOperateRequest: {
			timerWalk.stop()
			timerRun.stop()
			spriteSequence.jumpTo("operate")
		}
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
		readonly property real _walk: ep && ep.qrcData ? ep.qrcData.walk : 0

		running: root.isWalking

		triggeredOnStart: true
		onTriggered: {
			var r = ep.cosGame.running
			if(readyToStop || !r) {
				spriteToIdle()
			}

			if (r) {
				if (root.facingLeft)
					root.x -= _walk
				else
					root.x += _walk

				ep.autoMove(Qt.point(root.x, root.y))
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

		readonly property real _run: ep && ep.qrcData ? ep.qrcData.run : 0

		property bool readyToStop: false

		running: root.isRunning

		triggeredOnStart: true
		onTriggered: {
			var r = ep.cosGame.running
			if(readyToStop || !r) {
				spriteSequence.jumpTo("runend")
			}

			if (r) {
				if (root.facingLeft)
					root.x -= _run
				else
					root.x += _run
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




	Timer {
		id: timerClimbSound
		interval: 1700
		repeat: true

		triggeredOnStart: true

		running: isClimbing

		onTriggered: cosClient.playSound(ep.playSoundEffect("climb"), CosSound.PlayerVoice)
	}


	Timer {
		id: timerWalkSound
		interval: 400
		repeat: true

		triggeredOnStart: true

		running: isWalking

		onTriggered: cosClient.playSound(ep.playSoundEffect("walk"), CosSound.PlayerSfx)
	}

	Timer {
		id: timerRunSound
		interval: 300
		repeat: true

		triggeredOnStart: true

		running: isRunning

		onTriggered: cosClient.playSound(ep.playSoundEffect("run"), CosSound.PlayerSfx)
	}



	spriteSequence.onCurrentSpriteChanged: {
		if (ep.ladderMode == GamePlayerPrivate.LadderClimbFinish && spriteSequence.currentSprite == "idle") {
			ep.ladderClimbFinish()
		}

		if (spriteSequence.currentSprite == "climbupend" || spriteSequence.currentSprite == "climbdownend" ) {
			cosClient.playSound(ep.playSoundEffect("ladder"), CosSound.PlayerVoice)
		}
	}



	function stopMoving() {
		ep.moveToPoint = Qt.point(0,0)
		timerWalk.readyToStop = true
		timerRun.readyToStop = true
		timerClimb.nextSprite = "climbpause"
	}


	function turnRight() {
		if(!ep.cosGame.running || !ep.isAlive)
			return

		ep.moveToPoint = Qt.point(0,0)

		if (isFalling || isOperating ||
				ep.ladderMode == GamePlayerPrivate.LadderClimb ||
				ep.ladderMode == GamePlayerPrivate.LadderClimbFinish)
			return

		root.facingLeft = false
	}


	function turnLeft() {
		if(!ep.cosGame.running || !ep.isAlive)
			return

		ep.moveToPoint = Qt.point(0,0)

		if (isFalling || isOperating ||
				ep.ladderMode == GamePlayerPrivate.LadderClimb ||
				ep.ladderMode == GamePlayerPrivate.LadderClimbFinish)
			return

		root.facingLeft = true
	}


	function walkRight() {
		if(!ep.cosGame.running || !ep.isAlive)
			return

		ep.moveToPoint = Qt.point(0,0)

		if (isFalling || isOperating ||
				ep.ladderMode == GamePlayerPrivate.LadderClimb ||
				ep.ladderMode == GamePlayerPrivate.LadderClimbFinish)
			return

		root.facingLeft = false

		if (!root.isWalking)
			spriteSequence.jumpTo("walk")
	}

	function walkLeft() {
		if(!ep.cosGame.running || !ep.isAlive)
			return

		ep.moveToPoint = Qt.point(0,0)

		if (isFalling || isOperating ||
				ep.ladderMode == GamePlayerPrivate.LadderClimb ||
				ep.ladderMode == GamePlayerPrivate.LadderClimbFinish)
			return

		root.facingLeft = true

		if (!root.isWalking)
			spriteSequence.jumpTo("walk")
	}


	function runRight() {
		if(!ep.cosGame.running || !ep.isAlive)
			return

		ep.moveToPoint = Qt.point(0,0)

		if (isFalling || isOperating ||
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

		ep.moveToPoint = Qt.point(0,0)

		if (isFalling || isOperating ||
				ep.ladderMode == GamePlayerPrivate.LadderClimb ||
				ep.ladderMode == GamePlayerPrivate.LadderClimbFinish)
			return

		root.facingLeft = true

		if (!root.isRunning)
			spriteSequence.jumpTo("run")
	}


	function moveUp() {
		if(!ep.cosGame.running || !ep.isAlive)
			return false

		ep.moveToPoint = Qt.point(0,0)

		if (isFalling || isOperating)
			return false

		if (ep.ladderMode == GamePlayerPrivate.LadderUpAvailable) {
			timerClimb.nextSprite = ""
			ep.ladderClimbUp()
			return true
		} else if (ep.ladderMode == GamePlayerPrivate.LadderClimb) {
			timerClimb.nextSprite = ""
			if (spriteSequence.currentSprite != "climbup2" && spriteSequence.currentSprite != "climbup3" && spriteSequence.currentSprite != "climbupend")
				spriteSequence.jumpTo("climbup2")
			return true
		}

		return false
	}


	function moveDown() {
		if(!ep.cosGame.running || !ep.isAlive)
			return false

		ep.moveToPoint = Qt.point(0,0)

		if (isFalling || isOperating)
			return false

		if (ep.ladderMode == GamePlayerPrivate.LadderDownAvailable) {
			timerClimb.nextSprite = ""
			ep.ladderClimbDown()
			return true
		} else if (ep.ladderMode == GamePlayerPrivate.LadderClimb) {
			timerClimb.nextSprite = ""
			if (spriteSequence.currentSprite != "climbdown2" && spriteSequence.currentSprite != "climbdown3" && spriteSequence.currentSprite != "climbdownend" )
				spriteSequence.jumpTo("climbdown2")
			return true
		}

		return false
	}



	function spriteToIdle() {
		if(!ep.isAlive)
			return

		spriteSequence.jumpTo("idle")
	}


	Timer {
		id: playPainTimer
		interval: 750
		running: false
		triggeredOnStart: false

		onTriggered: cosClient.playSound(ep.playSoundEffect("pain"), CosSound.PlayerVoice)
	}



	Timer {
		id: hpVisibleTimer
		interval: 1000
		running: false
		repeat: false
	}

}
