import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "Style"
import "JScript.js" as JS

Page {
	id: control

	implicitWidth: 800
	implicitHeight: 600

	property bool swipeMode: width < implicitWidth

	property string defaultTitle: ""
	property string defaultSubTitle: ""
	property string subtitle: ""

	property alias bgImage: bgImage
	property alias mainToolBar: toolbar
	property alias mainToolBarComponent: toolbarRight.sourceComponent

	property alias menuButton: menuButton

	property var mainMenuFunc: null

	property alias headerContent: header.children
	property alias tabBarContent: tabBar.contentChildren
	property alias swipeContent: layoutSwipe.contentChildren
	property alias content: layoutRow.data

	readonly property alias swipeCurrentIndex: layoutSwipe.currentIndex

	property AbstractActivity activity: null

	readonly property bool isCurrentItem: StackView.view && StackView.view.currentItem == control

	focus: true

	signal pageActivated()
	signal pageDeactivated()

	onActivityChanged: if (activity) {
						   activity.client = cosClient
					   }


	background: Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/internal/img/villa.png"
	}



	QToolBar {
		id: toolbar

		x: 0
		y: 0
		z: 10
		width: parent.width


		title: control.title.length ? control.title : control.defaultTitle
		subtitle: control.subtitle.length ? control.subtitle : control.defaultSubTitle

		backButtonIcon: CosStyle.iconBack
		backButton.visible: true
		backButton.onClicked: mainStack.back()

		QToolBusyIndicator {
			implicitHeight: CosStyle.pixelSize*1.7
			implicitWidth: implicitHeight
			height: implicitHeight
			width: running ? height : 0
			Layout.alignment: Qt.AlignCenter
			running: activity && activity.isBusy
			visible: running
		}

		Loader {
			id: toolbarRight
		}

		QToolButton {
			id: menuButton

			icon.source: CosStyle.iconMenu

			visible: mainMenuFunc

			Component {
				id: menuComponent
				QMenu {}
			}

			onClicked: JS.createMenu(menuButton, menuComponent, [mainMenuFunc])
		}
	}


	Item {
		id: header
		width: parent.width
		anchors.top: toolbar.bottom
		visible: children.length
		height: childrenRect.height
	}

	RowLayout {
		id: layoutRow

		visible: !swipeMode

		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

	}

	QTabBar {
		id: tabBar
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right

		currentIndex: layoutSwipe.currentIndex

		visible: swipeMode && contentChildren.length
	}

	SwipeView {
		id: layoutSwipe

		visible: swipeMode

		currentIndex: tabBar.currentIndex

		anchors.top: tabBar.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom
	}


	StackView.onRemoved: destroy()

	StackView.onActivated: {
		pageActivated()
	}

	StackView.onDeactivated:	pageDeactivated()


	function stackBack() {
		if (mainStack.depth > control.StackView.index+1) {
			if (!mainStack.get(control.StackView.index+1).stackBack()) {
				if (mainStack.depth > control.StackView.index+1) {
					mainStack.pop(control)
				}
			}
			return true
		}

		if (layoutBack()) {
			return true
		}

		if (pageStackBack()) {
			return true
		}

		return false
	}


	function swipeToPage(idx) {
		if (swipeMode)
			layoutSwipe.setCurrentIndex(idx)
	}


	function layoutBack() {
		if (swipeMode && layoutSwipe.currentIndex > 0) {
			layoutSwipe.decrementCurrentIndex()
			return true
		}

		return false
	}

}
