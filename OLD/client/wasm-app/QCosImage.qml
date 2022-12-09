import QtQuick 2.7
import QtGraphicalEffects 1.0

Item {
	id: item

	property alias textColor: overed.color
	property alias glowColor: glowed.color
	property alias glowRadius: glowed.radius
	property alias glowSamples: glowed.samples
	property alias glow: glowed.visible
	property int maxWidth: 0
	property int maxHeight: 0

	Image {
		id: img
		anchors.centerIn: parent
		sourceSize.width: item.maxWidth ? Math.min(parent.width, item.maxWidth) : parent.width
		sourceSize.height: item.maxHeight ? Math.min(parent.height, item.maxHeight) : parent.height
		width: sourceSize.width
		height: sourceSize.height
		fillMode: Image.PreserveAspectFit
		source: "qrc:/internal/img/callofsuli.svg"
		visible: false
	}

	ColorOverlay {
		id: overed
		anchors.fill: img
		source: img
		color: "blue"
	}

	Glow {
		id: glowed
		anchors.fill: img
		source: overed
		radius: 8
		samples: 17
		color: "cyan"
		visible: true
	}


}
