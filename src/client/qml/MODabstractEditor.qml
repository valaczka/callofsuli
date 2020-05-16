import QtQuick 2.12
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QAccordion {
	id: control

	property var editorData: null

	QCollapsible {
		title: qsTr("Általános")

		Column {
			width: parent.width

			QTextField {
				id: storageName
				width: parent.width

				visible: panelObjective.storageId !== -1

				onTextModified: {
					map.undoLogBegin(qsTr("Célpont módosítása"))
					map.storageUpdate(storageId, {
										  "name": storageName.text
									  }, {}, panelObjective.parentMissionId, panelObjective.parentSummaryId)
					map.undoLogEnd()
				}
			}

			Label {
				id: labelStorageName

				visible: panelObjective.objectiveId !== -1

				width: parent.width
			}

			Label {
				id: labelModule

				width: parent.width
			}

			QButton {
				id: buttonDelete
				text: panelObjective.objectiveId !== -1 ? qsTr("Fegyver törlése") : qsTr("Célpont törlése")

				icon.source: CosStyle.iconDelete
				themeColors: CosStyle.buttonThemeDelete

				onClicked: {
					var d = JS.dialogCreateQml("YesNo")
					d.item.title = panelObjective.objectiveId !== -1 ? qsTr("Biztosan törlöd a fegyvert?") : qsTr("Biztosan törlöd a célpontot?")
					d.item.text = (panelObjective.storageId !== -1 ? storageName.text : labelStorageName.text)+" - "+labelModule.text
					d.accepted.connect(function () {
						if (panelObjective.objectiveId !== -1) {
							var i = panelObjective.objectiveId
							panelObjective.objectiveId = -1
							map.undoLogBegin(qsTr("Fegyver törlése"))
							map.objectiveRemove(i)
							map.undoLogEnd()
						} else if (panelObjective.storageId !== -1) {
							i = panelObjective.storageId
							panelObjective.storageId = -1
							map.undoLogBegin(qsTr("Célpont törlése"))
							map.storageRemove(i)
							map.undoLogEnd()
						}
					})
					d.open()
				}
			}
		}
	}

	QCollapsible {
		title: qsTr("Kapcsolatok")

		visible: panelObjective.storageId !== -1

		Column {
			width: parent.width

			QTag {
				id: storageMissions
				title: qsTr("Küldetések:")
				width: parent.width
				defaultColor: CosStyle.colorAccentLight
				defaultBackground: CosStyle.colorAccentDark
				modelTextRole: "name"

				onClicked: loadDialogMissions()
			}

			QTag {
				width: parent.width
				id: storageCampaigns
				title: qsTr("Összegzések:")
				modelTextRole: "name"

				onClicked: loadDialogCampaigns()
			}
		}
	}




	QCollapsible {
		title: qsTr("Fegyver beállításai")
		visible: panelObjective.objectiveId !== -1

		Row {
			Label {
				anchors.verticalCenter: parent.verticalCenter
				text: qsTr("Szint:")
			}

			QSpinBox {
				id: spinLevel
				anchors.verticalCenter: parent.verticalCenter
				ToolTip.text: qsTr("Szint")
				from: 1
				to: 99

				onValueModified: saveJson()
			}
		}
	}


	onEditorDataChanged: {
		if (panelObjective.storageId !== -1) {
			storageName.text = editorData.name
			storageMissions.tags = editorData.missions
			storageCampaigns.tags = editorData.campaigns

			labelModule.text = map.storageInfo(editorData.module).label
		} else if (panelObjective.objectiveId !== -1) {
			labelStorageName.text = editorData.storageName
			labelModule.text = map.objectiveInfo(editorData.module).label
			spinLevel.value = editorData.level
		}


	}


	function loadDialogMissions() {
		var d = JS.dialogCreateQml("List")
		d.item.title = qsTr("Küldetések")
		d.item.newField.visible = false
		d.item.list.selectorSet = true
		d.item.list.modelTitleRole = "name"
		d.item.list.modelSelectedRole = "selected"

		var ml = map.execSelectQuery("SELECT mission.id as id, mission.name as name,
							  CASE WHEN storageid IS NOT NULL THEN true ELSE false END as selected
							  FROM mission LEFT JOIN bindMissionStorage ON (bindMissionStorage.missionid=mission.id AND bindMissionStorage.storageid=?)
							  ORDER BY selected DESC, mission.name", [panelObjective.storageId])
		JS.setModel(d.item.model, ml)

		d.accepted.connect(function() {
			var missions = JS.getSelectedIndices(d.item.model, "id")
			map.undoLogBegin(qsTr("Küldetések hozzárendelése"))
			map.storageMissionListSet(panelObjective.storageId, missions)
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
										CASE WHEN storageid IS NOT NULL THEN true ELSE false END as selected
										FROM summary
										LEFT JOIN campaign ON (campaign.id=summary.campaignid)
										LEFT JOIN bindSummaryStorage ON (bindSummaryStorage.summaryid=summary.id AND bindSummaryStorage.storageid=?)
										ORDER BY selected DESC, campaign.name", [panelObjective.storageId])
		JS.setModel(d.item.model, ml)

		d.accepted.connect(function() {
			var camps = JS.getSelectedIndices(d.item.model, "id")
			map.undoLogBegin(qsTr("Hadjáratok hozzárendelése"))
			map.storageSummaryListSet(panelObjective.storageId, camps)
			map.undoLogEnd()
			get()
		})
		d.open()
	}

	function saveJson() {
		if (panelObjective.storageId !== -1) {
			map.undoLogBegin(qsTr("Célpont módosítása"))
			map.storageUpdate(panelObjective.storageId, {}, saveJsonData(), panelObjective.parentMissionId, panelObjective.parentSummaryId)
			map.undoLogEnd()
		} else if (panelObjective.objectiveId !== -1) {
			map.undoLogBegin(qsTr("Fegyver módosítása"))
			map.objectiveUpdate(panelObjective.objectiveId, {level: spinLevel.value}, saveJsonData(), panelObjective.parentMissionId, panelObjective.parentSummaryId)
			map.undoLogEnd()
		}
	}
}
