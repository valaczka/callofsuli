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

	title: qsTr("Célpont")

	Label {
		id: noLabel
		opacity: chapterId == -1
		visible: opacity != 0

		text: qsTr("Válassz célpontot")

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}


	QAccordion {
		anchors.fill: parent
		opacity: chapterId != -1
		visible: opacity != 0

		Behavior on opacity { NumberAnimation { duration: 125 } }

		QCollapsible {
			title: qsTr("Általános")

			Column {
				width: parent.width

				QTextField {
					id: chapterName
					width: parent.width
				}

				QButton {
					label: qsTr("Célpont törlése")

					icon: "M\ue5cd"
					bgColor: CosStyle.colorErrorDarker
					borderColor: CosStyle.colorErrorDark

					onClicked: {
						map.undoLogBegin(qsTr("Célpont törlése"))
						pageChapterEditor.deleteChapter(chapterName.text)
						map.undoLogEnd()
					}

				}
			}
		}

		QCollapsible {
			title: qsTr("Kapcsolatok")

			Column {
				width: parent.width

				QTag {
					id: chapterMissions
					title: qsTr("Küldetések:")
					width: parent.width
					defaultColor: CosStyle.colorAccentLight
					defaultBackground: CosStyle.colorAccentDark
					modelTextRole: "name"

					onClicked: loadDialogMissions()
				}

				QTag {
					width: parent.width
					id: chapterCampaigns
					title: qsTr("Összegzések:")
					modelTextRole: "name"

					onClicked: loadDialogCampaigns()
				}
			}
		}


		MapEditorIntroWidget {
			id: mIntro

			map: panel.map
			isOutro: false
			parentId: chapterId
			parentType: Map.IntroChapter
		}


		QCollapsible {
			title: qsTr("Lőszerek és fegyverek")

			QListItemDelegate {
				id: list
				width: parent.width

				isObjectModel: true

				modelTitleRole: "name"
				modelDepthRole: "depth"

				onClicked: {
					var o = list.model.get(index)
					if (o.sid >= 0 && o.oid === -1)
						pageChapterEditor.storageSelected(o.sid)
					else if (o.sid === -1 && o.oid >= 0) {
						pageChapterEditor.objectiveSelected(o.oid)
					} else if (o.sid >= 0 && o.oid === -2) {
						loadDialogObjectives(o.sid, o.module)
					} else if (o.sid === -2) {
						loadDialogStorages()
					}
				}
			}

		}
	}


	Connections {
		target: map
		onChapterUpdated: if (id===chapterId) get()
		onUndone: get()
	}

	Component.onCompleted: get()

	onChapterIdChanged: get()


	function get() {
		var p

		if (map) {
			p = map.chapterGet(chapterId)
	//		chapterId = p.id
		} else
			chapterId = -1

		if (chapterId == -1) {
			list.model.clear()
			chapterName.text = ""
			return
		}

		list.model.clear()

		chapterName.text = p["name"]

		chapterMissions.tags = p.missions
		chapterCampaigns.tags = p.campaigns

		mIntro.introId = p.introId

		for (var i=0; i<p.storages.length; ++i) {
			var s = p.storages[i]

			var t = map.storageModule(s.module)

			list.model.append({sid: s.id, oid: -1, name: (t ? t.label : qsTr("???")), module: s.module, depth: 0})

			for (var j=0; j<s.objectives.length; ++j) {
				var o = s.objectives[j]
				var tt = map.objectiveModule(o.module)
				list.model.append({sid: -1, oid: o.id, name: (tt ? tt.label : qsTr("???")), module: o.module, depth: 1})
			}

			list.model.append({sid: s.id, oid: -2, name: qsTr("-- fegyver hozzáadása --"), module: s.module, depth: 1})
		}

		list.model.append({sid: -2, oid: -1, name: qsTr("-- lőszer hozzáadása --"), module: "", depth: 0})
	}



	function loadDialogStorages() {
		var d = JS.dialogCreateQml("List")
		d.item.title = qsTr("Lőszerek")
		d.item.newField.visible = false
		d.item.simpleSelect = true
		d.item.list.modelTitleRole = "label"

		d.item.model = map.storageModules

		d.accepted.connect(function(idx) {
			var s = map.storageModules[idx]
			map.undoLogBegin(qsTr("Lőszer hozzáadása"))
			map.storageAdd({chapterid: chapterId, module: s.type, data: "{}"})
			map.undoLogEnd()
			get()
		})
		d.open()
	}


	function loadDialogObjectives(sId, sType) {
		var d = JS.dialogCreateQml("List")
		d.item.title = qsTr("Fegyverek")
		d.item.newField.visible = false
		d.item.simpleSelect = true
		d.item.list.modelTitleRole = "label"

		d.item.model = map.storageObjectiveModules(sType)

		d.accepted.connect(function(idx) {
			var s = map.objectiveModule(d.item.model[idx].type)
			map.undoLogBegin(qsTr("Fegyver hozzáadása"))
			map.objectiveAdd({storageid: sId, module: s.type, data: "{}" })
			map.undoLogEnd()
			get()
		})
		d.open()
	}


	function loadDialogMissions() {
		var d = JS.dialogCreateQml("List")
		d.item.title = qsTr("Küldetések")
		d.item.newField.visible = false
		d.item.list.selectorSet = true
		d.item.list.modelTitleRole = "name"
		d.item.list.modelSelectedRole = "selected"

		var ml = map.execSelectQuery("SELECT mission.id as id, mission.name as name,
						  CASE WHEN chapterid IS NOT NULL THEN true ELSE false END as selected
						  FROM mission LEFT JOIN bindMissionChapter ON (bindMissionChapter.missionid=mission.id AND bindMissionChapter.chapterid=?)
						  ORDER BY selected DESC, mission.name", [chapterId])
		JS.setModel(d.item.model, ml)

		d.accepted.connect(function() {
			var missions = JS.getSelectedIndices(d.item.model, "id")
			map.undoLogBegin(qsTr("Küldetések hozzárendelése"))
			map.chapterMissionListSet(chapterId, missions)
			map.undoLogEnd()
			get()
		})
		d.open()
	}


	function loadDialogCampaigns() {
		var d = JS.dialogCreateQml("List")
		d.item.title = qsTr("Hadjáratok")
		d.item.newField.visible = false
		d.item.list.selectorSet = true
		d.item.list.modelTitleRole = "name"
		d.item.list.modelSelectedRole = "selected"

		var ml = map.execSelectQuery("SELECT summary.id as id, campaign.name as name,
									CASE WHEN chapterid IS NOT NULL THEN true ELSE false END as selected
									FROM summary
									LEFT JOIN campaign ON (campaign.id=summary.campaignid)
									LEFT JOIN bindSummaryChapter ON (bindSummaryChapter.summaryid=summary.id AND bindSummaryChapter.chapterid=?)
									ORDER BY selected DESC, campaign.name", [chapterId])
		JS.setModel(d.item.model, ml)

		d.accepted.connect(function() {
			var camps = JS.getSelectedIndices(d.item.model, "id")
			map.undoLogBegin(qsTr("Hadjáratok hozzárendelése"))
			map.chapterSummaryListSet(chapterId, camps)
			map.undoLogEnd()
			get()
		})
		d.open()
	}
}
