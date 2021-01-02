import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import "Style"
import "JScript.js" as JS

Item {
	id: control

	implicitWidth: visible ? Math.max(500, maximumWidth) : 0
	implicitHeight: 200

	visible: false

	focus: true

	property bool panelVisible: false

	property color borderColor: CosStyle.colorPrimaryDarker
	property color titleColor: CosStyle.colorAccentLighter

	property bool layoutFillWidth: false
	property string icon: ""
	property string title: ""
	property string subtitle: ""
	property QTabButton tabButton: null

	property alias panelData: panelData
	default property alias panelDataData: panelData.data

	property int horizontalPadding: 20
	property int verticalPadding: 10

	property int maximumWidth: 0
	property int maximumHeight: 0

	property var contextMenuFunc: null


	readonly property bool swipeMode: parent.SwipeView.view
	property bool _isCurrent: parent.SwipeView.isCurrentItem

	signal populated()
	signal panelActivated()

	on_IsCurrentChanged: {
		if (_isCurrent) {
			parent.SwipeView.view.parentPage.contextMenuFunc = contextMenuFunc
			parent.SwipeView.view.parentPage.title = control.title
			parent.SwipeView.view.parentPage.subtitle = control.subtitle
			panelActivated()
		}
	}


	Component.onDestruction: {
		state = ""
		if (tabButton)
			tabButton.destroy()
	}



	Item {
		id: panel

		width: control.swipeMode ? control.width : (maximumWidth ? Math.min(maximumWidth, control.width-2*horizontalPadding) : control.width-2*horizontalPadding)
		height: control.swipeMode ? control.height : (maximumHeight ? Math.min(maximumHeight, control.height-2*verticalPadding) : control.height-2*verticalPadding)
		x: (control.width-width)/2
		y: (control.height-height)/2


		DropShadow {
			visible: !control.swipeMode
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

		Rectangle {
			id: blackbg
			color: JS.setColorAlpha("black", 0.4)
			visible: false
			anchors.fill: panel
		}

		OpacityMask {
			id: opacity1
			anchors.fill: panel
			source: control.swipeMode ? blackbg : metalbg
			maskSource: border2
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

				visible: labelTitle.text.length && !swipeMode

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
					anchors.right: menuButton.left
					anchors.bottom: parent.bottom
					font.weight: Font.Thin
					font.pixelSize: CosStyle.pixelSize*1.4
					font.capitalization: Font.AllUppercase
					elide: Text.ElideRight
					color: titleColor
					verticalAlignment: "AlignVCenter"
					leftPadding: iconImage.visible ? CosStyle.pixelSize/2 : CosStyle.pixelSize
				}

				QToolButton {
					id: menuButton

					anchors.verticalCenter: parent.verticalCenter
					anchors.right: parent.right

					icon.source: CosStyle.iconMenu

					visible: contextMenuFunc && !swipeMode

					Component {
						id: menuComponent
						QMenu { }
					}

					onClicked: if (contextMenuFunc)
								   JS.createMenu(menuButton, menuComponent, [contextMenuFunc])
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
			visible: !control.swipeMode
		}

	}


	states: [
		State {
			name: "VISIBLE"
			when: panelVisible || swipeMode
		}
	]

	transitions: [
		Transition {
			from: "*"
			to: "VISIBLE"

			SequentialAnimation {
				PropertyAction {
					target: control
					property: "visible"
					value: true
				}

				ParallelAnimation {
					NumberAnimation {
						target: panel
						property: "scale"
						from: 0.75
						to: 1.0
						duration: 175
						easing.type: Easing.InOutQuad
					}

					NumberAnimation {
						target: panel
						property: "opacity"
						to: 1.0
						duration: 75
						easing.type: Easing.InOutQuad
					}
				}
			}
		},


		Transition {
			from: "VISIBLE"
			to: "*"

			SequentialAnimation {
				ParallelAnimation {
					NumberAnimation {
						target: panel
						property: "scale"
						to: 0.5
						duration: 125
						easing.type: Easing.InOutQuad
					}

					NumberAnimation {
						target: panel
						property: "opacity"
						to: 0.0
						duration: 125
						easing.type: Easing.InOutQuad
					}

				}

				PropertyAction {
					target: control
					property: "visible"
					value: false
				}
			}
		}
	]



}
