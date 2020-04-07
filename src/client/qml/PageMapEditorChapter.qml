import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Map map: null
	property int chapterId: -1
	property int parentMissionId: -1
	property int parentSummaryId: -1

	title: qsTr("Célpont")

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
				id: chapterName
				width: parent.width

				onEditingFinished: if (chapterId != -1) map.chapterUpdate(chapterId, { "name": chapterName.text }, parentMissionId, parentSummaryId)
			}

			Label {
				id: chapterMissions
			}

			Label {
				id: chapterCampaigns
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

		onClicked: {
			var o = list.model.get(index)
			/*if (o.id >= 0)
				pageEditor.missionSelected(modelIndex, o.id)
			else if (o.id === -1) {
				var d = JS.dialogCreate(dlgMissionName)
				d.open()
			} else if (o.id === -3) {
				if (o.summaryId > -1)
					pageEditor.summarySelected(modelIndex, o.summaryId)
				else {
					var sumId = map.campaignSummaryAdd(chapterId)
					if (sumId !== -1)
						pageEditor.summarySelected(modelIndex, sumId)
				}
			}*/
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
		onChapterSelected: {
			chapterId = id
		}
	}


	Connections {
		target: map
		onChapterUpdated: if (id===chapterId) get()
	}

	Component.onCompleted: get()

	onChapterIdChanged: get()

	function get() {
		if (chapterId == -1 || map == null) {
			list.model.clear()
			chapterName.text = ""
			return
		}

		list.model.clear()

		var p = map.chapterGet(chapterId)
		chapterName.text = p["name"]



		var q = "Küldetések: "
		for (var i=0; i<p.missions.length; i++) {
			var o = p.missions[i]
			o.label = o.name
			q += " "+o.name
		}
		chapterMissions.text = q

		q = "Hadjáratok: "
		for (i=0; i<p.campaigns.length; i++) {
			o = p.campaigns[i]
			o.label = o.name
			q += " "+o.name
		}
		chapterCampaigns.text = q


		if (p.introId !== -1) {
			list.model.append({
								  id: -2,
								  introId: p.introId,
								  labelTitle: qsTr(" -- Bevezető --")
							  })
		} else {
			list.model.append({
								  id: -2,
								  introId: -1,
								  labelTitle: qsTr("-- Bevezető hozzáadása --")
							  })
		}


		/*for (i=0; i<p.chapters.length; i++) {
			var o3 = p.chapters[i]
			o3.labelTitle = o3.name
			listChapters.model.append(o3)
		}*/


	}
}
