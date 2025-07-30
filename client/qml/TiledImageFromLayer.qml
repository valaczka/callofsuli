import QtQuick
import Qt5Compat.GraphicalEffects
import CallOfSuli


TiledVisualItemImpl {
	id: _control

	required property Item tiledLayer

	property alias sourceRect: _shader.sourceRect

	width: sourceRect.width
	height: sourceRect.height

	property bool _initShow: true

	layer.enabled: true

	ShaderEffectSource {
		id: _shader
		sourceItem: _control.tiledLayer
		width: sourceRect.width
		height: sourceRect.height
		visible: false
	}

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


	Glow {
		id: glow
		opacity: _initShow || _control.glowEnabled ? 1.0 : 0.0
		visible: opacity != 0
		color: _control.glowColor

		source: _shader
		anchors.fill: _shader

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

		source: _shader
		anchors.fill: _shader

		Behavior on opacity {
			NumberAnimation { duration: 300 }
		}
	}

}
