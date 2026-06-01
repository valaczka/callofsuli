import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

Page {
	id: root

	property RpgGame game: null


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

		/*if (game && game.config.gameState == RpgConfig.StateError)
			return true


		if (game && game.rpgGame && game.rpgGame.gameQuestion && game.rpgGame.gameQuestion.objectiveUuid != "")
			return true

		if (_multiplayer)
			return true

		*/

		if (_rpgVisible && !game.gameItem.paused && !_forceExit) {
			game.gameItem.paused = true
			return false
		}

		return true
	}


	readonly property bool _rpgVisible: game && (game.gameState == RpgGame.GameStatePrepare ||
												 game.gameState == RpgGame.GameStateInit ||
												 game.gameState == RpgGame.GameStatePlay)


	property bool _oldWindowState: Client.fullScreenHelper
	property bool _forceExit: false



	Component {
		id: _cmpPause

		RpgPauseDialog {
			game: root.game ? root.game.gameItem : null

			onClosed: root.game.gameItem.paused = false

			onExitRequest: {
				_forceExit = true
				Client.stackPop()
			}

		}

	}

	Rectangle {
		anchors.fill: parent
		color: Qaterial.Colors.black
	}

	StackView {
		id: _stack
		anchors.fill: parent

		property Component activeComponent: null

		onActiveComponentChanged: replace(null, activeComponent, {}, StackView.Immediate)
	}


	Component {
		id: _cmpLobby

		RpgLobby {
			game: root.game
		}
	}

	/*Component {
		id: _cmpCharacterSelect

		RpgCharacterSelect {
			game: root.game

			onMarketRequest: Client.stackPushPage("PageMarket.qml")
		}
	}*/

	Component {
		id: _cmpRpg

		RpgGameItem {
			game: root.game

			//onCloseRequest: Client.stackPop(root)
		}
	}

	Component {
		id: _cmpError

		RpgError {
			game: root.game
		}
	}

	Component {
		id: _cmpFirstConnect

		RpgReconnect {
			firstConnect: true
			//game: root.game
		}
	}

	Component {
		id: _cmpReconnect

		RpgReconnect {
			//game: root.game
		}
	}


	Component {
		id: _cmpStaticDownload

		DownloaderItem {
			downloader: Client.downloader
		}
	}


	/*RpgReconnect {
		id: _reconnect

		anchors.fill: parent

		visible: game && game.isReconnecting
	}*/


	StudentDashboardNotification {
		id: _notification
		alwaysHide: _rpgVisible
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

		/*function onGameModeChanged() {
			if (game.gameMode == ActionRpgGame.MultiPlayerHost)
				Client.snack(qsTr("You are the host now"))
		}*/


		function onDownloadRequest(size) {
			JS.questionDialog({
								  onAccepted: function()
								  {
									  game.downloadAccepted()
								  },
								  onRejected: function()
								  {
									  Client.stackPop()
								  },
								  text: qsTr("Az akciójátékhoz %1 adatot le kell tölteni. Biztosan letöltöd?").arg(size),
								  iconSource: Qaterial.Icons.downloadNetwork,
								  title: qsTr("Adatok letöltése")
							  })
		}


		function onGameStateChanged() {
			/*if (game.isReconnecting && game.config.gameState != RpgConfig.StateError)
				return*/

			/*if (game.isReconnecting && game.config.gameState != RpgConfig.StateError) {
				_stack.activeComponent = _cmpReconnect
				return
			}*/

			switch (game.gameState) {
			case RpgGame.GameStatePrepare:
			case RpgGame.GameStateInit:
			case RpgGame.GameStatePlay:
				_stack.activeComponent = _cmpRpg
				break

			case RpgGame.GameStateError:
				_stack.activeComponent = _cmpError
				break

			case RpgGame.GameStateCharacterSelect:
				_stack.activeComponent = _cmpCharacterSelect
				break

			case RpgGame.GameStateFinished:
				//if (!_multiplayer)
				Client.stackPop(root)
				return

			case RpgGame.GameStateDownloadContent:
				_stack.activeComponent = _cmpStaticDownload
				return

			case RpgGame.GameStateLobby:
				_stack.activeComponent = _cmpLobby
				break

			default:
				_stack.activeComponent = _cmpFirstConnect
				break
			}
		}
	}



	Connections {
		target: game && game.gameMode == RpgGame.SinglePlayer ? game.gameItem : null

		function onPausedChanged() {
			if (game.gameItem.paused)
				Qaterial.DialogManager.openFromComponent(_cmpPause)
		}
	}



	StackView.onDeactivating: {
	}

	StackView.onActivated: {
		_notification.check()

		if (game)
			game.menuBgMusicPlay()

		if (Qt.platform.os != "android" && Qt.platform.os != "ios" && !Client.debug) {
			_oldWindowState = Client.fullScreenHelper
			Client.fullScreenHelper = true
		}
	}

	StackView.onRemoved: {
		if (game)
			game.menuBgMusicStop()

		if (Qt.platform.os != "android" && Qt.platform.os != "ios" && !Client.debug) {
			if (_oldWindowState != Client.fullScreenHelper)
				Client.fullScreenHelper = false
		}
	}
}
