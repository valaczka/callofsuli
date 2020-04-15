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

			QTextField {
				id: chapterName
				width: parent.width

				onEditingFinished: if (chapterId != -1) map.chapterUpdate(chapterId, { "name": chapterName.text }, parentMissionId, parentSummaryId)
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
					title: qsTr("Hadjáratok:")
					modelTextRole: "name"

					onClicked: loadDialogCampaigns()
				}
			}
		}

		QCollapsible {
			title: qsTr("Bevezető")

			QButton {
				id: bIntro
				property int introId: -1;

				onClicked: if (introId !== -1)
							   pageEditor.introSelected(modelIndex, introId, chapterId, Map.IntroChapter)
						   else  {
							   var d = JS.dialogCreate(dlgIntroName)
							   d.open()
						   }
			}
		}

		QCollapsible {
			title: qsTr("Fegyverek")

			QListItemDelegate {
				id: list
				width: parent.width

				modelTitleRole: "name"

				//			modelTitleSet: true
				//			modelSubtitleSet: true

				onClicked: {
					//var o = list.model.get(index)
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

		}
	}

	Component {
		id: dlgIntroName

		QDialogTextField {
			title: qsTr("Új bevezető")

			onDlgAccept: {
				var intId = map.introAdd({ "ttext": data })
				if (intId !== -1 && map.chapterIntroAdd(chapterId, intId))
					pageEditor.introSelected(modelIndex, intId, chapterId, Map.IntroChapter)

			}
		}
	}


	Component {
		id: dlgMissions

		QDialogList {
			id: dlgList
			title: qsTr("Küldetések")
			newField.visible: false
			list.selectorSet: true
			list.modelTitleRole: "name"
			list.modelSelectedRole: "selected"
			onDlgAccept: {
				var i
				var plus = JS.getSelectedIndices(dlgList.model, true, "id")
				for (i=0; i<plus.length; ++i) {
					map.missionChapterAdd({ "missionid" : plus[i], "chapterid": chapterId })
				}

				var minus = JS.getSelectedIndices(dlgList.model, false, "id")
				for (i=0; i<minus.length; ++i) {
					map.missionChapterRemove(minus[i], chapterId)
				}

				get()
			}
		}
	}


	Component {
		id: dlgCampaigns

		QDialogList {
			id: dlgList
			title: qsTr("Hadjáratok")
			newField.visible: false
			list.selectorSet: true
			list.modelTitleRole: "name"
			list.modelSelectedRole: "selected"

			onDlgAccept: {
				var i
				var plus = JS.getSelectedIndices(dlgList.model, true, "id")
				for (i=0; i<plus.length; ++i) {
					map.summaryChapterAdd({ "summaryid" : plus[i], "chapterid": chapterId })
				}

				var minus = JS.getSelectedIndices(dlgList.model, false, "id")
				for (i=0; i<minus.length; ++i) {
					map.summaryChapterRemove(minus[i], chapterId)
				}

				get()
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

		if (p.introId !== -1) {
			bIntro.introId = p.introId
			bIntro.label = p.introText
		} else {
			bIntro.introId = -1
			bIntro.label = qsTr("-- Bevezető hozzáadása --")
		}

	}


	function loadDialogMissions() {
		var d = JS.dialogCreate(dlgMissions)
		var ml = map.execSelectQuery("SELECT mission.id as id, mission.name as name,
						  CASE WHEN chapterid IS NOT NULL THEN true ELSE false END as selected
						  FROM mission LEFT JOIN bindMissionChapter ON (bindMissionChapter.missionid=mission.id AND bindMissionChapter.chapterid=?)
						  ORDER BY selected DESC, mission.name", [chapterId])
		JS.setModel(d.item.model, ml)
		d.open()
	}


	function loadDialogCampaigns() {
		var d = JS.dialogCreate(dlgCampaigns)
		d.item.model = map.execSelectQuery("SELECT summary.id as id, campaign.name as name,
									CASE WHEN chapterid IS NOT NULL THEN true ELSE false END as selected
									FROM summary
									LEFT JOIN campaign ON (campaign.id=summary.campaignid)
									LEFT JOIN bindSummaryChapter ON (bindSummaryChapter.summaryid=summary.id AND bindSummaryChapter.chapterid=?)
									ORDER BY selected DESC, campaign.name", [chapterId])
		d.open()
	}
}
