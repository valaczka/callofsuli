import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import "Style"
import "JScript.js" as JS

Item {
	id: item

	implicitWidth: 500
	implicitHeight: 200


	property alias blurSource: effectsource.sourceItem
	property alias blurRect: effectsource.sourceRect
	property alias blurBrightness: brightnessEffect.brightness

	property color borderColor: CosStyle.colorPrimaryDark
	property color titleBgColor: Qt.darker(borderColor)
	property color titleColor: CosStyle.colorPrimaryLight

	property string icon: ""
	property alias title: labelTitle.text

	property alias panelData: panelData
	default property alias panelDataData: panelData.data

	property int horizontalPadding: 20
	property int verticalPadding: 10

	property int maximumWidth: 0
	property int maximumHeight: 0

	property Menu pageContextMenu: null

	readonly property bool swipeMode: item.SwipeView.view
	property bool _isCurrent: item.SwipeView.isCurrentItem

	signal panelActivated()
	signal populated()

	on_IsCurrentChanged: {
		if (_isCurrent) {
			item.SwipeView.view.parentPage.mainMenu = pageContextMenu
			panelActivated()
		}
	}


	Item {
		id: panel

		width: item.swipeMode ? item.width : (maximumWidth ? Math.min(maximumWidth, item.width-2*horizontalPadding) : item.width-2*horizontalPadding)
		height: item.swipeMode ? item.height : (maximumHeight ? Math.min(maximumHeight, item.height-2*verticalPadding) : item.height-2*verticalPadding)
		x: (item.width-width)/2
		y: (item.height-height)/2



		ShaderEffectSource {
			id: effectsource
			anchors.fill: parent
			sourceRect: Qt.rect(parent.x, parent.y, parent.width, parent.height)
			visible: false
		}

		FastBlur {
			id: blurEffect
			anchors.fill: effectsource
			source: effectsource
			radius: 30
			visible: false
		}

		BrightnessContrast {
			id: brightnessEffect
			anchors.fill: blurEffect
			source: blurEffect
			brightness: -0.5
			visible: false
		}

		OpacityMask {
			id: opacityEffect
			source: brightnessEffect
			maskSource: bgRect
			anchors.fill: brightnessEffect
			visible: true
		}





		Item {
			id: realContent
			anchors.fill: parent
			visible: true

			Rectangle {
				id: hdrRect
				color: titleBgColor
				anchors.top: parent.top
				anchors.left: parent.left
				anchors.right: parent.right
				height: labelTitle.implicitHeight*1.5

				visible: !item.swipeMode

				QLabel {
					id: labelTitle
					anchors.top: parent.top
					anchors.left: parent.left
					anchors.right: menuButton.left
					anchors.bottom: parent.bottom
					font.weight: Font.DemiBold
					font.pixelSize: CosStyle.pixelSize*0.95
					font.capitalization: Font.AllUppercase
					color: titleColor
					verticalAlignment: "AlignVCenter"
					leftPadding: 15
				}

				QToolButton {
					id: menuButton

					anchors.verticalCenter: parent.verticalCenter
					anchors.right: parent.right

					icon.source: CosStyle.iconMenu

					visible: pageContextMenu

					onClicked: if (pageContextMenu) {
								   pageContextMenu.popup(menuButton, 0, menuButton.height)
							   }
				}
			}


			Item {
				id: panelData
				anchors.top: hdrRect.visible ? hdrRect.bottom : parent.top
				anchors.left: parent.left
				anchors.bottom: parent.bottom
				anchors.right: parent.right
				//anchors.margins: 10
				visible: true
			}
		}




		// BORDER

		BorderImage {
			id: bgRect
			source: "qrc:/img/border.svg"
			visible: false

			width: parent.width
			height: parent.height
			border.left: 15; border.top: 10
			border.right: 15; border.bottom: 10

			horizontalTileMode: BorderImage.Repeat
			verticalTileMode: BorderImage.Repeat
		}



		BorderImage {
			id: bgRectLine
			source: "qrc:/img/borderLine15.svg"
			visible: false

			anchors.fill: bgRect
			border.left: 15; border.top: 10
			border.right: 15; border.bottom: 10

			horizontalTileMode: BorderImage.Repeat
			verticalTileMode: BorderImage.Repeat
		}

		Rectangle {
			id: borderRectData
			anchors.fill: bgRect
			visible: false
			color: borderColor
		}

		OpacityMask {
			id: borderRect
			source: borderRectData
			maskSource: bgRectLine
			anchors.fill: borderRectData
			visible: !item.swipeMode
		}
	}

}
