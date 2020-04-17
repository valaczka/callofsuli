import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Map map: null

	title: qsTr("Küldetések")

	rightLoader.sourceComponent: QCloseButton {
		visible: modelIndex > 0
		onClicked: if (view) {
					   view.model.remove(modelIndex)
				   }
	}


	QListItemDelegate {
		id: list
		anchors.fill: parent

		modelTitleRole: "name"

		onClicked: pageEditor.missionSelected(modelIndex, list.model[index].id, -1)
	}


	Connections {
		target: map
		onMissionListUpdated: getList()
	}


	Component.onCompleted: getList()

	function getList() {
		list.model = map.missionListGet()
	}
}