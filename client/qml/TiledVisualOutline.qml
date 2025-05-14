import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli
import QtQuick.Shapes


TiledVisualBlend {
	id: root

	required property TiledObjectImpl target
	property color color: "red"

	modeUpper: true

	parent: target ? target.scene : null

	x: target ? target.visualItem.x : 0
	y: target ? target.visualItem.y : 0
	sourceZ: target ? target.visualItem.z : 0
	z: 98


	width: target ? target.visualItem.width	: 0
	height: target ? target.visualItem.height : 0


	Glow {
		id: _glow
		visible: false
		color: root.color
		anchors.fill: parent

		source: ColorOverlay {
			source: target ? target.visualItem.spriteThreshold : null
			width: root.width
			height: root.height
			color: "black"
			visible: false
		}

		radius: 4
		samples: 9
	}


	OpacityMask {
		source: _glow
		anchors.fill: parent
		maskSource: current
	}
}
