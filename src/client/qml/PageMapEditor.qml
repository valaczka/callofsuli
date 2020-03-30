import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: page

	property int mapId: -1
	property string mapName: ""
	property alias map: map
	property bool mapBinaryFormat: true

	Map {
		id: map

		client: cosClient
		mapType: Map.MapEditor

		onMapRefreshed:	lblJson.text = data
	}

	header: QToolBar {
		id: toolbar

		backButton.visible: true
		backButton.onClicked: mainStack.back()

		rightLoader.sourceComponent: Row {
			QToolBusyIndicator { running: false }
			QMenuButton {
				MenuItem {
					text: qsTr("SAVE ")+mapBinaryFormat
					onClicked:  {
						map.save(mapId, mapBinaryFormat)
					}
				}
			}
		}
	}

	Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/img/villa.png"
	}

	Label {
		id: lblJson
	}





	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.title = mapName
	}

	StackView.onDeactivated: {
		/* UNLOAD */
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
