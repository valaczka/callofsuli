import QtQuick 2.12
import QtQuick.Controls 2.12
import Bacon2D 1.0
import COS.Client 1.0
import QtGraphicalEffects 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: control

	//color: CosStyle.colorBg

	property string terrainDirName: "qrc:/terrain/"+"terrain1"
	property var terrainData: cosClient.readJsonFile(terrainDirName+"/data.json")

	CosGame {
		id: gameWindow
		anchors.fill: parent
		currentScene: scene

		playerCharacter: "character2"
		level: 1


		onGameStateChanged: console.info("game state ", gameState)

		onWidthChanged: setXOffset()

		TiledScene {
			id: scene
			debug: true
			physics: true
			source: terrainDirName+"/"+terrainData.tmx

			y: Math.max(0, gameWindow.height-scene.height)

			viewport: Viewport {
				id: vp
				yOffset: 50 //50-player.y
				width: gameWindow.width
				height: gameWindow.height
			}

			onRunningChanged: console.info("SCENE run ", running)

			layers: [
				TiledLayer {
					id: playerLayer
					name: "Player"
					objects: TiledObject { }
				},

				TiledLayer {
					name: "Ground"
					objects: [
						TiledObject {
							id: groundObject

							fixtures: Box {
								width: groundObject.width
								height: groundObject.height
								density: 1
								restitution: 0
								friction: 1
								categories: Box.Category1
								collidesWith: Box.Category1
								property string boxType: "ground"
							}
						},

						TiledObject {
							name: "polyground"

							fixtures: Polygon {
								density: 1
								restitution: 0
								friction: 1

								categories: Box.Category1
								collidesWith: Box.Category1

								property string boxType: "ground"
							}
						}
					]
				},


				TiledLayer {
					id: ladderLayer
					name: "Ladders"
					objects: TiledObject { }
				},

				TiledLayer {
					id: coinLayer
					name: "Coins"
					objects: TiledObject { }
				},

				TiledLayer {
					id: boundariesLayer
					name: "Boundaries"
					objects: TiledObject {
						fixtures: Chain {
							density: 1
							restitution: 1
							friction: 1
						}
					}
				},

				TiledLayer {
					id: ladderTilesLayer
					name: "LadderTiles"

				},

				TiledLayer {
					id: enemyLayer
					name: "Enemies"
					objects: TiledObject {
						id: enemyObjects
						fixtures: Chain {
							categories: Box.Category3
							collidesWith: Box.Category3

						}
					}
				}

			]


			TiledPaintedLayer {
				anchors.fill: parent
				name: "Tiles3"
				scene: scene
			}

			Component {
				id: enemySoldierComponent

				GameEnemySoldier {  }
			}

			GamePlayer {
				id: player

				onXChanged: setXOffset()
			}


			TiledPaintedLayer {
				id: tiledLayer1
				anchors.fill: parent
				name: "LadderTiles"
				scene: scene
				visible: false
			}

			Glow {
				anchors.fill: tiledLayer1
				radius: 3
				samples: 7
				color: "yellow"
				source: tiledLayer1
			}

			/*Dog { id: player }

			Component {
				id: coinComponent
				Coin {}
			}*/

			Component {
				id: ladderComponent
				TMXladder {}
			}

			/**************************** INPUT HANDLING ***************************/
			// Key handling
			Keys.onPressed: {
				scene.forceActiveFocus()
				switch(event.key) {
				case Qt.Key_Left:
					if (event.modifiers & Qt.ShiftModifier)
						player.runLeft()
					else
						player.walkLeft()
					break;
				case Qt.Key_Right:
					if (event.modifiers & Qt.ShiftModifier)
						player.runRight()
					else
						player.walkRight()
					break;
				case Qt.Key_Up:
					player.jump()
					break
				}

				event.accepted = true
			}

			Keys.onReleased: {
				switch(event.key) {
				case Qt.Key_Left:
					if(!event.isAutoRepeat)
						player.stopMoving();
					break;
				case Qt.Key_Right:
					if(!event.isAutoRepeat)
						player.stopMoving();
					break;
				case Qt.Key_F3:
					loadEnemies()
					break;
				}

				event.accepted = true
			}
			/*************************** END OF INPUT HANDLING ************************/

			Component.onCompleted: {
				//player.x = playerObject.x;
				//player.y = playerObject.y

				var pos = 2

				for (var i =0; i<playerLayer.objects.length; ++i) {
					var po = playerLayer.objects[i]
					while (po.next()) {
						var poPos = po.getProperty("position")
						var poBlock = po.getProperty("block")
						console.info("PLAYER "+i+": "+po.getProperty("id")+"  "+poPos)

						if (poBlock == 1 && poPos === 1) {
							console.info("PLACE on "+poPos+"  "+po.x+","+po.y)
							player.x = po.x-player.width
							player.y = po.y-player.height
						}
					}
				}

				// Create coins
				// Loop through every "collision" of the coin object layer
				for(i=0; i<ladderLayer.objects.length; ++i)
				{
					var collision = ladderLayer.objects[i];
					while(collision.next())
					{
						var ladder = ladderComponent.createObject(scene);
						ladder.x = collision.x;
						ladder.y = collision.y
						ladder.width = collision.width
						ladder.height = collision.height
					}
				}
			}
		}
	}



	VirtualJoystick {
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.margins: 15

		onJoystickMoved: {
			if (x > 0.3) {
				if (x > 0.9)
					player.runRight()
				else
					player.walkRight()
			} else if (x < -0.3) {
				if (x < -0.9)
					player.runLeft()
				else
					player.walkLeft()
			} else {
				player.stopMoving();
			}

			if (y > 0.5)
				player.jump()
		}
	}

	StackView.onRemoved: destroy()

	StackView.onActivated: {
		gameWindow.gameState = Bacon2D.Running
	}


	function setXOffset() {
		if (!gameWindow.width)
			return

		if (player.facingLeft && (player.x-vp.xOffset < 500))
			vp.xOffset = player.x-500
		else if (!player.facingLeft && (player.x-(vp.xOffset+gameWindow.width)+500) > 0)
			vp.xOffset = player.x - gameWindow.width + 500
	}


	function loadEnemies() {
		var collision = enemyObjects;
		console.debug("LOAD enemIeS", collision.count, collision)

		collision.first()

		while(collision.next())
		{
			/*var ladder = ladderComponent.createObject(scene);
			ladder.x = collision.x;
			ladder.y = collision.y
			ladder.width = collision.width
			ladder.height = collision.height */

			console.debug("ENEMY", collision, collision.x, collision.y, collision.width, collision.height)

			console.debug("fixtures", collision.body, collision.body.fixtures, collision.body.fixtures.length)

			for (var i=0; i< collision.body.fixtures.length; ++i) {
				var f = collision.body.fixtures[i]
				console.debug("fixture", i, f, f.vertices)

				var startx = 0
				var endx = 0

				for (var j=0; f.vertices && j<f.vertices.length; j++) {
					var v = f.vertices[j]
					console.debug("vertex", j, v, v.x)
					startx = Math.min(startx, v.x)
					endx = Math.max(endx, v.x)
				}

				if (startx != endx) {

					startx += collision.x
					endx += collision.x

					console.debug("====>", startx, endx)

					var enemy = enemySoldierComponent.createObject(scene);
					enemy.x = collision.x;
					enemy.y = collision.y-enemy.height;
					enemy.boundLeft = startx
					enemy.boundRight = endx
				}
			}
		}
	}


	function windowClose() {
		return true
	}


	function stackBack() {
		return false
	}

}
