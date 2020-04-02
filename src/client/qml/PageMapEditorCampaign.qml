import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Map map: null
	property int campaignId: -1

	title: qsTr("Hadjárat küldetései")

	rightLoader.sourceComponent: QMenuButton {
		MenuItem {
			text: qsTr("Új küldetés")
		}
	}

	QPageHeader {
		id: header

		height: col.height

		Column {
			id: col
			width: parent.width

			QTextField {
				id: campaignName
				width: parent.width
			}
		}
	}

	QListItemDelegate {
		id: list
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom


		modelRightSet: true
		//			modelTitleSet: true
		//			modelSubtitleSet: true

		//onClicked:

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
		target: pageEditor
		onCampaignSelected: campaignId = id
	}

	Component.onCompleted: get()

	onCampaignIdChanged: get()

	function get() {
		if (campaignId == -1) {
			list.model.clear()
			campaignName.text = ""
			return
		}

		var m = map.getMissionList(campaignId)
		list.model.clear()
		for (var i=0; i<m.length; i++) {
			var o = m[i]
			o.labelTitle = o.name
			o.labelRight = o.num
			list.model.append(o)
		}
		list.model.append({
							  id: -1,
							  name: "",
							  labelTitle: qsTr("-- új küldetés --"),
							  labelRight: "",
							  num: m.length+1
						  })
	}
}
