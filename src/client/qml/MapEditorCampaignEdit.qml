import QtQuick 2.12
import QtQuick.Controls 2.12
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


	title: switch (editType) {
		   case MapEditorCampaignEdit.Campaign:
			   qsTr("Hadjárat")
			   break
		   case MapEditorCampaignEdit.Mission:
			   qsTr("Küldetés")
			   break
		   default:
			   ""
		   }



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
				return
			}

			if (data.id !== campaignId)
				return

			modelCampaignLock.clear()

			editType = MapEditorCampaignEdit.Campaign
			JS.setSqlFields([
								textCampaignName
							], data)

			modelCampaignLock.setVariantList(data.locks, "lock")

			campaignLocksCollapsible.collapsed = !data.locks.length

			if (swipeMode)
				parent.parentPage.swipeToPage(1)
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
				return
			}

			if (data.uuid !== missionUuid)
				return

			modelMissionLock.clear()
			modelMissionLevel.clear()

			editType = MapEditorCampaignEdit.Mission
			JS.setSqlFields([
								textMissionName,
								checkMandatory,
								comboCampaign
							], data)

			modelMissionLock.setVariantList(data.locks, "lock")
			modelMissionLevel.setVariantList(data.levels, "level")

			missionLocksCollapsible.collapsed = !data.locks.length


			if (swipeMode)
				parent.parentPage.swipeToPage(1)
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
		anchors.fill: parent

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
		}

	}









	QAccordion {
		id: accordionMission
		anchors.fill: parent

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

			QToolButtonBig {
				anchors.horizontalCenter: parent.horizontalCenter
				action: actionMissionLevelAdd
				visible: !modelMissionLevel.count
			}

			QVariantMapProxyView {
				id: missionLevelsView

				visible: modelMissionLevel.count

				model: SortFilterProxyModel {
					id: missionLevelsProxyModel

					sourceModel: modelMissionLevel

					sorters: [
						RoleSorter { roleName: "level" }
					]

					proxyRoles: [
						ExpressionRole {
							name: "img"
							expression: model.imageFolder.length && model.imageFile.length ?
											"image://mapdb/"+model.imageFolder+"/"+model.imageFile :
											"qrc:/internal/game/bg.png"

						},
						ExpressionRole {
							name: "fullname"
							expression: model.level+qsTr(". szint")
						},
						SwitchRole {
							name: "weight"
							defaultValue: Font.Medium
						}
					]
				}

				autoSelectorChange: false

				modelTitleRole: "fullname"
				modelTitleWeightRole: "weight"
				modelSubtitleRole: "terrain"

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
					}
				}

				width: parent.width-missionLevelsCol.width

				onClicked: {
					mapEditor.levelSelected(model.get(index).rowid, missionUuid)
				}

			}

			Column {
				id: missionLevelsCol
				anchors.left: missionLevelsView.right

				visible: missionLevelsView.visible

				QToolButton {
					display: AbstractButton.IconOnly
					action: actionMissionLevelAdd
				}

				QToolButton {
					display: AbstractButton.IconOnly
					action: actionMissionLevelRemove
				}
			}

		}



	}



	Action {
		id: actionMissionLockAdd
		text: qsTr("Hozzáadás")
		icon.source: CosStyle.iconLockAdd

		onTriggered: mapEditor.run("missionLockGetList", {uuid: missionUuid})
	}

	Action {
		id: actionMissionLockRemove
		text: qsTr("Eltávolítás")
		icon.source: CosStyle.iconLockDisabled
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
		icon.source: CosStyle.iconLockDisabled
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
		id: actionMissionLevelAdd
		text: qsTr("Hozzáadás")
		icon.source: CosStyle.iconLockAdd

		onTriggered: {
			var model = mapEditor.newModel(["details", "name"])

			var d = JS.dialogCreateQml("List", {
										   roles: ["details", "name"],
										   icon: CosStyle.iconLockAdd,
										   title: qsTr("Harcmező kiválasztása"),
										   selectorSet: false,
										   sourceModel: model
									   })

			model.setVariantList(cosClient.mapToList(cosClient.terrainMap(), "name"), "name")

			d.accepted.connect(function(data) {
				mapEditor.run("missionLevelAdd", {mission: missionUuid, terrain: d.item.sourceModel.get(data).name })
			})
			d.open()
		}
	}


	Action {
		id: actionMissionLevelRemove
		text: qsTr("Utolsó szint eltávolítása")
		icon.source: CosStyle.iconLockDisabled
		enabled: !mapEditor.isBusy && modelMissionLevel.count
		onTriggered: {
			mapEditor.run("missionLevelRemove", {uuid: missionUuid })
		}
	}







	Action {
		id: actionLockAdd
		text: qsTr("Hozzáadás")
		icon.source: CosStyle.iconLockAdd

		onTriggered: mapEditor.run("campaignLockGetList", {"id": campaignId})
	}

	Action {
		id: actionLockRemove
		text: qsTr("Eltávolítás")
		icon.source: CosStyle.iconLockDisabled
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
			var d = JS.dialogCreateQml("YesNo")

			if (editType === MapEditorCampaignEdit.Campaign) {
				d.item.title = textCampaignName.text
				d.item.text = qsTr("Biztosan törlöd a hadjáratot a küldetéseivel együtt?")

				d.accepted.connect(function(data) {
					mapEditor.run("campaignRemove", {"id": campaignId})
				})
			} else if (editType === MapEditorCampaignEdit.Mission) {
				d.item.title = textMissionName.text
				d.item.text = qsTr("Biztosan törlöd a küldetést?")

				d.accepted.connect(function(data) {
					mapEditor.run("missionRemove", {"uuid": missionUuid})
				})
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

