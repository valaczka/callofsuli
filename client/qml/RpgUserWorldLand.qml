import QtQuick
import QtQuick.Controls
import QtQuick.VectorImage
import CallOfSuli
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Item {
	id: root

	property RpgWorldLandData landData: null
	property bool selected: false

	signal clicked()
	signal doubleClicked()

	width: landData && landData.imageSource.toString() != "" ? _imgMap.width : 100
	height: landData && landData.imageSource.toString() != "" ? _imgMap.height : 100

	x: landData ? landData.posX : 0
	y: landData ? landData.posY : 0

	readonly property bool _selectable: landData && (
											landData.landState == RpgWorldLandData.LandSelectable ||
											landData.landState == RpgWorldLandData.LandAchieved)

	readonly property bool _hoverable: landData &&
									   landData.landState != RpgWorldLandData.LandInvalid &&
									   landData.landState != RpgWorldLandData.LandUnused


	VectorImage {
		id: _imgMap
		visible: false
		source: landData ? landData.imageSource : ""
		fillMode: VectorImage.NoResize
	}


	Image {
		id: _bg
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		visible: false
		source: landData ? landData.backgroundSource : ""
	}

	OpacityMask {
		anchors.fill: _bg
		source: _bg
		maskSource: _imgMap
		visible: selected && landData && landData.backgroundSource != ""
	}

	ColorOverlay {
		anchors.fill: _imgMap
		source: _imgMap
		color: {
			if (!landData)
				return Qaterial.Colors.gray800

			switch(landData.landState)
			{
			case RpgWorldLandData.LandAchieved:
				return root.selected ? Qaterial.Colors.amber500 : Qaterial.Colors.green500

			case RpgWorldLandData.LandSelectable:
				return root.selected ? Qaterial.Colors.amber500 : Qaterial.Colors.gray800

			case RpgWorldLandData.LandLocked:
				return root.selected ? Qaterial.Colors.cyan800 : Qaterial.Colors.gray800

			default:
				return Qaterial.Colors.gray800
			}
		}

		opacity: root.selected ? 0.3 : 0.7
	}



	Image {
		id: _imgBorder
		width: _imgMap.width
		height: _imgMap.height
		fillMode: VectorImage.PreserveAspectFit
		visible: false
		source: landData ? landData.borderSource : ""
	}

	ColorOverlay {
		id: _overlayBorder
		anchors.fill: _imgBorder
		source: _imgBorder
		color: root.selected ? (landData && landData.landState == RpgWorldLandData.LandLocked ?
									Qaterial.Colors.cyan700:
									Qaterial.Colors.amber600) :
							   landData && landData.landState == RpgWorldLandData.LandAchieved ?
								   Qaterial.Colors.green300 :
								   Qaterial.Colors.green600

		visible: false
	}


	ColorOverlay {
		id: _hover
		anchors.fill: _imgMap
		source: _imgMap
		color: Qaterial.Colors.white
		opacity: _mouse.containsMouse ? 0.5 : 0.0
		visible: _imgMap.source.toString() != "" && _hoverable

		Behavior on opacity {
			NumberAnimation { duration: 250; easing.type: Easing.InQuad }
		}
	}

	OpacityMask {
		anchors.fill: _overlayBorder
		source: _overlayBorder
		maskSource: _imgMap
		visible: landData && (landData.landState == RpgWorldLandData.LandAchieved ||
							  landData.landState == RpgWorldLandData.LandSelectable)
	}


	MaskedMouseArea {
		id: _mouse
		anchors.fill: _imgMap
		maskSource: _imgMap.source
		enabled: _hoverable

		onClicked: {
			root.clicked()
		}

		onDoubleClicked: {
			root.doubleClicked()
		}
	}
}
