import QtQuick 2.12
import QtQuick.Controls 2.14
import COS.Client 1.0
import QtQuick.Layouts 1.14
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	implicitWidth: 700

	property MapEditor map: null
	property int missionId: -1
	property bool isSummary: false

	title: isSummary ? qsTr("Összegző küldetés") : qsTr("Küldetés")

	QLabel {
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
					themeColors: CosStyle.buttonThemeDelete

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
									  })
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

				delegate: QRectangleBg {
					id: delegateitem
					width: listLevels.width
					height: row.height+6

					RowLayout {
						id: row

						width: parent.width

						anchors.verticalCenter: parent.verticalCenter

						QLabel {
							id: labelLevel
							Layout.fillHeight: true
							Layout.fillWidth: false
							verticalAlignment: Text.AlignVCenter
							horizontalAlignment: Text.AlignHCenter
							text: model.level
							Layout.leftMargin: 10
							Layout.rightMargin: 10
						}

						QLabel {
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

						QLabel {
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

						QFlipable {
							id: flipCorrect

							visible: !isSummary && spinHP.visible

							width: height
							height: comboMode.height

							frontIcon: CosStyle.iconUnchecked
							backIcon: CosStyle.iconChecked
							color: CosStyle.colorAccent
							flipped: model.showCorrect

							mouseArea.onClicked: {
								model.showCorrect = !model.showCorrect
								delegateitem.missionLevelUpdate()
							}
						}

						Item { Layout.fillWidth: true }

						QToolButton {
							visible: spinHP.visible
							ToolTip.text: qsTr("Lejátszás")
							icon.source: CosStyle.iconOK

							Layout.alignment: Qt.AlignVCenter
							Layout.fillWidth: false

							onClicked: {
								var o = JS.createPage("Game", {}, page)
								o.pagePopulated.connect(function() {
									o.game.map = panel.map
									o.game.missionId = panel.missionId
									o.game.isSummary = panel.isSummary
									o.game.level = model.level
									o.game.gamePlayMode = Game.GamePlayOffline
									o.game.prepare()
								})
							}
						}


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
														   "hp": spinHP.value,
														   "showCorrect": flipCorrect.flipped
													   })
								map.undoLogEnd()
							} else {
								map.undoLogBegin(qsTr("Küldetés szint módosítása"))
								map.missionLevelUpdate(model.id, missionId, {
														   "sec": JS.mmSStoSec(textTime.text),
														   "hp": spinHP.value,
														   "mode": comboMode.currentValue,
														   "showCorrect": flipCorrect.flipped
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
														"hp": spinHP.value,
														"showCorrect": flipCorrect.flipped
													})
								map.undoLogEnd()
							} else {
								map.undoLogBegin(qsTr("Küldetés szint hozzáadása"))
								map.missionLevelAdd({
														"missionid": missionId,
														"level": model.level,
														"sec": JS.mmSStoSec(textTime.text),
														"hp": spinHP.value,
														"mode": comboMode.currentValue,
														"showCorrect": flipCorrect.flipped
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
			title: qsTr("Célpontok")

			MapEditorStorageWidget {
				id: storageWidget

				width: parent.width
				map: panel.map

				missionId: isSummary ? -1 : panel.missionId
				summaryId: isSummary ? panel.missionId : -1

				onStorageSelected: pageEditor.storageSelected(id, isSummary ? -1 : missionId, isSummary ? missionId : -1)
				onObjectiveSelected: pageEditor.objectiveSelected(id, isSummary ? -1 : missionId, isSummary ? missionId : -1)
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
		onStorageListUpdated: if ((mId===-1 && sId===-1) || (!isSummary && mId===missionId) || (isSummary && sId===missionId))
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
			listLevels.model.clear()
			missionName.text = ""
			return
		}

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
									showCorrect: false,
									canRemove: false
								})


		mIntro.introId = p.introId
		mOutro.introId = p.outroId

		storageWidget.load(p.storages)
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
