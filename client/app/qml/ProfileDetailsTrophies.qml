import QtQuick 2.15
import QtQuick.Controls 2.15
import "."
import "Style"

Grid {
	id: control

	property real imageSize: CosStyle.pixelSize*2.5

	property alias t1: gridRepeaterT1.model
	property alias t2: gridRepeaterT2.model
	property alias t3: gridRepeaterT3.model
	property alias d1: gridRepeaterD1.model
	property alias d2: gridRepeaterD2.model
	property alias d3: gridRepeaterD3.model

	columns: Math.floor(parent.width/(imageSize+spacing))
	spacing: 5

	Repeater {
		id: gridRepeaterT1
		Image {
			source: "qrc:/internal/trophy/trophyt1.png"
			width: control.imageSize
			height: control.imageSize
			fillMode: Image.PreserveAspectFit
		}
	}

	Repeater {
		id: gridRepeaterD1
		Image {
			source: "qrc:/internal/trophy/trophyd1.png"
			width: control.imageSize
			height: control.imageSize
			fillMode: Image.PreserveAspectFit
		}
	}

	Repeater {
		id: gridRepeaterT2
		Image {
			source: "qrc:/internal/trophy/trophyt2.png"
			width: control.imageSize
			height: control.imageSize
			fillMode: Image.PreserveAspectFit
		}
	}

	Repeater {
		id: gridRepeaterD2
		Image {
			source: "qrc:/internal/trophy/trophyd2.png"
			width: control.imageSize
			height: control.imageSize
			fillMode: Image.PreserveAspectFit
		}
	}

	Repeater {
		id: gridRepeaterT3
		Image {
			source: "qrc:/internal/trophy/trophyt3.png"
			width: control.imageSize
			height: control.imageSize
			fillMode: Image.PreserveAspectFit
		}
	}

	Repeater {
		id: gridRepeaterD3
		Image {
			source: "qrc:/internal/trophy/trophyd3.png"
			width: control.imageSize
			height: control.imageSize
			fillMode: Image.PreserveAspectFit
		}
	}

}
