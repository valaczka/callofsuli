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
		if (_marketLoader.status != Loader.Null) {
			_marketLoader.source = ""
			return false
		}

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

		visible: _marketLoader.status != Loader.Ready

		property Component activeComponent: null

		onActiveComponentChanged: replace(null, activeComponent, {}, StackView.Immediate)
	}


	Component {
		id: _cmpConnect

		QScrollable {
			id: root

			contentCentered: true

			Qaterial.IconLabel {
				anchors.horizontalCenter: parent.horizontalCenter
				color: Qaterial.Colors.red400
				icon.source: Qaterial.Icons.alertCircle
				icon.width: Qaterial.Style.dashboardButtonSize*0.4
				icon.height: Qaterial.Style.dashboardButtonSize*0.4
				text: qsTr("Missing implementation!")
			}

		}
	}

	Component {
		id: _cmpCharacterSelect

		RpgCharacterSelect {
			game: root.game
		}
	}

	Component {
		id: _cmpRpg

		RpgGameItem {
			game: root.game
		}
	}

	Component {
		id: _cmpError

		ConquestError {
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



	Loader {
		id: _marketLoader

		anchors.fill: parent
		z: 10

		onLoaded: {
			item.activate()
			item.enabled = true
			item.forceActiveFocus()
			game.marketLoaded()
		}

		onStatusChanged: {
			if (status != Loader.Ready)
				game.marketUnloaded()
		}

		function loadMarket() {
			setSource("PageMarket.qml", {
						  game: root.game
					  })
		}
	}

	Connections {
		target: game

		/*function onHostModeChanged() {
			if (game.hostMode == ConquestGame.ModeHost)
				Client.snack(qsTr("Te vagy a host"))
		}*/


		function onMarketRequest() {
			_marketLoader.loadMarket()
		}

		function onConfigChanged() {
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
		target: game ? game.rpgGame : null

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
