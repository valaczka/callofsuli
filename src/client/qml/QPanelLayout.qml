import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Item {
	id: control

	height: 400
	width: 400

	focus: false

	property int requiredPanelWidth: 600
	property int requiredWidthToDrawer: 2*requiredPanelWidth+drawer.width

	readonly property bool noDrawer: width > requiredWidthToDrawer || leftPanel === null
	property bool swipeMode: control.width-(leftItem.visible ? leftItem.width : 0) < requiredPanelWidth*panels.length

	property alias drawer: drawer

	property var panels: []

	default property Component leftPanel: null


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

		visible: !swipeMode

		Repeater {
			id: mainRepeater
			model: ListModel {
				onCountChanged: {
					if (count == 0) {
						console.debug("reseted")
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

				ParallelAnimation {
					id: animShow
					running: false

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


	SwipeView {
		id: mainSwipe

		anchors.top: parent.top
		anchors.left: leftItem.visible ? leftItem.right : parent.left
		anchors.bottom: parent.bottom
		anchors.right: parent.right
		visible: swipeMode
	}

	PageIndicator {
		id: indicator

		visible: mainSwipe.visible

		count: mainSwipe.count
		currentIndex: mainSwipe.currentIndex

		anchors.bottom: mainSwipe.bottom
		anchors.bottomMargin: 10
		anchors.horizontalCenter: parent.horizontalCenter
	}



	//onNoDrawerChanged: drawerReset()
	onPanelsChanged: panelReset()
	onSwipeModeChanged: panelReset()


	function drawerReset() {
		if (noDrawer)
			drawer.close()
		else if (panels.length === 0)
			drawer.open()
	}



	function panelReset() {
		for (var i=mainSwipe.count-1; i>=0; --i) {
			var p = mainSwipe.itemAt(i)
			mainSwipe.removeItem(p)
		}

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
		for (var i=0; i<panels.length; ++i) {
			var panel = panels[i]
			if (swipeMode) {
				var pp = JS.createObject(panel.url, mainSwipe, panel.params)
				if (pp) {
					mainSwipe.addItem(pp)
					mainSwipe.setCurrentIndex(mainSwipe.count-1)
				}
			} else {
				mainRepeater.model.append(panel)
			}
		}
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
