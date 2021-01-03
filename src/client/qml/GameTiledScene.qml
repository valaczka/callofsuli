import QtQuick 2.12
import QtQuick.Controls 2.12
import Bacon2D 1.0
import COS.Client 1.0
import QtMultimedia 5.12
import "."
import "Style"
import "JScript.js" as JS


Scene {
	id: scene
	//debug: true
	physics: true
	focus: true

	width: scenePrivate.implicitWidth
	height: scenePrivate.implicitHeight

	property alias scenePrivate: scenePrivate

	Audio {
		id: startSound
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/voiceover/fight.ogg"
	}

	Audio {
		id: successSound
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/voiceover/winner.ogg"
	}

	Audio {
		id: failedSound
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/voiceover/loser.ogg"
	}

	GameScenePrivate {
		id: scenePrivate

		onSceneLoaded: {
			game.currentScene = gameScene
			scene.forceActiveFocus()
		}
	}


	Connections {
		target: game
		function onGameStarted() {
			createLadders()
		}
	}


	PhysicsEntity {
		x: 0
		y: scene.height+5
		width: scene.width
		height: 10

		fixtures: Box {
			width: scene.width
			height: 10
			density: 1
			restitution: 0
			friction: 1
			categories: Box.Category1
			collidesWith: (Box.Category2|Box.Category5)
			readonly property bool baseGround: true
		}
	}


	MouseArea {
		id: area
		anchors.fill: parent
		hoverEnabled: true
	}

	Keys.onBackPressed: mainStack.back()

	Keys.onPressed: {
		if (game.player) {
			switch(event.key) {
			case Qt.Key_Left:
				if (event.modifiers & Qt.ShiftModifier)
					game.player.runLeft()
				else
					game.player.walkLeft()
				break;
			case Qt.Key_Right:
				if (event.modifiers & Qt.ShiftModifier)
					game.player.runRight()
				else
					game.player.walkRight()
				break;
			case Qt.Key_Up:
				game.player.moveUp()
				break;
			case Qt.Key_Down:
				game.player.moveDown()
				break;
			case Qt.Key_Enter:
			case Qt.Key_Return:
				game.player.entityPrivate.gunOn = !game.player.entityPrivate.gunOn
				break
			case Qt.Key_Space:
				if (!game.player.entityPrivate.gunOn)
					game.player.entityPrivate.gunOn = true
				else
					game.player.entityPrivate.attackByGun()
				break
			}
		}

		event.accepted = true
	}

	Keys.onReleased: {
		if (game.player) {
			switch(event.key) {
			case Qt.Key_Left:
				if(!event.isAutoRepeat)
					game.player.stopMoving();
				break;
			case Qt.Key_Right:
				if(!event.isAutoRepeat)
					game.player.stopMoving();
				break;
			case Qt.Key_Up:
				if(!event.isAutoRepeat)
					game.player.stopMoving();
				break;
			case Qt.Key_Down:
				if(!event.isAutoRepeat)
					game.player.stopMoving();
				break;


				/* -------------------- Cheats -------------------- */
			case Qt.Key_N:
				if (event.modifiers & (Qt.ShiftModifier|Qt.ControlModifier) && game.isStarted) {
					game.gameCompleted()
				}
				break;
			case Qt.Key_X:
				if (event.modifiers & Qt.ShiftModifier && area.containsMouse && game.player && game.isStarted) {
					game.player.x = area.mouseX
					game.player.y = area.mouseY
					game.player.entityPrivate.ladderClimbFinish()
				}
				break;

				/* ----------------- Cheats end -------------------- */
			}
		}

		event.accepted = true
	}

	Timer {
		id: startTimer
		interval: 750
		running: false
		triggeredOnStart: false
		onTriggered: {
			stop()
			startSound.play()
		}
	}



	Component {
		id: playerComponent

		GamePlayer { }
	}


	Component {
		id: ladderComponent

		GameLadder { }
	}

	Component {
		id: enemySoldierComponent

		GameEnemySoldier { }
	}

	Component {
		id: questionComponent

		GameQuestion { }
	}


	function createPlayer() : Item {
		if (!game.player) {
			var p = playerComponent.createObject(scene)
			p.loadSprites()
			return p
		}
			return null
		}


			function createComponent(enemyType: int) : Item {
				var obj = null

				switch (enemyType) {
					case GameEnemyData.EnemySoldier:
						obj = enemySoldierComponent.createObject(scene)
					break
					case GameEnemyData.EnemyOther:
						obj = enemySoldierComponent.createObject(scene)
					break
				}

				return obj
			}


				function createLadders() {
					if (!game || !game.ladderCount)
					return

					for (var i=0; i<game.ladderCount; i++) {
						var l = game.ladderAt(i)

						var obj = ladderComponent.createObject(scene,{
							ladder: l
						})
					}
				}


				function createQuestion(questionData) : Item {
					if (!game)
					return

					startTimer.start()

					var obj = questionComponent.createObject(game.itemPage,{
						questionData: questionData
					})

					obj.succeed.connect(function(){successSound.play()})
					obj.failed.connect(function(){failedSound.play()})

					return obj
				}
				}

