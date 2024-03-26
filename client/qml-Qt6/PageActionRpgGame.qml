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

	Connections {
		target: game

		function onConfigChanged() {
			if (Client.server && !Client.server.dynamicContentReady) {
				_stack.activeComponent = _cmpDownload
				return
			}

			/*if (game.config.gameState == RpgConfig.StateFinished && _isUnprepared) {
				_stack.activeComponent = _cmpConnect
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
				Client.stackPop(root)
				return

			default:
				_stack.activeComponent = _cmpConnect
				break
			}
		}
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

			//StackView.onActivated: _isUnprepared = false
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

			Qaterial.IconLabel {
				anchors.horizontalCenter: parent.horizontalCenter
				color: Qaterial.Style.accentColor
				icon.source: Qaterial.Icons.download
				icon.width: Qaterial.Style.dashboardButtonSize*0.4
				icon.height: Qaterial.Style.dashboardButtonSize*0.4
				text: qsTr("Pályák letöltése folyamatban...")
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
	}

	StackView.onRemoved: {
		if (game)
			game.stopMenuBgMusic()
	}
}
