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


		onClicked: pageEditor.chapterSelected(modelIndex, list.model[index].id, -1, -1)
	}



	Connections {
		target: map
		onChapterListUpdated: getList()
	}



	Component.onCompleted: getList()

	function getList() {
		list.model = map.chapterListGet()
	}
}
