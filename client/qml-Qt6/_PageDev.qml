import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

import Box2D

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

		followedItem: _character

		scene.debugView: true


		IsoGameObjectImpl {
			id: _character
			scene: _flick.scene

			x: 1800
			y: 400

			z: 1

			body.fixtures: Circle {
				radius: 15
				x: (_character.width-2*radius)/2
				y: (_character.height-2*radius)*0.8

				density: 1
				restitution: 0
				friction: 1
				categories: Box.Category2
				collidesWith: (Box.Category1|Box.Category5)
			}

			Component.onCompleted: bodyComplete()
		}

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

		Qaterial.LabelBody1 {
			text: "z: "+_character.z
		}
	}

	GameJoystick {
		id: _gameJoystick
		anchors.bottom: parent.bottom
		anchors.left: parent.left
	}

	Component.onCompleted: {
		_flick.scene.mapLoader.source = "file:///home/valaczka/teszt.tmx"
		_flick.scene.forceActiveFocus()
		_character.load()
	}

}


