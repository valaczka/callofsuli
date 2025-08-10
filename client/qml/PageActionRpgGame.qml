import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as J

Page {
	id: root

	property ActionRpgGame game: null

	readonly property ActionRpgMultiplayerGame _multiplayer: game && (game instanceof ActionRpgMultiplayerGame) ? game : null


	property string closeQuestion: _rpgVisible && !_forceExit ? qsTr("Biztosan kilépsz a játékból?") : ""
	property var onPageClose: function() {
		if (game)
			game.gameAbort()
	}

	property var stackPopFunction: function() {
		if (_stack.activeComponent == _cmpRpg) {
			if (_stack.currentItem.minimapVisible === true) {
				_stack.currentItem.minimapVisible = false
				return false
			}
		}

		if (game && game.config.gameState == RpgConfig.StateError)
			return true


		if (game && game.rpgGame && game.rpgGame.gameQuestion && game.rpgGame.gameQuestion.objectiveUuid != "")
			return true

		if (_multiplayer)
			return true

		if (_rpgVisible && !game.rpgGame.paused && !_forceExit) {
			game.rpgGame.paused = true
			return false
		}

		return true
	}


	readonly property bool _rpgVisible: game && (game.config.gameState == RpgConfig.StatePlay ||
												 game.config.gameState == RpgConfig.StatePrepare)


	property bool _oldWindowState: Client.fullScreenHelper
	property bool _forceExit: false



	Component {
		id: _cmpPause

		RpgPauseDialog {
			game: root.game ? root.game.rpgGame : null

			onClosed: root.game.rpgGame.paused = false

			onExitRequest: {
				_forceExit = true
				Client.stackPop()
			}

		}

	}

	StackView {
		id: _stack
		anchors.fill: parent

		property Component activeComponent: null

		onActiveComponentChanged: replace(null, activeComponent, {}, StackView.Immediate)
	}


	Component {
		id: _cmpConnect

		RpgConnect {
			game: root.game
		}
	}

	Component {
		id: _cmpCharacterSelect

		RpgCharacterSelect {
			game: root.game

			onMarketRequest: Client.stackPushPage("PageMarket.qml")
		}
	}

	Component {
		id: _cmpRpg

		RpgGameItem {
			game: root.game

			onCloseRequest: Client.stackPop(root)
		}
	}

	Component {
		id: _cmpError

		RpgError {
			game: root.game
		}
	}

	Component {
		id: _cmpReconnect

		RpgReconnect {
			//game: root.game
		}
	}

	Component {
		id: _cmpDownload

		DownloaderItem {
			downloader: game ? game.downloader : null
		}
	}

	Component {
		id: _cmpStaticDownload

		DownloaderItem {
			downloader: Client.downloader
		}
	}


	RpgReconnect {
		id: _reconnect

		anchors.fill: parent

		visible: game && game.isReconnecting
	}


	Qaterial.AppBarButton
	{
		id: _backButton
		anchors.left: parent.left
		anchors.leftMargin: Client.safeMarginLeft
		anchors.top: parent.top
		anchors.topMargin: Client.safeMarginTop
		icon.source: Qaterial.Icons.arrowLeft

		visible: _stack.activeComponent != _cmpRpg

		onClicked: Client.stackPop()
	}



	Connections {
		target: game

		function onGameModeChanged() {
			if (game.gameMode == ActionRpgGame.MultiPlayerHost)
				Client.snack(qsTr("You are the host now"))
		}

		function onConfigChanged() {
			if (game.isReconnecting && game.config.gameState != RpgConfig.StateError)
				return

			/*if (game.isReconnecting && game.config.gameState != RpgConfig.StateError) {
				_stack.activeComponent = _cmpReconnect
				return
			}*/

			switch (game.config.gameState) {
			case RpgConfig.StatePrepare:
			case RpgConfig.StatePlay:
				_stack.activeComponent = _cmpRpg
				break

			case RpgConfig.StateError:
				_stack.activeComponent = _cmpError
				break

			case RpgConfig.StateCharacterSelect:
				_stack.activeComponent = _cmpCharacterSelect
				break

			case RpgConfig.StateFinished:
				if (!_multiplayer)
					Client.stackPop(root)
				return

			case RpgConfig.StateDownloadStatic:
				_stack.activeComponent = _cmpStaticDownload
				return

			case RpgConfig.StateDownloadContent:
				_stack.activeComponent = _cmpDownload
				return

			default:
				_stack.activeComponent = _cmpConnect
				break
			}
		}
	}



	Connections {
		target: game && !_multiplayer ? game.rpgGame : null

		function onPausedChanged() {
			if (game.rpgGame.paused)
				Qaterial.DialogManager.openFromComponent(_cmpPause)
		}
	}



	StackView.onActivated: {
		if (game)
			game.playMenuBgMusic()

		if (Qt.platform.os != "android" && Qt.platform.os != "ios" && !Client.debug) {
			_oldWindowState = Client.fullScreenHelper
			Client.fullScreenHelper = true
		}
	}

	StackView.onRemoved: {
		if (game) {
			game.stopMenuBgMusic()
		}

		if (Qt.platform.os != "android" && Qt.platform.os != "ios" && !Client.debug) {
			if (_oldWindowState != Client.fullScreenHelper)
				Client.fullScreenHelper = false
		}
	}
}
