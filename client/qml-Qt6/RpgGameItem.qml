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
		if (game)
			game.rpgGame = _game
	}

	Rectangle {
		anchors.fill: parent
		color: Qaterial.Colors.black
	}

	RpgGameImpl {
		id: _game
		anchors.fill: parent
		joystick: _gameJoystick
		messageList: _messageList
		defaultMessageColor: Qaterial.Style.iconColor()

		focus: true

		onGameLoadFailed: Client.messageError("FAILED")

		Component.onCompleted: forceActiveFocus()
	}

	Row {
		id: _playerRow

		anchors.left: parent.left
		anchors.leftMargin: Math.max(5 * Qaterial.Style.pixelSizeRatio, Client.safeMarginLeft)
		anchors.top: parent.top
		anchors.topMargin: Math.max(5 * Qaterial.Style.pixelSizeRatio, Client.safeMarginTop)

		spacing: 5 * Qaterial.Style.pixelSizeRatio

		/*readonly property real _itemWidth: Math.min((root.width -x -Client.safeMarginRight)/
													Math.max((game ? game.playersModel.count : 1), 1),
													250 * Qaterial.Style.pixelSizeRatio)*/


		GameButton {
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

		/*Repeater {
			model: SortFilterProxyModel {
				sourceModel: game ? game.playersModel : null

				sorters: [
					FilterSorter {
						filters: ValueFilter {
							enabled: Client.server
							roleName: "username"
							value: Client.server ? Client.server.user.username : ""
						}
						priority: 2
					},
					RoleSorter {
						roleName: "playerId"
						priority: 1
					}
				]

			}
			delegate: ConquestPlayerItem {
				id: _cpItem
				width: _playerRow._itemWidth - _playerRow.spacing
				anchors.verticalCenter: parent.verticalCenter

				visible: game && game.config.gameState !== ConquestConfig.StateFinished

				username: model.username
				character: model.character
				fullNickName: model.fullNickName
				xp: model.xp
				hp: model.hp
				streak: model.streak
				playerId: model.playerId
				online: model.online
				game: root.game
				targetFighter1: _battleRow.itemFighter1
				targetFighter2: _battleRow.itemFighter2
			}
		}*/
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
			if (hp < _oldHP)
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


	GameMessageList {
		id: _messageList

		anchors.horizontalCenter: parent.horizontalCenter

		y: parent.height*0.25
		z: 6

		width: Math.min(450*Qaterial.Style.pixelSizeRatio, parent.width-Client.safeMarginLeft-Client.safeMarginRight)
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
		id: pickRButton
		size: 50

		anchors.horizontalCenter: _shot.horizontalCenter
		anchors.bottom: weaponButton.top
		anchors.bottomMargin: 5

		visible: _game.controlledPlayer && _game.controlledPlayer.currentPickable

		color: Qaterial.Colors.green600
		border.color: fontImage.color
		border.width: 1

		opacity: 1.0

		fontImage.icon: Qaterial.Icons.hand
		fontImage.color: "white"
		fontImageScale: 0.6
		fontImage.anchors.horizontalCenterOffset: -2

		onClicked: {
			_game.controlledPlayer.pickCurrentObject()
			//_game.transport(_game.controlledPlayer, _game.controlledPlayer.currentTransport)
		}
	}


	GameButton {
		id: weaponButton
		size: 50

		anchors.horizontalCenter: _shot.horizontalCenter
		anchors.bottom: pickButton.top
		anchors.bottomMargin: 5

		visible: _game.controlledPlayer

		//enabled: _game.controlledPlayer && _game.controlledPlayer.currentTransport && _game.controlledPlayer.currentTransport.

		color: Qaterial.Colors.blue600
		border.color: fontImage.color
		border.width: 1

		opacity: 1.0

		fontImage.icon: Qaterial.Icons.arrowRightBold
		fontImage.color: "white"
		fontImageScale: 0.6
		fontImage.anchors.horizontalCenterOffset: -2

		onClicked: {
			_game.controlledPlayer.armory.nextWeapon()
		}
	}


	GameButton {
		id: pickButton
		size: 50

		anchors.horizontalCenter: _shot.horizontalCenter
		anchors.bottom: _shot.top
		anchors.bottomMargin: 5

		visible: _game.controlledPlayer && _game.controlledPlayer.currentTransport

		//enabled: _game.controlledPlayer && _game.controlledPlayer.currentTransport && _game.controlledPlayer.currentTransport.

		color: Qaterial.Colors.green600
		border.color: fontImage.color
		border.width: 1

		opacity: 1.0

		fontImage.icon: Qaterial.Icons.doorOpen
		fontImage.color: "white"
		fontImageScale: 0.6
		fontImage.anchors.horizontalCenterOffset: -2

		onClicked: {
			_game.transport(_game.controlledPlayer, _game.controlledPlayer.currentTransport)
		}
	}

	GameButton {
		id: _shot
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

		/*Connections {
			target: player

			function onAttack() {
				tapAnim.start()
			}
		}*/
	}


	StackView.onActivated: {
		if (game)
			game.rpgGameActivated()
	}


	StackView.onDeactivating: {
		_game.visible = false
	}

}



