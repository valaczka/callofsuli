import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

ConquestStateImpl {
	id: root

	property real mapScale: 1.0

	readonly property string _mapSource: stateId > 0 && world != "" ? "qrc:/conquest/"+world+"/state-%1.svg".arg(stateId) : ""
	readonly property string _borderSource: stateId > 0 && world != "" ? "qrc:/conquest/"+world+"/state-%1-border.svg".arg(stateId) : ""

	implicitWidth: _mapSource ? _imgMap.width : 100
	implicitHeight: _mapSource ? _imgMap.height : 100

	x: baseX*mapScale
	y: baseY*mapScale

	Image {
		id: _imgMap
		width: sourceSize.width*mapScale
		height: sourceSize.height*mapScale
		fillMode: Image.PreserveAspectFit
		visible: false
		source: _mapSource
	}

	ColorOverlay {
		anchors.fill: _imgMap
		source: _imgMap
		color: root.color
		opacity: 0.6
		visible: isActive
	}


	Image {
		id: _imgBorder
		width: _imgMap.width
		height: _imgMap.height
		fillMode: Image.PreserveAspectFit
		visible: false
		source: _borderSource
	}

	ColorOverlay {
		id: _overlayBorder
		anchors.fill: _imgBorder
		source: _imgBorder
		color: root.color
		visible: false
	}


	ColorOverlay {
		id: _hover
		anchors.fill: _imgMap
		source: _imgMap
		color: Qaterial.Colors.white
		opacity: _mouse.containsMouse ? 0.5 : 0.0
		visible: _mapSource

		Behavior on opacity {
			NumberAnimation { duration: 250; easing.type: Easing.InQuad }
		}
	}

	OpacityMask {
		anchors.fill: _overlayBorder
		source: _overlayBorder
		maskSource: _imgMap
		visible: isActive
	}

	MaskedMouseArea {
		id: _mouse
		anchors.fill: _imgMap
		maskSource: _mapSource
		alphaThreshold: 0.4
		scaleImage: mapScale
		onClicked: isActive = ! isActive
	}

}
