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
	property SwipeView swipeView: null
	property int swipeViewIndex: -1

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

				onEditingFinished: map.campaignUpdate(campaignId, { "name": campaignName.text })
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
		onCampaignSelected: {
			campaignId = id
			swipeToCurrent()
		}
	}


	Connections {
		target: map
		onCampaignUpdated: if (id===campaignId) get()
	}

	Component.onCompleted: get()

	onCampaignIdChanged: get()

	function swipeToCurrent() {
		if (swipeView)
			swipeView.setCurrentIndex(swipeViewIndex)
	}

	function get() {
		if (campaignId == -1) {
			list.model.clear()
			campaignName.text = ""
			return
		}

		list.model.clear()

		var p = map.campaignGet(campaignId)
		campaignName.text = p["name"]

		if (p["introId"] !== -1) {
			list.model.append({
								  id: -2,
								  introId: p["introId"],
								  labelTitle: qsTr(" -- Bevezető --"),
								  labelRight: "",
								  num: 0
							  })
		} else {
			list.model.append({
								  id: -2,
								  introId: -1,
								  labelTitle: qsTr("-- Bevezető hozzáadása --"),
								  labelRight: "",
								  num: 0
							  })
		}

		var m = map.missionListGet(campaignId)
		for (var i=0; i<m.length; i++) {
			var o = m[i]
			o.labelTitle = o.name
			o.labelRight = ""
			list.model.append(o)
		}
		list.model.append({
							  id: -1,
							  name: "",
							  labelTitle: qsTr("-- küldetés hozzáadása --"),
							  labelRight: "",
							  num: m.length+1,
						  })

		if (p["summaryId"] !== -1) {
			list.model.append({
								  id: -3,
								  summaryId: p["summaryId"],
								  labelTitle: qsTr("Összegzés"),
								  labelRight: "",
								  num: m.length+2
							  })
		} else if (m.length) {
			list.model.append({
								  id: -3,
								  summaryId: -1,
								  labelTitle: qsTr("-- Összegzés hozzáadása --"),
								  labelRight: "",
								  num: m.length+2
							  })
		}
	}
}
