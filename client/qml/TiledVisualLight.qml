import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli
import QtQuick.Shapes


TiledVisualBlend {
	id: root

	property color color: "yellow"
	property real gradientStop: 0.6

	Shape {
		id: _blend

		anchors.horizontalCenter: parent.horizontalCenter
		visible: false

		readonly property real baseSize: Math.min(root.width, root.height)

		width: baseSize
		height: baseSize

		ShapePath {
			strokeWidth: 0
			fillGradient: RadialGradient {
				centerX: _blend.width/2
				centerY: _blend.height/2
				centerRadius: Math.min(_blend.width, _blend.height)/2
				focalX: centerX
				focalY: centerY

				GradientStop { position: 0; color: root.color }
				GradientStop { position: root.gradientStop; color: root.color }
				GradientStop { position: 1.0; color: "transparent" }
			}

			PathAngleArc {
				centerX: _blend.width/2
				centerY: _blend.height/2
				radiusX: _blend.width/2
				radiusY: _blend.height/2
				startAngle: 0
				sweepAngle: 360
			}
		}
	}


	Blend {
		id: _lastBlend
		visible: source !== null
		anchors.fill: parent
		source: OpacityMask {
			source: root.current
			anchors.fill: root
			maskSource: _blend
		}
		foregroundSource: _blend
		mode: "softLight"
	}
}
