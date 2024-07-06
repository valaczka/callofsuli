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


	property string closeQuestion: _rpgVisible ? qsTr("Biztosan kilépsz a játékból?") : ""
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

		/*if (game && game.config.gameState == ConquestConfig.StateFinished && _stack.activeComponent == _cmpScene) {
			game.sendWebSocketMessage({
										  cmd: "prepare",
										  engine: game.engineId,
										  unprepare: true
									  })
			_isUnprepared = true
			return false
		}*/

		return true
	}

	readonly property bool _rpgVisible: game && (game.config.gameState == RpgConfig.StatePlay ||
												 game.config.gameState == RpgConfig.StatePrepare)


	property bool _oldWindowState: Client.fullScreenHelper

	//property bool _isUnprepared: false

	/*Image {
		anchors.fill: parent
		cache: true

		source: "qrc:/internal/img/bgConquest.jpg"
		fillMode: Image.PreserveAspectCrop
	}*/

	StackView {
		id: _stack
		anchors.fill: parent

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

		QScrollable {
			contentCentered: true
			spacing: 30 * Qaterial.Style.pixelSizeRatio

			Qaterial.IconLabel {
				readonly property real _progress: game && game.downloader.fullSize > 0 ?
													  game.downloader.downloadedSize/game.downloader.fullSize :
													  0.

				anchors.horizontalCenter: parent.horizontalCenter
				color: Qaterial.Style.accentColor
				icon.source: Qaterial.Icons.download
				icon.width: Qaterial.Style.dashboardButtonSize*0.4
				icon.height: Qaterial.Style.dashboardButtonSize*0.4
				text: qsTr("Tartalom letöltése folyamatban...\n%1%").arg(game ? Math.floor(_progress*100.) : 0)
			}

			Qaterial.ProgressBar
			{
				width: Math.min(250 * Qaterial.Style.pixelSizeRatio, parent.width*0.75)
				anchors.horizontalCenter: parent.horizontalCenter
				from: 0
				to: game ? game.downloader.fullSize : 0
				value: game ? game.downloader.downloadedSize : 0
				color: Qaterial.Colors.green400

				Behavior on value {
					NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
				}
			}
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

	Connections {
		target: game

		/*function onHostModeChanged() {
			if (game.hostMode == ConquestGame.ModeHost)
				Client.snack(qsTr("Te vagy a host"))
		}*/

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

			case RpgConfig.StateDownloadContent:
				_stack.activeComponent = _cmpDownload
				return

			default:
				_stack.activeComponent = _cmpConnect
				break
			}
		}

		function onFinishDialogRequest(text, icon, success) {
			Qaterial.DialogManager.showDialog(
						{
							onAccepted: function() { if (game) game.finishGame()  },
							onRejected: function() { if (game) game.finishGame() },
							text: text,
							title: qsTr("Game over"),
							iconSource: icon,
							iconColor: success ? Qaterial.Colors.green500 : Qaterial.Colors.red500,
							textColor: success ? Qaterial.Colors.green500 : Qaterial.Colors.red500,
							iconFill: false,
							iconSize: Qaterial.Style.roundIcon.size,
							standardButtons: DialogButtonBox.Ok
						})
		}
	}


	StackView.onActivated: {
		if (game)
			game.playMenuBgMusic()

		if (Qt.platform.os != "android" && Qt.platform.os != "ios") {
			_oldWindowState = Client.fullScreenHelper
			Client.fullScreenHelper = true
		}
	}

	StackView.onRemoved: {
		if (game) {
			game.stopMenuBgMusic()
		}

		if (Qt.platform.os != "android" && Qt.platform.os != "ios") {
			if (_oldWindowState != Client.fullScreenHelper)
				Client.fullScreenHelper = false
		}
	}
}
