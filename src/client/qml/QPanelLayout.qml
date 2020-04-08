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

	property int requiredPanelWidth: 400
	property int requiredWidthToDrawer: 2*requiredPanelWidth+drawer.width

	readonly property bool noDrawer: width > requiredWidthToDrawer

	property alias drawer: drawer
	property alias panel: panel
	property alias model: panel.model

	default property Component leftPanel


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

	ListView {
		id: panel
		x: noDrawer ? drawer.width : 0
		height: parent.height
		width: noDrawer ? control.width-drawer.width : control.width

		clip: true

		orientation: Qt.Horizontal
		snapMode: ListView.SnapToItem
		boundsBehavior: Flickable.StopAtBounds

		model: ListModel { }

		delegate: Component {
			Loader {
				height: panel.height
				width: Math.max(panel.width/(Math.floor(panel.width/requiredPanelWidth)), Math.floor(panel.width/panel.model.count))
				id: ldr

				property int modelIndex: index
				property ListView view: panel

				Component.onCompleted: ldr.setSource(model.url, model.params)
			}

		}

	}

	onNoDrawerChanged: reset()


	function reset() {
		if (noDrawer)
			drawer.close()
		else if (panel.model.count === 0)
			drawer.open()
	}


	function loadPage(modelIndex, id, page, _params) {
		if (model.count > modelIndex+1) {
			if (model.get(modelIndex+1).url === page) {
				if (model.count > modelIndex+2) {
					model.remove(modelIndex+2, model.count-(modelIndex+2))
				}
				return;
			}

			model.remove(modelIndex+1, model.count-(modelIndex+1))
		}

		model.append( { url: page, params: _params } )
		panel.positionViewAtEnd();
	}


	function drawerToggle() {
		if (drawer.position === 1) {
			drawer.close()
		} else if (!noDrawer) {
			drawer.open()
		}
	}


	function layoutBack() {
		if (!panel.atXBeginning) {
			panel.positionViewAtBeginning()
			return true
		}

		return false
	}
}
