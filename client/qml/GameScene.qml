import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import QtMultimedia 5.12
import Box2D 2.0
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

/*

  z layers:

  0-4: TiledLayers
  5: Ladders
  6-8: Objects
  9: Enemies
  10: Player
  11-15: TiledLayers
  16: Crosshair

  */

GameSceneImpl {
	id: control

	property real _sceneScale: 1.0
	readonly property real _sceneZoom: game && game.pageItem ?
										   Math.max(Math.min(game.pageItem.width/control.width, game.pageItem.height/control.height, 0.7), 0.35) :
										   1.0


	y: parent.height-(control.height*control.scale)

	scale: _sceneZoom+((1.0-_sceneZoom)*_sceneScale)
	transformOrigin: Item.TopLeft

	world: World {  }

	mouseArea: area



	// Base ground

	GameObject {
		x: 0
		y: control.height
		width: control.width
		height: 10

		body.fixtures: Box {
			width: control.width
			height: 10
			density: 1
			restitution: 0
			friction: 1
			categories: Box.Category1
			collidesWith: (Box.Category2|Box.Category5)
			readonly property bool baseGround: true
		}

		Component.onCompleted: bodyComplete()
	}


	MouseArea {
		id: area
		anchors.fill: parent
		hoverEnabled: true
	}


	DebugDraw {
		id: debugDraw
		anchors.fill: parent
		world: control.world
		opacity: 0.5
		visible: control.debugView
	}


	Connections {
		target: game

		function onPlayerChanged() {
			showPlayerLocator()
		}
	}

	onZoomOverviewChanged: if (zoomOverview)
							   showPlayerLocator()


	/*

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

			case Qt.Key_F3:				// Pinch zoom
				scenePrivate.game.gameSceneScaleToggleRequest()
				break

			case Qt.Key_W:				// Water
				if (game.player && game.player.entityPrivate.fire && game.gameMatch.water)
					game.player.entityPrivate.operate(game.player.entityPrivate.fire)
				break

			case Qt.Key_P:				// Pliers
				if (game.player && game.player.entityPrivate.fence && game.gameMatch.pliers)
					game.player.entityPrivate.operate(game.player.entityPrivate.fence)
				break

			case Qt.Key_I:				// Invisible
				if (game.player && game.gameMatch.camouflage)
					game.player.startInvisibility(10000)
				break

			case Qt.Key_T:				// Teleport
				if (game.player && game.player.entityPrivate.teleport && game.gameMatch.teleporter)
					game.player.entityPrivate.teleportToNext()
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
					if (event.modifiers & Qt.ControlModifier && game.isStarted) {
						game.addSecs(-30)
					}
					break;

				case Qt.Key_T:
					if (event.modifiers & Qt.ShiftModifier && game.isStarted) {
						game.increaseTeleporter(1)
					}
					break;

				case Qt.Key_B:
					if (event.modifiers & Qt.ShiftModifier && game.isStarted) {
						game.player.entityPrivate.diedByBurn()
					}
					break;

				case Qt.Key_G:
					if (event.modifiers & Qt.ShiftModifier && game.isStarted) {
						game.increaseCamouflage(1)
					}
					break;

				}
			}
		}

		event.accepted = true
	}





	function createComponent(enemyType: int) : Item {
		var obj = null

		switch (enemyType) {
			case GameEnemyData.EnemySoldier:
				obj = enemySoldierComponent.createObject(scene)
			break
			case GameEnemyData.EnemySniper:
				obj = enemySniperComponent.createObject(scene)
			break
			case GameEnemyData.EnemyOther:
				obj = enemySoldierComponent.createObject(scene)
			break
		}

		return obj
	}


	function createPickable(pickableType: int, pickableData) : Item {
		if (!game)
		return

		var obj = null
		var img = ""

		switch (pickableType) {
			case GamePickableImpl.PickableHealth:
				obj = pickableComponentHealth.createObject(scene, {
															   cosGame: game,
															   pickableData: pickableData
														   })
			break

			case GamePickableImpl.PickableTime:
				if (pickableData.secs >= 60)
					img = "qrc:/internal/game/time-60.png"
				else if (pickableData.secs >= 30)
					img = "qrc:/internal/game/time-30.png"

			obj = pickableComponentGeneral.createObject(scene, {
				cosGame: game,
				type: GamePickableImpl.PickableTime,
				image: img,
				pickableData: pickableData
			})
			break

			case GamePickableImpl.PickableShield:
				if (pickableData.num >= 5)
					img = "qrc:/internal/game/shield-gold.png"
				else if (pickableData.num >= 3)
					img = "qrc:/internal/game/shield-red.png"
				else if (pickableData.num >= 2)
					img = "qrc:/internal/game/shield-blue.png"
				else
					img = "qrc:/internal/game/shield-green.png"

			obj = pickableComponentGeneral.createObject(scene, {
				cosGame: game,
				type: GamePickableImpl.PickableShield,
				image: img,
				pickableData: pickableData
			})
			break

			case GamePickableImpl.PickablePliers:
				obj = pickableComponentGeneral.createObject(scene, {
																cosGame: game,
																type: GamePickableImpl.PickablePliers,
																image: "qrc:/internal/game/pliers.png",
																pickableData: pickableData
															})
			break

			case GamePickableImpl.PickableWater:
				obj = pickableComponentGeneral.createObject(scene, {
																cosGame: game,
																type: GamePickableImpl.PickableWater,
																image: "qrc:/internal/game/water.svg",
																imageWidth: 30,
																imageHeight: 30,
																imageSourceWidth: 50,
																imageSourceHeight: 50,
																pickableData: pickableData
															})
			break

			case GamePickableImpl.PickableCamouflage:
				obj = pickableComponentGeneral.createObject(scene, {
																cosGame: game,
																type: GamePickableImpl.PickableCamouflage,
																image: "qrc:/internal/game/camouflage.png",
																pickableData: pickableData
															})
			break

			case GamePickableImpl.PickableTeleporter:
				obj = pickableComponentGeneral.createObject(scene, {
																cosGame: game,
																type: GamePickableImpl.PickableTeleporter,
																image: "qrc:/internal/game/teleporter.png",
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

	function createTeleport() : Item {
		var obj = teleportComponent.createObject(scene)
		return obj
	}

*/


	Component {
		id: playerLocatorComponent
		GamePlayerLocator { }
	}

	function showPlayerLocator() {
		console.debug("LOCATOR", game.player)
		if (game.player) {
			var r = playerLocatorComponent.createObject(control)
			r.anchors.centerIn = game.player
		}
	}
}