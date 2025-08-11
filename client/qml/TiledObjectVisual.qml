import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli
import QtQuick.Shapes


Item {
	id: root

	property TiledObjectImpl baseObject: null
	property alias spriteHandler: _spriteHandler
	property alias spriteHandlerAuxFront: _spriteHandlerAuxFront
	property alias spriteHandlerAuxBack: _spriteHandlerAuxBack
	property alias spriteThreshold: _threshold

	property color ellipseColor: "transparent"
	property real ellipseSize: 0
	property real ellipseWidth: 75

	parent: baseObject ? baseObject.scene : null


	property bool _initShow: false

	Timer {
		running: true
		repeat: false
		interval: 200
		onTriggered: _initShow = false
	}

	Shape {
		id: _ellipse

		visible: false // ellipseSize > 0

		anchors.horizontalCenter: parent.horizontalCenter
		anchors.verticalCenter: parent.verticalCenter
		anchors.horizontalCenterOffset: baseObject ? baseObject.bodyOffset.x : 0
		anchors.verticalCenterOffset: baseObject ? baseObject.bodyOffset.y : 0

		width: ellipseWidth
		height: ellipseWidth

		ShapePath {
			fillColor: "transparent"
			strokeColor: ellipseColor
			strokeWidth: ellipseSize > 0 ? ellipseSize : 1
			capStyle: ShapePath.FlatCap

			PathAngleArc {
				centerX: _ellipse.width/2
				centerY: _ellipse.height/2
				radiusX: _ellipse.width/2
				radiusY: _ellipse.width*Math.cos(Math.PI/6)/2
				startAngle: 0
				sweepAngle: 315

				NumberAnimation on startAngle {
					from: 0
					to: 360
					loops: Animation.Infinite
					duration: 1000
					//easing.type: Easing.InOutQuad
				}
			}
		}
	}

	Glow {
		id: _ellipseGlow

		visible: ellipseSize > 0

		color: ellipseColor

		source: _ellipse
		anchors.fill: _ellipse

		radius: 8
		samples: 17
	}


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
		opacity: _initShow || (baseObject && baseObject.glowEnabled) ? 1.0 : 0.0
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

		opacity: _initShow || (baseObject && baseObject.overlayEnabled) ? 0.5 : 0.0
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
