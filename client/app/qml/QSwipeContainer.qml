import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import "Style"
import "JScript.js" as JS

Item {
	id: control

	property color borderColor: CosStyle.colorPrimaryDarker
	property color titleColor: CosStyle.colorAccentLighter

	property string icon: ""
	property string title: ""

	property int horizontalPadding: 20
	property int verticalPadding: 10

	property alias reparented: movableContent.reparented
	property alias reparentedParent: movableContent.reparentedParent

	default property alias movableContentData: movableContent.data

	Layout.fillWidth: true
	Layout.fillHeight: true

	Item {
		id: panel

		width: control.width-2*horizontalPadding
		height: control.height-2*verticalPadding

		anchors.centerIn: parent

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


		QImageInnerShadow {
			width: Math.min(parent.width*0.5, parent.height*0.5)
			height: width

			anchors.centerIn: parent

			image: control.icon
			contentItem: metalbg
		}


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

				QFontImage {
					id: iconImage

					anchors.left: parent.left
					anchors.leftMargin: CosStyle.pixelSize/2
					anchors.verticalCenter: parent.verticalCenter

					height: visible ? CosStyle.pixelSize*2 : 0
					width: height
					size: Math.min(height*0.8, 32)

					icon: control.icon

					visible: control.icon.length

					color: titleColor
				}

				QLabel {
					id: labelTitle
					text: control.title
					anchors.top: parent.top
					anchors.left: iconImage.right
					anchors.right: parent.right
					anchors.bottom: parent.bottom
					font.weight: Font.Thin
					font.pixelSize: CosStyle.pixelSize*1.4
					font.capitalization: Font.AllUppercase
					elide: Text.ElideRight
					color: titleColor
					verticalAlignment: "AlignVCenter"
					leftPadding: iconImage.visible ? CosStyle.pixelSize/2 : CosStyle.pixelSize
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

				Rectangle {
					id: movableContent

					anchors.fill: parent

					property bool reparented: false
					property Item reparentedParent: null

					color: reparented ? JS.setColorAlpha("black", 0.4) : "transparent"

					//visible: control.enabled

					states: State {
						when: reparented
						ParentChange { target: movableContent; parent: reparentedParent }
					}
				}
			}
		}



		// BORDER

		BorderImage {
			id: border1
			source: "qrc:/internal/img/border1.svg"
			visible: false

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

	}

}
