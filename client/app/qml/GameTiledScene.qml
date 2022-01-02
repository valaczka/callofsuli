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
	property bool isSceneZoom: false

	property alias playerLocatorComponent: playerLocatorComponent

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
					game.player.walkLeft()
				else
					game.player.runLeft()
				break;
			case Qt.Key_Right:
				if (event.modifiers & Qt.ShiftModifier)
					game.player.walkRight()
				else
					game.player.runRight()
				break;
			case Qt.Key_Up:
				game.player.moveUp()
				break;
			case Qt.Key_Down:
				game.player.moveDown()
				break;
			case Qt.Key_Space:
				if (!game.player.isOperating)
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

			case Qt.Key_F3:
				scenePrivate.game.gameSceneScaleToggleRequest()
				break
			}


			if (DEBUG_MODE) {
				switch(event.key) {

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

				case Qt.Key_T:
					if (event.modifiers & (Qt.ShiftModifier|Qt.ControlModifier) && game.isStarted) {
						game.addSecs(-30)
					}
					break;


				case Qt.Key_O:
					if (event.modifiers & (Qt.ShiftModifier|Qt.ControlModifier) && game.isStarted) {
						game.player.operate()
					}
					break;


				case Qt.Key_B:
					if (event.modifiers & (Qt.ShiftModifier|Qt.ControlModifier) && game.isStarted) {
						game.player.entityPrivate.diedByBurn()
					}
					break;

				}
			}
		}

		event.accepted = true
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
		id: fireComponent
		GameFire {}
	}

	Component {
		id: fenceComponent
		GameFence {}
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
		id: pickableComponentPliers
		GamePickablePliers { }
	}

	Component {
		id: pickableComponentWater
		GamePickableWater { }
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



	function createPickable(pickableType: int, pickableData) : Item {
		if (!game)
			return

		var obj = null

		switch (pickableType) {
			case GamePickablePrivate.PickableHealth:
				obj = pickableComponentHealth.createObject(scene, {
															   cosGame: game,
															   pickableData: pickableData
														   })
				break

			case GamePickablePrivate.PickableTime:
				obj = pickableComponentTime.createObject(scene, {
															   cosGame: game,
															   pickableData: pickableData
														   })
				break

			case GamePickablePrivate.PickableShield:
				obj = pickableComponentShield.createObject(scene, {
															   cosGame: game,
															   pickableData: pickableData
														   })
				break

			case GamePickablePrivate.PickablePliers:
				obj = pickableComponentPliers.createObject(scene, {
															   cosGame: game,
															   pickableData: pickableData
														   })
				break

			case GamePickablePrivate.PickableWater:
				obj = pickableComponentWater.createObject(scene, {
															   cosGame: game,
															   pickableData: pickableData
														   })
				break
		}

		return obj
	}


	function createFire() : Item {
		var obj = fireComponent.createObject(scene)
		return obj
	}

	function createFence() : Item {
		var obj = fenceComponent.createObject(scene, {
					cosGame: game
				})
		return obj
	}
}
