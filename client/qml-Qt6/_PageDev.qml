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

	TiledGameImpl {
		id: _game
		anchors.fill: parent
		joystick: _gameJoystick
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
		visible: _game.controlledPlayer
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
			_game.switchScene()
		}
	}

	GameButton {
		id: _shot
		size: 50

		anchors.right: parent.right
		anchors.bottom: parent.bottom

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


	Component.onCompleted: {
		_game.load()
	}

}


