import QtQuick 2.12
import QtQuick.Controls 2.14
import COS.Client 1.0
import QtQuick.Layouts 1.14
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Map map: null
	property int missionId: -1
	property bool isSummary: false
	property int parentCampaignId: -1

	title: isSummary ? qsTr("Összegző küldetés") : qsTr("Küldetés")

	Label {
		id: noLabel
		opacity: missionId == -1
		visible: opacity != 0

		text: qsTr("Válassz küldetést")

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}


	QAccordion {
		id: accordion

		anchors.fill: parent
		opacity: missionId != -1
		visible: opacity != 0

		Behavior on opacity { NumberAnimation { duration: 125 } }


		QCollapsible {
			id: header

			title: qsTr("Általános")

			Column {
				id: col
				width: parent.width

				QTextField {
					id: missionName
					width: parent.width

					visible: !isSummary

					placeholderText: qsTr("Küldetés neve")

					onTextModified: header.missionUpdate()
				}

				QButton {
					text: isSummary ? qsTr("Összegzés törlése") : qsTr("Küldetés törlése")

					icon.source: CosStyle.iconDelete
					backgroundColor: CosStyle.colorErrorDarker
					borderColor: CosStyle.colorErrorDark

					onClicked: {
						var d = JS.dialogCreateQml("YesNo")
						d.item.title = isSummary ? qsTr("Biztosan törlöd az összegzést?") : qsTr("Biztosan törlöd a küldetést?")
						d.item.text = isSummary ? "" : missionName.text
						d.accepted.connect(function () {
							map.undoLogBegin(qsTr("Összegzés/küldetés törlése"))
							if ((isSummary && map.summaryRemove(missionId)) || (!isSummary && map.missionRemove(missionId))) {
								missionId = -1
							}
							map.undoLogEnd()
						})

						d.open()
					}
				}

			}

			function missionUpdate() {
				if (missionId != -1 && !isSummary) {
					map.undoLogBegin(qsTr("Összegzés/küldetés módosítása"))
					map.missionUpdate(missionId, {
										  "name": missionName.text
									  }, parentCampaignId)
					map.undoLogEnd()
				}
			}
		}

		QCollapsible {
			title: qsTr("Kapcsolatok")

			QTag {
				id: missionCampaigns
				title: qsTr("Hadjáratok:")
				width: parent.width
				defaultColor: CosStyle.colorAccentLight
				defaultBackground: CosStyle.colorAccentDark
				modelTextRole: "name"

				readOnly: isSummary

				onClicked: if (!isSummary) loadDialogCampaigns()
			}
		}



		QCollapsible {
			id: levels

			title: qsTr("Szintek")

			QListView {
				id: listLevels

				width: parent.width

				model: ListModel {}

				delegate: Rectangle {
					id: delegateitem
					width: listLevels.width
					height: row.height+6

					color: "transparent"

					RowLayout {
						id: row

						width: parent.width

						anchors.verticalCenter: parent.verticalCenter

						Label {
							id: labelLevel
							Layout.fillHeight: true
							Layout.fillWidth: false
							verticalAlignment: Text.AlignVCenter
							horizontalAlignment: Text.AlignHCenter
							text: model.level
							Layout.leftMargin: 10
							Layout.rightMargin: 10
						}

						Label {
							text: model.id === -1 ? qsTr("(Új szint hozzáadása) Idő:") : qsTr("Idő:")
							Layout.fillHeight: true
							Layout.fillWidth: false
							verticalAlignment: Text.AlignVCenter
							horizontalAlignment: Text.AlignHCenter
							Layout.leftMargin: 10
							Layout.rightMargin: 5
						}

						QTextField {
							id: textTime
							Layout.fillHeight: true
							Layout.fillWidth: false
							placeholderText: qsTr("MM:SS")
							text: JS.secToMMSS(model.sec)

							validator: RegExpValidator { regExp: /\d\d:\d\d/ }

							onTextModified: if (model.id !== -1 && acceptableInput)
												   delegateitem.missionLevelUpdate()

							onAccepted: model.id === -1 ?
											delegateitem.missionLevelAdd() :
											delegateitem.missionLevelUpdate()
						}

						Label {
							visible: spinHP.visible
							text: qsTr("HP:")
							Layout.fillHeight: true
							Layout.fillWidth: false
							verticalAlignment: Text.AlignVCenter
							horizontalAlignment: Text.AlignHCenter
							Layout.leftMargin: 10
							Layout.rightMargin: 5
						}

						QSpinBox {
							id: spinHP
							visible: model.id !== -1 && textTime.acceptableInput
							Layout.fillHeight: true
							Layout.fillWidth: false
							ToolTip.text: qsTr("HP")
							from: 1
							to: 99

							value: model.hp

							onValueModified: delegateitem.missionLevelUpdate()
						}

						QComboBox {
							id: comboMode

							visible: !isSummary && spinHP.visible

							ToolTip.text: qsTr("Célpontok elrendezése")

							valueRole: "value"
							textRole: "text"

							model: [
								{ value: 0, text: qsTr("[]->[]") },				// Fejezetenként egyben és egymás után
								{ value: 1, text: qsTr("[]X") },				// Fejezetenként egyben összekeverve
								{ value: 2, text: qsTr("XX") }					// Teljesen összekeverve
							]

							onActivated: delegateitem.missionLevelUpdate()
						}

						Item { Layout.fillWidth: true }

						QRemoveButton {
							buttonVisible: model.canRemove

							onClicked: if (isSummary) {
										   map.undoLogBegin(qsTr("Összegzés szint törlése"))
										   map.summaryLevelRemove(model.id, missionId)
										   map.undoLogEnd()
									   } else {
										   map.undoLogBegin(qsTr("Küldetés szint törlése"))
										   map.missionLevelRemove(model.id, missionId)
										   map.undoLogEnd()
									   }

							Layout.fillHeight: true
							Layout.fillWidth: false
							Layout.leftMargin: 5
							Layout.rightMargin: 5
						}

						Component.onCompleted: {
							comboMode.currentIndex = comboMode.indexOfValue(model.mode)
						}
					}

					function missionLevelUpdate() {
						if (missionId != -1) {
							if (isSummary) {
								map.undoLogBegin(qsTr("Összegzés szint módosítása"))
								map.summaryLevelUpdate(model.id, missionId, {
														   "sec": JS.mmSStoSec(textTime.text),
														   "hp": spinHP.value
													   })
								map.undoLogEnd()
							} else {
								map.undoLogBegin(qsTr("Küldetés szint módosítása"))
								map.missionLevelUpdate(model.id, missionId, {
														   "sec": JS.mmSStoSec(textTime.text),
														   "hp": spinHP.value,
														   "mode": comboMode.currentValue
													   })
								map.undoLogEnd()
							}
						}
					}

					function missionLevelAdd() {
						if (missionId != -1) {
							if (isSummary) {
								map.undoLogBegin(qsTr("Összegzés szint hozzáadása"))
								map.summaryLevelAdd({
														"summaryid": missionId,
														"level": model.level,
														"sec": JS.mmSStoSec(textTime.text),
														"hp": spinHP.value
													})
								map.undoLogEnd()
							} else {
								map.undoLogBegin(qsTr("Küldetés szint hozzáadása"))
								map.missionLevelAdd({
														"missionid": missionId,
														"level": model.level,
														"sec": JS.mmSStoSec(textTime.text),
														"hp": spinHP.value,
														"mode": comboMode.currentValue
													})
								map.undoLogEnd()
							}
						}
					}
				}
			}

		}

		MapEditorIntroWidget {
			id: mIntro

			map: panel.map
			isOutro: false
			parentId: missionId
			parentType: isSummary ? Map.IntroSummary : Map.IntroMission
		}

		QCollapsible {
			title: "Célpontok"

			Column {
				width: parent.width


				QTextField {
					id: newChapterName
					width: parent.width

					placeholderText: qsTr("új célpont hozzáadása")
					onAccepted: {
						if (missionId !== -1) {
							map.undoLogBegin(qsTr("Célpont hozzáadása"))
							var i = map.chapterAdd({ "name": newChapterName.text })
							if (i !== -1 && isSummary)
								map.summaryChapterAdd({ "summaryid" : missionId, "chapterid": i })
							else if (i !== -1 && !isSummary)
								map.missionChapterAdd({ "missionid" : missionId, "chapterid": i })

							map.undoLogEnd()
							clear()
						}
					}
				}

				QListItemDelegate {
					id: listChapters

					width: parent.width

					modelTitleRole: "name"

					onClicked: pageEditor.chapterSelected(listChapters.model[index].id, isSummary ? -1 : missionId, isSummary ? missionId : -1)

				}
			}
		}

		MapEditorIntroWidget {
			id: mOutro

			map: panel.map
			isOutro: true
			parentId: missionId
			parentType: isSummary ? Map.IntroSummary : Map.IntroMission
		}

	}





	Connections {
		target: pageEditor
		onMissionSelected: {
			missionId = -1
			isSummary = false
			missionId = id
		}

		onSummarySelected: {
			missionId = -1
			isSummary = true
			missionId = id
		}
	}


	Connections {
		target: map
		onMissionUpdated: if (!isSummary && id===missionId) get()
		onSummaryUpdated: if (isSummary && id===missionId) get()
		onIntroListUpdated: if (parentId === -1 || ((type===Map.IntroMission || type===Map.IntroSummary) && parentId===missionId)) get()
		onChapterListUpdated: if (mId===-1 || (!isSummary && mId===missionId) || (isSummary && sId===missionId))
								  get()
		onUndone: get()
	}


	onMissionIdChanged: get()

	function populated() { }

	function get() {
		var p

		if (map) {
			if (isSummary) {
				p = map.summaryGet(missionId)
			} else {
				p = map.missionGet(missionId)
			}
			missionId = p.id
		} else
			missionId = -1

		if (missionId == -1) {
			listChapters.model = []
			listLevels.model.clear()
			missionName.text = ""
			return
		}

		listChapters.model = []
		listLevels.model.clear()

		missionName.text = isSummary ? "" : p.name
		missionCampaigns.tags = p.campaigns


		var levels = p.levels.length
		var levelNum = 0;

		for (var i=0; i<levels; i++) {
			var o2 = p.levels[i]
			o2.canRemove = (i===levels-1 && i>0) ? true : false

			if (o2.level>levelNum)
				levelNum=o2.level

			listLevels.model.append(o2)
		}

		listLevels.model.append({
									id: -1,
									level: levelNum+1,
									sec: 0,
									hp: 5,
									mode: 0,
									canRemove: false
								})


		mIntro.introId = p.introId
		mOutro.introId = p.outroId

		listChapters.model = p.chapters
	}


	function loadDialogCampaigns() {
		var d = JS.dialogCreateQml("List")
		d.item.title = qsTr("Hadjáratok")
		d.item.newField.visible = false
		d.item.list.selectorSet = true
		d.item.list.modelTitleRole = "name"
		d.item.list.modelSelectedRole = "selected"

		var ml = map.execSelectQuery("SELECT campaign.id as id, campaign.name as name,
						  CASE WHEN missionid IS NOT NULL THEN true ELSE false END as selected
						  FROM campaign LEFT JOIN bindCampaignMission ON (bindCampaignMission.campaignid=campaign.id AND bindCampaignMission.missionid=?)
						  ORDER BY selected DESC, campaign.name", [missionId])

		JS.setModel(d.item.model, ml)

		d.accepted.connect(function() {
			var camps = JS.getSelectedIndices(d.item.model, "id")
			map.undoLogBegin(qsTr("Hadjáratok hozzárendelése"))
			map.missionCampaignListSet(missionId, camps)
			map.undoLogEnd()
			get()
		})
		d.open()
	}
}
