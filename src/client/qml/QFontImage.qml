import QtQuick 2.12
import QtGraphicalEffects 1.0
import "Style"


Item {
	id: control

	height: CosStyle.pixelSize*1.1
	width: CosStyle.pixelSize*1.1

	property int size: CosStyle.pixelSize*1.1
	property alias icon: img.source
	property alias color: overlay.color

	Image {
		id: img
		anchors.centerIn: parent
		source: CosStyle.iconMenu
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
