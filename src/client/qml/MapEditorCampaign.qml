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

	rightLoader.sourceComponent: QCloseButton {
		onClicked: if (view) {
					   view.model.remove(modelIndex)
				   }
	}


	QAccordion {
		anchors.fill: parent

		QCollapsible {
			title: qsTr("Általános")

			QTextField {
				id: campaignName
				width: parent.width

				onEditingFinished: if (campaignId != -1) map.campaignUpdate(campaignId, { "name": campaignName.text })
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
				readOnly: true
				modelTextRole: "name"
			}
		}

		QCollapsible {
			title: qsTr("Bevezető")

			QButton {
				id: bIntro
				property int introId: -1;

				onClicked: if (introId !== -1)
							   pageEditor.introSelected(modelIndex, introId, campaignId, Map.IntroCampaign)
						   else  {
							   var d = JS.dialogCreate(dlgMissionName)
							   d.item.mode = 1
							   d.open()
						   }

			}
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
						pageEditor.missionSelected(modelIndex, o.id, campaignId)
					else if (o.id === -1) {
						var d = JS.dialogCreate(dlgMissionName)
						d.item.mode = 0
						d.open()
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


		QCollapsible {
			title: qsTr("Összegzés")

			QButton {
				id: bSummary
				property int summaryId: -1;

				onClicked: if (summaryId !== -1)
							   pageEditor.summarySelected(modelIndex, summaryId, campaignId)
						   else  {
							   var sumId = map.campaignSummaryAdd(campaignId)
							   if (sumId !== -1)
								   pageEditor.summarySelected(modelIndex, sumId, campaignId)
						   }

			}
		}


		QCollapsible {
			title: qsTr("Kivezető")

			QButton {
				id: bOutro
				property int outroId: -1;

				onClicked: if (outroId !== -1)
							   pageEditor.introSelected(modelIndex, outroId, campaignId, Map.IntroCampaign)
						   else  {
							   var d = JS.dialogCreate(dlgMissionName)
							   d.item.mode = 2
							   d.open()
						   }

			}
		}
	}



	Component {
		id: dlgMissionName

		QDialogTextField {
			id: dlgYesNo

			property int mode: 0

			title: switch (mode) {
				   case 0: qsTr("Új küldetés neve"); break
				   case 1: qsTr("Új bevezető"); break
				   case 2: qsTr("Új kivezető"); break
				   }

			onDlgAccept: {
				if (mode == 0) {
					var misId = map.missionAdd({ "name": data })
					if (misId !== -1) {
						if (map.campaignMissionAdd(campaignId, misId))
							pageEditor.missionSelected(modelIndex, misId, campaignId)
					}
				} else if (mode == 1 || mode == 2) {
					var intId = map.introAdd({ "ttext": data })
					if (intId !== -1) {
						if (map.campaignIntroAdd(campaignId, intId, (mode === 2)))
							pageEditor.introSelected(modelIndex, intId, campaignId, Map.IntroCampaign)
					}
				}
			}
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
		onMissionListUpdated: if (id===campaignId) get()
		onIntroListUpdated: if (type===Map.IntroCampaign && parentId===campaignId) get()
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

		if (p.introId !== -1) {
			bIntro.introId = p.introId
			bIntro.label = p.introText
		} else {
			bIntro.introId = -1
			bIntro.label = qsTr("-- Intro hozzáadása --")
		}


		if (p.outroId !== -1) {
			bOutro.outroId = p.outroId
			bOutro.label = p.outroText
		} else {
			bOutro.outroId = -1
			bOutro.label = qsTr("-- Outro hozzáadása --")
		}


		var l = map.missionListGet(campaignId)

		l.push({
							  id: -1,
							  name: qsTr("-- küldetés hozzáadása --"),
							  num: model.length+1,
						  })

		list.model = l


		if (p.summaryId !== -1) {
			bSummary.summaryId = p.summaryId
			bSummary.label = qsTr("Összegzés")
		} else {
			bSummary.summaryId = -1
			bSummary.label = qsTr("-- Összegzés hozzáadása --")
		}

	}
}
