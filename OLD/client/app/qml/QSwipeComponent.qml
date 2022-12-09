import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "Style"
import "JScript.js" as JS

Item {
	id: control

	implicitWidth: 1200
	implicitHeight: 400

	property bool swipeMode: width < implicitWidth

	property alias headerContent: header.children
	property alias tabBarContent: tabBar.contentChildren
	property alias swipeContent: layoutSwipe.contentChildren
	property alias content: layoutRow.data

	readonly property alias swipeCurrentIndex: layoutSwipe.currentIndex

	property QBasePage basePage: null

	Item {
		id: header
		width: parent.width
		anchors.top: parent.top
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

		visible: swipeMode && contentChildren.length > 1
	}

	SwipeView {
		id: layoutSwipe

		visible: swipeMode

		currentIndex: tabBar.currentIndex

		anchors.top: tabBar.visible ? tabBar.bottom : header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		onCurrentItemChanged: if (basePage) {
								  if (currentItem && currentItem.contextMenuFunc)
									  basePage.contextMenuFunc = currentItem.contextMenuFunc
								  else
									  basePage.contextMenuFunc = null

								  //currentItem.populated()
							  }
	}


	onSwipeModeChanged: if (basePage) {
		if (swipeMode && layoutSwipe.currentItem && layoutSwipe.currentItem.contextMenuFunc)
			basePage.contextMenuFunc = layoutSwipe.currentItem.contextMenuFunc
		else
			basePage.contextMenuFunc = null
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
