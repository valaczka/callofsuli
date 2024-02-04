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


	///////property string closeQuestion: _mapVisible ? qsTr("Biztosan kilépsz a játékból?") : ""
	property var onPageClose: function() { if (game) game.gameAbort() }

	property bool _mapVisible: game && (game.config.gameState == ConquestConfig.StatePlay || game.config.gameState == ConquestConfig.StatePrepare)

	Image {
		anchors.fill: parent
		cache: true

		source: "qrc:/internal/img/bgConquest.jpg"
		fillMode: Image.PreserveAspectCrop
	}

	StackView {
		id: _stack
		anchors.fill: parent

		property Component activeComponent: game && (game.config.gameState === ConquestConfig.StatePrepare ||
													 game.config.gameState === ConquestConfig.StatePlay) ?
												_cmpScene :
												_cmpConnect

		onActiveComponentChanged: replace(null, activeComponent, {}, StackView.Immediate)
	}

	Component {
		id: _cmpConnect

		ConquestConnect {
			game: root.game
		}
	}

	Component {
		id: _cmpScene

		ConquestScene {
			game: root.game
			playerRowLeftMargin: _backButton.x+_backButton.width + 5*Qaterial.Style.pixelSizeRatio
		}
	}


	GameButton {
		id: _backButton
		size: 25

		anchors.left: parent.left
		anchors.leftMargin: Math.max(5 * Qaterial.Style.pixelSizeRatio, Client.safeMarginLeft)
		anchors.top: parent.top
		anchors.topMargin: Math.max(5 * Qaterial.Style.pixelSizeRatio, Client.safeMarginTop)

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

	/*Connections {
		target: game

		function onConfigChanged() {
			if (game.config.gameState == ConquestConfig.StatePrepare || game.config.gameState == ConquestConfig.StatePlay)
				_stack.replace(null, _cmpScene, {}, StackView.Immediate)
		}
	}*/
}
