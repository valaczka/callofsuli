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


	Image {
		id: bg

		property real scaleFactorWidth: 1.3
		property real scaleFactorHeight: 1.1

		x: -(flick.visibleArea.xPosition/(1-flick.visibleArea.widthRatio))*(width-parent.width)
		y: flick.contentHeight > parent.height ?
			-(flick.visibleArea.yPosition/(1-flick.visibleArea.heightRatio))*(height-parent.height) :
			   -(height-parent.height)

		onYChanged: console.debug(flick.visibleArea.yPosition, flick.visibleArea.heightRatio)

		source: "qrc:/terrain/terrain1/bg.png"
		fillMode: Image.PreserveAspectCrop
		clip: false
		height: parent.height*scaleFactorHeight
		width: parent.width*scaleFactorWidth
	}



	Flickable {
		id: flick
		contentWidth: game.width
		contentHeight: game.height


		height: Math.min(contentHeight, parent.height)
		width: Math.min(contentWidth, parent.width)
		anchors.horizontalCenter: parent.horizontalCenter
		y: parent.height-height


		boundsBehavior: Flickable.StopAtBounds
		flickableDirection: Flickable.HorizontalAndVerticalFlick

		CosGame {
			id: game
			width: currentScene.width
			height: currentScene.height
			currentScene: mainScene

			gameScene: gameScene

			//scale: (Qt.platform.os === "android" ? 0.8 : 1.0)

			// létra tetejéről leesik!!!

			terrain: "terrain1"
			playerCharacter: "character2"
			level: 1
			startBlock: 3
			startHp: 5

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

			Connections {
				target: game.player ? game.player : null
				onXChanged: flick.setXOffset()
				onFacingLeftChanged: flick.setXOffset()
				onYChanged: flick.setYOffset()

			}

			onPlayerChanged: if (player) {
								 flick.setXOffset()
								 flick.setYOffset()
							 }

			Component.onCompleted: {
				loadTerrainData()
			}
		}

		onWidthChanged: setYOffset()
		onHeightChanged: setYOffset()


		SmoothedAnimation {
			id: animX
			target: flick
			running: true
			property: "contentX"
			duration: 300
		}

		function setXOffset() {
			if (!game.player)
				return

			var fw = flick.width
			var spaceRequired = Math.min(fw*0.7, 500)
			var px = game.player.x
			var pw = game.player.width
			var cx = flick.contentX
			var cw = flick.contentWidth
			var x = -1

			if (game.player.facingLeft) {
				if (px+pw+10 > cx+fw) {
					x = px+pw+10-fw
					if (x<0)
						x = 0
				} else if (px-spaceRequired < cx) {
					x = px-spaceRequired

					if (x+fw > cw)
						x = cw-fw
				}
			} else  {
				if (px-10 < cx) {
					x = px-10
					if (x<0)
						x = 0
				} else if (px+pw+spaceRequired > (cx+fw)) {
					x = px+pw+spaceRequired-fw

					if (x+fw > cw)
						x = cw-fw
				}
			}

			if (x > -1) {
				if (animX.running) {
					animX.to = x
				} else if (Math.abs(cx-x) > 75) {
					animX.to = x
					animX.restart()
				} else {
					flick.contentX = x
				}
			}
		}


		function setYOffset() {
			if (!game.player)
				return

			var fh = flick.height
			var spaceRequired = Math.min(fh*0.3, 250)
			var py = game.player.y
			var ph = game.player.height
			var cy = flick.contentY
			var ch = flick.contentHeight
			var y = -1

			if (py-spaceRequired < cy) {
				y = py-spaceRequired

				if (y<0)
					y = 0
			} else if (py+ph+spaceRequired > (cy+fh)) {
				y = py+ph+spaceRequired-fh

				if (y+fh > ch)
					y = ch-fh
			}

			if (y>-1)
				flick.contentY = y
		}
	}

	/*
	Rectangle {
		width: 500
		height: 300

		color: "blue"

		anchors.right: parent.right
		anchors.bottom: parent.bottom
		anchors.margins: 30

		ShaderEffectSource {
			anchors.fill: parent
			anchors.margins: 5
			sourceItem: game
			//sourceRect: Qt.rect(game.width/2-250, game.height-300, 500, 300)
			live: true
		}

	}
*/
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
								 game.player.turnRight()
							 } else if (x < -0.5) {
								 if (x < -0.95)
									 game.player.runLeft()
								 else
									 game.player.walkLeft()
							 } else if (x < -0.1) {
								 game.player.turnLeft()
							 } else {
								 game.player.stopMoving()
							 }

							 if (y > 0.95)
								 game.player.moveUp()
							 else if (y < -0.95)
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
