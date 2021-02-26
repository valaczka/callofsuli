import QtQuick 2.15
import QtQuick.Controls 2.15
import Bacon2D 1.0
import COS.Client 1.0
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.0
import QtMultimedia 5.12
import "."
import "Style"
import "JScript.js" as JS

/*

  z layers:

  0-4: TiledLayers
  5: Ladders
  6-8: Objects
  9: Enemies
  10: Player
  11-15: TiledLayers

  */

Page {
	id: control

	property bool _closeEnabled: false
	property bool _sceneLoaded: false
	property bool _animStartEnded: false
	property bool _animStartReady: true

	property alias gameMatch: game.gameMatch
	property bool deleteGameMatch: false

	on_SceneLoadedChanged: doStep()
	on_AnimStartEndedChanged: doStep()
	on_AnimStartReadyChanged: doStep()

	GameActivity {
		id: gameActivity
		client: cosClient
		game: game

		onPreparedChanged: {
			doStep()
		}

		onPrepareFailed: {
			cosClient.sendMessageError(qsTr("Játék előkészítése sikertelen"), qsTr("Nem sikerült előkészíteni a játékot!"))
			_closeEnabled = true
			mainStack.back()
		}
	}


	Image {
		id: bg

		property real scaleFactorWidth: 1.3
		property real scaleFactorHeight: 1.1

		visible: !bgSaturate.visible

		source: game.gameMatch ? game.gameMatch.bgImage : ""

		x: -(flick.visibleArea.xPosition/(1-flick.visibleArea.widthRatio))*(width-parent.width)
		y: -(height-parent.height)

		fillMode: Image.PreserveAspectCrop
		clip: false
		height: parent.height*scaleFactorHeight
		width: parent.width*scaleFactorWidth
	}

	Desaturate {
		id: bgSaturate

		visible: desaturation

		anchors.fill: bg
		source: bg

		desaturation: 1.0
	}


	Flickable {
		id: flick
		contentWidth: game.width
		contentHeight: game.height

		enabled: !game.question

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

			opacity: 0.0
			visible: false

			gameScene: gameScene
			itemPage: control
			activity: gameActivity

			property bool _timeSound: true
			property bool _finalSound: true

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
					text: qsTr("Finished")
				}
			}



			GameTiledScene {
				id: gameScene
				game: game
				scenePrivate.game: game
			}

			Connections {
				target: game.player ? game.player : null
				function onXChanged(x) {
					flick.setXOffset()
					flick.setYOffset()
				}

				function onFacingLeftChanged(facingLeft) {
					flick.setXOffset()
				}

				function onYChanged(y) {
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
			}

			onPlayerChanged: if (player) {
								 flick.setXOffset()
								 flick.setYOffset()
							 }

			onGameSceneLoaded: {
				_sceneLoaded = true
				cosClient.playSound(game.backgroundMusicFile, CosSound.Music)
			}

			onGameSceneLoadFailed: {
				cosClient.sendMessageError(qsTr("Játék betöltése sikertelen"), qsTr("Nem sikerült betölteni a játékot!"))
				_closeEnabled = true
				mainStack.back()
			}

			onIsPreparedChanged: doStep()

			onGameTimeout: {
				setEnemiesMoving(false)
				setRunning(false)

				var d = JS.dialogMessageError(qsTr("Lejárt az idő"), qsTr("Lejárt az idő"))
				d.rejected.connect(function() {
					_closeEnabled = true
					mainStack.back()
				})
			}

			onGameCompleted: {
				gameOverCompletedTimer.start()
			}


			onMsecLeftChanged: {
				if (msecLeft > 60*1000 && !_finalSound)
					_finalSound = true

				if (msecLeft > 2*60*1000 && !_timeSound)
					_timeSound = true


				if (msecLeft <= 2*60*1000 && _timeSound) {
					_timeSound = false
					messageList.message(qsTr("You have 2 minutes left"), 1)
					cosClient.playSound("qrc:/sound/voiceover/time.ogg", CosSound.VoiceOver)
					infoTime.marked = true
				}

				if (msecLeft <= 60*1000 && _finalSound) {
					_finalSound = false
					messageList.message(qsTr("You have 60 seconds left"), 1)
					cosClient.playSound("qrc:/sound/voiceover/final_round.ogg", CosSound.VoiceOver)
					infoTime.marked = true
				}
			}


			onGameMessageSent: {
				messageList.message(message, colorCode)
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


	Image {
		id: painhudImage
		source: "qrc:/internal/game/painhud.png"
		opacity: 0.0
		visible: opacity
		anchors.fill: parent


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

		source: "qrc:/internal/img/skull.svg"
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


	GameLabel {
		id: infoHP

		anchors.top: parent.top
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.margins: 5
		color: CosStyle.colorErrorLighter
		text: "%1 HP"
		value: game.player ? game.player.entityPrivate.hp : 0
		image.visible: false
	}

	Column {
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.margins: 7
		spacing: 5

		GameLabel {
			id: labelXP
			anchors.right: parent.right
			color: "white"
			image.visible: false

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
			image.icon: "qrc:/internal/game/shield-blank.png"
			progressBar.width: Math.min(control.width*0.125, 100)

			property int shield: game.player ? game.player.entityPrivate.shield : 0

			onShieldChanged: {
				if (shield > progressBar.to)
					progressBar.to = shield

				if (shield < 3)
					infoShield.marked = true
			}
		}



		GameInfo {
			id: infoTarget
			anchors.right: parent.right
			color: CosStyle.colorAccentDarker
			image.icon: "qrc:/internal/img/target2.svg"
			label.text: Math.floor(progressBar.value)

			progressBar.from: 0
			progressBar.to: 0
			progressBar.value: enemies
			progressBar.width: Math.min(control.width*0.125, 100)

			property int enemies: game.activeEnemies

			onEnemiesChanged: if (enemies>progressBar.to)
								  progressBar.to = enemies
		}

		GameButton {
			size: 30

			anchors.right: parent.right

			color: "transparent"
			border.color: fontImage.color
			border.width: 2

			fontImage.icon: CosStyle.iconPreferences
			fontImage.color: CosStyle.colorAccentLighter
			fontImageScale: 0.7

			onClicked: {
				var d = JS.dialogCreateQml("GameSound", {
											   volumeMusic: cosClient.volume(CosSound.MusicChannel),
											   volumeSfx: cosClient.volume(CosSound.SfxChannel),
											   volumeVoiceover: cosClient.volume(CosSound.VoiceoverChannel)
										   })

				d.item.volumeMusicModified.connect(function(volume) { cosClient.setVolume(CosSound.MusicChannel, volume) })
				d.item.volumeSfxModified.connect(function(volume) { cosClient.setVolume(CosSound.SfxChannel, volume) })
				d.item.volumeVoiceoverModified.connect(function(volume) { cosClient.setVolume(CosSound.VoiceoverChannel, volume) })
				d.closedAndDestroyed.connect(function() {
					gameScene.forceActiveFocus()
				})
				d.open()
			}
		}
	}








	VirtualJoystick {
		id: joystick

		enabled: !game.question

		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.margins: 5

		width: Math.min(200, control.width*0.5)
		height: Math.min(120, control.width*0.4)

		visible: game.currentScene == gameScene && game.player && game.player.entityPrivate.isAlive

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
								 if (x > 0.5)
									 game.player.runRight()
								 else
									 game.player.walkRight()
							 } else if (x > 0.1) {
								 game.player.turnRight()
							 } else if (x < -0.3) {
								 if (x < -0.5)
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
		anchors.margins: 10

		visible: game.currentScene == gameScene && game.player && game.player.entityPrivate.isAlive

		readonly property bool enemyAimed: game.player && game.player.entityPrivate && game.player.entityPrivate.enemy

		color: enemyAimed ? JS.setColorAlpha(CosStyle.colorErrorLighter, 0.7) : "transparent"
		opacity: enemyAimed ? 1.0 : 0.6

		border.color: enemyAimed ? "black" : "white"

		fontImage.icon: "qrc:/internal/img/target1.svg"
		fontImage.color: "white"
		fontImage.opacity: enemyAimed ? 0.6 : 1.0
		tap.enabled: !game.question
		tap.onTapped: game.player.entityPrivate.attackByGun()
	}



	GameButton {
		id: pickButton
		size: 50

		visible: game.currentScene == gameScene && game.player
		enabled: game.player && game.pickable

		anchors.horizontalCenter: shotButton.horizontalCenter
		anchors.bottom: shotButton.top
		anchors.bottomMargin: 10

		color: enabled ? JS.setColorAlpha(CosStyle.colorOKLighter, 0.5) : "transparent"
		border.color: enabled ? fontImage.color : "white"
		border.width: 1

		opacity: enabled ? 1.0 : 0.6

		fontImage.icon: "image://font/Material Icons/\ue925"
		fontImage.color: "white"
		fontImageScale: 0.6
		fontImage.anchors.horizontalCenterOffset: -2

		onClicked: {
			game.pickPickable()
		}
	}


	GameMessageList {
		id: messageList

		anchors.left: parent.left
		anchors.top: parent.top

		width: Math.min(implicitWidth, control.width*0.55)
		maximumHeight: Math.min(implicitMaximumHeight, control.height*0.25)
	}

	GameLabel {
		id: infoTime
		color: CosStyle.colorPrimary
		image.visible: false

		anchors.left: parent.left
		anchors.top: messageList.bottom
		anchors.margins: 5

		property int secs: game.msecLeft/1000

		label.text: JS.secToMMSS(secs)
	}



	Rectangle {
		id: blackRect
		anchors.fill: parent
		color: "black"
		visible: opacity
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
		text: game.gameMatch ? game.gameMatch.name : ""
		opacity: 0.0
		visible: opacity
		width: parent.width*0.7
		horizontalAlignment: Text.AlignHCenter
		x: (parent.width-width)/2
		y: (parent.height-height)/2
		wrapMode: Text.Wrap
	}




	Timer {
		id: gameOverCompletedTimer
		interval: 2000
		running: false
		triggeredOnStart: false
		onTriggered: {
			stop()
			game.setEnemiesMoving(false)
			game.setRunning(false)

			var d = JS.dialogMessage("success", qsTr("Game over"), qsTr("MISSION COMPLETED"))
			d.rejected.connect(function() {
				_closeEnabled = true
				mainStack.back()
			})
		}
	}



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
				desaturation: 0.0
				opacity: 0.0
			}
			PropertyChanges {
				target: game
				opacity: 0.0
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
				target: game
				opacity: 1.0
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
				ScriptAction {
					script: {
						game.visible = true
					}
				}

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

				PropertyAction {
					target: control
					property: "_animStartReady"
					value: true
				}

				PauseAnimation {
					duration: 700
				}

				PropertyAction {
					target: control
					property: "_animStartEnded"
					value: true
				}
			}
		},
		Transition {
			from: "start"
			to: "run"
			SequentialAnimation {
				ParallelAnimation {
					PropertyAction {
						target: game
						property: "opacity"
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

				ScriptAction {
					script: {
						cosClient.playSound("qrc:/sound/voiceover/begin.ogg", CosSound.VoiceOver)
					}
				}
			}
		}
	]



	StackView.onRemoved: {
		cosClient.stopSound(game.backgroundMusicFile, CosSound.Music)
		destroy()
	}

	StackView.onActivated: {
		state = "start"
		game.loadScene()
		gameActivity.prepare()
	}

	Component.onCompleted: {
		cosClient.playSound("qrc:/sound/voiceover/prepare_yourself.ogg", CosSound.VoiceOver)
	}

	Component.onDestruction: {
		if (deleteGameMatch && gameMatch)
			delete gameMatch
	}




	function doStep() {
		if (_sceneLoaded && _animStartEnded && game.isPrepared && gameActivity.prepared) {
			control.state = "run"
			game.gameStarted()
		}

	}



	function windowClose() {
		if (!_closeEnabled) {
			var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan megszakítod a játékot?")})
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
		if (!_closeEnabled) {
			var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan megszakítod a játékot?")})
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

}
