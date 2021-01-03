import Bacon2D 1.0
import QtQuick 2.14
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


	Audio {
		id: deadSound
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/sfx/dead.mp3"
	}

	Audio {
		id: falldeadSound
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/sfx/falldead.mp3"
	}

	Audio {
		id: shotSound
		volume: CosStyle.volumeShot
		source: "qrc:/sound/sfx/shot.ogg"
	}


	SoundEffect {
		id: gunLoad
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/sfx/gunload.wav"
	}

	SoundEffect {
		id: run1
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/sfx/run1.wav"
	}

	SoundEffect {
		id: run2
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/sfx/run2.wav"
	}

	SoundEffect {
		id: step1
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/sfx/step1.wav"
	}

	SoundEffect {
		id: step2
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/sfx/step2.wav"
	}


	SoundEffect {
		id: climbup1
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/sfx/ladderup1.wav"
	}

	SoundEffect {
		id: climbup2
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/sfx/ladderup2.wav"
	}

	SoundEffect {
		id: climbend
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/sfx/ladder.wav"
	}


	SoundEffect {
		id: pain1
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/sfx/pain1.wav"
	}

	SoundEffect {
		id: pain2
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/sfx/pain2.wav"
	}

	SoundEffect {
		id: pain3
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/sfx/pain3.wav"
	}



	Audio {
		id: readySound
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/sfx/playerReady.ogg"
		autoPlay: true
	}

	readonly property bool isClimbing: Array("climbup", "climbup2", "climbup3",
											 "climbdown", "climbdown2", "climbdown3").includes(spriteSequence.currentSprite)


	GamePlayerPrivate {
		id: ep

		property bool wantToMove: false
		property bool gunOn: false
		readonly property bool gunPreparing: Array("gunon", "idlegun", "shot").includes(spriteSequence.currentSprite)

		hasGun: Array("idlegun", "shot").includes(spriteSequence.currentSprite)

		onGunOnChanged: {
			if (spriteSequence.currentSprite == "idle" && ep.gunOn && !ep.wantToMove)
				spriteSequence.jumpTo("gunon")
			else if (gunPreparing && !ep.gunOn)
				spriteSequence.jumpTo("gunoff")
			gunLoad.play()
		}

		rayCastEnabled: ladderMode != GamePlayerPrivate.LadderClimb && ladderMode != GamePlayerPrivate.LadderClimbFinish

		property int _fallStartY: -1

		onKilledByEnemy: {
			spriteSequence.jumpTo("dead")
			deadSound.play()
		}

		onAttack: {
			spriteSequence.jumpTo("shot")
			shotSound.stop()
			shotSound.play()
		}

		onHurt: playPainTimer.start()

		onIsOnGroundChanged: {
			if (isOnGround) {
				if (_fallStartY == -1) {
					ep.wantToMove = false
					spriteSequence.jumpTo("idle")
				} else {
					ep.wantToMove = false
					if (root.y-_fallStartY > ep.cosGame.levelData.player.deathlyFall || ep.isOnBaseGround) {
						spriteSequence.jumpTo("falldeath")
						falldeadSound.play()
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




	Timer {
		id: timerClimbSound
		interval: 1700
		repeat: true

		triggeredOnStart: true

		running: isClimbing

		property bool playFirst: true

		onTriggered: {
			if (climbup1.playing || climbup2.playing)
				return

			if (playFirst)
				climbup1.play()
			else
				climbup2.play()

			playFirst = !playFirst
		}
	}


	Timer {
		id: timerWalkSound
		interval: 400
		repeat: true

		triggeredOnStart: true

		running: isWalking

		property bool playFirst: true

		onTriggered: {
			if (playFirst)
				step1.play()
			else
				step2.play()

			playFirst = !playFirst
		}

		onRunningChanged: if (!running) playFirst = true
	}

	Timer {
		id: timerRunSound
		interval: 300
		repeat: true

		triggeredOnStart: true

		running: isRunning

		property bool playFirst: true

		onTriggered: {
			if (playFirst)
				run1.play()
			else
				run2.play()

			playFirst = !playFirst
		}

		onRunningChanged: if (!running) playFirst = true
	}



	spriteSequence.onCurrentSpriteChanged: {
		if (ep.ladderMode == GamePlayerPrivate.LadderClimbFinish && spriteSequence.currentSprite == "idle") {
			ep.ladderClimbFinish()
		}

		if (spriteSequence.currentSprite == "idle") {
			if (ep.gunOn && !ep.wantToMove) {
				spriteSequence.jumpTo("gunon")
				gunLoad.play()
			}
		}

		if (spriteSequence.currentSprite == "climbupend" || spriteSequence.currentSprite == "climbdownend" ) {
			climbend.play()
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


	Timer {
		id: playPainTimer
		interval: 750
		running: false
		triggeredOnStart: false

		property int painNum: 1

		onTriggered: {
			if (pain1.playing || pain2.playing || pain3.playing) {
				stop()
				return
			}

			if (painNum == 2) {
				pain2.play()
				painNum = 3
			} else if (painNum == 3) {
				pain3.play()
				painNum = 1
			} else {
				pain1.play()
				painNum = 2
			}

			stop()
		}
	}


}
