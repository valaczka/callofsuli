import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import "Style"
import "JScript.js" as JS

Item {
	id: control

	implicitWidth: 500
	implicitHeight: 200


	property color borderColor: CosStyle.colorPrimaryDarker
	property color titleColor: CosStyle.colorAccentLighter

	property string icon: ""
	property alias title: labelTitle.text

	property alias panelData: panelData
	default property alias panelDataData: panelData.data

	property int horizontalPadding: 20
	property int verticalPadding: 10

	property int maximumWidth: 0
	property int maximumHeight: 0

	property Menu pageContextMenu: null


	readonly property bool swipeMode: control.SwipeView.view
	property bool _isCurrent: control.SwipeView.isCurrentItem

	signal panelActivated()
	signal populated()

	on_IsCurrentChanged: {
		if (_isCurrent) {
			control.SwipeView.view.parentPage.mainMenu = pageContextMenu
			panelActivated()
		}
	}


	Item {
		id: panel

		width: control.swipeMode ? control.width-18 : (maximumWidth ? Math.min(maximumWidth, control.width-2*horizontalPadding) : control.width-2*horizontalPadding)
		height: control.swipeMode ? control.height-18 : (maximumHeight ? Math.min(maximumHeight, control.height-2*verticalPadding) : control.height-2*verticalPadding)
		x: (control.width-width)/2
		y: (control.height-height)/2


		DropShadow {
			anchors.fill: panel
			horizontalOffset: 3
			verticalOffset: 3
			color: JS.setColorAlpha("black", 0.75)
			source: border2
		}

		BorderImage {
			id: border2
			source: "qrc:/internal/img/border2.svg"
			visible: false

			//sourceSize.height: 141
			//sourceSize.width: 414

			anchors.fill: panel
			border.top: 10
			border.left: 5
			border.right: 80
			border.bottom: 10

			horizontalTileMode: BorderImage.Repeat
			verticalTileMode: BorderImage.Repeat
		}


		Image {
			id: metalbg
			source: "qrc:/internal/img/metalbg.png"
			visible: false
			fillMode: Image.Tile
			anchors.fill: panel
		}

		OpacityMask {
			id: opacity1
			anchors.fill: panel
			source: metalbg
			maskSource: border2
		}



		/*ShaderEffectSource {
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
		}*/





		Item {
			id: realContent
			anchors.fill: parent
			visible: true

			anchors.topMargin: 5
			anchors.leftMargin: 0
			anchors.rightMargin: 0
			anchors.bottomMargin: 10

			Rectangle {
				id: hdrRect
				color: "transparent"
				anchors.top: parent.top
				anchors.left: parent.left
				anchors.right: parent.right
				height: labelTitle.implicitHeight*1.6

				visible: labelTitle.text.length

				DropShadow {
					anchors.fill: labelTitle
					horizontalOffset: 2
					verticalOffset: 2
					radius: 1
					color: "black"
					source: labelTitle
				}

				QLabel {
					id: labelTitle
					anchors.top: parent.top
					anchors.left: parent.left
					anchors.right: menuButton.left
					anchors.bottom: parent.bottom
					font.weight: Font.Thin
					font.pixelSize: CosStyle.pixelSize*1.4
					font.capitalization: Font.AllUppercase
					color: titleColor
					verticalAlignment: "AlignVCenter"
					leftPadding: CosStyle.pixelSize
				}

				QToolButton {
					id: menuButton

					anchors.verticalCenter: parent.verticalCenter
					anchors.right: parent.right

					icon.source: CosStyle.iconMenu

					visible: pageContextMenu && !swipeMode

					onClicked: if (pageContextMenu) {
								   pageContextMenu.popup(menuButton, 0, menuButton.height)
							   }
				}
			}

			Rectangle {
				id: rectLine
				visible: hdrRect.visible
				anchors.top: hdrRect.bottom
				anchors.left: parent.left
				anchors.right: parent.right
				height: 1
				border.width: 0
					gradient: Gradient {
						orientation: Gradient.Horizontal
						GradientStop { position: 0.0; color: "transparent" }
						GradientStop { position: 0.3; color: control.titleColor }
						GradientStop { position: 0.7; color: control.titleColor }
						GradientStop { position: 1.0; color: "transparent" }
					}
			}


			Item {
				id: panelData
				anchors.top: hdrRect.visible ? hdrRect.bottom : parent.top
				anchors.left: parent.left
				anchors.bottom: parent.bottom
				anchors.right: parent.right
				anchors.topMargin: 10
				visible: true
			}
		}





		// BORDER

		BorderImage {
			id: border1
			source: "qrc:/internal/img/border1.svg"
			visible: false

			//sourceSize.height: 141
			//sourceSize.width: 414

			anchors.fill: panel
			border.top: 15
			border.left: 10
			border.right: 60
			border.bottom: 25

			horizontalTileMode: BorderImage.Repeat
			verticalTileMode: BorderImage.Repeat
		}

		ColorOverlay {
			anchors.fill: border1
			source: border1
			color: borderColor
		}


		/*BorderImage {
			id: bgRect
			source: "qrc:/internal/img/border.svg"
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
			source: "qrc:/internal/img/borderLine15.svg"
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
		}*/
	}

}
