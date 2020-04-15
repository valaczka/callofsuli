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
		onClicked: if (view) {
					   view.model.remove(modelIndex)
				   }
	}

	QPageHeader {
		id: header

		height: col.height

		Column {
			id: col
			width: parent.width


			QTextField {
				id: newChapterName
				width: parent.width

				placeholderText: qsTr("új célpont hozzáadása")
				onAccepted: {
					if (map.chapterAdd({ "name": newChapterName.text }))
						clear()
				}
			}
		}
	}

	QListItemDelegate {
		id: list
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		modelTitleRole: "name"


		onClicked: pageEditor.chapterSelected(modelIndex, list.model[index].id, -1, -1)

		onLongPressed: {
			menu.modelIndex = index
			menu.popup()
		}

		onRightClicked: {
			menu.modelIndex = index
			menu.popup()
		}

		Keys.onPressed: {
			if (event.key === Qt.Key_Insert) {
			} else if (event.key === Qt.Key_F4 && list.currentIndex !== -1) {
			} else if (event.key === Qt.Key_Delete && list.currentIndex !== -1) {
			}
		}
	}

	QMenu {
		id: menu

		property int modelIndex: -1


		MenuItem {
			text: qsTr("Szerkesztés")
			//onClicked:
		}

		MenuItem {
			text: qsTr("Törlés")
		}

		MenuSeparator {}

		MenuItem {
			text: qsTr("Új küldetés")
		}
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
