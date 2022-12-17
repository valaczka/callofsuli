import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.0
import QtMultimedia 5.12
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


Page {
	id: control

	property ActionGame game: null
	property string closeDisabled: qsTr("A játék előkészítése alatt nem lehet bezárni a lapot!")

	/*GameActivity {
		id: gameActivity
		game: game

		onPreparedChanged: {
			doStep()
		}

		onPrepareFailed: {
			cosClient.sendMessageErrorImage("qrc:/internal/icon/tools.svg",qsTr("Játék előkészítése sikertelen"), qsTr("Nem sikerült előkészíteni a játékot!"))
			_backDisabled = false
			_closeEnabled = true
			mainStack.back()
		}

		onQuestionFailed: {
			if (gameMatch && liteHP <= 0)
				skullImageAnim.start()
			else
				painhudImageAnim.start()
		}

	}*/


	Image {
		id: bg

		readonly property real reqWidth: control.width*(1.0+(0.4*gameScene._sceneScale))
		readonly property real reqHeight: control.height*(1.0+(0.15*gameScene._sceneScale))
		readonly property real reqScale: implicitWidth>0 && implicitHeight>0 ? Math.max(reqHeight/implicitHeight, reqWidth/implicitWidth) : 1.0
		readonly property real horizontalRegion: reqWidth-control.width
		readonly property real verticalRegion: reqHeight-control.height

		height: implicitHeight*reqScale
		width: implicitWidth*reqScale

		cache: false

		visible: false

		source: game.backgroundImage

		clip: false

		anchors.horizontalCenter: parent.horizontalCenter
		anchors.horizontalCenterOffset: +horizontalRegion/2
										-(flick.visibleArea.widthRatio >= 1.0 ? 0.5
																			  : (flick.visibleArea.xPosition/(1-flick.visibleArea.widthRatio)))
										*horizontalRegion

		anchors.bottom: parent.bottom
		anchors.bottomMargin: -verticalRegion
							  +(flick.visibleArea.heightRatio >= 1.0 ? 1.0
																	 : (flick.visibleArea.yPosition/(1-flick.visibleArea.heightRatio)))
							  *verticalRegion

	}

	Desaturate {
		id: bgSaturate

		visible: desaturation && bg.visible

		anchors.fill: bg
		source: bg

		desaturation: 1.0
	}



	Flickable {
		id: flick
		contentWidth: placeholderItem.width
		contentHeight: placeholderItem.height

		//enabled: !game.question && gameMatch.mode == GameMatch.ModeNormal

		height: Math.min(contentHeight, parent.height)
		width: Math.min(contentWidth, parent.width)
		anchors.horizontalCenter: parent.horizontalCenter
		y: parent.height-height

		boundsBehavior: Flickable.StopAtBounds
		flickableDirection: Flickable.HorizontalAndVerticalFlick

		interactive: true //false

		Item {
			id: placeholderItem
			width: gameScene.width*gameScene.scale
			height: Math.max(gameScene.height*gameScene.scale, control.height)

			focus: true

			GameSceneItem {
				id: gameScene
				game: control.game

				//opacity: 0.0
				visible: false

				focus: true

				states: [
					State {
						name: "isZoom"
						when: gameScene.zoomOverview
						PropertyChanges {
							target: gameScene
							_sceneScale: 0.0
						}
						PropertyChanges {
							target: bgSaturate
							desaturation: 1.0
						}
					}
				]


				transitions: [
					Transition {
						from: "*"
						to: "*"
						SequentialAnimation {
							ScriptAction {
								script: {
									/*if (animX.running)
										animX.stop()*/
								}
							}

							ParallelAnimation {
								PropertyAnimation {
									target: gameScene
									property: "_sceneScale"
									easing.type: Easing.OutQuart
									duration: 750
								}
								PropertyAnimation {
									target: bgSaturate
									properties: "desaturation";
									easing.type: Easing.OutQuart;
									duration: 750
								}
							}

							ScriptAction {
								script: {
								/*	flick.setXOffset()
									flick.setYOffset()
									if (gameScene.zoomOverview && game.player) {
										var r = gameScene.playerLocatorComponent.createObject(gameScene)
										r.anchors.centerIn = game.player
									}*/
								}
							}
						}

					}
				]
			}

			Desaturate {
				id: gameSaturate

				anchors.fill: gameScene
				source: gameScene

				opacity: 0.0
				visible: desaturation

				desaturation: 1.0

				Behavior on opacity { NumberAnimation { duration: 750 } }
			}

			/*
			CosGame {
				id: game
				width: gameMatch.mode == GameMatch.ModeNormal ? gameScene.width : mainScene.width
				height: gameMatch.mode == GameMatch.ModeNormal ? gameScene.height : mainScene.height
				currentScene: mainScene

				y: gameMatch.mode == GameMatch.ModeNormal ? parent.height-(gameScene.height*gameScene.scale) : 0

				opacity: 0.0
				visible: false

				gameScene: gameScene
				itemPage: control
				activity: gameActivity

				property bool _timeSound: false
				property bool _finalSound: true





				GameTiledScene {
					id: gameScene
					game: game
					scenePrivate.game: game


				}


				Connections {
					target: game.player ? game.player : null
					function onXChanged(x) {
						gameScene.zoomOverview = false
						flick.setXOffset()
						flick.setYOffset()
					}

					function onFacingLeftChanged(facingLeft) {
						flick.setXOffset()
					}

					function onYChanged(y) {
						gameScene.zoomOverview = false
						flick.setYOffset()
					}

				}

				Connections {
					target: game.player && game.player.entityPrivate ? game.player.entityPrivate : null

					function onHurt() {
						painhudImageAnim.start()
					}

					function onKilledByEnemy(enemy) {
						messageList.message(qsTr("Your man has died"), 3)
						skullImageAnim.start()
						cosClient.playSound("qrc:/sound/sfx/dead.mp3", CosSound.PlayerVoice)
					}

					function onDiedByFall() {
						messageList.message(qsTr("Your man has died"), 3)
						skullImageAnim.start()
						cosClient.playSound("qrc:/sound/sfx/falldead.mp3", CosSound.PlayerVoice)
					}

					function onDiedByBurn() {
						messageList.message(qsTr("Your man has died"), 3)
						skullImageAnim.start()
						cosClient.playSound("qrc:/sound/sfx/dead.mp3", CosSound.PlayerVoice)
					}
				}

				onPlayerChanged: if (player) {
									 flick.setXOffset()
									 flick.setYOffset()
								 }

				onGameSceneLoaded: {
					_sceneLoaded = true
					if (gameMatch.mode == GameMatch.ModeNormal)
						cosClient.playSound(game.backgroundMusicFile, CosSound.Music)
				}

				onGameSceneLoadFailed: {
					cosClient.sendMessageErrorImage("qrc:/internal/icon/tools.svg",qsTr("Játék betöltése sikertelen"), qsTr("Nem sikerült betölteni a játékot!"))
					_backDisabled = false
					_closeEnabled = true
					mainStack.back()
				}

				onIsPreparedChanged: doStep()

				onGameTimeout: {
					setEnemiesMoving(false)
					setRunning(false)

					var d = JS.dialogMessageErrorImage("qrc:/internal/icon/timer-sand-complete.svg", qsTr("Game over"), qsTr("Lejárt az idő"))
					d.rejected.connect(function() {
						_closeEnabled = true
						mainStack.back()
					})
				}


				onGameLost: {
					setEnemiesMoving(false)
					setRunning(false)


					var t = gameMatch.mode == GameMatch.ModeNormal ? qsTr("Your man has died") : qsTr("Sikertelen feladatmegoldás")

					var d = JS.dialogMessageErrorImage(gameMatch.mode == GameMatch.ModeNormal ? "qrc:/internal/icon/skull-crossbones.svg" : "",
													   qsTr("Game over"), t)
					d.rejected.connect(function() {
						_closeEnabled = true
						mainStack.back()
					})
				}

				onGameCompletedReady: {
					setEnemiesMoving(false)
					setRunning(false)

					if (!studentMaps) {
						_closeEnabled = true
						mainStack.back()
					}
				}


				onMsecLeftChanged: {
					if (gameMatch.mode != GameMatch.ModeNormal)
						return

					if (msecLeft > 30*1000 && !_finalSound)
						_finalSound = true

					if (msecLeft > 60*1000 && !_timeSound)
						_timeSound = true


					if (msecLeft <= 60*1000 && _timeSound) {
						infoTime.marked = true
						_timeSound = false
						messageList.message(qsTr("You have 1 minute left"), 1)
						cosClient.playSound("qrc:/sound/voiceover/time.mp3", CosSound.VoiceOver)
					}

					if (msecLeft <= 30*1000 && _finalSound) {
						infoTime.marked = true
						_finalSound = false
						messageList.message(qsTr("You have 30 seconds left"), 1)
						cosClient.playSound("qrc:/sound/voiceover/final_round.mp3", CosSound.VoiceOver)
					}
				}

				onGameSecondsAdded: {
					infoTime.marked = true
				}

				onGameMessageSent: {
					messageList.message(message, colorCode)
				}

				onGameSceneScaleToggleRequest: {
					if (!_backDisabled && gameMatch.mode == GameMatch.ModeNormal)
						gameScene.zoomOverviewToggle()
				}

				onGameToolTipRequest: {
					gameToolTip(setting, image, text, details)
				}
			}


			Desaturate {
				id: gameSaturate

				anchors.fill: game
				source: game

				opacity: 0.0
				visible: desaturation

				desaturation: 1.0

				Behavior on opacity { NumberAnimation { duration: 750 } }
			}


		}*/

		}

		/*PinchArea {
			anchors.fill: parent

			MouseArea {								// Workaround (https://bugreports.qt.io/browse/QTBUG-77629)
				anchors.fill: parent
			}

			//enabled: !_backDisabled && !game.question && gameMatch.mode == GameMatch.ModeNormal

			onPinchUpdated: {
				if (pinch.scale < 0.9) {
					gameScene.zoomOverview = true
				} else if (pinch.scale > 1.1) {
					gameScene.zoomOverview = false
				}
			}
		}*/

		/*
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
			var spaceRequired = fw*0.45
			var px = game.player.x*gameScene.scale
			var pw = game.player.width*gameScene.scale
			var cx = flick.contentX
			var cw = flick.contentWidth
			var x = 0
			var newX = false

			if (game.player.facingLeft || gameScene.isSceneZoom) {
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

				if (game.player.isRunning) {
					if (animX.running)
						animX.stop()
					flick.contentX = x
				} else if (animX.running || Math.abs(cx-x) > 50) {
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
			var py = game.player.y*gameScene.scale
			var ph = game.player.height*gameScene.scale
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


		function setOffsetTo(_x, _y) {
			var fh = flick.height
			var py = _y*gameScene.scale
			var cy = flick.contentY
			var ch = flick.contentHeight
			var y = py-fh/2

			if (y<0)
				y = 0

			if (y+fh > ch)
				y = ch-fh

			flick.contentY = y


			var fw = flick.width
			var px = _x*gameScene.scale
			var cx = flick.contentX
			var cw = flick.contentWidth
			var x = px-fw/2

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

		*/
	}

	Image {
		id: painhudImage
		source: "qrc:/internal/game/painhud.png"
		opacity: 0.0
		visible: opacity
		anchors.fill: parent

		z: 10

		SequentialAnimation {
			id: painhudImageAnim

			PropertyAnimation {
				target: painhudImage
				property: "opacity"
				from: 0.0
				to: 1.0
				duration: 100
				easing.type: Easing.OutQuad
			}
			PropertyAnimation {
				target: painhudImage
				property: "opacity"
				from: 1.0
				to: 0.0
				duration: 375
				easing.type: Easing.InQuad
			}
		}
	}



	Image {
		id: skullImage
		opacity: 0.0
		visible: opacity
		anchors.centerIn: parent

		z: 9

		source: "qrc:/internal/game/skull.svg"
		sourceSize.width: 200
		sourceSize.height: 200

		width: 200
		height: 200
		fillMode: Image.PreserveAspectFit

		ParallelAnimation {
			id: skullImageAnim

			SequentialAnimation {
				PropertyAnimation {
					target: skullImage
					property: "opacity"
					from: 0.0
					to: 0.6
					duration: 350
					easing.type: Easing.InQuad
				}
				PropertyAnimation {
					target: skullImage
					property: "opacity"
					from: 0.6
					to: 0.0
					duration: 600
					easing.type: Easing.InQuad
				}
			}
			PropertyAnimation {
				target: skullImage
				property: "scale"
				from: 0.1
				to: 8.0
				duration: 950
				easing.type: Easing.OutInQuad
			}
		}
	}


	/*

	GameLabel {
		id: infoHP

		anchors.top: parent.top
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.margins: 5
		anchors.topMargin: Math.max(mainWindow.safeMarginTop, 5)
		color: CosStyle.colorErrorLighter
		text: "%1 HP"
		value: game.player ? game.player.entityPrivate.hp
						   : (gameMatch.mode == GameMatch.ModeLite ? gameActivity.liteHP : 0)
		//image.visible: false
		image.icon: "qrc:/internal/icon/heart-pulse.svg"

		visible: !gameScene.isSceneZoom && gameMatch.mode != GameMatch.ModeExam

		onValueChanged: marked = true
	}

	Column {
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.margins: 7
		anchors.rightMargin: Math.max(mainWindow.safeMarginRight, 7)
		spacing: 5

		visible: !gameScene.isSceneZoom

		GameLabel {
			id: labelXP
			anchors.right: parent.right
			color: "white"
			image.visible: false

			pixelSize: 16

			text: "%1 XP"

			value: game.gameMatch ? game.gameMatch.xp : 0
		}

		GameInfo {
			id: infoShield
			anchors.right: parent.right
			color: CosStyle.colorOK
			label.text: Math.floor(progressBar.value)
			progressBar.from: 0
			progressBar.to: 0
			progressBar.value: shield
			image.icon: "qrc:/internal/icon/shield.svg"
			progressBar.width: Math.min(control.width*0.125, 100)

			visible: gameMatch.mode == GameMatch.ModeNormal

			property int shield: game.player ? game.player.entityPrivate.shield : 0

			onShieldChanged: {
				if (shield > progressBar.to)
					progressBar.to = shield

				infoShield.marked = true
			}
		}



		GameInfo {
			id: infoTarget
			anchors.right: parent.right
			color: CosStyle.colorWarning
			image.icon: "qrc:/internal/icon/target-account.svg"
			label.text: Math.floor(progressBar.value)

			progressBar.from: 0
			progressBar.to: 0
			progressBar.value: enemies
			progressBar.width: Math.min(control.width*0.125, 100)

			property int enemies: game.activeEnemies

			onEnemiesChanged: {
				infoTarget.marked = true
				if (enemies>progressBar.to)
					progressBar.to = enemies
			}
		}

		Grid {
			id: itemGrid
			anchors.right: parent.right
			width: infoShield.width
			layoutDirection: Qt.RightToLeft
			horizontalItemAlignment: Grid.AlignHCenter
			verticalItemAlignment: Grid.AlignVCenter

			visible: gameMatch.mode == GameMatch.ModeNormal

			bottomPadding: 10

			property real size: CosStyle.pixelSize*1.3

			spacing: 5

			columns: Math.floor(width/size)

			QFontImage {
				size: itemGrid.size
				icon: "qrc:/internal/icon/pliers.svg"
				color: "brown"
				visible: game.gameMatch.pliers
			}

			QFontImage {
				size: itemGrid.size
				icon: "qrc:/internal/icon/remote.svg"
				color: "#00FFFF"
				visible: game.gameMatch.teleporter
			}

			Repeater {
				model: game.gameMatch.water

				QFontImage {
					size: itemGrid.size
					icon: "qrc:/internal/icon/water.svg"
					color: "blue"
				}
			}

			Repeater {
				model: game.gameMatch.camouflage

				QFontImage {
					size: itemGrid.size
					icon: "qrc:/internal/icon/domino-mask.svg"
					color: "gold"
				}
			}
		}




		GameButton {
			id: setttingsButton
			size: 30

			anchors.right: parent.right

			visible: gameMatch.mode == GameMatch.ModeNormal

			color: "transparent"
			border.color: fontImage.color
			border.width: 2

			fontImage.icon: "qrc:/internal/icon/cog.svg"
			fontImage.color: CosStyle.colorAccentLighter
			fontImageScale: 0.7

			onClicked: {
				var d = JS.dialogCreateQml("GameSettings", {
											   volumeMusic: cosClient.volume(CosSound.MusicChannel),
											   volumeSfx: cosClient.volume(CosSound.SfxChannel),
											   volumeVoiceover: cosClient.volume(CosSound.VoiceoverChannel),
											   joystickSize: joystick.size
										   })

				d.item.volumeMusicModified.connect(function(volume) { cosClient.setVolume(CosSound.MusicChannel, volume) })
				d.item.volumeSfxModified.connect(function(volume) { cosClient.setVolume(CosSound.SfxChannel, volume) })
				d.item.volumeVoiceoverModified.connect(function(volume) { cosClient.setVolume(CosSound.VoiceoverChannel, volume) })
				d.item.joystickSizeModified.connect(function(size) { joystick.size = size })
				d.closedAndDestroyed.connect(function() {
					gameScene.forceActiveFocus()
					cosClient.setSetting("game/joystickSize", joystick.size)
				})
				d.open()
			}
		}


		Row {
			anchors.right: parent.right
			spacing: 10

			visible: DEBUG_MODE

			GameButton {
				id: invincibleButton
				size: 30

				anchors.verticalCenter: parent.verticalCenter

				color: gameMatch.invincible ? JS.setColorAlpha(CosStyle.colorErrorLighter, 0.7) : "transparent"
				opacity: gameScene.isSceneZoom ? 0.2 : 1.0

				border.color: gameMatch.invincible ? "black" : "white"
				border.width: 2

				fontImage.opacity: gameMatch.invincible ? 1.0 : 0.6
				fontImage.icon: "qrc:/internal/icon/check-bold.svg"
				fontImage.color: "white"
				fontImageScale: 0.7

				onClicked: {
					gameMatch.invincible = !gameMatch.invincible
				}
			}

			GameButton {
				id: winButton
				size: 30

				anchors.verticalCenter: parent.verticalCenter

				color: JS.setColorAlpha(CosStyle.colorOK, 0.7)
				border.color: "white"
				border.width: 2

				fontImage.icon: "qrc:/internal/icon/check-bold.svg"
				fontImage.color: CosStyle.colorOKLight
				fontImageScale: 0.7

				onClicked: {
					game.gameCompleted()
				}
			}

		}
	}

	GameMessageList {
		id: messageList

		anchors.left: parent.left
		anchors.top: parent.top
		anchors.leftMargin: Math.max(mainWindow.safeMarginLeft/2, 10)

		width: Math.min(implicitWidth, control.width*0.55)
		maximumHeight: Math.min(implicitMaximumHeight, control.height*0.25)

		visible: !gameScene.isSceneZoom && gameMatch.mode == GameMatch.ModeNormal
	}

	Row {
		id: rowTime

		anchors.left: parent.left
		anchors.top: gameMatch.mode == GameMatch.ModeNormal ? messageList.bottom : parent.top
		anchors.margins: 5
		anchors.leftMargin: Math.max(mainWindow.safeMarginLeft/2, 10)

		GameButton {
			id: backButton
			size: 25

			anchors.verticalCenter: parent.verticalCenter

			visible: !gameScene.isSceneZoom

			color: JS.setColorAlpha(CosStyle.colorError, 0.7)
			border.color: "white"
			border.width: 1

			fontImage.icon: "qrc:/internal/icon/close.svg"
			fontImage.color: "white"
			fontImageScale: 0.7

			onClicked: {
				mainStack.back()
			}
		}

		GameLabel {
			id: infoTime
			color: CosStyle.colorPrimary
			//image.visible: false

			anchors.verticalCenter: parent.verticalCenter

			image.icon: CosStyle.iconClock1

			property int secs: game.msecLeft/1000

			label.text: game.msecLeft>=60000 ? JS.secToMMSS(secs) : Number(game.msecLeft/1000).toFixed(1)

			visible: !gameScene.isSceneZoom
		}
	}


	GameLabel {
		id: infoInvisibilityTime
		color: CosStyle.colorAccent
		//image.visible: false

		image.icon: "qrc:/internal/icon/domino-mask.svg"

		anchors.left: parent.left
		anchors.top: rowTime.bottom
		anchors.margins: 5
		anchors.leftMargin: Math.max(mainWindow.safeMarginLeft, 5)
		pixelSize: 24

		property int secs: game.msecLeft/1000

		label.text: game.player ? Number(game.player.invisibleMsec/1000).toFixed(1) : ""

		visible: !gameScene.isSceneZoom && game.player && game.player.entityPrivate.invisible

		onVisibleChanged: if (visible)
							  marked = true
	}








	GameJoystick {
		id: joystick

		property real size: 175

		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.bottomMargin: 5 //Math.max(5, mainWindow.safeMarginBottom)
		anchors.leftMargin: 5 //Math.max(5, mainWindow.safeMarginLeft)

		width: Math.min(size, control.width*0.5)
		height: Math.min((120/175)*size, control.width*0.4)

		visible: game.currentScene == gameScene && game.player && game.player.entityPrivate.isAlive

		opacity: gameScene.isSceneZoom ? 0.2 : 1.0

		onHasTouchChanged: if (!hasTouch && game.player)
							   game.player.stopMoving()

		onJoystickMoved: if (game.player) {
							 if (y > 0.6) {
								 if (game.player.moveUp())
									 return
							 } else if (y < -0.6) {
								 if (game.player.moveDown())
									 return
							 } else if (game.player.isClimbing) {
								 game.player.stopMoving()
								 return
							 }

							 if (x > 0.3) {
								 if (x > 0.6)
									 game.player.runRight()
								 else
									 game.player.walkRight()
							 } else if (x > 0.1) {
								 game.player.turnRight()
							 } else if (x < -0.3) {
								 if (x < -0.6)
									 game.player.runLeft()
								 else
									 game.player.walkLeft()
							 } else if (x < -0.1) {
								 game.player.turnLeft()
							 } else {
								 game.player.stopMoving()
							 }

						 }
	}



	GameButton {
		id: shotButton
		size: 55

		width: Math.min(100, control.width*0.5)
		height: Math.min(100, control.width*0.4)

		anchors.right: parent.right
		anchors.bottom: parent.bottom
		anchors.rightMargin: Math.max(10, mainWindow.safeMarginRight)
		anchors.bottomMargin: Math.max(10, mainWindow.safeMarginBottom)

		visible: game.currentScene == gameScene && game.player && game.player.entityPrivate.isAlive

		readonly property bool enemyAimed: game.player && game.player.entityPrivate && game.player.entityPrivate.enemy

		color: enemyAimed ? JS.setColorAlpha(CosStyle.colorErrorLighter, 0.7) : "transparent"
		opacity: gameScene.isSceneZoom ? 0.2 : (enemyAimed ? 1.0 : 0.6)

		border.color: enemyAimed ? "black" : "white"

		fontImage.icon: "qrc:/internal/game/target1.svg"
		fontImage.color: "white"
		fontImage.opacity: enemyAimed ? 0.6 : 1.0
		tap.enabled: !game.question
		tap.onTapped: if (!game.player.isOperating) game.player.entityPrivate.attackByGun()
	}





	GameButton {
		id: pickButton
		size: 50

		width: shotButton.width
		height: 60

		anchors.horizontalCenter: shotButton.horizontalCenter
		anchors.bottom: shotButton.top

		visible: game.currentScene == gameScene && game.player && game.player.entityPrivate.isAlive
		enabled: game.player && game.pickable


		color: enabled ? JS.setColorAlpha(CosStyle.colorOKLighter, 0.5) : "transparent"
		border.color: enabled ? fontImage.color : "white"
		border.width: 1

		opacity:  gameScene.isSceneZoom ? 0.2 : (enabled ? 1.0 : 0.6)

		fontImage.icon: "qrc:/internal/icon/hand-back-right.svg"
		fontImage.color: "white"
		fontImageScale: 0.6
		fontImage.anchors.horizontalCenterOffset: -2

		onClicked: {
			game.pickPickable()
		}
	}


	Column {
		anchors.bottom: shotButton.bottom
		anchors.bottomMargin: (shotButton.height-pliersButton.height)/2
		anchors.right: shotButton.left

		spacing: 30

		visible: gameMatch.mode == GameMatch.ModeNormal

		GameButton {
			id: pliersButton
			size: 50

			width: size
			height: size

			enabled: game.gameMatch.pliers
			visible: game.currentScene == gameScene && game.player && game.player.entityPrivate.isAlive && game.player.entityPrivate.fence

			anchors.horizontalCenter: parent.horizontalCenter

			color: enabled ? "#7FFF7F50" : "transparent"
			border.color: enabled ? fontImage.color : "white"
			border.width: 1

			opacity:  gameScene.isSceneZoom ? 0.2 : (enabled ? 1.0 : 0.6)

			fontImage.icon: "qrc:/internal/icon/pliers.svg"
			fontImage.color: "white"
			fontImageScale: 0.6
			fontImage.anchors.horizontalCenterOffset: -2

			onClicked: {
				game.player.entityPrivate.operate(game.player.entityPrivate.fence)
			}
		}

		GameButton {
			id: waterButton
			size: 50

			width: size
			height: size

			enabled: game.gameMatch.water
			visible: game.currentScene == gameScene && game.player && game.player.entityPrivate.isAlive && game.player.entityPrivate.fire

			anchors.horizontalCenter: parent.horizontalCenter

			color: enabled ? "#7F4169E1" : "transparent"
			border.color: enabled ? fontImage.color : "white"
			border.width: 1

			opacity:  gameScene.isSceneZoom ? 0.2 : (enabled ? 1.0 : 0.6)

			fontImage.icon: "qrc:/internal/game/drop.png"
			fontImage.color: "white"
			fontImageScale: 0.6
			fontImage.anchors.horizontalCenterOffset: -2

			onClicked: {
				game.player.entityPrivate.operate(game.player.entityPrivate.fire)
			}
		}

		GameButton {
			id: teleportButton
			size: 50

			width: size
			height: size

			enabled: game.gameMatch.teleporter
			visible: game.player && game.player.entityPrivate.isAlive && game.player.entityPrivate.teleport

			anchors.horizontalCenter: parent.horizontalCenter

			color: enabled ? "#00FFFF" : "transparent"
			border.color: enabled ? fontImage.color : "white"
			border.width: 1

			opacity:  gameScene.isSceneZoom ? 0.2 : (enabled ? 1.0 : 0.6)

			fontImage.icon: "qrc:/internal/icon/remote.svg"
			fontImage.color: "white"
			fontImageScale: 0.6
			//fontImage.anchors.horizontalCenterOffset: -2

			onClicked: {
				game.player.entityPrivate.teleportToNext()
			}
		}

		GameButton {
			id: camouflageButton
			size: 50

			width: size
			height: size

			enabled: game.player && !game.player.entityPrivate.invisible
			visible: game.currentScene == gameScene && game.player && game.player.entityPrivate.isAlive && game.gameMatch.camouflage

			anchors.horizontalCenter: parent.horizontalCenter

			color: enabled ? "gold" : "transparent"
			border.color: enabled ? fontImage.color : "white"
			border.width: 1

			opacity:  gameScene.isSceneZoom ? 0.2 : (enabled ? 1.0 : 0.6)

			fontImage.icon: "qrc:/internal/icon/domino-mask.svg"
			fontImage.color: enabled ? "black" : "white"
			fontImageScale: 0.6
			//fontImage.anchors.horizontalCenterOffset: -2

			onClicked: {
				if (game.player)
					game.player.startInvisibility(10000)
			}
		}
	}


*/


	Rectangle {
		id: blackRect
		anchors.fill: parent
		color: "black"
		visible: opacity
		opacity: 1.0
	}

	DropShadow {
		id: shadowName
		anchors.fill: nameLabel
		source: nameLabel
		color: "black"
		opacity: 0.0
		visible: opacity
		radius: 3
		samples: 7
		horizontalOffset: 3
		verticalOffset: 3
	}

	Label {
		id: nameLabel
		color: "white"
		font.family: "HVD Peace"
		font.pixelSize: Math.min(Math.max(30, (control.width/1000)*50), 60)
		text: game.name
		opacity: 0.0
		visible: opacity
		width: parent.width*0.7
		horizontalAlignment: Text.AlignHCenter
		x: (parent.width-width)/2
		y: (parent.height-height)/2
		wrapMode: Text.Wrap
	}

	/*Label {
			id: previewLabel
			color: CosStyle.colorPrimaryLighter
			font.pixelSize: Math.min(Math.max(30, (control.width/1000)*50), 60)
			opacity: 0.0
			visible: opacity
			width: Math.min(parent.width*0.7, parent.width-mainWindow.safeMarginLeft-mainWindow.safeMarginRight)
			horizontalAlignment: Text.AlignHCenter
			x: (parent.width-width)/2
			y: parent.height*0.8-height/2
			wrapMode: Text.Wrap
			font.weight: Font.DemiBold
			style: Text.Outline
			styleColor: "black"
		}*/


	/*Connections {
			target: studentMaps

			function onGameFinishDialogReady(data) {
				_closeEnabled = true
				mainStack.back()
			}

			function onExamContentReady(data) {
				gameActivity.prepareExam(data)
			}
		}*/



	state: "default"


	states: [
		State {
			name: "default"
			PropertyChanges {
				target: bgSaturate
				desaturation: 1.0
			}
			PropertyChanges {
				target: gameSaturate
				opacity: 0.0
			}
			PropertyChanges {
				target: gameScene
				visible: false
			}
			PropertyChanges {
				target: shadowName
				scale: 15.0
			}
			PropertyChanges {
				target: nameLabel
				scale: 15.0
			}
			PropertyChanges {
				target: blackRect
				opacity: 1.0
			}
		},
		State {
			name: "start"
			PropertyChanges {
				target: shadowName
				scale: 1.0
			}
			PropertyChanges {
				target: nameLabel
				scale: 1.0
			}
			PropertyChanges {
				target: blackRect
				opacity: 0.0
			}
			PropertyChanges {
				target: gameSaturate
				opacity: 1.0
			}
		},
		State {
			name: "run"

			PropertyChanges {
				target: gameScene
				//opacity: 1.0
				visible: true
			}
			PropertyChanges {
				target: bgSaturate
				desaturation: 0.0
			}
			PropertyChanges {
				target: gameSaturate
				desaturation: 0.0
			}
			PropertyChanges {
				target: blackRect
				opacity: 0.0
			}
			PropertyChanges {
				target: nameLabel
				opacity: 0.0
			}
			PropertyChanges {
				target: shadowName
				opacity: 0.0
			}
		}
	]


	transitions: [
		Transition {
			from: "default"
			to: "start"
			SequentialAnimation {
				ParallelAnimation {
					PropertyAnimation {
						targets: [shadowName, nameLabel]
						property: "opacity"
						to: 1.0
						easing.type: Easing.InOutQuad;
						duration: 150
					}
					PropertyAnimation {
						targets: [shadowName, nameLabel]
						property: "scale"
						easing.type: Easing.InOutQuad;
						duration: 1000
					}
				}

				ScriptAction {
					script: {
						bg.visible = true
					}
				}

				PropertyAnimation {
					target: blackRect
					property: "opacity"
					easing.type: Easing.InExpo;
					duration: 700
				}

				PropertyAnimation  {
					target: gameSaturate
					property: "opacity"
					easing.type: Easing.InOutQuad;
					duration: 800
				}

				ScriptAction {
					script: {
						gameScene.onScenePrepared()
					}
				}

			}
		},
		Transition {
			from: "start"
			to: "run"
			SequentialAnimation {
				ParallelAnimation {
					PropertyAction {
						target: gameScene
						property: "visible"
					}
					PropertyAnimation {
						target: shadowName
						property: "opacity"
						easing.type: Easing.InOutQuad;
						duration: 500
					}
					PropertyAnimation {
						target: nameLabel
						property: "opacity"
						easing.type: Easing.InOutQuad;
						duration: 1200
					}
					PropertyAnimation {
						targets: [bgSaturate, gameSaturate]
						properties: "desaturation";
						easing.type: Easing.InOutQuad;
						duration: 1000
					}
				}

				/*ScriptAction {
					script: {
						_backDisabled = false
						if (gameMatch.mode == GameMatch.ModeNormal) {
							messageList.message(qsTr("LEVEL %1").arg(gameMatch.level), 3)

							previewAnimation.start()
						} else {
							game.onGameStarted()
						}
					}
				}*/
			}
		}
	]



	/*


	SequentialAnimation {
		id: previewAnimation
		running: false
		loops: Animation.Infinite
		property int num: 0

		ScriptAction {
			script:  {
				if (gameMatch.mode != GameMatch.ModeNormal)
					return

				if (previewAnimation.num >= game.terrainData.preview.length || gameMatch.skipPreview) {
					if (gameMatch && gameMatch.deathmatch) {
						messageList.message(qsTr("SUDDEN DEATH"), 3)
						cosClient.playSound("qrc:/sound/voiceover/sudden_death.mp3", CosSound.VoiceOver)
					} else
						cosClient.playSound("qrc:/sound/voiceover/begin.mp3", CosSound.VoiceOver)

					previewAnimation.stop()
					flick.interactive = true
					previewLabel.text = ""
					game.onGameStarted()

					if (previewAnimation.num > 0) {
						game.previewCompleted()
					}

				} else {
					var d = game.terrainData.preview[previewAnimation.num]

					flick.setOffsetTo(d.point.x, d.point.y)
					previewLabel.text = qsTr(d.text)

					var r = gameScene.playerLocatorComponent.createObject(gameScene, {
																			  color: CosStyle.colorPrimaryLight
																		  })
					r.x = d.point.x-(r.width/2)
					r.y = d.point.y-(r.height/2)

					previewAnimation.num++
				}
			}
		}

		NumberAnimation {
			target: previewLabel
			property: "opacity"
			to: 1.0
			duration: 850
			easing.type: Easing.InQuad
		}

		PauseAnimation {
			duration: 1500
		}

		NumberAnimation {
			target: previewLabel
			property: "opacity"
			to: 0.0
			duration: 450
			easing.type: Easing.OutQuad
		}
	}


	Timer {
		id: startTimer
		interval: 750
		running: false
		triggeredOnStart: false
		onTriggered: {
			stop()
			cosClient.playSound("qrc:/sound/voiceover/fight.mp3", CosSound.VoiceOver)
		}
	}

	Component {
		id: questionComponent

		GameQuestion {  }
	}

	Component {
		id: tooltipComponent

		GameToolTip {  }
	}

	Item {
		id: questionPlaceholder
		anchors.fill: parent
		visible: game.question || tooltip

		property GameToolTip tooltip: null

		z: 5
	}

	function createQuestion(questionPrivate : GameQuestionPrivate) : Item {
		if (gameMatch.mode == GameMatch.ModeNormal) {
			startTimer.start()
		}

		questionPlaceholder.visible = true

		var obj = questionComponent.createObject(questionPlaceholder,{
			questionPrivate: questionPrivate
		})

		return obj
	}


	function gameToolTip(_setting, _image, _text, _details) {
		if (cosClient.getServerSettingBool(_setting, true) === true) {
			game.running = false

			var d = tooltipComponent.createObject(questionPlaceholder, {
				text: _text,
				details: _details,
				image: _image
			})

			questionPlaceholder.tooltip = d

			d.finished.connect(function() {
				game.running = true
				game.currentScene.forceActiveFocus()
			})
				cosClient.setServerSetting(_setting, false)
		}
	}


*/
	StackView.onRemoved: {
		//cosClient.stopSound(game.backgroundMusicFile, CosSound.Music)
		//destroy()
	}

	StackView.onActivated: {
		state = "start"
		gameScene.load()

		/*if (gameMatch.mode == GameMatch.ModeNormal)
			game.loadScene()
		else
			_sceneLoaded = true

		if (gameMatch.mode == GameMatch.ModeExam)
			//gameActivity.prepare()
			studentMaps.getExamContent()
		else
			gameActivity.prepare() */
	}

	Component.onCompleted: {
		if (!game) {
			console.error(qsTr("null game"))
		}

		gameScene.forceActiveFocus()

		//joystick.size = cosClient.getSetting("game/joystickSize", joystick.size) */
	}

	Component.onDestruction: {
		/*if (deleteGameMatch && gameMatch)
			delete gameMatch*/
	}




	/*

	property var closeCallbackFunction: function () {
		if (!_closeEnabled) {
			var d = JS.dialogCreateQml("YesNo", {
										   text: qsTr("Biztosan megszakítod a játékot?"),
										   image: "qrc:/internal/icon/close-octagon-outline.svg"
									   })

			d.accepted.connect(function() {
				game.currentScene = exitScene
				bg.source = ""
				game.abortGame()
				_closeEnabled = true
				mainWindow.close()
			})
			d.open()
			return true
		}
		return false
	}


	function stackBack() {
		if (_backDisabled)
			return true

		if (!_closeEnabled) {
			var d = JS.dialogCreateQml("YesNo", {
										   text: qsTr("Biztosan megszakítod a játékot?"),
										   image: "qrc:/internal/icon/close-octagon-outline.svg"
									   })
			d.accepted.connect(function() {
				game.currentScene = exitScene
				bg.source = ""
				game.abortGame()
				_closeEnabled = true
				mainStack.back()
			})
			d.open()
			return true
		}
		return false
	}
*/
}
