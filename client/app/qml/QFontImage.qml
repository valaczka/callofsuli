import QtQuick 2.15
import QtGraphicalEffects 1.0
import "Style"


Item {
	id: control

	implicitWidth: size
	implicitHeight: size

	property real size: CosStyle.pixelSize*1.1
	property alias icon: img.source
	property alias color: overlay.color
	property alias fillMode: img.fillMode

	Image {
		id: img
		anchors.centerIn: parent
		sourceSize: Qt.size(control.size, control.size)
		visible: false
	}

	ColorOverlay {
		id: overlay
		anchors.fill: img
		source: img
		color: CosStyle.colorAccent
	}
}
