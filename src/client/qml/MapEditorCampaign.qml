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


	Label {
		id: noLabel
		opacity: campaignId == -1
		visible: opacity != 0

		text: qsTr("Válassz hadjáratot")

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}

	QAccordion {
		anchors.fill: parent

		opacity: campaignId != -1
		visible: opacity != 0

		Behavior on opacity { NumberAnimation { duration: 125 } }

		QCollapsible {
			title: qsTr("Általános")

			Column {
				width: parent.width

				QTextField {
					id: campaignName
					width: parent.width

					onEditingFinished: if (campaignId != -1) map.campaignUpdate(campaignId, { "name": campaignName.text })
				}

				QButton {
					label: qsTr("Hadjárat törlése")

					icon: "M\ue5cd"
					bgColor: CosStyle.colorErrorDarker
					borderColor: CosStyle.colorErrorDark

					onClicked: {
						var d = JS.dialogCreateQml("YesNo")
						d.item.title = qsTr("Biztosan törlöd a hadjáratot?")
						d.item.text = campaignName.text
						d.accepted.connect(function () {
							if (map.campaignRemove(campaignId)) {
								campaignId = -1
							}
						})
						d.open()
					}
				}
			}
		}

		QCollapsible {
			title: qsTr("Kapcsolatok")

			QTag {
				id: locks
				title: qsTr("Zárolja:")
				width: parent.width
				defaultColor: CosStyle.colorAccentLight
				defaultBackground: CosStyle.colorAccentDark
				modelTextRole: "name"

				onClicked: loadDialogCampaigns()
			}
		}



		MapEditorIntroWidget {
			id: mIntro

			map: panel.map
			isOutro: false
			parentId: campaignId
			parentType: Map.IntroCampaign
		}



		QCollapsible {
			title: qsTr("Küldetések")


			QListItemDelegate {
				id: list
				width: parent.width

				modelTitleRole: "name"

				onClicked: {
					var o = list.model[index]
					if (o.id >= 0)
						pageEditor.missionSelected(o.id, campaignId)
					else if (o.id === -1) {
						var d = JS.dialogCreateQml("TextField")
						d.item.title = qsTr("Új küldetés neve")

						d.accepted.connect(function(data) {
							var misId = map.missionAdd({ "name": data })
							if (misId !== -1) {
								map.missionLevelAdd({
														missionid: misId,
														level: 1,
														sec: 600,
														hp: 5,
														mode: 0
													})
								if (map.campaignMissionAdd(campaignId, misId))
									pageEditor.missionSelected(misId, campaignId)
							}
						})
						d.open()
					} else if (o.id === -2) {
						if (o.summaryId !== -1)
							pageEditor.summarySelected(o.summaryId, campaignId)
						else  {
							var sumId = map.campaignSummaryAdd(campaignId)
							if (sumId !== -1) {
								map.summaryLevelAdd({
														summaryid: sumId,
														level: 1,
														sec: 600,
														hp: 5,
													})
								pageEditor.summarySelected(sumId, campaignId)
							}
						}
					}
				}
			}

		}



		MapEditorIntroWidget {
			id: mOutro

			map: panel.map
			isOutro: true
			parentId: campaignId
			parentType: Map.IntroCampaign
		}

	}







	Connections {
		target: pageEditor
		onCampaignSelected: {
			campaignId = id
		}
	}


	Connections {
		target: map
		onCampaignUpdated: if (id===campaignId) get()
		onMissionListUpdated: if (id===campaignId || id===-1) get()
		onIntroListUpdated: if ((type===Map.IntroCampaign && parentId===campaignId) || parentId === -1) get()
	}

	Component.onCompleted: get()

	onCampaignIdChanged: get()

	function get() {
		if (campaignId == -1 || map == null) {
			list.model=[]
			campaignName.text = ""
			return
		}

		list.model=[]

		var p = map.campaignGet(campaignId)
		campaignName.text = p["name"]


		locks.tags = p.locks

		mIntro.introId = p.introId
		mOutro.introId = p.outroId

		var l = map.missionListGet(campaignId)

		l.push({
				   id: -1,
				   name: qsTr("-- küldetés hozzáadása --"),
				   num: list.model.length+1
			   })

		l.push({
				   id: -2,
				   name: p.summaryId === -1 ? qsTr("-- összegzés hozzáadása --") : qsTr("ÖSSZEGZÉS"),
				   num: list.model.length+2,
				   summaryId: p.summaryId
			   })


		list.model = l
	}


	function loadDialogCampaigns() {
		var d = JS.dialogCreateQml("List")
		d.item.title = qsTr("Hadjáratok")
		d.item.newField.visible = false
		d.item.list.selectorSet = true
		d.item.list.modelTitleRole = "name"
		d.item.list.modelSelectedRole = "selected"

		var ml = map.execSelectQuery("SELECT campaign.id as id, campaign.name as name,
										CASE WHEN lockId IS NOT NULL THEN true ELSE false END as selected
										FROM campaign
										LEFT JOIN campaignLock ON (campaignLock.lockId=campaign.id AND campaignLock.campaignId=?)
										WHERE campaign.id<>?
										ORDER BY selected DESC, campaign.name ", [campaignId, campaignId])
		JS.setModel(d.item.model, ml)

		d.accepted.connect(function() {
			var camps = JS.getSelectedIndices(d.item.model, "id")
			map.campaignLockSet(campaignId, camps)
			get()
		})
		d.open()
	}
}
