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

	readonly property bool _isPrepared: _prGameSet && _prGameLoaded && _prCmpCompleted && _prStackActivated

	property bool _prGameSet: false
	property bool _prGameLoaded: false
	property bool _prCmpCompleted: false
	property bool _prStackActivated: false

	onWidthChanged: setBaseScale()

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

		focus: true

		onGameLoadFailed: Client.messageError("FAILED")

		onGameLoaded: _prGameLoaded = true

		Component.onCompleted: forceActiveFocus()
	}

	Row {
		id: _rowTime

		anchors.left: parent.left
		anchors.top: parent.top
		anchors.margins: 5
		anchors.topMargin: Math.max(Client.safeMarginTop, 5)
		anchors.leftMargin: Math.max(Client.safeMarginLeft, 10)

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
		visible: _game.controlledPlayer && _game.controlledPlayer.hp > 0
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



	property int _oldXP: -1
	readonly property int xp: 0//_game.controlledPlayer ? _game.xp

	onXpChanged: {
		if (_oldXP != -1) {
			let d = xp-_oldXP

			if (d > 0) {
				_messageList.message("+%1 XP".arg(d), Qaterial.Colors.green400)
			} else {
				_messageList.message("%1 XP".arg(d), Qaterial.Colors.red400)
			}
		}

		_oldXP = xp
	}




	Column {
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.topMargin: Math.max(Client.safeMarginTop, 5)
		anchors.rightMargin: Math.max(Client.safeMarginRight, 7)
		spacing: 5 * Qaterial.Style.pixelSizeRatio

		GameLabel {
			id: _labelXP
			anchors.right: parent.right
			color: "white"

			pixelSize: 16 * Qaterial.Style.pixelSizeRatio

			//text: (multiPlayerGame ? "MULTIPLAYER " : "") + "%1 XP"
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
			id: _infoBullet
			anchors.right: parent.right
			color: Qaterial.Colors.green500
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


	GameHpLabel {
		id: infoHP
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.topMargin: Math.max(5, Client.safeMarginTop)
		anchors.top: parent.top
		value: _game.controlledPlayer ? _game.controlledPlayer.hp : 0
		visible: _game.controlledPlayer
		onValueChanged: marked = true
	}




	GameButton {
		id: _nextWeaponButton
		size: 40

		anchors.left: _rowTime.left
		anchors.top: _rowTime.bottom
		anchors.topMargin: 5

		readonly property TiledWeapon weapon: _game.controlledPlayer ? _game.controlledPlayer.armory.nextWeapon : null

		visible: weapon && _game.controlledPlayer && _game.controlledPlayer.armory.currentWeapon != weapon

		color: Qaterial.Colors.blue600
		border.color: fontImage.color
		border.width: 1

		opacity: 1.0

		fontImage.icon: weapon ? weapon.icon : ""
		fontImage.color: "white"
		fontImageScale: 0.6
		//fontImage.anchors.horizontalCenterOffset: -2

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

		visible: _game.controlledPlayer && _game.controlledPlayer.currentPickable

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
			enabled: _game.controlledPlayer && _game.controlledPlayer.currentTransport &&
					 _game.controlledPlayer.currentTransport.isOpen

			color: enabled ? Qaterial.Colors.green600 : "transparent"
			border.color: enabled ? fontImage.color : "white"
			border.width: 1

			opacity: enabled ? 1.0 : 0.6

			fontImage.icon: enabled ? Qaterial.Icons.doorOpen : Qaterial.Icons.doorClosedLock
			fontImage.color: "white"
			fontImageScale: 0.6
			//fontImage.anchors.horizontalCenterOffset: -2

			onClicked: {
				_game.transport(_game.controlledPlayer, _game.controlledPlayer.currentTransport)
			}
		}


	}





	GameButton {
		id: _shotButton
		size: 60

		anchors.right: parent.right
		anchors.bottom: parent.bottom
		anchors.margins: 10

		readonly property TiledWeapon weapon: _game.controlledPlayer ? _game.controlledPlayer.armory.currentWeapon : null
		readonly property bool enemyAimed: _game.controlledPlayer && _game.controlledPlayer.enemy

		visible: weapon

		enabled: weapon && (weapon.canHit || weapon.canShot)

		color: enemyAimed && enabled ? Client.Utils.colorSetAlpha(Qaterial.Colors.red700, 0.7) : "transparent"

		border.color: enemyAimed && enabled ? Qaterial.Colors.black : Qaterial.Colors.white
		border.width: 1

		fontImage.icon: weapon ? weapon.icon : ""
		fontImage.color: Qaterial.Colors.white
		fontImage.opacity: enemyAimed ? 0.6 : 1.0
		fontImage.anchors.horizontalCenterOffset: -2

		tap.onTapped: {
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

		y: parent.height*0.25
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

						value: _game ? _game.joystick.size : 0

						onValueModified: {
							_game.joystick.size = value
							Client.Utils.settingsSet("window/joystick", _game.joystick.size)
						}
					}
				}

				SettingsSound {
					width: parent.width
				}
			}
		}
	}

	Component.onCompleted: _prCmpCompleted = true

	StackView.onActivated: {
		_prStackActivated = true
		if (game)
			game.rpgGameActivated()
	}

	on_IsPreparedChanged: {
		if (_isPrepared)
			game.gamePrepared()
	}



	StackView.onDeactivating: {
		_game.visible = false
	}

}



