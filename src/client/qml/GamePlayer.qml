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


	readonly property bool isClimbing: Array("climbup", "climbup2", "climbup3",
											 "climbdown", "climbdown2", "climbdown3").includes(spriteSequence.currentSprite)


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
			spriteSequence.jumpTo("dead")
		}

		onAttack: {
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

		onLadderModeChanged: {
			if (ladderMode == GamePlayerPrivate.LadderClimb) {
				root.bodyType = Body.Kinematic
			} else if (ladderMode == GamePlayerPrivate.LadderUnavaliable) {
				root.bodyType = Body.Dynamic
			}
		}

		onRayCastPerformed: {
			if (cosGame.gameScene.debug)
				setray(rect)
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

		running: root.isWalking

		triggeredOnStart: true
		onTriggered: {
			if(readyToStop || !ep.cosGame.running) {
				spriteToIdle()
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
		timerWalk.readyToStop = true
		timerRun.readyToStop = true
		timerClimb.nextSprite = "climbpause"
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
			return false

		if (isFalling)
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

		if (isFalling)
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
