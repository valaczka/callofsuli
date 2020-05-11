import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property MapEditor map: null

	title: qsTr("Bevezetők")



	QListItemDelegate {
		id: list
		anchors.fill: parent

		modelTitleRole: "ttext"

		onClicked: pageEditor.introSelected(list.model[index].id, -1, Map.IntroUndefined)

	}


	Connections {
		target: map
		onIntroListUpdated: getList()
		onUndone: getList()
	}


	function populated() { getList() }

	function getList() {
		list.model = map.introListGet()
	}
}
