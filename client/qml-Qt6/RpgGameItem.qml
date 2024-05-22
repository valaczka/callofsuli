import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


FocusScope {
	id: root

	property ActionRpgGame game: null

	property alias minimapVisible: _mapRect.visible

	readonly property bool _isPrepared: _prGameSet && _prGameLoaded && _prCmpCompleted && _prStackActivated

	property bool _prGameSet: false
	property bool _prGameLoaded: false
	property bool _prCmpCompleted: false
	property bool _prStackActivated: false

	onWidthChanged: {
		setBaseScale()
		_shotButton.reset()
	}

	onHeightChanged: {
		_shotButton.reset()
	}

	function setBaseScale() {
		if (width < 576 * Qaterial.Style.devicePixelSizeCorrection)
			_game.baseScale = 0.5
		else if (width < 786 * Qaterial.Style.devicePixelSizeCorrection)
			_game.baseScale = 0.6
		else if (width < 992 * Qaterial.Style.devicePixelSizeCorrection)
			_game.baseScale = 0.7
		else if (width < 1200 * Qaterial.Style.devicePixelSizeCorrection)
			_game.baseScale = 0.8
		else
			_game.baseScale = 1.0
	}

	onGameChanged: {
		if (game) {
			game.rpgGame = _game
			_prGameSet = true
		}
	}

	Rectangle {
		anchors.fill: parent
		color: Qaterial.Colors.black
	}

	RpgGameImpl {
		id: _game
		anchors.fill: parent
		joystick: _gameJoystick
		gameQuestion: _gameQuestion
		messageList: _messageList
		defaultMessageColor: Qaterial.Style.iconColor()

		visible: _isPrepared

		focus: true
		layer.enabled: true

		onGameLoadFailed: errorString => Client.messageError(errorString, qsTr("Pálya betöltése sikertelen"))

		onGameLoaded: _prGameLoaded = true

		onMinimapToggleRequest: _mapRect.visible = !_mapRect.visible

		Component.onCompleted: forceActiveFocus()
	}

	Row {
		id: _rowTime

		anchors.left: parent.left
		anchors.top: parent.top
		anchors.margins: 5
		anchors.topMargin: Math.max(Client.safeMarginTop, 5)
		anchors.leftMargin: Math.max(Client.safeMarginLeft, 10)

		visible: _isPrepared

		GameButton {
			id: _backButton
			size: 25

			anchors.verticalCenter: parent.verticalCenter

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
			id: _infoTime
			color: Qaterial.Colors.cyan300

			anchors.verticalCenter: parent.verticalCenter

			iconLabel.icon.source: Qaterial.Icons.timerOutline

			iconLabel.text: game.msecLeft >= 60000 ?
								Client.Utils.formatMSecs(game.msecLeft) :
								Client.Utils.formatMSecs(game.msecLeft, 1, false)
		}
	}







	GameJoystick {
		id: _gameJoystick
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.leftMargin: Math.max(Client.safeMarginLeft,
							   (Qt.platform.os == "android" || Qt.platform.os == "ios") ? 50 : 0
							   )
		anchors.bottomMargin: Math.max(Client.safeMarginBottom,
							   (Qt.platform.os == "android" || Qt.platform.os == "ios") ? 50 : 0
							   )
		visible: _game.controlledPlayer && _game.controlledPlayer.hp > 0 && _isPrepared
		//size: Client.Utils.settingsGet("window/joystick", 160)
	}






	GameSkullImage {
		id: _skullImage
		anchors.centerIn: parent
		z: 9

		Connections {
			target: _game.controlledPlayer

			function onBecameDead() {
				_skullImage.play()
			}
		}
	}

	GamePainHud {
		id: _painhudImage
		anchors.fill: parent
		z: 10

		property int _oldHP: -1
		readonly property int hp: _game.controlledPlayer ? _game.controlledPlayer.hp : -1

		onHpChanged: {
			if (hp < _oldHP && _oldHP != -1)
				play()
			else if (hp > _oldHP && _oldHP != -1)
				_messageList.message("+%1 HP".arg(hp-_oldHP), Qaterial.Colors.red400)

			_oldHP = hp
		}

		Connections {
			target: _game.controlledPlayer

			function onHurt() {
				_painhudImage.play()
			}
		}
	}




	Column {
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.topMargin: Math.max(Client.safeMarginTop, 5)
		anchors.rightMargin: Math.max(Client.safeMarginRight, 7)
		spacing: 5 * Qaterial.Style.pixelSizeRatio

		visible: _isPrepared

		GameLabel {
			id: _labelXP
			anchors.right: parent.right
			color: "white"

			pixelSize: 16 * Qaterial.Style.pixelSizeRatio

			text: "%1 XP"

			value: game ? game.xp : 0
		}



		GameInfo {
			id: _infoTarget
			anchors.right: parent.right
			color: Qaterial.Colors.orange700
			iconLabel.icon.source: Qaterial.Icons.targetAccount
			text: Math.floor(progressBar.value)

			progressBar.from: 0
			progressBar.to: 0
			progressBar.value: enemies
			progressBar.width: Math.min(root.width*0.125, 50)

			property int enemies: _game ? _game.enemyCount : 0

			onEnemiesChanged: {
				_infoTarget.marked = true
				if (enemies>progressBar.to)
					progressBar.to = enemies
			}
		}

		GameInfo {
			id: _infoShield
			anchors.right: parent.right
			color: Qaterial.Colors.green500
			text: Math.floor(progressBar.value)
			progressBar.from: 0
			progressBar.to: 0
			progressBar.value: shield
			iconLabel.icon.source: Qaterial.Icons.shield
			progressBar.width: Math.min(root.width*0.125, 50)

			visible: shield > 0

			readonly property int shield: _game.controlledPlayer ? _game.controlledPlayer.shieldCount : 0

			onShieldChanged: {
				if (shield > progressBar.to)
					progressBar.to = shield
			}
		}

		GameInfo {
			id: _infoBullet
			anchors.right: parent.right
			color: Qaterial.Colors.blue500
			text: Math.floor(progressBar.value)
			progressBar.from: 0
			progressBar.to: maxBullet
			progressBar.value: bullet
			iconLabel.icon.source: Qaterial.Icons.bullet
			progressBar.width: Math.min(root.width*0.125, 50)

			opacity: canShot ? 1.0 : 0.0

			readonly property bool canShot:  _game.controlledPlayer && _game.controlledPlayer.armory.currentWeapon &&
											 _game.controlledPlayer.armory.currentWeapon.canShot

			readonly property int bullet: canShot ?
											  _game.controlledPlayer.armory.currentWeapon.bulletCount :
											  0

			readonly property int maxBullet: canShot ?
												 _game.controlledPlayer.armory.currentWeapon.maxBulletCount :
												 0

			onBulletChanged: {
				_infoBullet.marked = true
			}
		}


		Grid {
			id: _inventoryGrid
			anchors.right: parent.right
			width: _infoShield.width
			layoutDirection: Qt.RightToLeft
			horizontalItemAlignment: Grid.AlignHCenter
			verticalItemAlignment: Grid.AlignVCenter

			bottomPadding: 10 * Qaterial.Style.pixelSizeRatio

			property real size: Qaterial.Style.pixelSize*1.3

			spacing: 5 * Qaterial.Style.pixelSizeRatio

			columns: Math.floor(width/size)

			Repeater {
				model: _game.controlledPlayer ? _game.controlledPlayer.inventory : null

				Qaterial.Icon {
					size: _inventoryGrid.size
					icon: model.icon
					color: model.iconColor
					visible: true
					width: size
					height: size
				}
			}

		}


		Row {
			anchors.right: parent.right

			spacing: 5

			GameButton {
				id: _mapButton
				size: Qt.platform.os == "android" || Qt.platform.os == "ios" ? 40 : 30

				anchors.verticalCenter: parent.verticalCenter

				color: "transparent"
				border.color: fontImage.color
				border.width: 2

				fontImage.icon: Qaterial.Icons.map
				fontImage.color: Qaterial.Colors.cyan300
				fontImageScale: 0.7

				onClicked: {
					_mapRect.visible = !_mapRect.visible
				}
			}

			GameButton {
				id: _setttingsButton
				size: Qt.platform.os == "android" || Qt.platform.os == "ios" ? 40 : 30

				anchors.verticalCenter: parent.verticalCenter

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


	}


	GameHpLabel {
		id: infoHP
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.topMargin: Math.max(5, Client.safeMarginTop)
		anchors.top: parent.top
		value: _game.controlledPlayer ? _game.controlledPlayer.hp : 0
		visible: _game.controlledPlayer && _isPrepared
		onValueChanged: marked = true
	}




	GameButton {
		id: _nextWeaponButton
		size: 40

		anchors.left: _rowTime.left
		y: Math.min((parent.height-height)/2, _gameJoystick.y-10-height)

		readonly property TiledWeapon weapon: _game.controlledPlayer ? _game.controlledPlayer.armory.nextWeapon : null

		visible: weapon && _game.controlledPlayer && _game.controlledPlayer.armory.currentWeapon != weapon && _isPrepared

		color: "transparent" //Qaterial.Colors.blue600
		border.color: Qaterial.Colors.white
		border.width: 1

		opacity: 1.0

		fontImage.icon: weapon ? weapon.icon : ""
		fontImage.color: "transparent"
		fontImageScale: 0.7

		onClicked: {
			_game.controlledPlayer.armory.changeToNextWeapon()
		}
	}


	GameButton {
		id: _pickButton
		size: 50

		anchors.horizontalCenter: _shotButton.horizontalCenter
		anchors.bottom: _shotButton.top
		anchors.bottomMargin: 5

		visible: _game.controlledPlayer && _game.controlledPlayer.currentPickable && _isPrepared

		color: Qaterial.Colors.green600
		border.color: fontImage.color
		border.width: 1

		opacity: 1.0

		fontImage.icon: Qaterial.Icons.hand
		fontImage.color: "white"
		fontImageScale: 0.6
		//fontImage.anchors.horizontalCenterOffset: -2

		onClicked: {
			_game.controlledPlayer.pickCurrentObject()
		}
	}



	Column {
		id: _columnButtons

		anchors.bottom: _shotButton.bottom
		anchors.bottomMargin: (_shotButton.height-50)/2
		anchors.right: _shotButton.left
		anchors.rightMargin: 10

		visible: _isPrepared

		spacing: 30

		/*GameButton {
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
		}*/

		GameButton {
			id: _transportButton
			size: 50

			anchors.horizontalCenter: parent.horizontalCenter

			visible: _game.controlledPlayer && _game.controlledPlayer.currentTransport

			readonly property bool isOpen: _game.controlledPlayer && _game.controlledPlayer.currentTransport &&
										   _game.controlledPlayer.currentTransport.isOpen

			color: isOpen ? Qaterial.Colors.green600 : "transparent"
			border.color: isOpen ? fontImage.color : "white"
			border.width: 1

			opacity: isOpen ? 1.0 : 0.6

			fontImage.icon: isOpen ? Qaterial.Icons.doorOpen : Qaterial.Icons.doorClosedLock
			fontImage.color: "white"
			fontImageScale: 0.6
			//fontImage.anchors.horizontalCenterOffset: -2

			onClicked: {
				_game.transportPlayer()
			}
		}

		GameButton {
			id: _containerButton
			size: 50

			anchors.horizontalCenter: parent.horizontalCenter

			visible: _game.controlledPlayer && _game.controlledPlayer.currentContainer &&
										   _game.controlledPlayer.currentContainer.isActive

			color: Qaterial.Colors.amber500
			border.color: fontImage.color
			border.width: 1

			fontImage.icon: Qaterial.Icons.eye
			fontImage.color: "white"
			fontImageScale: 0.6
			//fontImage.anchors.horizontalCenterOffset: -2

			onClicked: {
				_game.useContainer()
			}
		}


	}






	RpgShotButton {
		id: _shotButton
		size: 60

		/*anchors.right: parent.right
		anchors.bottom: parent.bottom
		anchors.margins: Math.max(10, Client.safeMarginRight, Client.safeMarginBottom,
								  (Qt.platform.os == "android" || Qt.platform.os == "ios") ? 30 : 0)*/

		readonly property TiledWeapon weapon: _game.controlledPlayer ? _game.controlledPlayer.armory.currentWeapon : null
		readonly property bool _canAttack: weapon && (weapon.canHit || weapon.canShot)

		visible: weapon && _isPrepared

		color: _canAttack ? Qaterial.Colors.red800 : "transparent"

		border.color: _canAttack ? Qaterial.Colors.white : Qaterial.Colors.red800
		border.width: 1

		fontImage.icon: weapon ? weapon.icon : ""
		fontImage.opacity: _canAttack ? 1.0 : 0.6
		fontImage.color: "transparent"
		fontImageScale: 0.7

		onClicked: {
			if (_game.controlledPlayer)
				_game.controlledPlayer.attackCurrentWeapon()
		}

		Connections {
			target: _game.controlledPlayer

			function onAttackDone() {
				_shotButton.tapAnim.start()
			}
		}
	}




	GameQuestionAction {
		id: _gameQuestion

		anchors.fill: parent
		anchors.topMargin: Client.safeMarginTop
		anchors.bottomMargin: Client.safeMarginBottom
		anchors.leftMargin: Client.safeMarginLeft
		anchors.rightMargin: Client.safeMarginRight

		game: root.game

		z: 5
	}

	GameMessageList {
		id: _messageList

		anchors.horizontalCenter: parent.horizontalCenter

		y: Math.max(infoHP.y+infoHP.height, parent.height*0.1)
		z: 6

		width: Math.min(450*Qaterial.Style.pixelSizeRatio, parent.width-Client.safeMarginLeft-Client.safeMarginRight)
	}




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
				/*Row {
					spacing: 5 * Qaterial.Style.pixelSizeRatio

					Qaterial.IconLabel {
						icon.source: Qaterial.Icons.gamepad
						anchors.verticalCenter: parent.verticalCenter
						text: qsTr("Joystick:")
					}

					QSpinBox {
						anchors.verticalCenter: parent.verticalCenter
						from: 90
						to: 600
						stepSize: 10

						font: Qaterial.Style.textTheme.body1

						value: _game ? _game.joystick.size : 0

						onValueModified: {
							_game.joystick.size = value
							Client.Utils.settingsSet("window/joystick", _game.joystick.size)
						}
					}
				}*/

				QFormSwitchButton
				{
					text: qsTr("Karakter mozgatása kattintással")

					checked: _game.mouseNavigation
					onToggled: {
						Client.Utils.settingsSet("game/mouseNavigation", checked)
						_game.mouseNavigation = checked
					}
				}


				QFormSwitchButton
				{
					text: qsTr("Lövés kattintással")

					checked: _game.mouseAttack
					onToggled: {
						Client.Utils.settingsSet("game/mouseAttack", checked)
						_game.mouseAttack = checked
					}
				}



				SettingsSound {
					width: parent.width
				}
			}
		}
	}

	RpgGameMinimap {
		id: _mapRect

		anchors.fill: parent
		view.anchors.leftMargin: Math.max(20, Client.safeMarginLeft)
		view.anchors.rightMargin: Math.max(20, Client.safeMarginRight)
		view.anchors.topMargin: Math.max(20, Client.safeMarginTop, _rowTime.y+_backButton.y+_backButton.height)
		view.anchors.bottomMargin: Math.max(20, Client.safeMarginBottom)

		game: _game
		visible: false
	}

	Rectangle {
		id: _loadingRect
		anchors.fill: parent
		color: Qaterial.Colors.black
		z: 999

		Row {
			spacing: 20
			anchors.centerIn: parent

			Qaterial.BusyIndicator {
				anchors.verticalCenter: parent.verticalCenter
				height: txt.height
				width: txt.height
			}

			Qaterial.LabelBody1 {
				id: txt
				anchors.verticalCenter: parent.verticalCenter
				text: qsTr("Betöltés")
				color: Qaterial.Style.accentColor
			}
		}
	}


	Component.onCompleted: _prCmpCompleted = true

	StackView.onActivated: {
		_prStackActivated = true
		if (game)
			game.rpgGameActivated()
		_delayTimer.start()
	}

	on_IsPreparedChanged: startGame()

	Timer {
		id: _delayTimer
		interval: 1700
		triggeredOnStart: false
		running: false
		repeat: false

		property bool _finished: false

		onTriggered: {
			_finished = true
			startGame()
		}
	}

	function startGame() {
		if (_isPrepared && _delayTimer._finished) {
			game.gamePrepared()
			_loadingRect.visible = false
		}
	}



}



