import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: control

	implicitWidth: 800
	implicitHeight: 600

	property int requiredPanelWidth: 600

	property bool swipeMode: control.width < requiredPanelWidth*panelComponents.length

	property alias bgImage: bgImage
	property alias mainSwipe: mainSwipe
	property alias mainRow: mainRow
	property alias mainToolBar: toolbar
	property alias mainToolBarComponent: toolbarRight.sourceComponent
	property string defaultTitle: ""
	property string defaultSubTitle: ""
	property string subtitle: ""

	property alias menuButton: menuButton

	property var mainMenuFunc: null
	property var contextMenuFunc: null

	property list<Component> panelComponents

	property AbstractActivity activity: null

	readonly property bool isCurrentItem: StackView.view && StackView.view.currentItem == control

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




	RowLayout {
		id: mainRow
		anchors.fill: parent
		anchors.topMargin: toolbar.height

		visible: !swipeMode

		Repeater {
			model: !swipeMode ? panelComponents : null

			Loader {
				id: ldrRow
				height: parent.height
				Layout.fillWidth: item.visible && item.layoutFillWidth
				Layout.fillHeight: true
				sourceComponent: panelComponents[index]

				property Page parentPage: control

				onLoaded: item.populated()
			}
		}
	}



	SwipeView {
		id: mainSwipe

		anchors.fill: parent
		anchors.topMargin: toolbar.height-15

		property Page parentPage: control

		currentIndex: tabBar.currentIndex

		visible: swipeMode

		Repeater {
			model: swipeMode ? panelComponents : null

			Loader {
				id: ldrSwipe

				property Page parentPage: control

				sourceComponent: panelComponents[index]

				onLoaded: {
					var item = ldrSwipe.item
					var ppp = JS.createObject("QTabButton.qml", tabBar, {
												  text: item.title ? item.title : "",
												  "icon.source": item.icon ? item.icon : "",
												  iconColor: item.borderColor ? item.borderColor : CosStyle.colorError

											  })

					if (ppp) {
						tabBar.addItem(ppp)
						item.tabButton = ppp
					}

					item.populated()
				}
			}
		}
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

			visible: mainMenuFunc || contextMenuFunc

			Component {
				id: menuComponent
				QMenu {}
			}

			onClicked: JS.createMenu(menuButton, menuComponent, [contextMenuFunc, mainMenuFunc])
		}
	}


	footer: QTabBar {
		id: tabBar

		visible: mainSwipe.visible && tabBar.count>1

		currentIndex: mainSwipe.currentIndex

		onContentChildrenChanged: {
			setCurrentIndex(mainSwipe.currentIndex)
		}
	}


	onSwipeModeChanged: {
		contextMenuFunc = null
		if (!swipeMode)
			title = defaultTitle
	}

	StackView.onRemoved: destroy()

	StackView.onActivated:		 pageActivated()
	StackView.onDeactivated:	pageDeactivated()




	function swipeToPage(idx) {
		if (swipeMode) {
			mainSwipe.setCurrentIndex(idx)
		}
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

		if (layoutBack()) {
			return true
		}

		if (pageStackBack()) {
			return true
		}

		return false
	}



	function layoutBack() {
		if (swipeMode && mainSwipe.currentIndex > 0) {
			mainSwipe.decrementCurrentIndex()
			return true
		}

		return false
	}

}
