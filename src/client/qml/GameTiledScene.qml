import QtQuick 2.15
import QtQuick.Controls 2.15
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
	property bool showPickables: false
	property bool showTargets: false

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
			case Qt.Key_Space:
				game.player.entityPrivate.attackByGun()
				break;
			case Qt.Key_Enter:
			case Qt.Key_Return:
				if (game.pickable) {
					game.pickPickable()
				}
				break;
			case Qt.Key_F10:
				showPickables = true
				break
			case Qt.Key_F11:
				showTargets = true
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
			case Qt.Key_F10:
				showPickables = false
				break
			case Qt.Key_F11:
				showTargets = false
				break


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
			cosClient.playSound("qrc:/sound/voiceover/fight.ogg", CosSound.VoiceOver)
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

		GameQuestion {

		}
	}


	Component {
		id: pickableComponentHealth
		GamePickableHealth { }
	}

	Component {
		id: pickableComponentTime
		GamePickableTime { }
	}

	Component {
		id: pickableComponentShield
		GamePickableShield { }
	}


	Component {
		id: playerLocatorComponent
		GamePlayerLocator { }
	}


	function createPlayer() : Item {
		if (!game.player) {
			var r = playerLocatorComponent.createObject(scene)
			var p = playerComponent.createObject(scene)
			r.anchors.centerIn = p
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

					obj.successSound.connect(function(){cosClient.playSound("qrc:/sound/sfx/correct.ogg", CosSound.GameSound)})
					obj.succeed.connect(function(){cosClient.playSound("qrc:/sound/voiceover/winner.ogg", CosSound.VoiceOver)})
					obj.failed.connect(function(){cosClient.playSound("qrc:/sound/voiceover/loser.ogg", CosSound.VoiceOver)})

					return obj
				}




	function createPickable(pickableType: int, pickableData) : Item {
		if (!game)
			return

		var obj = null

		switch (pickableType) {
			case GameEnemyData.PickableHealth:
				obj = pickableComponentHealth.createObject(scene, {
															   cosGame: game,
															   pickableData: pickableData
														   })
				break

			case GameEnemyData.PickableTime:
				obj = pickableComponentTime.createObject(scene, {
															   cosGame: game,
															   pickableData: pickableData
														   })
				break

			case GameEnemyData.PickableShield:
				obj = pickableComponentShield.createObject(scene, {
															   cosGame: game,
															   pickableData: pickableData
														   })
				break
		}

		return obj
	}

}
