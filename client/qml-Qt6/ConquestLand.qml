import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

ConquestLandImpl {
	id: root

	property real mapScale: 1.0

	implicitWidth: _imgMap.source.toString() != "" ? _imgMap.width : 100
	implicitHeight: _imgMap.source.toString() != "" ? _imgMap.height : 100

	Image {
		id: _imgMap
		fillMode: Image.PreserveAspectFit
		visible: false
		source: landData ? landData.imgMap : ""
	}

	ColorOverlay {
		anchors.fill: _imgMap
		source: _imgMap
		color: root.baseColor
		opacity: 0.6
		visible: false//isActive
	}


	Image {
		id: _imgBorder
		width: _imgMap.width
		height: _imgMap.height
		fillMode: Image.PreserveAspectFit
		visible: false
		source: landData ? landData.imgBorder : ""
	}

	ColorOverlay {
		id: _overlayBorder
		anchors.fill: _imgBorder
		source: _imgBorder
		color: root.baseColor
		visible: false
	}


	ColorOverlay {
		id: _hover
		anchors.fill: _imgMap
		source: _imgMap
		color: Qaterial.Colors.white
		opacity: _mouse.containsMouse ? 0.5 : 0.0
		visible: _imgMap.source.toString() != ""

		Behavior on opacity {
			NumberAnimation { duration: 250; easing.type: Easing.InQuad }
		}
	}

	OpacityMask {
		anchors.fill: _overlayBorder
		source: _overlayBorder
		maskSource: _imgMap
		visible: false //isActive
	}

	MaskedMouseArea {
		id: _mouse
		anchors.fill: _imgMap
		maskSource: _imgMap.source

		//scaleImage: mapScale
		/*onClicked: {
			if (!game)
				return

			game.sendWebSocketMessage({
										  cmd: "test",
										  engine: game.engineId,
										  stateId: stateId,
										  value: !isActive
									  })
		}*/
	}

}
