import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import "Style"
import "JScript.js" as JS

Item {
	id: control

	implicitHeight: 400
	implicitWidth: StackView.view && StackView.view.requiredWidth ? StackView.view.requiredWidth : 800

	property string icon: ""
	property string title: ""
	property string contentTitle: ""
	property QMenu menu: null
	property bool forceMainMenu: true
	property Action action: null

	property real maximumWidth: StackView.view && StackView.view.maximumPanelWidth ? StackView.view.maximumPanelWidth : -1

	property color borderColor: CosStyle.colorPrimaryDarker
	property color titleColor: CosStyle.colorAccentLighter

	property int horizontalPadding: 20
	property int verticalPadding: 10

	property bool compact: false

	property QTabPage tabPage: null
	property var backCallbackFunction: null
	property var closeCallbackFunction: null


	readonly property alias panelWidth: panel.width
	readonly property alias panelHeight: panel.height
	readonly property bool isCurrentItem: StackView.view && StackView.view.currentItem == control

	default property alias movableContentData: movableContent.data


	signal populated()

	StackView.onActivated: populated()

	Item {
		id: panel

		width: !compact ? (maximumWidth>0 ? Math.min(control.width-2*horizontalPadding, maximumWidth) : control.width-2*horizontalPadding) : control.width
		height: !compact ? control.height-2*verticalPadding : control.height

		anchors.centerIn: parent

		visible: !compact

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
					anchors.right: panelMenu.visible ? panelMenu.left : parent.right
					anchors.bottom: parent.bottom
					font.weight: Font.Thin
					font.pixelSize: CosStyle.pixelSize*1.4
					font.capitalization: Font.AllUppercase
					elide: Text.ElideRight
					color: titleColor
					verticalAlignment: "AlignVCenter"
					leftPadding: iconImage.visible ? CosStyle.pixelSize/2 : CosStyle.pixelSize
				}

				QMenuButton {
					id: panelMenu
					anchors.verticalCenter: parent.verticalCenter
					anchors.right: parent.right
					menu: !forceMainMenu ? control.menu : null
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
				anchors.topMargin: !compact ? 10 : 0

				Item {
					id: movableContent

					anchors.fill: parent

					states: State {
						when: compact
						ParentChange { target: movableContent; parent: control }
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
			visible: !compact
		}

	}

	Connections {
		target: tabPage

		function onCompactChanged() {
			compact = tabPage.compact
		}

	}

	onTabPageChanged: if (tabPage) {
						  compact = tabPage.compact
					  }



	StackView.onDeactivating: {
		if (tabPage) {
			tabPage.toolButtonAction = null
			tabPage.menuButton.menu = null
		}
	}

	onPopulated: {
		if (tabPage) {
			tabPage.contentTitle = contentTitle
			tabPage.toolButtonAction = action
			if (compact || forceMainMenu)
				tabPage.menuButton.menu = menu
			else
				tabPage.menuButton.menu = null
		}
	}
}
