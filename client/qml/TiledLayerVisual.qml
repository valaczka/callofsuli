import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli


Item {
	id: root

	property bool glowEnabled: true
	property alias glowColor: glow.color

	property bool overlayEnabled: false
	property alias overlayColor: overlay.color

	property Item baseItem: null

	implicitWidth: 100
	implicitHeight: 100

	width: baseItem ? baseItem.width : implicitWidth
	height: baseItem ? baseItem.height : implicitHeight
	x: baseItem ? baseItem.x : 0
	y: baseItem ? baseItem.y : 0
	z: baseItem ? baseItem.z : 0



	onVisibleChanged: if (baseItem)
						  baseItem.visible = visible

	ThresholdMask {
		id: _threshold
		visible: false

		source: baseItem
		maskSource: baseItem
		anchors.fill: parent

		threshold: 0.7
	}

	Glow {
		id: glow
		opacity: glowEnabled ? 1.0 : 0.0
		visible: opacity != 0

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

		opacity: overlayEnabled ? 0.5 : 0.0
		visible: opacity != 0

		source: _threshold
		anchors.fill: _threshold

		Behavior on opacity {
			NumberAnimation { duration: 300 }
		}
	}
}
