import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


FocusScope {
	id: root

	property alias game: _item.game
	property alias changer: _changerDialog

	property alias minimapVisible: _mapRect.visible
	property real gameControlRatio: 1.0
	readonly property real _controlRatioMin: 1.0
	readonly property real _controlRatioMax: 2.5

	signal closeRequest()

	onWidthChanged: {
		_item.resetBaseScale()
	}

	onHeightChanged: {
	}



	RpgGameItemImpl {
		id: _item
		anchors.fill: parent

		joystickA: _gameJoystickMove
		joystickB: _gameJoystickControl
		joystickC: _gameJoystickShot

		readonly property bool multiplayer: game && game.gameMode == RpgGame.MultiPlayer

		messageList: _messageList
		defaultMessageColor: Qaterial.Style.iconColor()

		visible: isContentReady

		focus: true
		layer.enabled: true

		onGameLoadFailed: errorString => Client.messageError(errorString, qsTr("Pálya betöltése sikertelen"))
		onIsContentReadyChanged: if (isContentReady) startGame()

		onMinimapToggleRequest: _mapRect.visible = !_mapRect.visible
		onChangerRequest: _changerDialog.open()
		onMpMarkerRequest: _infoMP.marked = true

		onStageChanged: _infoTime.marked = true

		function resetBaseScale() {
			if (width < 576 * Qaterial.Style.devicePixelSizeCorrection)
				baseScale = 0.5
			else if (width < 786 * Qaterial.Style.devicePixelSizeCorrection)
				baseScale = 0.6
			else if (width < 992 * Qaterial.Style.devicePixelSizeCorrection)
				baseScale = 0.7
			else if (width < 1200 * Qaterial.Style.devicePixelSizeCorrection)
				baseScale = 0.8
			else
				baseScale = 1.0
		}

		Component.onCompleted: forceActiveFocus()
	}


	onGameChanged: {
		if (game)
			game.gameQuestion = _gameQuestion
	}



	Row {
		id: _rowTime

		anchors.left: parent.left
		anchors.top: parent.top
		anchors.margins: 5
		anchors.topMargin: Math.max(Client.safeMarginTop, 5)
		anchors.leftMargin: Math.max(Client.safeMarginLeft, 10)

		visible: _item.isContentReady

		GameButton {
			id: _backButton
			size: Qt.platform.os === "android" || Qt.platform.os === "ios" ? 35 : 25

			anchors.verticalCenter: parent.verticalCenter

			color: Qaterial.Colors.red800
			border.color: "white"
			border.width: 1

			fontImage.icon: _item.multiplayer || _gameQuestion.objectiveUuid != "" ? Qaterial.Icons.close : Qaterial.Icons.pause
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

			iconLabel.text: !game ? "---" :
									game.msecLeft >= 60000 ?
										Client.Utils.formatMSecs(game.msecLeft) :
										Client.Utils.formatMSecs(game.msecLeft, 1, false)
		}

		GameLabel {
			id: _playerZ
			visible: Qt.platform.os === "linux" && Client.debug && game && game.controlledPlayer && game.controlledPlayer.visualItem
			iconLabel.text: visible ? game.controlledPlayer.visualItem.z : ""
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
				///Qaterial.DialogManager.openFromComponent(_settingsDialog)
			}
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

			visible: _item.currentScene && _item.currentScene.scale !== _item.baseScale

			size: Qt.platform.os === "android" || Qt.platform.os === "ios" ? 40 : 30

			color: "transparent"
			border.color: fontImage.color
			border.width: 2

			fontImage.icon: Qaterial.Icons.magnifyClose
			fontImage.color: Qaterial.Colors.white
			fontImageScale: 0.7

			onClicked: {
				_item.currentScene.scaleResetRequest()
			}
		}
	}


	GameButton {
		id: _utilityButton
		size: 40

		anchors.verticalCenter: parent.verticalCenter
		anchors.left: _rowTime.left

		color: game && game.controlledPlayer && game.controlledPlayer.canUseUtility ? Qaterial.Colors.pink400 : "transparent"

		visible: game && game.controlledPlayer ? game.controlledPlayer.hasUtility : false

		border.color: fontImage.color
		border.width: 2

		fontImage.icon: Qaterial.Icons.wifi
		fontImage.color: Qaterial.Colors.white
		fontImageScale: 0.7

		onClicked: {
			game.controlledPlayer.useCurrentUtility()
		}
	}



	RpgChangerButton {
		id: _changerButton

		anchors.verticalCenter: parent.verticalCenter
		anchors.right: _colInfo.right

		changer: _changerDialog
	}




	GameJoystick {
		id: _gameJoystickMove

		anchors.left: parent.left
		anchors.bottom: parent.bottom

		visible: game && game.controlledPlayer && game.controlledPlayer.hp > 0 && _item.isContentReady

		///extendedSize: !game.mouseAttac&& !_game.mouseNavigation

		size: 120 * Qaterial.Style.pixelSizeRatio * gameControlRatio
		thumbSize: 40 * Qaterial.Style.pixelSizeRatio * gameControlRatio

		bounding: Qt.rect(0, parent.height*0.5, parent.width*0.5, parent.height*0.5)

		moveToTap: true
	}



	GameJoystick {
		id: _gameJoystickShot

		anchors.right: parent.right
		anchors.bottom: _gameJoystickControl.top

		visible: game && game.controlledPlayer && game.controlledPlayer.hp > 0 && _item.isContentReady &&
				 game.controlledPlayer.bullet > 0

		size: 90 * Qaterial.Style.pixelSizeRatio * gameControlRatio
		thumbSize: 40 * Qaterial.Style.pixelSizeRatio * gameControlRatio

		fontImage.icon: "qrc:/internal/game/target1.svg"
		fontImage.color: Qaterial.Colors.white

		thumb.color: Qaterial.Colors.red700
		thumb.border.color: Qaterial.Colors.black

		bounding: Qt.rect(parent.width*0.6, parent.height*0.5, parent.width*0.4, parent.height*0.5)
	}


	GameJoystick {
		id: _gameJoystickControl

		anchors.right: parent.right
		anchors.bottom: parent.bottom

		visible: game && game.controlledPlayer && game.controlledPlayer.hp > 0 && _item.isContentReady &&
				 game.controlledPlayer.hasDefender

		size: 90 * Qaterial.Style.pixelSizeRatio * gameControlRatio
		thumbSize: 40 * Qaterial.Style.pixelSizeRatio * gameControlRatio

		//fontImage.icon: "qrc:/internal/game/target1.svg"
		//fontImage.color: Qaterial.Colors.white

		thumb.color: Qaterial.Colors.green700
		thumb.border.color: Qaterial.Colors.black

		bounding: Qt.rect(parent.width*0.6, parent.height*0.5, parent.width*0.4, parent.height*0.5)
	}








	Column {
		id: _colInfo

		anchors.right: parent.right
		anchors.top: parent.top
		anchors.topMargin: Math.max(Client.safeMarginTop, 5)
		anchors.rightMargin: Math.max(Client.safeMarginRight, 7)
		spacing: 5 * Qaterial.Style.pixelSizeRatio

		visible: _item.isContentReady

		Row {
			anchors.right: parent.right

			spacing: 0

			GameLabel {
				id: _infoCurrencyTeam

				anchors.verticalCenter: parent.verticalCenter

				pixelSize: 16 * Qaterial.Style.pixelSizeRatio

				readonly property bool success: game && game.questPtsRq > 0 && game.ptsTeam >= game.questPtsRq

				color: success ? Qaterial.Colors.green400 :
								 game ? game.colorTeam : Qaterial.Colors.white

				iconLabel.icon.source: "qrc:/rpg/coin/coins.png"
				iconLabel.icon.color: "transparent"

				value: game ? game.ptsTeam : 0

				onSuccessChanged: marked = true
			}

			GameLabel {
				id: _infoCurrencyOpp

				anchors.verticalCenter: parent.verticalCenter

				pixelSize: 16 * Qaterial.Style.pixelSizeRatio

				color: game ? game.colorOpponent : Qaterial.Colors.white
				//iconLabel.icon.source: "qrc:/rpg/coin/coins.png"
				//iconLabel.icon.color: "transparent"

				value: game ? game.ptsOpponent : 0

				visible: _item.multiplayer
			}
		}


		GameInfo {
			id: _questQuestion
			anchors.right: parent.right

			readonly property bool success: game && game.questQuestionRq > 0 && game.questQuestion >= game.questQuestionRq

			color: success ? Qaterial.Colors.green400 :Qaterial.Colors.cyan400
			iconLabel.icon.source: success ? Qaterial.Icons.checkCircle : Qaterial.Icons.headQuestionOutline

			visible: game && game.controlledPlayer && _item.isContentReady

			text: game ? game.questQuestion : ""

			progressBar.from: 0
			progressBar.to: game ? game.questQuestionRq : 0
			progressBar.value: game ? game.questQuestion : 0
			progressBar.width: Math.min(root.width*0.125, 50)

			onSuccessChanged: marked = true
		}

		GameInfo {
			id: _questStreak
			anchors.right: parent.right

			readonly property bool success: game && game.questStreakRq > 0 && game.questStreak >= game.questStreakRq

			color: success ? Qaterial.Colors.green400 :Qaterial.Colors.yellow600
			iconLabel.icon.source: success ? Qaterial.Icons.checkCircle : Qaterial.Icons.tableRow

			visible: game && game.controlledPlayer && _item.isContentReady

			text: game ? game.questStreak : ""

			progressBar.from: 0
			progressBar.to: game ? game.questStreakRq : 0
			progressBar.value: game ? game.questStreak : 0
			progressBar.width: Math.min(root.width*0.125, 50)

			onSuccessChanged: marked = true
		}
	}


	GameHpLabel {
		id: infoHP
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.topMargin: Math.max(5, Client.safeMarginTop)
		anchors.top: parent.top
		value: game && game.controlledPlayer ? game.controlledPlayer.hp : 0
		visible: game && game.controlledPlayer && _item.isContentReady
		//onValueChanged: marked = true
	}

	GameInfo {
		id: _infoMP
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.top: infoHP.bottom
		color: Qaterial.Colors.pink300
		iconLabel.icon.source: Qaterial.Icons.shimmer
		text: qsTr("%1/%2 MP").arg(Math.floor(progressBar.value)).arg(progressBar.to)

		visible: game && game.controlledPlayer && _item.isContentReady

		progressBar.from: 0
		progressBar.to: game && game.controlledPlayer ? game.controlledPlayer.maxMp : 0
		progressBar.value: game && game.controlledPlayer ? game.controlledPlayer.mp : 0
		progressBar.width: Math.min(root.width*0.3, 60)
	}


	GameInfo {
		id: _infoBullet

		anchors.horizontalCenter: parent.horizontalCenter
		anchors.top: _infoMP.bottom

		color: Qaterial.Colors.orange500
		text: Math.floor(progressBar.value)

		progressBar.from: 0
		progressBar.to: game && game.controlledPlayer ? game.controlledPlayer.maxBullet : 0
		progressBar.value: bullet
		iconLabel.icon.source: Qaterial.Icons.bullet
		progressBar.width: Math.min(root.width*0.25, 50)

		//opacity: canShot ? 1.0 : 0.0

		visible: bullet > 0

		readonly property int bullet: game && game.controlledPlayer ?
										  game.controlledPlayer.bullet :
										  0

		property int _oldBullet: -1


		onBulletChanged: {
			_infoBullet.marked = true

			if (bullet == 0 && _oldBullet > 0)
				_messageList.message(qsTr("No bullet"), Qaterial.Colors.red400)

			_oldBullet = bullet
		}
	}





	RpgChangerDialog {
		id: _changerDialog

		anchors.fill: parent
		anchors.topMargin: Client.safeMarginTop
		anchors.bottomMargin: Client.safeMarginBottom
		anchors.leftMargin: Client.safeMarginLeft
		anchors.rightMargin: Client.safeMarginRight

		game: root.game

		z: 4

		onActiveChanged: {
			if (!active)
				_item.forceActiveFocus()
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

		property int msecLeft: 0
		property color progressColor: Qaterial.Style.iconColor()

		onMsecLeftChanged: {
			if (msecLeft > 0) {
				_questionProgress.from = msecLeft
				_questionProgress.to = root.game.msecLeft
			} else {
				_questionProgress.from = 0
				_questionProgress.to = 0
			}
		}
	}


	ProgressBar {
		id: _questionProgress
		visible: to > 0
		anchors.bottom: parent.bottom
		width: parent.width

		z: 5

		from: 0
		to: 0
		value: to > 0 ? game.msecLeft : 0

		Material.accent: _gameQuestion.progressColor

		Behavior on value {
			NumberAnimation { duration: 100; easing.type: Easing.Linear }
		}
	}



	GameSkullImage {
		id: _skullImage
		anchors.centerIn: parent
		z: 9

		Connections {
			target: game ? game.controlledPlayer : nullptr

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
		readonly property int hp: game && game.controlledPlayer ? game.controlledPlayer.hp : -1

		onHpChanged: {
			if (hp < _oldHP && _oldHP != -1)
				play()
			else if (hp > _oldHP && _oldHP != -1)
				_messageList.message("+%1 HP".arg(hp-_oldHP), Qaterial.Colors.red400)

			_oldHP = hp
		}

		Connections {
			target: game ? game.controlledPlayer : nullptr

			function onHurt() {
				_painhudImage.play()
			}
		}
	}



	GameMessageList {
		id: _messageList

		anchors.horizontalCenter: parent.horizontalCenter

		y: Math.max(infoHP.y+infoHP.height, parent.height*0.1)
		z: 6

		width: Math.min(450*Qaterial.Style.pixelSizeRatio, parent.width-Client.safeMarginLeft-Client.safeMarginRight)
	}


	/*

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


	*/

	RpgGameMinimap {
		id: _mapRect

		anchors.fill: parent
		view.anchors.leftMargin: Math.max(20, Client.safeMarginLeft)
		view.anchors.rightMargin: Math.max(20, Client.safeMarginRight)
		view.anchors.topMargin: Math.max(20, Client.safeMarginTop, _rowTime.y+_backButton.y+_backButton.height)
		view.anchors.bottomMargin: Math.max(20, Client.safeMarginBottom)

		game: _item
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
	}

	StackView.onActivated: {
		_delayTimer.start()
		game.loadGameItem()
	}

	Timer {
		id: _delayTimer
		interval: 100 ///1700
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

		function onQuestSelectDataChanged() {
			if (!game.questSelectData)
				return

			_dialogLoader.sourceComponent = _questSelectDialog
		}

		function onQuestResultDataChanged() {
			if (!game.questResultData)
				return

			_dialogLoader.sourceComponent = _finishDialog
		}

		function onQuestSelectCompleted() {
			_dialogLoader.sourceComponent = undefined
		}
	}

	function startGame() {
		if (_item.isContentReady && _delayTimer._finished) {
			_loadingRect.visible = false
			game.gameItemPrepared()
		}
	}


	function setGameControlRatio(ratio) {
		gameControlRatio = Math.min(_controlRatioMax, Math.max(_controlRatioMin, ratio))
	}


	Loader {
		id: _dialogLoader

		anchors.fill: parent
	}

	Component {
		id: _finishDialog

		RpgFinishDialog {
			result: game.questResultData

			onCloseRequest: root.closeRequest()
		}
	}

	Component {
		id: _questSelectDialog

		RpgQuestSelectDialog {
			game: _item.game
			questData: game.questSelectData

			Component.onDestruction: {
				_item.forceActiveFocus()
			}
		}
	}


}



