import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


Page {
	id: root

	Rectangle {
		anchors.fill: parent
		color: _flick.scene.debugView ? Qaterial.Colors.blue900 : "black"
	}

	TiledFlickableScene {
		id: _flick
		anchors.fill: parent

		joystick: _gameJoystick
		scene.debugView: true
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
		visible: _flick.scene.controlledItem
	}

	Component.onCompleted: {
		_flick.scene.load("qrc:/teszt.tmx")
		_flick.scene.forceActiveFocus()
	}

}


