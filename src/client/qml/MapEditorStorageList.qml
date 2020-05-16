import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property MapEditor map: null

	title: qsTr("Célpontok")




	MapEditorStorageWidget {
		id: storageWidget

		width: parent.width
		map: panel.map

		onStorageSelected: pageEditor.storageSelected(id, -1, -1)
		onObjectiveSelected: pageEditor.objectiveSelected(id, -1, -1)
	}



	Connections {
		target: map
		onStorageListUpdated: getList()
		onUndone: getList()
	}



	function populated() { getList() }

	function getList() {
		storageWidget.load(map.storageListGet())
	}
}
