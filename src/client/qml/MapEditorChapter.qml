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

	QAccordion {
		anchors.fill: parent

		QCollapsible {
			title: qsTr("Általános")

			Column {
				width: parent.width

				QTextField {
					id: chapterName
					width: parent.width

					onEditingFinished: if (chapterId != -1) map.chapterUpdate(chapterId, { "name": chapterName.text }, parentMissionId, parentSummaryId)
				}

				QButton {
					label: qsTr("Célpont törlése")

					icon: "M\ue5cd"
					bgColor: CosStyle.colorErrorDarker
					borderColor: CosStyle.colorErrorDark

					onClicked: {
						var d = JS.dialogCreateQml("YesNo")
						d.item.title = qsTr("Biztosan törlöd a célpontot?")
						d.item.text = chapterName.text
						d.accepted.connect(function () {
							if (map.chapterRemove(chapterId)) {
								chapterId = -1
								if (view) {
									view.model.remove(modelIndex)
								}
							}
						})
						d.open()
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
			title: qsTr("Fegyverek")

			QListItemDelegate {
				id: list
				width: parent.width

				modelTitleRole: "name"

			}

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
			list.model = []
			chapterName.text = ""
			return
		}

		list.model = []

		var p = map.chapterGet(chapterId)
		chapterName.text = p["name"]


		chapterMissions.tags = p.missions
		chapterCampaigns.tags = p.campaigns

		mIntro.introId = p.introId

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
			map.chapterMissionListSet(chapterId, missions)
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
			map.chapterSummaryListSet(chapterId, camps)
			get()
		})
		d.open()
	}
}
