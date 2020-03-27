import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: page

	property string fileName: ""

	Map {
		id: map

		client: cosClient
	}

	header: QToolBar {
		id: toolbar

		backButton.visible: true
		backButton.onClicked: mainStack.back()
	}

	Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/img/villa.png"
	}

	Column {
		QButton {
			label: "NEW"
			onClicked: map.create()
		}

		QButton {
			label: "SAVE"
			onClicked: map.saveToFile("/tmp/aaaaaa.cosm")
		}

		QButton {
			label: "LOAD"
			onClicked: map.loadFromFile("/tmp/aaaaaa.cosm")
		}
	}


	/* CONTENT */
	BusyIndicator {
		id: busy
		anchors.centerIn: parent
		running: false
	}
	/* CONTENT */

	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.title = qsTr("Pályaszerkesztő")
		/* LOAD */
	}

	StackView.onDeactivated: {
		/* UNLOAD */
	}


	Component.onCompleted: {
		if (!map.databaseOpen())
			mainStack.back()
		/*		if (fileName.length) {
			map.loadFromFile(fileName)
		} else {
			map.create()
		} */
	}

	function stackBack() {
		if (mainStack.depth > page.StackView.index+1) {
			if (!mainStack.get(page.StackView.index+1).stackBack()) {
				if (mainStack.depth > page.StackView.index+1) {
					mainStack.pop(page)
				}
			}
			return true
		}

		/* BACK */

		return false
	}
}
