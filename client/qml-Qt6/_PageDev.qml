import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


Page {
	id: root

	/*Rectangle {
		anchors.fill: parent
		color: _flick.scene.debugView ? Qaterial.Colors.blue900 : "black"
	}*/

	TiledRpgGameImpl {
		id: _game
		anchors.fill: parent
		joystick: _gameJoystick

		onGameLoadFailed: Client.messageError("FAILED")
	}

	Qaterial.AppBarButton
	{
		anchors.left: parent.left
		anchors.leftMargin: Client.safeMarginLeft
		anchors.top: parent.top
		anchors.topMargin: Client.safeMarginTop
		icon.source: Qaterial.Icons.arrowLeft

		onClicked: Client.stackPop()
	}


	GameJoystick {
		id: _gameJoystick
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		visible: _game.controlledPlayer && _game.controlledPlayer.hp > 0
	}


	GamePainHud {
		id: _painhudImage
		anchors.fill: parent
		z: 10

		Connections {
			target: _game.controlledPlayer

			function onHurt() {
				_painhudImage.play()
			}
		}
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
		id: bowButton
		size: 50

		anchors.horizontalCenter: _shot.horizontalCenter
		anchors.bottom: pickButton.top
		anchors.bottomMargin: 5

		visible: _game.controlledPlayer

		//enabled: _game.controlledPlayer && _game.controlledPlayer.currentTransport && _game.controlledPlayer.currentTransport.

		color: Qaterial.Colors.blue400
		border.color: fontImage.color
		border.width: 1

		opacity: 1.0

		fontImage.icon: Qaterial.Icons.bowArrow
		fontImage.color: "white"
		fontImageScale: 0.6
		fontImage.anchors.horizontalCenterOffset: -2

		onClicked: {
			_game.controlledPlayer.shot()
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

		visible: _game.controlledPlayer

		//enabled: _game.controlledPlayer && _game.controlledPlayer.currentTransport && _game.controlledPlayer.currentTransport.

		color: Qaterial.Colors.red600
		border.color: fontImage.color
		border.width: 1

		opacity: 1.0

		fontImage.icon: Qaterial.Icons.sword
		fontImage.color: "white"
		fontImageScale: 0.6
		fontImage.anchors.horizontalCenterOffset: -2

		onClicked: {
			_game.controlledPlayer.hit()
		}
	}


	StackView.onActivated: {
		_game.load()
	}


	StackView.onDeactivating: {
		_game.visible = false
	}
}


