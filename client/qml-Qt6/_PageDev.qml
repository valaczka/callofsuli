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
		color: Qaterial.Colors.blue900
	}

	TiledFlickableScene {
		id: _flick
		anchors.fill: parent

		joystick: _gameJoystick
		scene.debugView: true

	}


	Row {
		QButton {
			text: "-"
			onClicked: _flick.scene.scale -= 0.1
		}

		QButton {
			text: "+"
			onClicked: _flick.scene.scale += 0.1
		}
	}

	GameJoystick {
		id: _gameJoystick
		anchors.bottom: parent.bottom
		anchors.left: parent.left
	}

	Component.onCompleted: {
		_flick.scene.load("file:///home/valaczka/teszt.tmx")
		_flick.scene.forceActiveFocus()
	}

}


