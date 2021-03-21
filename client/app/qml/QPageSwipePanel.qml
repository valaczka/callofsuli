import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import "Style"
import "JScript.js" as JS

Item {
	id: control

	implicitWidth: visible ? Math.max(500, maximumWidth) : 0
	implicitHeight: 200

	focus: true

	property string icon: ""
	property string title: ""
	property string subtitle: ""

	property var contextMenuFunc: null

	property bool stackMode: false
	property StackView pageStack: null


	property alias headerContent: header.children
	property alias tabBarContent: tabBar.contentChildren
	property alias swipeContent: layoutSwipe.contentChildren
	default property alias rowContent: layoutRow.children

	Layout.fillWidth: visible

	signal populated()


	Component.onDestruction: {
		state = ""
	}

	StackView.onActivating: {
		if (pageStack) {
			pageStack.parentPage.title = control.title
			pageStack.parentPage.subtitle = control.subtitle
		}
	}

	onTitleChanged: if (pageStack) pageStack.parentPage.title = control.title
	onSubtitleChanged: if (pageStack) pageStack.parentPage.subtitle = control.subtitle

	StackView.onActivated: {
		if (pageStack) {
			pageStack.parentPage.contextMenuFunc = contextMenuFunc

			if (pageStack.parentPage.isCurrentItem)
				populated()
		}
	}

	StackView.onRemoved: destroy()

	Item {
		id: header
		width: parent.width
		visible: children.length
		height: childrenRect.height
	}

	RowLayout {
		id: layoutRow

		visible: !stackMode

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

		visible: stackMode && contentChildren.length
	}

	SwipeView {
		id: layoutSwipe

		visible: stackMode

		currentIndex: tabBar.currentIndex

		anchors.top: tabBar.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom
	}
}
