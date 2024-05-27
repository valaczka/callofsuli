import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


Page {
	id: control

	property ActionGame game: null
	property string closeDisabled: qsTr("A játék előkészítése alatt nem lehet bezárni a lapot!")
	property string closeQuestion: qsTr("Biztosan megszakítod a játékot?")
	property var onPageClose: function() { if (game) game.gameAbort() }

	readonly property int stackViewIndex: StackView.index
	property alias scene: gameScene

	property bool itemsVisible: false

	readonly property MultiPlayerGame multiPlayerGame: game instanceof MultiPlayerGame ? game : null

	layer.enabled: true

	Image {
		id: bg

		readonly property real reqWidth: control.width*(1.0+(0.4*gameScene._sceneScale))
		readonly property real reqHeight: control.height*(1.0+(0.15*gameScene._sceneScale))
		readonly property real reqScale: implicitWidth>0 && implicitHeight>0 ? Math.max(reqHeight/implicitHeight, reqWidth/implicitWidth) : 1.0
		readonly property real horizontalRegion: reqWidth-control.width
		readonly property real verticalRegion: reqHeight-control.height

		height: implicitHeight*reqScale
		width: implicitWidth*reqScale

		cache: true

		visible: itemsVisible

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

		visible: itemsVisible

		height: Math.min(contentHeight, parent.height)
		width: Math.min(contentWidth, parent.width)
		anchors.horizontalCenter: parent.horizontalCenter
		y: parent.height-height

		boundsBehavior: Flickable.StopAtBounds
		flickableDirection: Flickable.HorizontalAndVerticalFlick

		interactive: !gameQuestion.questionComponent

		Item {
			id: placeholderItem
			width: gameScene.width*gameScene.scale
			height: Math.max(gameScene.height*gameScene.scale, control.height)

			focus: true

			GameScene {
				id: gameScene
				game: control.game

				messageList: messageList

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
									if (animX.running)
										animX.stop()
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
									flick.setXOffset()
									flick.setYOffset()
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



		}

		PinchArea {
			anchors.fill: parent

			MouseArea {								// Workaround (https://bugreports.qt.io/browse/QTBUG-77629)
				anchors.fill: parent
			}

			onPinchUpdated: pinch => {
				if (pinch.scale < 0.9) {
					gameScene.zoomOverview = true
				} else if (pinch.scale > 1.1) {
					gameScene.zoomOverview = false
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
			var spaceRequired = fw*0.45
			var px = game.player.x*gameScene.scale
			var pw = game.player.width*gameScene.scale
			var cx = flick.contentX
			var cw = flick.contentWidth
			var x = 0
			var newX = false

			if (game.player.facingLeft || gameScene.zoomOverview) {
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

				if (game.player.playerState == GamePlayer.Run) {
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


	}


	GamePainHud {
		id: _painhudImage
		anchors.fill: parent
		z: 10
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


	GameHpLabel {
		id: infoHP
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.topMargin: Math.max(5, Client.safeMarginTop)
		anchors.top: parent.top
		value: game.player ? game.player.hp : 0
		visible: !gameScene.zoomOverview && itemsVisible
		onValueChanged: marked = true
	}

	Column {
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.topMargin: Math.max(Client.safeMarginTop, 5)
		anchors.rightMargin: Math.max(Client.safeMarginRight, 7)
		spacing: 5 * Qaterial.Style.pixelSizeRatio

		visible: !gameScene.zoomOverview && itemsVisible

		GameLabel {
			id: labelXP
			anchors.right: parent.right
			color: "white"

			pixelSize: 16 * Qaterial.Style.pixelSizeRatio

			//text: (multiPlayerGame ? "MULTIPLAYER " : "") + "%1 XP"
			text: "%1 XP"

			value: game ? game.xp : 0
		}

		GameInfo {
			id: infoShield
			anchors.right: parent.right
			color: Qaterial.Colors.green500
			text: Math.floor(progressBar.value)
			progressBar.from: 0
			progressBar.to: 0
			progressBar.value: shield
			iconLabel.icon.source: Qaterial.Icons.shield
			progressBar.width: Math.min(control.width*0.125, 100)

			property int shield: game.player ? game.player.shield : 0

			onShieldChanged: {
				if (shield > progressBar.to)
					progressBar.to = shield

				infoShield.marked = true
			}
		}



		GameInfo {
			id: infoTarget
			anchors.right: parent.right
			color: Qaterial.Colors.orange700
			iconLabel.icon.source: Qaterial.Icons.targetAccount
			text: Math.floor(progressBar.value)

			progressBar.from: 0
			progressBar.to: 0
			progressBar.value: enemies
			progressBar.width: Math.min(control.width*0.125, 100)

			property int enemies: game ? game.activeEnemies : 0

			onEnemiesChanged: {
				infoTarget.marked = true
				if (enemies>progressBar.to)
					progressBar.to = enemies
			}
		}

		Grid {
			id: toolsIconGrid
			anchors.right: parent.right
			width: infoShield.width
			layoutDirection: Qt.RightToLeft
			horizontalItemAlignment: Grid.AlignHCenter
			verticalItemAlignment: Grid.AlignVCenter

			bottomPadding: 10 * Qaterial.Style.pixelSizeRatio

			property real size: Qaterial.Style.pixelSize*1.3

			spacing: 5 * Qaterial.Style.pixelSizeRatio

			columns: Math.floor(width/size)

			Repeater {
				model: control.game ? control.game.toolListIcons : []

				Qaterial.Icon {
					size: toolsIconGrid.size
					icon: modelData.icon
					color: modelData.iconColor
					visible: true
					width: size
					height: size
				}
			}

		}




		GameButton {
			id: setttingsButton
			size: 30

			anchors.right: parent.right

			color: Qaterial.Colors.white
			border.color: fontImage.color
			border.width: 2

			fontImage.icon: Qaterial.Icons.cog
			fontImage.color: Qaterial.Colors.blueGray600
			fontImageScale: 0.7

			onClicked: {
				Qaterial.DialogManager.openFromComponent(_settingsDialog)
			}
		}

	}


	Row {
		id: rowTime

		visible: itemsVisible

		anchors.left: parent.left
		anchors.top: parent.top
		anchors.margins: 5
		anchors.topMargin: Math.max(Client.safeMarginTop, 5)
		anchors.leftMargin: Math.max(Client.safeMarginLeft, 10)

		GameButton {
			id: backButton
			size: 25

			anchors.verticalCenter: parent.verticalCenter

			visible: !gameScene.zoomOverview

			color: Qaterial.Colors.red800
			border.color: "white"
			border.width: 1

			fontImage.icon: Qaterial.Icons.close
			fontImage.color: "white"
			fontImageScale: 0.7

			onClicked: {
				Client.stackPop()
			}
		}

		GameLabel {
			id: infoTime
			color: Qaterial.Colors.cyan300

			anchors.verticalCenter: parent.verticalCenter

			iconLabel.icon.source: Qaterial.Icons.timerOutline

			iconLabel.text: game.msecLeft>=60000 ?
								Client.Utils.formatMSecs(game.msecLeft) :
								Client.Utils.formatMSecs(game.msecLeft, 1, false)

			visible: !gameScene.zoomOverview
		}
	}

	GameLabel {
		id: infoInvisibleTime
		color: Qaterial.Colors.yellow400

		anchors.left: parent.left
		anchors.top: rowTime.bottom
		anchors.margins: 5
		anchors.leftMargin: Math.max(Client.safeMarginLeft, 10)

		iconLabel.icon.source: Qaterial.Icons.dominoMask

		iconLabel.text: game && game.player ?
							(game.player.invisibleTime >= 60000 ?
								 Client.Utils.formatMSecs(game.player.invisibleTime) :
								 Client.Utils.formatMSecs(game.player.invisibleTime, 1, false)) :
							""

		visible: !gameScene.zoomOverview && game && game.player && game.player.invisible

		onVisibleChanged: if (visible)
							  marked = true
	}



	GameJoystick {
		id: joystick

		property real size: 200

		enabled: game && game.running

		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.bottomMargin: Math.max(5, Client.safeMarginBottom)
		anchors.leftMargin: Math.max(5, Client.safeMarginLeft)

		//width: Math.min(size, control.width*0.5)
		//height: Math.min((130/175)*size, control.width*0.4)

		visible: game && game.player && game.player.isAlive

		opacity: gameScene.zoomOverview ? 0.2 : 1.0

		onHasTouchChanged: if (!hasTouch && game.player) {
							   game.player.standbyMovingFlags()
						   }

		onJoystickMoved: (x,y) => {
							 if (game.player && hasTouch) {
								 var f = GamePlayer.Standby

								 if (y > 0.75) {
									 f |= GamePlayer.MoveUp
								 } else if (y < -0.75) {
									 f |= GamePlayer.MoveDown
								 } else {
									 f &= ~GamePlayer.MoveUp
									 f &= ~GamePlayer.MoveDown
								 }

								 if (x > 0.3) {
									 if (x > 0.85) {
										 f |= GamePlayer.MoveRight
										 f &= ~GamePlayer.SlowModifier
									 } else{
										 f |= GamePlayer.MoveRight
										 f |= GamePlayer.SlowModifier
									 }
								 } else if (x > 0.1) {
									 game.player.turnRight()
									 return
								 } else if (x < -0.3) {
									 if (x < -0.85) {
										 f |= GamePlayer.MoveLeft
										 f &= ~GamePlayer.SlowModifier
									 } else {
										 f |= GamePlayer.MoveLeft
										 f |= GamePlayer.SlowModifier
									 }
								 } else if (x < -0.1) {
									 game.player.turnLeft()
									 return
								 }

								 game.player.movingFlags = f

							 }
						 }
	}



	GameShotButton {
		id: shotButton

		visible: itemsVisible

		enabled: game && game.running

		player: game ? game.player : null
		opacity: gameScene && gameScene.zoomOverview ? 0.2 : (enemyAimed ? 1.0 : 0.6)

		anchors.bottom: parent.bottom
		anchors.right: parent.right
		anchors.rightMargin: Math.max(10, Client.safeMarginRight)
		anchors.bottomMargin: Math.max(10, Client.safeMarginBottom)

	}





	GameButton {
		id: pickButton
		size: 50

		width: shotButton.width
		height: 60

		anchors.horizontalCenter: shotButton.horizontalCenter
		anchors.bottom: shotButton.top

		visible: game && game.player

		property bool hasPickable: game && game.pickable

		enabled: game && game.running

		color: hasPickable ? Qaterial.Colors.green600 : "transparent"
		border.color: hasPickable ? fontImage.color : "white"
		border.width: 1

		opacity:  gameScene.zoomOverview ? 0.2 : (hasPickable ? 1.0 : 0.6)

		fontImage.icon: Qaterial.Icons.handRight
		fontImage.color: "white"
		fontImageScale: 0.6
		fontImage.anchors.horizontalCenterOffset: -2

		onClicked: {
			game.pickablePick()
		}
	}


	Column {
		id: toolColumn

		anchors.bottom: shotButton.bottom
		anchors.bottomMargin: (shotButton.height-50)/2
		anchors.right: shotButton.left

		spacing: 30

		Repeater {
			model: control.game ? control.game.tools : []

			GameButton {
				id: btn
				size: 50
				width: size
				height: size

				enabled: false
				visible: false

				anchors.horizontalCenter: parent.horizontalCenter

				color: enabled ? modelData.iconColor : "transparent"
				border.color: enabled ? fontImage.color : "white"
				border.width: 1

				opacity:  gameScene.zoomOverview ? 0.2 : (enabled ? 1.0 : 0.6)

				fontImage.icon: modelData.icon
				fontImage.color: "white"
				fontImageScale: 0.6

				onClicked: game.toolUse(modelData.type)

				Connections {
					target: game

					function onToolChanged(type, count) {
						if (type === modelData.type) {
							btn.enabled = game.toolCount(type) > 0

							if (modelData.dependency.length === 0)
								btn.visible = game.toolCount(type) > 0
						}
					}

					function onPlayerChanged() {
						if (!game.player) {
							if (modelData.dependency.length !== 0)
								btn.visible = false
						}
					}
				}

				Connections {
					target: game ? game.player : null

					enabled: modelData.dependency.length

					function onTerrainObjectChanged(type, object) {
						if (modelData.dependency.includes(type)) {
							btn.visible = object
						}
					}
				}

			}

		}


	}





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




	GameQuestionAction {
		id: gameQuestion

		anchors.fill: parent
		anchors.topMargin: Client.safeMarginTop
		anchors.bottomMargin: Client.safeMarginBottom
		anchors.leftMargin: Client.safeMarginLeft
		anchors.rightMargin: Client.safeMarginRight

		game: control.game

		z: 5
	}


	GameMessageList {
		id: messageList

		anchors.horizontalCenter: parent.horizontalCenter

		y: parent.height*0.25

		z: 6

		width: Math.min(450*Qaterial.Style.pixelSizeRatio, parent.width-Client.safeMarginLeft-Client.safeMarginRight)

		visible: !gameScene.zoomOverview && itemsVisible
	}


	Connections {
		target: game ? game.player : null

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

		function onHurt() {
			_painhudImage.play()
		}

		function onKilled() {
			//messageList.message(qsTr("Your man has died"), 3)
			skullImageAnim.start()
		}


		function onMovingFlagsChanged(flags) {
			if ((flags & GamePlayer.JoystickInteraction) || joystick.hasTouch)
				return

			var px = 0.5
			var py = 0.5

			if ((flags & GamePlayer.MoveLeft) && (flags & GamePlayer.SlowModifier))
				px = 0.25
			else if (flags & GamePlayer.MoveLeft)
				px = 0.0
			else if ((flags & GamePlayer.MoveRight) && (flags & GamePlayer.SlowModifier))
				px = 0.75
			else if (flags & GamePlayer.MoveRight)
				px = 1.0

			if (flags & GamePlayer.MoveUp)
				py = 0.0
			else if (flags & GamePlayer.MoveDown)
				py = 1.0

			//joystick.moveThumb(px*joystick.width, py*joystick.height)
			joystick.moveThumbRelative(px, py)
		}
	}


	Connections {
		target: game

		function onPlayerChanged() {
			if (game.player) {
				flick.setXOffset()
				flick.setYOffset()
			}
		}

		function onTimeNotify() {
			infoTime.marked = true
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
		},
		State {
			name: "start"
			PropertyChanges {
				target: control
				itemsVisible: true
			}
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
			PropertyChanges {
				target: control
				itemsVisible: true
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

				PropertyAction {
					target: control
					property: "itemsVisible"
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

				ScriptAction {
					script: {
						gameScene.onSceneAnimationReady()
					}
				}
			}
		}
	]




	Component {
		id: _settingsDialog
		Qaterial.ModalDialog
		{
			id: _dialog

			dialogImplicitWidth: 400 * Qaterial.Style.pixelSizeRatio

			horizontalPadding: 0
			bottomPadding: 1
			drawSeparator: true

			title: qsTr("Beállítások")

			standardButtons: DialogButtonBox.Close
			contentItem: QScrollable {
				leftPadding: 10 * Qaterial.Style.pixelSizeRatio
				rightPadding: 10 * Qaterial.Style.pixelSizeRatio
				topPadding: 5 * Qaterial.Style.pixelSizeRatio
				bottomPadding: 10 * Qaterial.Style.pixelSizeRatio
				Row {
					spacing: 5 * Qaterial.Style.pixelSizeRatio

					Qaterial.IconLabel {
						icon.source: Qaterial.Icons.gamepad
						anchors.verticalCenter: parent.verticalCenter
						text: qsTr("Joystick:")
					}

					QSpinBox {
						anchors.verticalCenter: parent.verticalCenter
						from: 200
						to: 600
						stepSize: 10

						font: Qaterial.Style.textTheme.body1

						value: joystick.size

						onValueModified: {
							joystick.size = value
							Client.Utils.settingsSet("window/joystick", joystick.size)
						}
					}
				}

				SettingsSound {
					width: parent.width
				}
			}
		}
	}

	StackView.onRemoved: {
		gameScene.stopSoundMusic()
	}

	StackView.onActivated: {
		state = "start"
		gameScene.load()
	}

	Component.onCompleted: {
		if (!game) {
			console.error(qsTr("null game"))
			return
		}

		gameScene.playSoundVoiceOver("qrc:/sound/voiceover/prepare_yourself.mp3")

		gameScene.forceActiveFocus()

		joystick.size = Client.Utils.settingsGet("window/joystick", joystick.size)
	}


	function messageFinish(_text : string, _icon : string, _success : bool) {
		closeDisabled = ""
		closeQuestion = ""
		onPageClose = null
		Qaterial.DialogManager.showDialog(
					{
						onAccepted: function() { Client.stackPop(control) },
						onRejected: function() { Client.stackPop(control) },
						text: _text,
						title: qsTr("Game over"),
						iconSource: _icon,
						iconColor: _success ? Qaterial.Colors.green500 : Qaterial.Colors.red500,
						textColor: _success ? Qaterial.Colors.green500 : Qaterial.Colors.red500,
						iconFill: false,
						iconSize: Qaterial.Style.roundIcon.size,
						standardButtons: DialogButtonBox.Ok
					})
	}


	function messageTooltip(_text : string, _icon : string, _title : string) {
		Qaterial.DialogManager.showDialog(
					{
						text: _text,
						title: _title,
						iconSource: _icon,
						iconColor: Qaterial.Style.iconColor(),
						textColor: Qaterial.Style.iconColor(),
						iconFill: true,
						iconSize: Qaterial.Style.roundIcon.size,
						standardButtons: DialogButtonBox.Ok
					})
	}

}
