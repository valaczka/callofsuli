import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Map map: null

	title: qsTr("Bevezetők")

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
				id: newIntroName
				width: parent.width

				placeholderText: qsTr("új bevezető hozzáadása")
				onAccepted: {
					if (map.introAdd({ "ttext": newIntroName.text }))
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

		modelTitleRole: "ttext"

		onClicked: pageEditor.introSelected(modelIndex, list.model[index].id, -1, Map.IntroUndefined)

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
		onIntroListUpdated: getList()
	}



	Component.onCompleted: getList()

	function getList() {
		list.model = map.introListGet()
	}
}
