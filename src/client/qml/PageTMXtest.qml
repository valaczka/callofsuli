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

	property bool _closeEnabled: false


	Image {
		id: bg

		property real scaleFactorWidth: 1.3
		property real scaleFactorHeight: 1.1

		x: -(flick.visibleArea.xPosition/(1-flick.visibleArea.widthRatio))*(width-parent.width)
		/*y: flick.contentHeight > parent.height ?
			-(flick.visibleArea.yPosition/(1-flick.visibleArea.heightRatio))*(height-parent.height) :
			   -(height-parent.height)*/
		y: -(height-parent.height)

		onYChanged: console.debug(flick.visibleArea.yPosition, flick.visibleArea.heightRatio)

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
			itemPage: control

			terrain: "terrain1"
			playerCharacter: "character2"
			level: 1
			startBlock: 2
			startHp: 3


			Scene {
				id: mainScene

				width: 600
				height: 600

				Label {
					id: lbl
					anchors.centerIn: parent
					text: qsTr("Loading")
				}
			}


			Scene {
				id: exitScene
				width: 600
				height: 600

				Label {
					anchors.centerIn: parent
					text: qsTr("Exit")
				}
			}

			onCurrentSceneChanged: {
				console.debug("Current scene", _closeEnabled, currentScene)
				if (currentScene == exitScene && _closeEnabled) {
					console.debug("Game aborted")
					mainStack.back()
				}
			}


			function loadScene() {
				if (terrainData) {
					gameScene.scenePrivate.loadScene(":/terrain/"+game.terrain+"/"+terrainData.tmx)
					bg.source = "qrc:/terrain/"+game.terrain+"/"+terrainData.background
				} else {
					bg.source = null
				}
			}


			GameTiledScene {
				id: gameScene
				game: game
				scenePrivate.game: game
				scenePrivate.onSceneLoadStarted: lbl.text = tmxFileName
				scenePrivate.onSceneLoadFailed: cosClient.sendMessageError(qsTr("Pálya betöltése sikertelen"), qsTr("Nem sikerült betölteni az adatokat"))
			}

			Connections {
				target: game.player ? game.player : null
				onXChanged: {
					flick.setXOffset()
					flick.setYOffset()
				}
				onFacingLeftChanged: flick.setXOffset()
				onYChanged: flick.setYOffset()

			}

			onPlayerChanged: if (player) {
								 flick.setXOffset()
								 flick.setYOffset()
							 }

			onGameAbortRequest: {
				_closeEnabled = true
				console.debug("Abort request", _closeEnabled, currentScene)
				if (currentScene == exitScene && _closeEnabled) {
					console.debug("Game aborted")
					mainStack.back()
				}
			}

		}

		onWidthChanged: setXOffset()
		onHeightChanged: setYOffset()
		onContentWidthChanged: setXOffset()
		onContentHeightChanged: setYOffset()



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
			var x = 0
			var newX = false

			if (game.player.facingLeft) {
				if (px+pw+10 > cx+fw) {
					x = px+pw+10-fw
					newX = true
				} else if (px-spaceRequired < cx) {
					x = px-spaceRequired
					newX = true
				}

			} else  {
				if (px-10 < cx) {
					x = px-10
					newX = true
				} else if (px+pw+spaceRequired > (cx+fw)) {
					x = px+pw+spaceRequired-fw
					newX = true
				}
			}

			if (newX) {
				if (x<0)
					x = 0
				if (x+fw > cw)
					x = cw-fw

				if (animX.running || Math.abs(cx-x) > 50) {
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
								 if (x > 0.9)
									 game.player.runRight()
								 else
									 game.player.walkRight()
							 } else if (x > 0.1) {
								 game.player.turnRight()
							 } else if (x < -0.5) {
								 if (x < -0.9)
									 game.player.runLeft()
								 else
									 game.player.walkLeft()
							 } else if (x < -0.1) {
								 game.player.turnLeft()
							 } else {
								 game.player.stopMoving()
							 }

							 if (y > 0.9)
								 game.player.moveUp()
							 else if (y < -0.9)
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
		game.loadScene()
	}

	Component.onDestruction: {
		console.debug("PAGET TXM TEST destructing")
	}




	function windowClose() {
		if (!_closeEnabled) {
			var d = JS.dialogCreateQml("YesNo", {title: qsTr("Biztosan megszakítod a játékot?")})
			d.accepted.connect(function() {
				game.currentScene = exitScene
				bg.source = ""
				game.abortGame()
				_closeEnabled = true
				mainWindow.close()
			})
			d.open()
			return false
		}
		return true
	}


	function stackBack() {
		if (!_closeEnabled) {
			var d = JS.dialogCreateQml("YesNo", {title: qsTr("Biztosan megszakítod a játékot?")})
			d.accepted.connect(function() {
				game.currentScene = exitScene
				bg.source = ""
				game.abortGame()
			})
			d.open()
			return true
		}
		return false
	}

}
