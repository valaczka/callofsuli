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
	property int requiredWidthToDrawer: 2*requiredPanelWidth+drawer.width

	readonly property bool noDrawer: width > requiredWidthToDrawer || leftPanel === null
	property bool swipeMode: control.width-(leftItem.visible ? leftItem.width : 0) < (onlyPanel ? requiredPanelWidth : requiredPanelWidth*panels.length)

	property alias drawer: drawer
	property alias bgImage: bgImage

	property var panels: []

	property Component leftPanel: null
	property Component onlyPanel: null

	property alias mainSwipe: mainSwipe
	property alias mainRow: mainRow
	property alias mainToolBar: toolbar
	property alias mainToolBarComponent: toolbarRight.sourceComponent

	property alias mainMenu: menuButton.menu

	property alias subtitle: toolbar.subtitle

	property Menu pageContextMenu: null

	property bool panelsVisible: true


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

		backButtonIcon: noDrawer ? CosStyle.iconBack : CosStyle.iconDrawer
		backButton.visible: true
		backButton.onClicked: {
			if (noDrawer)
				mainStack.back()
			else
				drawerToggle()
		}

		Loader {
			id: toolbarRight
		}

		QToolButton {
			id: menuButton

			icon.source: CosStyle.iconMenu
			property Menu menu: null

			visible: menu

			onClicked: if (menu) {
						   menu.popup(menuButton, 0, menuButton.height)
					   }
		}
	}


	Drawer {
		id: drawer

		width: 300
		height: control.height
		modal: true
		interactive: !noDrawer

		enabled: !noDrawer

		background: Rectangle {
			anchors.fill: parent
			color: CosStyle.colorPrimaryDark
			opacity: 0.5
		}

		Loader {
			anchors.fill: parent
			sourceComponent: noDrawer ? undefined : leftPanel
		}
	}


	Item {
		id: leftItem
		width: drawer.width
		height: parent.height
		visible: noDrawer && leftPanel !== null

		Rectangle {
			anchors.fill: parent
			color: CosStyle.colorPrimaryDark
			opacity: 0.5
		}

		Loader {
			anchors.fill: parent
			sourceComponent: noDrawer ? leftPanel : undefined
		}
	}


	RowLayout {
		id: mainRow
		anchors.top: parent.top
		anchors.left: leftItem.visible ? leftItem.right : parent.left
		anchors.bottom: parent.bottom
		anchors.right: parent.right

		visible: panelsVisible && !swipeMode

		Repeater {
			id: mainRepeater
			model: ListModel {
				onCountChanged: {
					if (count == 0) {
						panelResetAfterRemove()
					}
				}
			}

			Loader {
				height: parent.height
				Layout.fillWidth: fillWidth
				Layout.fillHeight: true
				id: ldr

				opacity: 0.0

				Component.onCompleted: ldr.setSource(url, params)

				SequentialAnimation {
					id: animShow
					running: false

					ParallelAnimation {
						NumberAnimation {
							target: ldr
							property: "scale"
							from: 0.75
							to: 1.0
							duration: 175
							easing.type: Easing.InOutQuad
						}

						NumberAnimation {
							target: ldr
							property: "opacity"
							to: 1.0
							duration: 75
							easing.type: Easing.InOutQuad
						}
					}

					ScriptAction {
						script: ldr.item.populated()
					}
				}


				ParallelAnimation {
					id: animHide
					running: false

					NumberAnimation {
						target: ldr
						property: "scale"
						to: 0.5
						duration: 125
						easing.type: Easing.InOutQuad
					}

					NumberAnimation {
						target: ldr
						property: "opacity"
						to: 0.0
						duration: 125
						easing.type: Easing.InOutQuad
					}

					onFinished: {
						mainRepeater.model.remove(index)
					}
				}

				onLoaded: animShow.start()

				function hide() {
					animHide.start()
				}
			}
		}
	}

	Loader {
		id: onlyLoader
		anchors.fill: mainRow
		visible: mainRow.visible && onlyPanel

		onStatusChanged: if (onlyLoader.status == Loader.Ready) item.populated()

	}


	SwipeView {
		id: mainSwipe

		anchors.top: parent.top
		anchors.left: leftItem.visible ? leftItem.right : parent.left
		anchors.bottom: parent.bottom
		anchors.right: parent.right
		visible: panelsVisible && swipeMode

		property Page parentPage: control
	}


	footer: QTabBar {
		id: tabBar

		visible: mainSwipe.visible && tabBar.count>1
		currentIndex: mainSwipe.currentIndex
		onCurrentIndexChanged: mainSwipe.currentIndex = currentIndex
	}


	onOnlyPanelChanged: { if (control.StackView.status === StackView.Active) { console.debug("onlypanelschanged", control, onlyPanel); panelReset() } }
	onPanelsChanged: { if (control.StackView.status === StackView.Active) { console.debug("panelschanged", control, panels); panelReset() } }
	onSwipeModeChanged: { if (control.StackView.status === StackView.Active) { console.debug("swipechanged", control, panels); panelReset() }  }


	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.resetTitle()
		pageActivated()
	}



	function swipeToPage(idx) {
		if (swipeMode)
			mainSwipe.currentIndex = idx
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

		drawer.close()

		return false
	}



	function drawerReset() {
		if (noDrawer)
			drawer.close()
		else if (panels.length === 0)
			drawer.open()
	}



	function panelReset() {
		console.debug("PANEL RESET ", control)
		if (swipeMode)
			mainMenu = null
		else
			mainMenu = pageContextMenu


		for (var i=tabBar.count-1; i>=0; --i)
			tabBar.removeItem(tabBar.itemAt(i))

		for (i=mainSwipe.count-1; i>=0; --i)
			mainSwipe.removeItem(mainSwipe.itemAt(i))

		if (mainRepeater.model.count) {
			for (i=mainRepeater.model.count-1; i>=0; --i) {
				var o = mainRepeater.itemAt(i)
				o.hide()
			}

			return
		}

		panelResetAfterRemove()
	}


	function panelResetAfterRemove() {
		if (onlyPanel) {
			if (swipeMode) {
				onlyLoader.sourceComponent = undefined
				var o = onlyPanel.createObject(mainSwipe)
				mainSwipe.addItem(o)
				o.populated()

				var ppp = JS.createObject("QTabButton.qml", tabBar, {text: o.title ? o.title : "", "icon.source": o.icon ? o.icon : "" })

				if (ppp)
					tabBar.addItem(ppp)

			} else {
				onlyLoader.sourceComponent = onlyPanel
			}
		} else {
			for (var i=0; i<panels.length; ++i) {
				var panel = panels[i]
				if (swipeMode) {
					var pp = JS.createObject(panel.url, control, panel.params)
					if (pp) {
						mainSwipe.addItem(pp)
						pp.populated()
					}

					ppp = JS.createObject("QTabButton.qml", tabBar, {
											  text: pp.title ? pp.title : "",
											  "icon.source": pp.icon ? pp.icon : "",
											  iconColor: pp.borderColor ? pp.borderColor : CosStyle.colorError

										  })

					if (ppp)
						tabBar.addItem(ppp)

				} else {
					mainRepeater.model.append(panel)
				}
			}
		}

		if (swipeMode)
			tabBar.setCurrentIndex(0)
	}



	function drawerToggle() {
		if (drawer.position === 1) {
			drawer.close()
		} else if (!noDrawer) {
			drawer.open()
		}
	}


	function layoutBack() {
		if (swipeMode && mainSwipe.currentIndex > 0) {
			mainSwipe.decrementCurrentIndex()
			return true
		}

		return false
	}

}
