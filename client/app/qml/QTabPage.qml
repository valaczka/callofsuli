import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.3
import COS.Client 1.0
import "Style"
import "JScript.js" as JS

Page {
	id: control

	implicitWidth: 800
	implicitHeight: 600

	property string contentTitle: ""

	property AbstractActivity activity: null
	property var backFunction: function() { mainStack.back() }

	property alias buttonModel: rptr.model
	property alias buttons: row.children

	property bool compact: width < 1280 || height < 720

	property alias headerItem: header
	property alias menuButton: menuButton
	property alias backButtonVisible: backButton.visible
	property alias tabBarVisible: tabbar.visible
	property alias labelTitle: labelTitle
	property alias stack: stack
	property alias toolButtonAction: toolButton.action
	property alias toolBarLoaderComponent: toolbarLoader.sourceComponent

	property color backgroundColor: "black"
	property color backgroundImageColor: JS.setColorAlpha(backgroundColor, 0.4)
	property color buttonColor: "#99ffffff"
	property color buttonActiveColor: CosStyle.colorAccent
	property alias buttonBackgroundColor: tabbar.color

	property var pageBackCallbackFunction: null
	property var closeCallbackFunction: windowCloseFunction

	readonly property real headerPadding: compact ? header.height : 0

	readonly property bool isCurrentItem: StackView.view && StackView.view.currentItem == control

	property bool _isFirstActivation: true

	focus: true

	signal pageActivated()
	signal pageDeactivated()
	signal pageActivatedFirst()

	signal activateButton(int index)


	background: Item {
		anchors.fill: parent
		Image {
			id: bgImage
			anchors.fill: parent
			fillMode: Image.PreserveAspectCrop
			source: "qrc:/internal/img/villa.png"
			visible: !compact
		}
		ColorOverlay {
			anchors.fill: bgImage
			source: bgImage
			color: backgroundImageColor
			visible: compact
		}
	}



	StackView {
		id: stack
		anchors.top: compact ? parent.top : header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: tabbar.visible ? tabbar.top : parent.bottom
		opacity: compact ? 0.0 : 1.0

		property real maximumPanelWidth: 800

		replaceEnter: Transition {
			PropertyAnimation {
				property: "opacity"
				from: 0.0
				to: 1.0
			}
		}

		replaceExit: Transition {
			PropertyAnimation {
				property: "opacity"
				from: 1.0
				to: 0.0
			}
		}
	}

	Item {
		id: maskItem
		anchors.fill: stack
		Rectangle {
			id: topRectangle
			anchors.left: parent.left
			anchors.top: parent.top
			anchors.right: parent.right
			height: header.height*1.25
			gradient: Gradient {
				orientation: Gradient.Vertical
				GradientStop { position: 0.0; color: "transparent" }
				GradientStop { position: 0.2; color: "transparent" }
				GradientStop { position: 0.4; color: JS.setColorAlpha("white", 0.1) }
				GradientStop { position: 0.7; color: JS.setColorAlpha("white", 0.4) }
				GradientStop { position: 1.0; color: "white" }
			}
		}
		Rectangle {
			anchors.left: parent.left
			anchors.right: parent.right
			anchors.top: topRectangle.bottom
			anchors.bottom: parent.bottom
			color: "white"
		}

		visible: false
	}

	OpacityMask {
		anchors.fill: stack
		source: stack
		maskSource: maskItem
		visible: compact
	}


	Item {
		id: header

		anchors.top: parent.top
		anchors.left: parent.left
		anchors.right: parent.right

		z: 10

		height: Math.max(indicator.height, menuButton.height, backButton.height, labelTitle.height, 48)

		Rectangle {
			id: headerBg
			color: "transparent"
			anchors.fill: parent
			visible: !compact

			DropShadow {
				id: dropshadow
				anchors.fill: border2
				horizontalOffset: 3
				verticalOffset: 3
				color: JS.setColorAlpha("black", 0.75)
				source: border2
			}

			BorderImage {
				id: border2
				source: "qrc:/internal/img/border3.svg"
				visible: false

				//sourceSize.height: 141
				//sourceSize.width: 414

				anchors.fill: parent
				anchors.rightMargin: dropshadow.horizontalOffset
				border.top: 10
				border.left: 5
				border.right: 80
				border.bottom: 10

				horizontalTileMode: BorderImage.Repeat
				verticalTileMode: BorderImage.Repeat
			}


			Image {
				id: metalbg
				source: "qrc:/internal/img/metalbg2.png"
				visible: false
				fillMode: Image.Tile
				anchors.fill: border2
			}

			OpacityMask {
				id: opacity1
				anchors.fill: border2
				source: metalbg
				maskSource: border2
			}
		}

		QToolButton {
			id: backButton

			anchors.left: parent.left
			anchors.verticalCenter: parent.verticalCenter
			anchors.verticalCenterOffset: compact ? 0 : -2

			visible: false

			icon.source: CosStyle.iconBack

			onClicked: {
				/*if (stack.depth > 1)
					stack.pop()
				else if (backFunction)
					backFunction()*/
				mainStack.back()
			}

			states: State {
				name: "VISIBLE"
				when: backFunction || stack.depth > 1
			}

			transitions: [
				Transition {
					from: "*"
					to: "VISIBLE"
					SequentialAnimation {
						PropertyAction {
							target: backButton
							property: "opacity"
							value: 0.0
						}
						PropertyAction {
							target: backButton
							property: "width"
							value: 0.0
						}
						PropertyAction {
							target: backButton
							property: "visible"
							value: true
						}
						ParallelAnimation {
							NumberAnimation {
								target: backButton
								property: "width"
								to: backButton.implicitWidth
								duration: 175
								easing.type: Easing.OutQuad
							}
							NumberAnimation {
								target: labelTitle
								property: "leftPadding"
								to: 0
								duration: 175
								easing.type: Easing.OutQuad
							}
						}
						NumberAnimation {
							target: backButton
							property: "opacity"
							to: 1.0
							duration: 175
						}
					}
				},
				Transition {
					from: "VISIBLE"
					to: "*"
					SequentialAnimation {
						NumberAnimation {
							target: backButton
							property: "opacity"
							to: 0.0
							duration: 125
							easing.type: Easing.InQuad
						}
						ParallelAnimation {
							NumberAnimation {
								target: backButton
								property: "width"
								to: 0
								duration: 125
								easing.type: Easing.InQuad
							}
							NumberAnimation {
								target: labelTitle
								property: "leftPadding"
								to: 10
								duration: 125
								easing.type: Easing.OutQuad
							}
						}
						PropertyAction {
							target: backButton
							property: "visible"
							value: false
						}
					}
				}
			]
		}

		QLabel {
			id: labelTitle
			anchors.left: backButton.visible ? backButton.right : parent.left
			anchors.right: indicator.visible ? indicator.left :
											   toolbarLoader.item ? toolbarLoader.left :
																	toolButton.visible ? toolButton.left :
																						 menuButton.visible ? menuButton.left :
																											  parent.right
			anchors.verticalCenter: parent.verticalCenter
			anchors.verticalCenterOffset: compact ? 0 : -2

			text: control.contentTitle != "" ? control.contentTitle
											 : control.title != "" ? control.title
																   : !compact ? cosClient.serverName
																			  : ""
			font.pixelSize: CosStyle.pixelSize*1.2
			font.weight: Font.Normal
			color: CosStyle.colorAccentLighter
			elide: Text.ElideRight
			leftPadding: 10
			rightPadding: 5
			topPadding: 5
			bottomPadding: 5
		}

		QToolBusyIndicator {
			id: indicator
			implicitHeight: CosStyle.pixelSize*1.7
			implicitWidth: implicitHeight
			height: implicitHeight
			width: running ? height : 0
			running: activity && activity.isBusy
			visible: running
			anchors.right: toolbarLoader.item ? toolbarLoader.left :
												toolButton.visible ? toolButton.left :
																	 menuButton.visible ? menuButton.left : parent.right
			anchors.rightMargin: !compact && !menuButton.visible && !toolButton.visible && !toolbarLoader.item ? 5 : 0
			anchors.verticalCenter: parent.verticalCenter
			anchors.verticalCenterOffset: compact ? 0 : -2
		}

		Loader {
			id: toolbarLoader
			anchors.right: toolButton.visible ? toolButton.left :
												menuButton.visible ? menuButton.left : parent.right
			anchors.rightMargin: !compact && !menuButton.visible && !toolButton.visible ? 5 : 0
			anchors.verticalCenter: parent.verticalCenter
			anchors.verticalCenterOffset: compact ? 0 : -2
		}

		QToolButton {
			id: toolButton
			action: null
			visible: action
			anchors.right: menuButton.visible ? menuButton.left : parent.right
			anchors.rightMargin: !compact && !menuButton.visible ? 5 : 0
			anchors.verticalCenter: parent.verticalCenter
			anchors.verticalCenterOffset: compact ? 0 : -2
		}

		QMenuButton {
			id: menuButton

			anchors.right: parent.right
			anchors.rightMargin: compact ? 0 : 5
			anchors.verticalCenter: parent.verticalCenter
			anchors.verticalCenterOffset: compact ? 0 : -2
		}
	}


	DropShadow {
		id: dropshadow2
		anchors.fill: tabbar
		horizontalOffset: -3
		verticalOffset: -3
		color: JS.setColorAlpha("black", 0.75)
		source: tabbar
		visible: tabbar.visible
	}

	Rectangle {
		id: tabbar
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom
		height: row.height
		color: compact ? backgroundColor : "transparent"

		Image {
			id: metalbg2
			source: "qrc:/internal/img/metalbg2.png"
			visible: !compact
			fillMode: Image.Tile
			anchors.fill: parent
		}


		Row {
			id: row

			Repeater {
				id: rptr
				model: ListModel { }

				ToolButton {
					anchors.verticalCenter: parent ? parent.verticalCenter : undefined
					autoExclusive: true
					Material.foreground: compact ? buttonColor : CosStyle.colorPrimaryDarker
					Material.accent: model.iconColor ? model.iconColor : buttonActiveColor

					font.pixelSize: CosStyle.pixelSize*0.85
					font.capitalization: Font.MixedCase
					font.weight: checked ? Font.DemiBold : Font.Normal
					icon.height: checked || model.raised ? CosStyle.pixelSize*2 : CosStyle.pixelSize*1.7
					icon.width: checked || model.raised ? CosStyle.pixelSize*2 : CosStyle.pixelSize*1.7

					display: AbstractButton.TextUnderIcon
					icon.source: model.icon
					icon.color: checked ? (model.iconColor ? model.iconColor : buttonActiveColor)
										: compact ? buttonColor : CosStyle.colorPrimaryDarker
					text: model.title
					onClicked: checked = true

					checked: model.checked

					onCheckedChanged: if (checked) {
										  if (model.func)
											  model.func()
									  }

					width: rptr.count ? tabbar.width/rptr.count : implicitWidth

					Connections {
						target: control

						function onActivateButton(idx) {
							if (idx === index) {
								checked = false
								checked = true
							}
						}
					}
				}
			}
		}
	}



	StackView.onRemoved: destroy()

	StackView.onActivated: {
		pageActivated()

		if (_isFirstActivation) {
			pageActivatedFirst()
			_isFirstActivation = false
		}
	}

	StackView.onDeactivated: pageDeactivated()


	function replaceContent(_component, _params) {
		var d = {}
		if (_params)
			d = _params
		d.tabPage = control
		if (_component)
			stack.replace(null, _component, d, StackView.ReplaceTransition)
		else
			stack.clear(StackView.ReplaceTransition)
	}

	function pushContent(_component, _params) {
		var d = {}
		if (_params)
			d = _params
		d.tabPage = control
		stack.push(_component, d)
	}


	function stackBack() {
		if (mainStack.depth > control.StackView.index+1) {
			if (!mainStack.get(control.StackView.index+1).stackBack()) {
				if (mainStack.depth > control.StackView.index+1) {
					mainStack.pop(control)
				}
			}
			return true
		}

		if (stack.currentItem) {
			if (stack.currentItem.backCallbackFunction) {
				if (stack.currentItem.backCallbackFunction())
					return true
			}
		}

		if (stack.depth > 1) {
			stack.pop()
			return true
		}

		if (pageBackCallbackFunction) {
			if (pageBackCallbackFunction())
				return true
		}

		return false
	}


	function windowCloseFunction() {
		if (stack.currentItem) {
			if (stack.currentItem.closeCallbackFunction) {
				if (stack.currentItem.closeCallbackFunction())
					return true
			}
		}

		return false
	}
}
