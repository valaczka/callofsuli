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

	CosGame {
		id: game
		anchors.fill: parent
		currentScene: mainScene

		gameScene: gameScene

		//scale: (Qt.platform.os === "android" ? 0.8 : 1.0)

		terrain: "terrain1"
		playerCharacter: "character2"
		level: 1
		startBlock: 3
		startHp: 5

		onWidthChanged: gameScene.setXOffset()

		Scene {
			id: mainScene

			width: 600
			height: 600

			Label {
				anchors.centerIn: parent
				text: "Szia"
			}
		}


		onTerrainDataChanged: {
			if (terrainData)
				gameScene.scenePrivate.source = "qrc:/terrain/"+game.terrain+"/"+terrainData.tmx
			else
				gameScene.scenePrivate.source = null
		}


		GameTiledScene {
			id: gameScene
			game: game
			scenePrivate.game: game
			scenePrivate.onLayersLoaded: game.currentScene = gameScene
		}


		Component.onCompleted: {
			loadTerrainData()
		}
	}

	ProgressBar {
		id: progressHp
		visible: game.player
		anchors.top: parent.top
		anchors.right: parent.right
		anchors.margins: 10
		width: 100
		from: 0
		to: game.player ? Math.max(game.startHp, game.player.entityPrivate.hp) : game.startHp
		value: game.player ? game.player.entityPrivate.hp : 0
	}

	VirtualJoystick {
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.margins: 10

		width: 80
		height: 80

		visible: game.currentScene == gameScene && game.player && game.player.entityPrivate.isAlive

		onJoystickMoved: if (game.player) {
							 if (x > 0.5) {
								 if (x > 0.95)
									 game.player.runRight()
								 else
									 game.player.walkRight()
							 } else if (x > 0.1) {
								game.player.facingLeft = false
							 } else if (x < -0.5) {
								 if (x < -0.95)
									 game.player.runLeft()
								 else
									 game.player.walkLeft()
							 } else if (x < -0.1) {
								game.player.facingLeft = true
							 } else {
								 game.player.stopMoving();
							 }

							 if (y > 0.5)
								 game.player.moveUp()
							 else if (y < -0.5)
								 game.player.moveDown()
						 }
	}

	Rectangle {
		width: 50
		height: 50
		radius: width/2

		anchors.bottom: parent.bottom
		anchors.right: parent.right
		anchors.margins: 10

		visible: game.currentScene == gameScene && game.player && game.player.entityPrivate.isAlive

		MouseArea {
			anchors.fill: parent
			acceptedButtons: Qt.LeftButton
			onClicked: game.player.entityPrivate.attackByGun()
		}
	}




	StackView.onRemoved: destroy()

	StackView.onActivated: {
		game.gameState = Bacon2D.Running
	}



	function windowClose() {
		return true
	}


	function stackBack() {
		return false
	}

}
