import QtQuick 2.14
import QtQuick.Controls 2.14
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
	property alias subtitle: toolbar.subtitle

	property var mainMenuFunc: null
	property var contextMenuFunc: null

	property list<Component> panelComponents

	signal pageActivated()


	background: Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/internal/img/villa.png"
	}

	header: QToolBar {
		id: toolbar

		title: control.title

		backButtonIcon: CosStyle.iconBack
		backButton.visible: true
		backButton.onClicked: mainStack.back()

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


	RowLayout {
		id: mainRow
		anchors.fill: parent

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
		property Page parentPage: control

		onCurrentIndexChanged: tabBar.setCurrentIndex(currentIndex)

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


	footer: QTabBar {
		id: tabBar

		visible: mainSwipe.visible && tabBar.count>1

		swipeView: mainSwipe

		onContentChildrenChanged: {
			setCurrentIndex(mainSwipe.currentIndex)
		}
	}


	onSwipeModeChanged: contextMenuFunc = null

	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.resetTitle()
		pageActivated()
	}




	function swipeToPage(idx) {
		if (swipeMode) {
			mainSwipe.setCurrentIndex(idx)
		}
	}


	function stackBack() {
		if (layoutBack()) {
			return true
		}

		if (mainStack.depth > control.StackView.index+1) {
			if (!mainStack.get(control.StackView.index+1).stackBack()) {
				if (mainStack.depth > control.StackView.index+1) {
					mainStack.pop(control)
				}
			}
			return true
		}

		if (pageStackBack())
			return true

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
