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
	readonly property ActionRpgMultiplayerGame _multiplayer: game && (game instanceof ActionRpgMultiplayerGame) ? game : null

	property alias minimapVisible: _mapRect.visible
	property real gameControlRatio: 1.0
	readonly property real _controlRatioMin: 1.0
	readonly property real _controlRatioMax: 2.5

	readonly property bool _isPrepared: _prGameSet && _prGameLoaded && _prCmpCompleted && _prStackActivated

	property bool _prGameSet: false
	property bool _prGameLoaded: false
	property bool _prCmpCompleted: false
	property bool _prStackActivated: false

	signal closeRequest()

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

		onQuestsRequest: showQuests()

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
			size: Qt.platform.os === "android" || Qt.platform.os === "ios" ? 35 : 25

			anchors.verticalCenter: parent.verticalCenter

			color: Qaterial.Colors.red800
			border.color: "white"
			border.width: 1

			fontImage.icon: _multiplayer || _gameQuestion.objectiveUuid != "" ? Qaterial.Icons.close : Qaterial.Icons.pause
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

		GameLabel {
			id: _playerZ
			visible: Qt.platform.os === "linux" && Client.debug && game && game.rpgGame.controlledPlayer && game.rpgGame.controlledPlayer.visualItem
			iconLabel.text: visible ? game.rpgGame.controlledPlayer.visualItem.z : ""
			color: Qaterial.Colors.white
		}
	}


	Row {
		anchors.left: _rowTime.left
		anchors.top: _rowTime.bottom
		anchors.topMargin: 15 * Qaterial.Style.pixelSizeRatio

		spacing: 5

		GameButton {
			id: _setttingsButton
			size: Qt.platform.os === "android" || Qt.platform.os === "ios" ? 40 : 30

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

		GameButton {
			id: _questsButton
			size: Qt.platform.os === "android" || Qt.platform.os === "ios" ? 40 : 30

			anchors.verticalCenter: parent.verticalCenter

			visible: !_multiplayer

			color: "transparent"
			border.color: fontImage.color
			border.width: 2

			fontImage.icon: Qaterial.Icons.crosshairsQuestion
			fontImage.color: Qaterial.Colors.purple400
			fontImageScale: 0.7

			onClicked: showQuests()
		}

		GameButton {
			id: _playersButton
			size: Qt.platform.os === "android" || Qt.platform.os === "ios" ? 40 : 30

			anchors.verticalCenter: parent.verticalCenter

			visible: _multiplayer

			color: "transparent"
			border.color: fontImage.color
			border.width: 2

			fontImage.icon: Qaterial.Icons.accountMultiple
			fontImage.color: Qaterial.Colors.purple400
			fontImageScale: 0.7

			onClicked: showQuests()
		}

		GameButton {
			id: _mapButton
			size: Qt.platform.os === "android" || Qt.platform.os === "ios" ? 40 : 30

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
			id: _resetZoomButton

			anchors.verticalCenter: parent.verticalCenter

			visible: _game.currentScene && _game.currentScene.scale !== _game.baseScale

			size: Qt.platform.os === "android" || Qt.platform.os === "ios" ? 40 : 30

			color: "transparent"
			border.color: fontImage.color
			border.width: 2

			fontImage.icon: Qaterial.Icons.magnifyClose
			fontImage.color: Qaterial.Colors.white
			fontImageScale: 0.7

			onClicked: {
				_game.currentScene.scaleResetRequest()
			}
		}
	}







	GameJoystick {
		id: _gameJoystick
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		visible: _game.controlledPlayer && _game.controlledPlayer.hp > 0 && _isPrepared &&
				 !_game.controlledPlayer.isHiding && !_game.controlledPlayer.isGameCompleted

		extendedSize: !_game.mouseAttack && !_game.mouseNavigation

		size: 120 * Qaterial.Style.pixelSizeRatio * gameControlRatio
		thumbSize: 40 * Qaterial.Style.pixelSizeRatio * gameControlRatio

		maxWidth: parent.width*0.4
		maxHeight: parent.height*0.4
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
			id: _infoCurrency

			anchors.right: parent.right

			pixelSize: 16 * Qaterial.Style.pixelSizeRatio

			color: Qaterial.Colors.yellow500
			iconLabel.icon.source: "qrc:/rpg/coin/coins.png"
			iconLabel.icon.color: "transparent"

			value: game && game.rpgGame ? game.rpgGame.currency : 0
		}

		GameLabel {
			id: _labelXP
			anchors.right: parent.right
			color: "white"

			pixelSize: 16 * Qaterial.Style.pixelSizeRatio

			text: "%1 XP"

			value: game ? game.xp : 0
		}



		GameInfo {
			id: _infoCollection
			anchors.right: parent.right
			color: Qaterial.Colors.orange700
			iconLabel.icon.source: Qaterial.Icons.mapMarkerMultiple
			text: Math.floor(progressBar.value)

			visible: progressBar.to > 0

			progressBar.from: 0
			progressBar.to: _game && _game.controlledPlayer ? _game.controlledPlayer.collectionRq : 0
			progressBar.value: collection
			progressBar.width: Math.min(root.width*0.125, 50)

			property int collection: _game && _game.controlledPlayer ? _game.controlledPlayer.collection : 0

			onCollectionChanged: {
				_infoCollection.marked = true
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
											 _game.controlledPlayer.armory.currentWeapon.bulletCount != -1

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

	}


	GameHpLabel {
		id: infoHP
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.topMargin: Math.max(5, Client.safeMarginTop)
		anchors.top: parent.top
		value: _game.controlledPlayer ? _game.controlledPlayer.hp : 0
		visible: _game.controlledPlayer && _isPrepared
		//onValueChanged: marked = true
	}

	/*GameInfo {
		id: _infoMP
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.top: infoHP.bottom
		color: Qaterial.Colors.pink300
		iconLabel.icon.source: Qaterial.Icons.shimmer
		text: qsTr("%1/%2 MP").arg(Math.floor(progressBar.value)).arg(progressBar.to)

		visible: _game.controlledPlayer && _game.controlledPlayer.armory.mageStaff

		progressBar.from: 0
		progressBar.to: _game.controlledPlayer ? _game.controlledPlayer.maxMp : 0
		progressBar.value: _game.controlledPlayer ? _game.controlledPlayer.mp : 0
		progressBar.width: Math.min(root.width*0.3, 85)
	}*/




	GameButton {
		id: _nextWeaponButton
		size: Qt.platform.os === "android" || Qt.platform.os === "ios" ? 50 : 40

		anchors.left: _rowTime.left
		//y: Math.min((parent.height-height)/2, _gameJoystick.y-10-height)
		anchors.verticalCenter: parent.verticalCenter

		readonly property RpgWeapon weapon: _game.controlledPlayer ? _game.controlledPlayer.armory.nextWeapon : null

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




	Column {
		id: _columnButtons

		anchors.bottom: _shotButton.bottom
		anchors.bottomMargin: (_shotButton.height-50)/2
		anchors.right: _shotButton.left
		anchors.rightMargin: 10

		visible: _isPrepared && _game.controlledPlayer && !_game.controlledPlayer.isGameCompleted

		spacing: 30


		GameButton {
			id: _controlButton
			size: 50

			anchors.horizontalCenter: parent.horizontalCenter

			visible: _game.controlledPlayer && _game.controlledPlayer.currentControl &&
					 _game.controlledPlayer.currentControl.isActive && _game.controlledPlayer.hp > 0 &&
					 !_game.controlledPlayer.isHiding

			readonly property bool isOpen: _game.controlledPlayer && _game.controlledPlayer.currentControl &&
										   !_game.controlledPlayer.currentControl.isLocked

			color: {
				if (!_game.controlledPlayer || !_game.controlledPlayer.currentControl || !isOpen)
					return "transparent"

				switch (_game.controlledPlayer.currentControl.type) {
				case RpgConfig.ControlContainer:
				case RpgConfig.ControlCollection:
					return Qaterial.Colors.amber500

				case RpgConfig.ControlGate:
				case RpgConfig.ControlTeleport:
				case RpgConfig.ControlPickable:
					return Qaterial.Colors.green600

				default:
					return Qaterial.Colors.yellow500

				}
			}

			border.color: isOpen ? fontImage.color : "white"
			border.width: 1

			opacity: isOpen ? 1.0 : 0.6

			fontImage.icon: {
				if (!_game.controlledPlayer || !_game.controlledPlayer.currentControl)
					return Qaterial.Icons.alertOutline

				switch (_game.controlledPlayer.currentControl.type) {
				case RpgConfig.ControlContainer:
					return Qaterial.Icons.eye

				case RpgConfig.ControlGate:
				case RpgConfig.ControlTeleport:
					return isOpen ? Qaterial.Icons.doorOpen : Qaterial.Icons.doorClosedLock

				case RpgConfig.ControlCollection:
				case RpgConfig.ControlPickable:
					return Qaterial.Icons.hand

				default:
					return Qaterial.Icons.hand

				}
			}
			fontImage.color: "white"
			fontImageScale: 0.6
			//fontImage.anchors.horizontalCenterOffset: -2

			onClicked: {
				_game.controlledPlayer.useCurrentControl()
			}
		}


	}






	RpgShotButton {
		id: _shotButton
		size: 60 * gameControlRatio

		/*anchors.right: parent.right
		anchors.bottom: parent.bottom
		anchors.margins: Math.max(10, Client.safeMarginRight, Client.safeMarginBottom,
								  (Qt.platform.os == "android" || Qt.platform.os == "ios") ? 30 : 0)*/

		readonly property RpgWeapon weapon: _game.controlledPlayer ? _game.controlledPlayer.armory.currentWeapon : null
		readonly property bool _canAttack: weapon && (weapon.canHit || weapon.canShot)

		visible: weapon && _isPrepared && _game.controlledPlayer && _game.controlledPlayer.hp > 0 &&
				 !_game.controlledPlayer.isHiding && !_game.controlledPlayer.isGameCompleted

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


	GameButton {
		id: _exitButton
		size: 50

		anchors.centerIn: _shotButton

		visible: _game.controlledPlayer && _game.controlledPlayer.hp > 0 &&
				 _game.controlledPlayer.isHiding && !_game.controlledPlayer.isGameCompleted


		color: Qaterial.Colors.amber700

		border.color: fontImage.color
		border.width: 1

		fontImage.icon: Qaterial.Icons.exitRun
		fontImage.color: "white"
		fontImageScale: 0.6
		//fontImage.anchors.horizontalCenterOffset: -2

		onClicked: {
			_game.controlledPlayer.exitHiding()
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

				QFormSwitchButton
				{
					text: qsTr("Pálya szabad mozgatása")

					checked: _game.flickableInteractive
					onToggled: {
						Client.Utils.settingsSet("game/flickableInteractive", checked)
						_game.flickableInteractive = checked
					}
				}

				QFormSwitchButton
				{
					text: qsTr("Karakter mozgatása kattintással")

					checked: _game.mouseNavigation
					onToggled: {
						_game.mouseNavigation = checked
						Client.Utils.settingsSet("game/mouseNavigation", _game.mouseNavigation)
						Client.Utils.settingsSet("game/mouseAttack", _game.mouseAttack)
					}
				}


				QFormSwitchButton
				{
					text: qsTr("Lövés kattintással")

					checked: _game.mouseAttack
					onToggled: {
						_game.mouseAttack = checked
						Client.Utils.settingsSet("game/mouseNavigation", _game.mouseNavigation)
						Client.Utils.settingsSet("game/mouseAttack", _game.mouseAttack)
					}
				}


				Row {
					width: parent.width

					spacing: 5 * Qaterial.Style.pixelSizeRatio

					Qaterial.IconLabel {
						id: _controlLabel
						text: qsTr("Joystick")
						icon.color: Qaterial.Style.primaryTextColor()
						icon.source: Qaterial.Icons.gamepad
						anchors.verticalCenter: parent.verticalCenter
					}

					Qaterial.Slider {
						anchors.verticalCenter: parent.verticalCenter
						width: parent.width - parent.spacing - _controlLabel.width

						from: _controlRatioMin
						to: _controlRatioMax
						stepSize: 0.1
						value: root.gameControlRatio
						snapMode: Slider.SnapAlways

						onMoved: {
							root.gameControlRatio = value
							Client.Utils.settingsSet("window/gameControls", value)
						}
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


	Component.onCompleted: {
		setGameControlRatio(Client.Utils.settingsGet("window/gameControls", 1.0))

		_prCmpCompleted = true
	}

	StackView.onActivated: {
		Client.contextHelper.unsetContext(ContextHelperData.ContextStudentPlayMultiplayer)
		Client.contextHelper.unsetContext(ContextHelperData.ContextStudentPlayRpg)

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



	Connections {
		target: game

		function onFinishDialogRequest(text, icon, success) {
			_finishIcon = icon
			_finishText = text
			_finishSuccess = success

			Qaterial.DialogManager.openFromComponent(_multiplayer ? _cmpUserDialog : _cmpFinishQuests)
		}
	}

	function startGame() {
		if (_isPrepared && _delayTimer._finished) {
			game.gamePrepared()
			_loadingRect.visible = false
			//showQuests()
		}
	}



	function showQuests() {
		if (_multiplayer) {
			_finishIcon = ""
			_finishText = ""
			_finishSuccess = false

			Qaterial.DialogManager.openFromComponent(_cmpUserDialog)
		} else {
			Qaterial.DialogManager.openFromComponent(_cmpQuests)
		}
	}


	function setGameControlRatio(ratio) {
		gameControlRatio = Math.min(_controlRatioMax, Math.max(_controlRatioMin, ratio))
	}


	Component {
		id: _cmpQuests

		RpgQuestsDialog {
			game: root.game ? root.game.rpgGame : null
		}

	}

	property string _finishIcon: ""
	property string _finishText: ""
	property bool _finishSuccess: false

	Component {
		id: _cmpFinishQuests

		RpgQuestsDialog {
			game: root.game ? root.game.rpgGame : null

			onAccepted: if (root.game) root.game.finishGame()
			onRejected: if (root.game) root.game.finishGame()

			title: qsTr("Game over")

			text: _finishText
			iconColor: _finishSuccess ? Qaterial.Colors.green500 : Qaterial.Colors.red500
			textColor: _finishSuccess ? Qaterial.Colors.green500 : Qaterial.Colors.red500

			iconSize: Qaterial.Style.roundIcon.size
			iconSource: _finishIcon
			showFailed: true
		}

	}


	Component {
		id: _cmpUserDialog

		RpgGameUserDialog {
			game: _multiplayer

			text: _finishText
			iconColor: _finishSuccess ? Qaterial.Colors.green500 : Qaterial.Colors.red500
			textColor: _finishSuccess ? Qaterial.Colors.green500 : Qaterial.Colors.red500

			iconSize: Qaterial.Style.roundIcon.size
			iconSource: _finishIcon

			onAccepted: if (root.game && root.game.config.gameState === RpgConfig.StateFinished) {
							closeRequest()
						}

			onRejected: if (root.game && root.game.config.gameState === RpgConfig.StateFinished) {
							closeRequest()
						}
		}
	}
}



