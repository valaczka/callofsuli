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
			labelTitle: qsTr("Célpontok")
		}
		ListElement {
			labelTitle: qsTr("Bevezetők")
		}
	}


	onClicked: {
		pageEditor.closeDrawer()
		switch (index) {
		case 0: pageEditor.loadSettings(); break
		case 1: pageEditor.loadCampaigns(); break
		case 2: pageEditor.loadMissions(); break
		case 3: pageEditor.loadChapters(); break
		case 4: pageEditor.loadIntros(); break
		}
	}

}
