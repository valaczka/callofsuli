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

	property int requiredWidthToDrawer: 1080
	property int requiredWidthToLayout: 800
	readonly property bool noDrawer: width > requiredWidthToDrawer
	readonly property bool isLayout: width > requiredWidthToLayout

	property alias drawer: drawer
	property alias panel: panel

	default property Component leftPanel
	property var components: []

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
		visible: noDrawer

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

	StackView {
		id: panel
		x: noDrawer ? drawer.width : 0
		height: parent.height
		width: noDrawer ? control.width-drawer.width : control.width

		focus: false

		initialItem: QCosImage {
			maxWidth: Math.min(panel.width*0.7, 800)
			glowRadius: 6
		}
	}

	onNoDrawerChanged: reset(false)
	onIsLayoutChanged: componentsReset()
	onComponentsChanged: componentsReset()


	function componentsReset() {
		var o = JS.createStackLayout(control.parent, isLayout, components)
		panelLayout.panel.replace(o)
	}


	function reset(prep) {
		if (noDrawer)
			drawer.close()
		else if (!isLayout && components.length === 0)
			drawer.open()

		componentsReset()
	}



	function drawerToggle() {
		if (drawer.position === 1) {
			drawer.close()
		} else if (!noDrawer) {
			drawer.open()
		}
	}


	function layoutBack() {
		if (!panel.currentItem.isLayout && panel.currentItem.swipe.currentIndex > 0) {
			panel.currentItem.swipe.decrementCurrentIndex()
			return true
		}

		return false
	}
}
