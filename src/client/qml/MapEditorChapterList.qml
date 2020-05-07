import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Map map: null

	title: qsTr("Célpontok")




	QListItemDelegate {
		id: list
		anchors.fill: parent

		modelTitleRole: "name"


		onClicked: pageEditor.chapterSelected(list.model[index].id, -1, -1)
	}



	Connections {
		target: map
		onChapterListUpdated: getList()
		onUndone: getList()
	}



	function populated() { getList() }

	function getList() {
		list.model = map.chapterListGet()
	}
}
