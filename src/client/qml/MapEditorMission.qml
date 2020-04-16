import QtQuick 2.12
import QtQuick.Controls 2.14
import COS.Client 1.0
import QtQuick.Layouts 1.12
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

	rightLoader.sourceComponent: QCloseButton {
		onClicked: if (view) {
					   view.model.remove(modelIndex)
				   }
	}

	QAccordion {
		id: accordion

		anchors.fill: parent


		QCollapsible {
			id: header

			title: qsTr("Általános")
			visible: !isSummary

			Column {
				id: col
				width: parent.width

				QTextField {
					id: missionName
					width: parent.width

					placeholderText: qsTr("Küldetés neve")

					onEditingFinished: header.missionUpdate()
				}

			}

			function missionUpdate() {
				if (missionId != -1 && !isSummary)
					map.missionUpdate(missionId, {
										  "name": missionName.text
									  }, parentCampaignId)
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

				onClicked: loadDialogCampaigns()
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

							onEditingFinished: if (model.id !== -1 && acceptableInput)
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

							onClicked: if (isSummary)
										   map.summaryLevelRemove(model.id, missionId)
									   else
										   map.missionLevelRemove(model.id, missionId)

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
							if (isSummary)
								map.summaryLevelUpdate(model.id, missionId, {
														   "sec": JS.mmSStoSec(textTime.text),
														   "hp": spinHP.value
													   })
							else
								map.missionLevelUpdate(model.id, missionId, {
														   "sec": JS.mmSStoSec(textTime.text),
														   "hp": spinHP.value,
														   "mode": comboMode.currentValue
													   })
						}
					}

					function missionLevelAdd() {
						if (missionId != -1) {
							if (isSummary)
								map.summaryLevelAdd({
														"summaryid": missionId,
														"level": model.level,
														"sec": JS.mmSStoSec(textTime.text),
														"hp": spinHP.value
													})
							else
								map.missionLevelAdd({
														"missionid": missionId,
														"level": model.level,
														"sec": JS.mmSStoSec(textTime.text),
														"hp": spinHP.value,
														"mode": comboMode.currentValue
													})
						}
					}
				}
			}

		}

		QCollapsible {
			title: qsTr("Bevezető")

			QButton {
				id: bIntro
				property int introId: -1;

				onClicked: if (introId !== -1)
							   pageEditor.introSelected(modelIndex, introId, missionId, isSummary ? Map.IntroSummary : Map.IntroMission)
						   else  {
							   var d = JS.dialogCreate(dlgIntroName)
							   d.item.mode = 1
							   d.open()
						   }

			}
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
							var i = map.chapterAdd({ "name": newChapterName.text })
							if (i !== -1 && isSummary)
								map.summaryChapterAdd({ "summaryid" : missionId, "chapterid": i })
							else if (i !== -1 && !isSummary)
								map.missionChapterAdd({ "missionid" : missionId, "chapterid": i })

							clear()
						}
					}
				}

				QListItemDelegate {
					id: listChapters

					width: parent.width

					modelTitleRole: "name"

					onClicked: {
						pageEditor.chapterSelected(modelIndex, listChapters.model[index].id, isSummary ? -1 : missionId, isSummary ? missionId : -1)
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
						} else if (event.key === Qt.Key_F4 && listChapters.currentIndex !== -1) {
						} else if (event.key === Qt.Key_Delete && listChapters.currentIndex !== -1) {
						}
					}
				}
			}
		}

		QCollapsible {
			title: qsTr("Kivezető")

			QButton {
				id: bOutro
				property int outroId: -1;

				onClicked: if (outroId !== -1)
							   pageEditor.introSelected(modelIndex, outroId, missionId, isSummary ? Map.IntroSummary : Map.IntroMission)
						   else  {
							   var d = JS.dialogCreate(dlgIntroName)
							   d.item.mode = 2
							   d.open()
						   }

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


	Component {
		id: dlgIntroName

		QDialogTextField {
			property int mode: 1

			title: switch (mode) {
				   case 1: qsTr("Új bevezető"); break
				   case 2: qsTr("Új kivezető"); break
				   }

			onDlgAccept: {
				var intId = map.introAdd({ "ttext": data })
				if (isSummary) {
					if (intId !== -1 && map.summaryIntroAdd(missionId, intId, (mode === 2)))
						pageEditor.introSelected(modelIndex, intId, missionId, Map.IntroSummary)

				} else {
					if (intId !== -1 && map.missionIntroAdd(missionId, intId, (mode === 2)))
						pageEditor.introSelected(modelIndex, intId, missionId, Map.IntroMission)

				}
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
				var camps = JS.getSelectedIndices(dlgList.model, "id")
				map.missionCampaignListSet(missionId, camps)
				get()
			}
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
		onIntroListUpdated: if ((type===Map.IntroMission || type===Map.IntroSummary) && parentId===missionId) get()
		onChapterListUpdated: if ((!isSummary && mId===missionId) || (isSummary && sId===missionId))
								  get()
	}

	Component.onCompleted: get()

	onMissionIdChanged: get()

	function get() {
		if (missionId == -1 || !map) {
			listChapters.model = []
			listLevels.model.clear()
			missionName.text = ""
			return
		}

		listChapters.model = []
		listLevels.model.clear()

		var p

		if (isSummary) {
			p = map.summaryGet(missionId)
		} else {
			p = map.missionGet(missionId)
			missionName.text = p.name
		}

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



		listChapters.model = p.chapters
	}


	function loadDialogCampaigns() {
		var d = JS.dialogCreate(dlgCampaigns)
		var ml = map.execSelectQuery("SELECT campaign.id as id, campaign.name as name,
						  CASE WHEN missionid IS NOT NULL THEN true ELSE false END as selected
						  FROM campaign LEFT JOIN bindCampaignMission ON (bindCampaignMission.campaignid=campaign.id AND bindCampaignMission.missionid=?)
						  ORDER BY selected DESC, campaign.name", [missionId])
		JS.setModel(d.item.model, ml)
		d.open()
	}
}
