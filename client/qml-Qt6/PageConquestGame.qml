import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as J

Page {
	id: root

	property ConquestGame game: null


	property string closeQuestion: _mapVisible ? qsTr("Biztosan kilépsz a játékból?") : ""
	property var onPageClose: function() { if (game) game.gameAbort() }

	property var stackPopFunction: function() {
		if (game && game.config.gameState == ConquestConfig.StateFinished && _stack.activeComponent == _cmpScene) {
			game.sendWebSocketMessage({
										  cmd: "prepare",
										  engine: game.engineId,
										  unprepare: true
									  })
			_isUnprepared = true
			return false
		}

		return true
	}

	readonly property bool _mapVisible: game && (game.config.gameState == ConquestConfig.StatePlay || game.config.gameState == ConquestConfig.StatePrepare)
	property bool _isUnprepared: false

	Image {
		anchors.fill: parent
		cache: true

		source: "qrc:/internal/img/bgConquest.jpg"
		fillMode: Image.PreserveAspectCrop
	}

	StackView {
		id: _stack
		anchors.fill: parent

		readonly property Component activeComponent: {
			if (!Client.server || !Client.server.dynamicContentReady)
				return _cmpDownload

			if (!game)
				return _cmpConnect

			if (game.config.gameState == ConquestConfig.StateFinished && _isUnprepared)
				return _cmpConnect

			switch (game.config.gameState) {
			case ConquestConfig.StatePrepare:
			case ConquestConfig.StatePlay:
			case ConquestConfig.StateFinished:
				return _cmpScene

			case ConquestConfig.StateError:
				return _cmpError

			case ConquestConfig.StateWorldSelect:
				return _cmpWorldSelect

			default:
				return _cmpConnect
			}
		}

		onActiveComponentChanged: replace(null, activeComponent, {}, StackView.Immediate)
	}

	Component {
		id: _cmpConnect

		ConquestConnect {
			game: root.game
		}
	}

	Component {
		id: _cmpWorldSelect

		ConquestWorldSelect {
			game: root.game
		}
	}

	Component {
		id: _cmpScene

		ConquestScene {
			game: root.game

			StackView.onActivated: _isUnprepared = false
		}
	}

	Component {
		id: _cmpError

		ConquestError {
			game: root.game
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

		visible: _stack.activeComponent != _cmpScene

		onClicked: Client.stackPop()
	}

	Connections {
		target: game

		function onHostModeChanged() {
			if (game.hostMode == ConquestGame.ModeHost)
				Client.snack(qsTr("Te vagy a host"))
		}
	}

	StackView.onRemoved: {
		if (game)
			game.stopMenuBgMusic()
	}
}
