import QtQuick
import Qt5Compat.GraphicalEffects
import CallOfSuli


TiledVisualItemImpl {
	id: _control

	implicitWidth: _image.sourceSize.width
	implicitHeight: _image.sourceSize.height

	Rectangle {
		visible: _control.scene && _control.scene.game && _control.scene.game.debugView
		color: "transparent"
		border.color: "blue"
		border.width: 2
		anchors.fill: parent
	}

	Image {
		id: _image
		anchors.fill: parent
		source: _control.source
	}

	ThresholdMask {
		id: _threshold
		visible: false

		source: _image
		maskSource: _image
		anchors.fill: _image

		threshold: 0.7
	}

	Glow {
		id: glow
		opacity: _control.glowEnabled ? 1.0 : 0.0
		visible: opacity != 0
		color: _control.glowColor

		source: _threshold
		anchors.fill: _threshold

		radius: 4
		samples: 9

		Behavior on opacity {
			NumberAnimation { duration: 200 }
		}
	}


	ColorOverlay {
		id: overlay

		opacity: _control.overlayEnabled ? 0.5 : 0.0
		visible: opacity != 0
		color: _control.overlayColor

		source: _threshold
		anchors.fill: _threshold

		Behavior on opacity {
			NumberAnimation { duration: 300 }
		}
	}

}
