import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QListItemDelegate {
	id: list
	anchors.fill: parent

	model: ListModel {
		ListElement {
			labelTitle: qsTr("Beállítások")
		}
		ListElement {
			labelTitle: qsTr("Hadjáratok")
		}
		ListElement {
			labelTitle: qsTr("Küldetések")
		}
		ListElement {
			labelTitle: qsTr("Fejezetek")
		}
		ListElement {
			labelTitle: qsTr("Célpontok")
		}
		ListElement {
			labelTitle: qsTr("Bevezetők")
		}
	}


	onClicked: {
		switch (index) {
		case 0: pageEditor.loadSettings(); break
		case 1: pageEditor.loadCampaigns(); break
		}
	}

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
}
