import QtQuick
import Qt5Compat.GraphicalEffects
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

TiledVisualItemImpl {
	id: _control

	implicitWidth: _image.sourceSize.width
	implicitHeight: _image.sourceSize.height

	property bool _initShow: false

	Timer {
		running: true
		repeat: false
		interval: 200
		onTriggered: _initShow = false
	}

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
		opacity: _initShow || _control.glowEnabled ? 1.0 : 0.0
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

		opacity: _initShow || _control.overlayEnabled ? 0.5 : 0.0
		visible: opacity != 0
		color: _control.overlayColor

		source: _threshold
		anchors.fill: _threshold

		Behavior on opacity {
			NumberAnimation { duration: 300 }
		}
	}

	Item {
		id: _labelItem

		width: _label.width
		height: _label.height
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.bottom: parent.top
		anchors.bottomMargin: 2

		visible: _control.displayName != ""

		Qaterial.Label {
			id: _label
			font.family: Qaterial.Style.textTheme.body1.family
			font.pixelSize: 10
			font.weight: Font.Bold
			color: "white"
			//elide: implicitWidth > width ? Text.ElideRight : Text.ElideNone
			wrapMode: Text.Wrap
			text: _control.displayName
			width: _control.width
			horizontalAlignment: Text.AlignHCenter
		}

		Glow {
			anchors.fill: _label
			source: _label
			color: "black"
			radius: 1
			spread: 0.9
			samples: 5
		}
	}
}
