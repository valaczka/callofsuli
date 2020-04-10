import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Map map: null


	title: qsTr("Hadjáratok")

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
				id: newCampaignName
				width: parent.width

				placeholderText: qsTr("új hadjárat hozzáadása")
				onAccepted: {
					if (map.campaignAdd({ "name": newCampaignName.text }))
						newCampaignName.clear()
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


		//			modelTitleSet: true
		//			modelSubtitleSet: true

		onClicked: pageEditor.campaignSelected(modelIndex, list.model.get(index).id)

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
		onCampaignListUpdated: getList()
	}

	Component.onCompleted: getList()


	function getList() {
		var m = map.campaignListGet()
		list.model.clear()
		for (var i=0; i<m.length; i++) {
			var o = m[i]
			o.labelTitle = o.name
			list.model.append(o)
		}
	}
}
