import QtQuick 2.12
import QtQuick.Controls 2.12
import Bacon2D 1.0
//import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: control

	//color: CosStyle.colorBg

	Game {
		id: gameWindow
		anchors.fill: parent
		currentScene: scene

		onGameStateChanged: console.info("game state ", gameState)

		TiledScene {
			id: scene
			debug: true
			physics: true
			source: "qrc:/map1/map.tmx"
			viewport: Viewport {
				yOffset: scene.height - gameWindow.height
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
							}
						},

						TiledObject {
							name: "polyground"

							fixtures: Polygon {
								density: 1
								restitution: 0
								friction: 1
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
				}
			]

			TMXplayer {
				id: player
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
				console.debug("Key pressed ", event.key)
				scene.forceActiveFocus()
				switch(event.key) {
				case Qt.Key_Left:
					player.moveLeft()
					break;
				case Qt.Key_Right:
					player.moveRight()
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
						player.stopMovingLeft();
					break;
				case Qt.Key_Right:
					if(!event.isAutoRepeat)
						player.stopMovingRight();
					break;
				}

				event.accepted = true
			}
			/*************************** END OF INPUT HANDLING ************************/

			Component.onCompleted: {
				//player.x = playerObject.x;
				//player.y = playerObject.y

				var pos = 1

				for (var i =0; i<playerLayer.objects.length; ++i) {
					var po = playerLayer.objects[i]
					while (po.next()) {
						var poPos = po.getProperty("position")
						console.info("PLAYER "+i+": "+po.getProperty("id")+"  "+poPos)

						if (poPos === pos) {
							console.info("PLACE on "+poPos+"  "+po.x+","+po.y)
							player.x = po.x
							player.y = po.y
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


	StackView.onRemoved: destroy()

	StackView.onActivated: {
		console.debug("ACTIVATED", gameWindow.gameState)
		gameWindow.gameState = Bacon2D.Running
	}

	function windowClose() {
		return true
	}


	function stackBack() {
		return false
	}

}
