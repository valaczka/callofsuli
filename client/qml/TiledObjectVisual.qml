import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli


Item {
	id: root

	property TiledObjectImpl baseObject: null
	property alias spriteHandler: _spriteHandler
	property alias spriteHandlerAuxFront: _spriteHandlerAuxFront
	property alias spriteHandlerAuxBack: _spriteHandlerAuxBack

	anchors.fill: parent

	TiledSpriteHandlerImpl {
		id: _spriteHandlerAuxBack

		baseObject: root.baseObject

		property bool alignToBody: false

		anchors.horizontalCenter: parent.horizontalCenter
		anchors.verticalCenter: parent.verticalCenter
		anchors.horizontalCenterOffset: alignToBody && baseObject ? baseObject.bodyOffset.x : 0
		anchors.verticalCenterOffset: alignToBody && baseObject ? baseObject.bodyOffset.y : 0

		handlerMaster: _spriteHandlerAuxFront
	}

	Rectangle {
		visible: baseObject && baseObject.scene && baseObject.scene.game && baseObject.scene.game.debugView
		color: "transparent"
		border.color: "black"
		border.width: 2
		anchors.fill: parent
	}

	TiledSpriteHandlerImpl {
		id: _spriteHandler
		anchors.fill: parent
		baseObject: root.baseObject
	}

	ThresholdMask {
		id: _threshold
		visible: false

		source: _spriteHandler
		maskSource: _spriteHandler
		anchors.fill: _spriteHandler

		threshold: 0.7
	}

	Glow {
		id: glow
		opacity: baseObject && baseObject.glowEnabled ? 1.0 : 0.0
		visible: opacity != 0
		color: baseObject ? baseObject.glowColor : "transparent"

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

		opacity: baseObject && baseObject.overlayEnabled ? 0.5 : 0.0
		visible: opacity != 0
		color: baseObject ? baseObject.overlayColor : "transparent"

		source: _threshold
		anchors.fill: _threshold

		Behavior on opacity {
			NumberAnimation { duration: 300 }
		}
	}

	TiledSpriteHandlerImpl {
		id: _spriteHandlerAuxFront

		baseObject: root.baseObject

		property bool alignToBody: false

		anchors.horizontalCenter: parent.horizontalCenter
		anchors.verticalCenter: parent.verticalCenter
		anchors.horizontalCenterOffset: alignToBody && baseObject ? baseObject.bodyOffset.x : 0
		anchors.verticalCenterOffset: alignToBody && baseObject ? baseObject.bodyOffset.y : 0

		handlerSlave: _spriteHandlerAuxBack
	}

}
