import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import "Style"

Item {
	id: control

	implicitWidth: 50
	implicitHeight: 50

	required property string image
	required property Item contentItem

	property alias brightness: brg.brightness

	Image {
		id: img
		source: control.image
		sourceSize.width: control.width
		sourceSize.height: control.height
		width: sourceSize.width
		height: sourceSize.height
		visible: false
		fillMode: Image.PreserveAspectFit
	}

	BrightnessContrast {
		id: brg
		anchors.fill: parent
		source: contentItem
		brightness: -0.1
		visible: false
	}

	OpacityMask {
		id: opacity2
		anchors.fill: brg
		source: brg
		maskSource: img
		visible: false
	}

	InnerShadow {
		anchors.fill: parent
		source: opacity2
		radius: 16
		samples: 24
		color: "black"
		horizontalOffset: 3
		verticalOffset: 3
		opacity: 0.4
	}
}
