import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPagePanel {
	id: panel

	enum EditType {
		Invalid,
		Campaign,
		Mission
	}

	property int editType: MapEditorCampaignEdit.Invalid
	property int campaignId: -1
	property string missionUuid: ""

	layoutFillWidth: true
	maximumWidth: 700


	title: qsTr("Adatok")



	icon: switch (editType) {
		  case MapEditorCampaignEdit.Campaign:
			  CosStyle.iconUserWhite
			  break
		  case MapEditorCampaignEdit.Mission:
			  CosStyle.iconUser
			  break
		  default:
			  CosStyle.iconUsers
		  }



	property var contextFuncCampaign: function (m) {
		m.addAction(actionRemove)
	}

	property var contextFuncMission: function (m) {
		m.addAction(actionRemove)
	}

	contextMenuFunc: switch (editType) {
					 case MapEditorCampaignEdit.Campaign:
						 contextFuncCampaign
						 break
					 case MapEditorCampaignEdit.Mission:
						 contextFuncMission
						 break
					 default:
						 null
					 }



	property VariantMapModel modelCampaignLock: mapEditor.newModel(["lock", "name"])
	property VariantMapModel modelMissionLock: mapEditor.newModel(["lock", "name", "level"])
	property VariantMapModel modelMissionLevel: mapEditor.newModel(["rowid", "level", "terrain", "startHP", "duration",
																	"startBlock", "imageFolder", "imageFile"])



	Connections {
		target: mapEditor

		function onCampaignSelected(id) {
			campaignId = id
		}


		function onCampaignLoaded(data) {
			if (!Object.keys(data).length) {
				editType = MapEditorCampaignEdit.Invalid
				campaignId = -1
				panel.title = ""
				return
			}

			if (data.id !== campaignId)
				return

			panel.title = data.name

			modelCampaignLock.clear()

			editType = MapEditorCampaignEdit.Campaign
			JS.setSqlFields([
								textCampaignName
							], data)

			modelCampaignLock.setVariantList(data.locks, "lock")

			campaignLocksCollapsible.collapsed = !data.locks.length
		}


		function onCampaignLockListLoaded(id, list) {
			if (!list.length) {
				cosClient.sendMessageWarning(qsTr("Zárolás hozzáadása"), qsTr("Nincs több hozzáadható zárolás!"))
				return
			}

			var model = mapEditor.newModel(["name", "id"])

			var d = JS.dialogCreateQml("List", {
										   roles: ["name", "id"],
										   icon: CosStyle.iconLockAdd,
										   title: qsTr("Zárolás hozzáadása"),
										   selectorSet: true,
										   sourceModel: model
									   })

			model.setVariantList(list, "id")

			d.accepted.connect(function(data) {
				mapEditor.run("campaignLockAdd", {"id": id, "list": model.getSelectedData("id") })
			})
			d.open()

		}


		function onCampaignRemoved() {
			reloadData()
		}


		function onCampaignModified(id) {
			if (id === campaignId)
				reloadData()
		}







		function onMissionSelected(uuid) {
			missionUuid = uuid
		}


		function onMissionLoaded(data) {
			if (!Object.keys(data).length) {
				editType = MapEditorCampaignEdit.Invalid
				missionUuid = ""
				panel.title = ""
				return
			}

			if (data.uuid !== missionUuid)
				return

			panel.title = data.name

			modelMissionLock.clear()
			modelMissionLevel.clear()

			editType = MapEditorCampaignEdit.Mission
			JS.setSqlFields([
								textMissionName,
								textMissionDescription,
								checkMandatory,
								comboCampaign
							], data)

			modelMissionLock.setVariantList(data.locks, "lock")

			var ll = data.levels
			ll.push({
						rowid: -1,
						level: ll.length+1,
						terrain: "",
						startHP: 0,
						duration: 0,
						startBlock: 0,
						imageFolder: "",
						imageFile: ""
					})

			modelMissionLevel.setVariantList(ll, "level")

			missionLocksCollapsible.collapsed = !data.locks.length

		}


		function onMissionRemoved() {
			reloadData()
		}

		function onMissionModified(uuid) {
			if (uuid === missionUuid)
				reloadData()
		}


		function onMissionLockLevelListLoaded(uuid, lock, locks) {
			if (uuid !== missionUuid || !locks.length)
				return

			var model2 = []

			for (var i=0; i<locks.length; i++) {
				var o=locks[i]
				if (o.level > 0)
					o.name = o.level+qsTr(". szint")
				else
					o.name = qsTr("Bármilyen szint")
				model2.push(o)
			}

			var model = mapEditor.newModel(["name", "level"])

			var d = JS.dialogCreateQml("List", {
										   roles: ["name", "level"],
										   icon: CosStyle.iconLockAdd,
										   title: qsTr("Zárolás hozzáadása"),
										   selectorSet: false,
										   sourceModel: model
									   })

			model.setVariantList(model2, "level")

			d.accepted.connect(function(data) {
				if (data > -1) {
					mapEditor.run("missionLockModify", {uuid: uuid, lock: lock, level: model.get(data).level })
				} else {
					console.warn("Invalid index", data)
				}
			})
			d.open()
		}



		function onMissionLockListLoaded(uuid, list) {
			if (!list.length) {
				cosClient.sendMessageWarning(qsTr("Zárolás hozzáadása"), qsTr("Nincs több hozzáadható zárolás!"))
				return
			}

			var model = mapEditor.newModel(["name", "uuid"])

			var d = JS.dialogCreateQml("List", {
										   roles: ["name", "uuid"],
										   icon: CosStyle.iconLockAdd,
										   title: qsTr("Zárolás hozzáadása"),
										   selectorSet: true,
										   sourceModel: model
									   })

			model.setVariantList(list, "uuid")

			d.accepted.connect(function(data) {
				mapEditor.run("missionLockAdd", {uuid: uuid, list: model.getSelectedData("uuid") })
			})
			d.open()
		}
	}

	Connections {
		target: mapEditor.db

		function onUndone() {
			reloadData()
		}
	}


	QLabel {
		id: noLabel
		opacity: editType === MapEditorCampaignEdit.Invalid
		visible: opacity != 0

		anchors.centerIn: parent

		text: qsTr("Válassz hadjáratot/küldetést")

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}

	QAccordion {
		id: accordionCampaign

		opacity: editType === MapEditorCampaignEdit.Campaign
		visible: opacity != 0

		Behavior on opacity { NumberAnimation { duration: 125 } }

		QCollapsible {
			title: qsTr("Általános")

			QGridLayout {
				width: parent.width

				watchModification: false

				//onAccepted: buttonSave.press()

				QGridLabel { field: textCampaignName }

				QGridTextField {
					id: textCampaignName
					fieldName: qsTr("Hadjárat neve")
					sqlField: "name"

					onTextModified: {
						mapEditor.run("campaignModify", {id: campaignId, data:{ name: text }})
					}

					validator: RegExpValidator { regExp: /.+/ }
				}

			}
		}


		QCollapsible {
			id: campaignLocksCollapsible

			title: qsTr("Zárolások")


			QVariantMapProxyView {
				id: locksView

				model: SortFilterProxyModel {
					id: locksProxyModel

					sourceModel: modelCampaignLock

					sorters: [
						StringSorter { roleName: "name" }
					]
				}

				autoSelectorChange: true

				delegateHeight: CosStyle.halfLineHeight

				modelTitleRole: "name"

				width: parent.width-locksCol.width
			}

			Column {
				id: locksCol
				anchors.left: locksView.right

				QToolButton {
					display: AbstractButton.IconOnly
					action: actionLockAdd
				}

				QToolButton {
					display: AbstractButton.IconOnly
					action: actionLockRemove
				}
			}

		}

		Item {
			height: CosStyle.pixelSize
			width: parent.width
		}

		QToolButtonBig {
			anchors.horizontalCenter: parent.horizontalCenter
			action: actionMissionNew
			color: CosStyle.colorOK
		}

	}









	QAccordion {
		id: accordionMission

		opacity: editType === MapEditorCampaignEdit.Mission
		visible: opacity != 0

		Behavior on opacity { NumberAnimation { duration: 125 } }

		QCollapsible {
			title: qsTr("Általános")

			QGridLayout {
				width: parent.width

				watchModification: false

				QGridLabel { field: textMissionName }

				QGridTextField {
					id: textMissionName
					fieldName: qsTr("Küldetés neve")
					sqlField: "name"

					onTextModified: {
						mapEditor.run("missionModify", {uuid: missionUuid, data:{ name: text }})
					}

					validator: RegExpValidator { regExp: /.+/ }
				}


				QGridLabel { text: qsTr("Hadjárat") }

				QGridComboBox {
					id: comboCampaign
					sqlField: "campaign"

					model: SortFilterProxyModel {
						sourceModel: mapEditor.modelCampaignList

						filters: [
							ValueFilter {
								roleName: "type"
								value: 0
							}
						]

						sorters: [
							RoleSorter { roleName: "noorphan"; priority: 1 },
							RoleSorter { roleName: "cname"; priority: 0 }
						]
					}

					textRole: "cname"
					valueRole: "cid"

					onActivated: {
						var cid = model.get(index).cid
						var c = null
						if (cid !== -1)
							c = cid
						mapEditor.run("missionModify", {uuid: missionUuid, data: { campaign: c }})
					}
				}

				QGridCheckBox {
					id: checkMandatory
					sqlField: "mandatory"
					text: qsTr("Kötelező elvégezni a hadjárat teljesítéséhez")

					onToggled: {
						mapEditor.run("missionModify", {uuid: missionUuid, data: { mandatory: checked }})
					}
				}

				QGridLabel { field: textMissionDescription }

				QGridTextArea {
					id: textMissionDescription
					fieldName: qsTr("Tájékoztató")
					sqlField: "description"

					onTextModified: {
						mapEditor.run("missionModify", {uuid: missionUuid, data:{ description: text }})
					}
				}
			}
		}


		QCollapsible {
			id: missionLocksCollapsible
			title: qsTr("Zárolások")

			QVariantMapProxyView {
				id: missionLocksView

				model: SortFilterProxyModel {
					id: missionLocksProxyModel

					sourceModel: modelMissionLock

					sorters: [
						StringSorter { roleName: "name" }
					]

					proxyRoles: [
						ExpressionRole {
							name: "fullname"
							expression: model.level ? model.name+" ("+model.level+". szint)" : model.name
						}
					]
				}

				autoSelectorChange: true

				delegateHeight: CosStyle.halfLineHeight

				modelTitleRole: "fullname"

				width: parent.width-missionLocksCol.width
			}

			Column {
				id: missionLocksCol
				anchors.left: missionLocksView.right

				QToolButton {
					display: AbstractButton.IconOnly
					action: actionMissionLockAdd
				}

				QToolButton {
					display: AbstractButton.IconOnly
					action: actionMissionLockRemove
				}

				QToolButton {
					display: AbstractButton.IconOnly
					action: actionMissionLockModify
				}
			}

		}




		QCollapsible {
			title: qsTr("Szintek")

			QVariantMapProxyView {
				id: missionLevelsView

				model: SortFilterProxyModel {
					id: missionLevelsProxyModel

					sourceModel: modelMissionLevel

					sorters: [
						RoleSorter { roleName: "level" }
					]

					proxyRoles: [
						ExpressionRole {
							name: "img"
							expression: model.rowid === -1 ? "" :
															 ( model.imageFolder.length && model.imageFile.length ?
																  "image://mapdb/"+model.imageFolder+"/"+model.imageFile :
																  "qrc:/internal/game/bg.png")

						},
						ExpressionRole {
							name: "fullname"
							expression: model.rowid === -1 ? qsTr("Új szint létrehozása") : model.level+qsTr(". szint")
						},
						SwitchRole {
							name: "textColor"
							filters: ValueFilter {
								roleName: "rowid"
								value: -1
								SwitchRole.value: CosStyle.colorOK
							}
							defaultValue: CosStyle.colorPrimary
						}
					]
				}

				autoSelectorChange: false

				modelTitleRole: "fullname"
				modelSubtitleRole: "terrain"

				modelTitleColorRole: "textColor"
				fontWeightTitle: Font.Medium

				delegateHeight: CosStyle.pixelSize*5

				leftComponent: Item {
					height: missionLevelsView.delegateHeight
					width: missionLevelsView.delegateHeight+10

					anchors.verticalCenter: parent.verticalCenter

					Image {
						width: parent.width-10
						height: parent.height*0.95

						fillMode: Image.PreserveAspectFit

						anchors.verticalCenter: parent.verticalCenter

						source: model && model.img ? model.img : ""

						QFontImage {
							anchors.centerIn: parent
							icon: "image://font/AcademicI/\uf125"
							size: CosStyle.pixelSize*1.5
							color: model ? model.textColor : CosStyle.colorPrimary
							visible: model && model.rowid === -1
						}
					}
				}

				rightComponent: QToolButton {
					display: AbstractButton.IconOnly
					icon.source: CosStyle.iconDelete
					visible: modelIndex === missionLevelsView.model.count-2

					ToolTip.text: qsTr("Utolsó szint törlése")

					color: CosStyle.colorErrorLighter
					onClicked: {
						mapEditor.run("missionLevelRemove", {uuid: missionUuid })
					}
				}

				width: parent.width

				onClicked: {
					var rowid = model.get(index).rowid

					if (rowid === -1) {
						var i = CosStyle.iconLockAdd
						var d = JS.dialogCreateQml("List", {
													   roles: ["details", "name"],
													   icon: i,
													   title: qsTr("Harcmező kiválasztása"),
													   selectorSet: false,
													   sourceModel: mapEditor.modelTerrains
												   })

						d.accepted.connect(function(data) {
							var p = d.item.sourceModel.get(data)
							mapEditor.run("missionLevelAdd", {mission: missionUuid, terrain: p.name, terrainData: p.data })
						})
						d.open()
					} else {
						mapEditor.levelSelected(rowid, missionUuid)
					}
				}

			}
		}



	}



	Action {
		id: actionMissionLockAdd
		text: qsTr("Hozzáadás")
		icon.source: CosStyle.iconAdd

		onTriggered: mapEditor.run("missionLockGetList", {uuid: missionUuid})
	}

	Action {
		id: actionMissionLockRemove
		text: qsTr("Eltávolítás")
		icon.source: CosStyle.iconRemove
		enabled: !mapEditor.isBusy && (missionLocksView.currentIndex !== -1 || modelMissionLock.selectedCount)
		onTriggered: {
			var o = missionLocksView.model.get(missionLocksView.currentIndex)
			var more = modelMissionLock.selectedCount

			if (more > 0)
				mapEditor.run("missionLockRemove", {uuid: missionUuid, list: modelMissionLock.getSelectedData("lock") })
			else
				mapEditor.run("missionLockRemove", {uuid: missionUuid, lock: o.lock})
		}
	}



	Action {
		id: actionMissionLockModify
		text: qsTr("Szint")
		icon.source: "image://font/School/\uf179"
		enabled: !mapEditor.isBusy && missionLocksView.currentIndex !== -1 && !missionLocksView.selectorSet

		onTriggered: {
			var l = missionLocksView.model.get(missionLocksView.currentIndex)
			mapEditor.run("missionLockGetLevelList", {
							  uuid: missionUuid,
							  lock: l.lock,
							  level: l.level
						  })
		}
	}













	Action {
		id: actionLockAdd
		text: qsTr("Hozzáadás")
		icon.source: CosStyle.iconAdd

		onTriggered: mapEditor.run("campaignLockGetList", {"id": campaignId})
	}

	Action {
		id: actionLockRemove
		text: qsTr("Eltávolítás")
		icon.source: CosStyle.iconRemove
		enabled: !mapEditor.isBusy && (locksView.currentIndex !== -1 || modelCampaignLock.selectedCount)
		onTriggered: {
			var o = locksView.model.get(locksView.currentIndex)
			var more = modelCampaignLock.selectedCount

			if (more > 0)
				mapEditor.run("campaignLockRemove", {"id": campaignId, "list": modelCampaignLock.getSelectedData("lock") })
			else
				mapEditor.run("campaignLockRemove", {"id": campaignId, "lock": o.lock})
		}
	}





	Action {
		id: actionRemove
		text: qsTr("Törlés")
		icon.source: CosStyle.iconDelete
		enabled: !mapEditor.isBusy && editType !== MapEditorCampaignEdit.Invalid
		onTriggered: {
			if (editType === MapEditorCampaignEdit.Campaign) {
				mapEditor.run("campaignRemove", {"id": campaignId})
			} else if (editType === MapEditorCampaignEdit.Mission) {
				mapEditor.run("missionRemove", {"uuid": missionUuid})
			}

			d.open()
		}
	}



	Action {
		id: actionMissionNew
		text: qsTr("Új küldetés")
		icon.source: CosStyle.iconAdd
		enabled: !mapEditor.isBusy && editType === MapEditorCampaignEdit.Campaign
		onTriggered: {
			var d = JS.dialogCreateQml("TextField")
			d.item.title = qsTr("Új küldetés neve")

			d.accepted.connect(function(data) {
				mapEditor.run("missionAdd", {"name": data, "campaign": campaignId})
			})
			d.open()
		}
	}




	function reloadData() {
		if (editType === MapEditorCampaignEdit.Campaign)
			mapEditor.run("campaignLoad", {id: campaignId})
		else if (editType === MapEditorCampaignEdit.Mission)
			mapEditor.run("missionLoad", {uuid: missionUuid})
	}


	Component.onCompleted: {
		if (editType === MapEditorCampaignEdit.Invalid && mapEditor.selectedMissionUUID !== "") {
			mapEditor.missionSelected(mapEditor.selectedMissionUUID)
			mapEditor.run("missionLoad", {uuid: mapEditor.selectedMissionUUID})
			mapEditor.selectedMissionUUID = ""
		}
	}
}

