import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	title: selectedChapterId == -1 ? "" : subtitle
	subtitle: ""
	icon: "image://font/AcademicI/\uf15f"

	property int selectedChapterId: -1

	property VariantMapModel modelObjectiveList: mapEditor.newModel([
																		"uuid",
																		"storage",
																		"module",
																		"data"
																	])

	property VariantMapModel modelMissionList: mapEditor.newModel([
																	  "blockid",
																	  "name",
																	  "level"
																  ])

	property VariantMapModel modelExcluedMissionList: mapEditor.newModel([
																			 "id",
																			 "mission",
																			 "level",
																			 "name"
																		 ])

	contextMenuFunc: function (m) {
		m.addAction(actionObjectiveAdd)
		m.addAction(actionObjectiveRemove)
		m.addAction(actionImport)
	}


	Connections {
		target: mapEditor

		/*function onGroupUserAdd(jsonData, binaryData) {
			mapEditor.send("groupGet", {"id": mapEditor.selectedGroupId})
		}

		function onGroupUserRemove(jsonData, binaryData) {
			mapEditor.send("groupGet", {"id": mapEditor.selectedGroupId})
		}*/


		function onChapterRemoved(id) {
			if (id === selectedChapterId) {
				panel.subtitle = ""
				selectedChapterId = -1
				modelObjectiveList.clear()
			}
		}


		function onChapterModified(id) {
			if (id === selectedChapterId)
				mapEditor.run("chapterLoad", {id: id})
		}


		function onChapterLoaded(data) {
			if (!Object.keys(data).length) {
				return
			}

			selectedChapterId = data.id
			panel.subtitle = data.name

			modelObjectiveList.unselectAll()
			modelObjectiveList.setVariantList(data.objectives, "uuid")

			modelMissionList.unselectAll()
			modelMissionList.setVariantList(data.missions, "blockid")
		}


		function onChapterListLoaded() {
			if (selectedChapterId != -1)
				mapEditor.run("chapterLoad", {id: selectedChapterId})
		}


		function onBlockChapterMapMissionListLoaded(id, list) {
			if (id !== selectedChapterId)
				return

			if (!list.length) {
				cosClient.sendMessageWarning(qsTr("Küldetés hozzáadása"), qsTr("Nincs több hozzáadható küldetés!"))
				return
			}

			var d = JS.dialogCreateQml("MissionList", {
										   icon: CosStyle.iconLockAdd,
										   title: qsTr("Küldetés hozzáadása"),
										   selectorSet: true,
										   sourceModel: modelExcluedMissionList
									   })

			modelExcluedMissionList.unselectAll()
			modelExcluedMissionList.setVariantList(list, "id")

			d.accepted.connect(function(data) {
				mapEditor.run("blockChapterMapMissionAdd", {id: selectedChapterId, list: modelExcluedMissionList.getSelectedData("id")})
			})

			d.open()
		}
	}


	Connections {
		target: mapEditor.db

		function onUndone() {
			if (selectedChapterId != -1)
				mapEditor.run("chapterLoad", {id: selectedChapterId})
		}
	}


	QAccordion {
		id: accordion

		opacity: selectedChapterId == -1 ? 0 : 1
		visible: opacity != 0

		Behavior on opacity { NumberAnimation { duration: 125 } }

		QCollapsible {
			title: qsTr("Hozzárendelt küldetések")

			collapsed: true

			QToolButtonBig {
				anchors.horizontalCenter: parent.horizontalCenter
				action: actionMissionAdd
				visible: !modelMissionList.count
				color: CosStyle.colorOK
			}

			QVariantMapProxyView {
				id: missionList

				visible: modelMissionList.count

				width: parent.width-missionCol.width

				Component {
					id: sectionHeading
					Rectangle {
						width: missionList.width
						height: childrenRect.height
						color: CosStyle.colorWarningDark

						required property string section

						QLabel {
							text: parent.section
							font.pixelSize: CosStyle.pixelSize*0.85
							font.weight: Font.DemiBold
							color: "white"

							leftPadding: 5
							topPadding: 2
							bottomPadding: 2
							rightPadding: 5

							elide: Text.ElideRight
						}
					}
				}

				model: SortFilterProxyModel {
					sourceModel: modelMissionList

					sorters: [
						StringSorter { roleName: "name"; priority: 1 },
						RoleSorter { roleName: "level" }
					]

					proxyRoles: [
						ExpressionRole {
							name: "fullname"
							expression: model.level+qsTr(". szint")
						}
					]
				}

				modelTitleRole: "fullname"

				autoSelectorChange: true

				section.property: "name"
				section.criteria: ViewSection.FullString
				section.delegate: sectionHeading

				pixelSizeTitle: CosStyle.pixelSize*0.9
				colorTitle: CosStyle.colorWarningLight

				delegateHeight: CosStyle.halfLineHeight
			}

			Column {
				id: missionCol
				anchors.left: missionList.right

				visible: missionList.visible

				QToolButton {
					display: AbstractButton.IconOnly
					action: actionMissionAdd
					color: CosStyle.colorWarningLight
				}

				QToolButton {
					display: AbstractButton.IconOnly
					action: actionMissionRemove
					color: CosStyle.colorWarningLight
				}
			}

		}


		QCollapsible {
			title: qsTr("Célpontok")

			QToolButtonBig {
				anchors.horizontalCenter: parent.horizontalCenter
				action: actionObjectiveAdd
				visible: !modelObjectiveList.count
				color: CosStyle.colorOK
			}

			QVariantMapProxyView {
				id: objectiveList

				visible: modelObjectiveList.count

				model: SortFilterProxyModel {
					id: userProxyModel

					sourceModel: modelObjectiveList

					sorters: [
						StringSorter { roleName: "module" }
					]

					proxyRoles: [
						ExpressionRole {
							name: "moduleName"
							expression: mapEditor.objectiveDataToStringList(model.module, model.data)[0]
						},
						ExpressionRole {
							name: "subtitle"
							expression: mapEditor.objectiveDataToStringList(model.module, model.data)[1]
						}
					]
				}

				autoSelectorChange: true
				autoUnselectorChange: true

				delegateHeight: CosStyle.twoLineHeight*1.5

				modelTitleRole: "moduleName"
				modelSubtitleRole: "subtitle"

				width: parent.width

				onRightClicked: contextMenuUser.popup()

				onLongPressed: {
					if (selectorSet) {
						contextMenuUser.popup()
						return
					}
				}

				onClicked: {
					var o = objectiveList.model.get(index)
					mapEditor.objectiveSelected(o.uuid)
					mapEditor.run("objectiveLoad", {uuid: o.uuid})
				}


				leftComponent: QFontImage {
					width: visible ? CosStyle.twoLineHeight*1.5 : 0
					height: width
					size: Math.min(height*0.8, 32)

					icon: model && model.module ? mapEditor.objectiveInfo(model.module).icon : ""

					color: CosStyle.colorPrimary
				}


				QMenu {
					id: contextMenuUser

					MenuItem { action: actionObjectiveAdd; text: qsTr("Új") }
					MenuItem { action: actionObjectiveRemove; text: qsTr("Törlés") }
				}


				onKeyInsertPressed: actionObjectiveAdd.trigger()
				//onKeyF2Pressed: actionRename.trigger()
				onKeyDeletePressed: actionObjectiveRemove.trigger()
			}
		}
	}



	Action {
		id: actionMissionAdd
		text: qsTr("Küldetés hozzáadása")
		icon.source: CosStyle.iconAdd
		enabled: !mapEditor.isBusy
		onTriggered: {
			mapEditor.run("blockChapterMapMissionGetList", {id: selectedChapterId})
		}
	}





	Action {
		id: actionMissionRemove
		icon.source: CosStyle.iconRemove
		text: qsTr("Küldetés eltávolítása")
		enabled: !mapEditor.isBusy && (missionList.currentIndex !== -1 || modelMissionList.selectedCount)
		onTriggered: {
			if (modelMissionList.selectedCount) {
				mapEditor.run("blockChapterMapMissionRemove", {id: selectedChapterId, list: modelMissionList.getSelectedData("blockid")})
			} else {
				var o = missionList.model.get(missionList.currentIndex)
				mapEditor.run("blockChapterMapMissionRemove", {id: selectedChapterId, blockid: o.blockid})
			}
		}
	}


	Action {
		id: actionObjectiveAdd
		text: qsTr("Új célpont")
		icon.source: CosStyle.iconAdd
		enabled: !mapEditor.isBusy && selectedChapterId != -1
		onTriggered: {
			var d = JS.dialogCreateQml("List", {
										   roles: ["name", "icon", "module"],
										   icon: CosStyle.iconLockAdd,
										   title: qsTr("Modul kiválasztása"),
										   selectorSet: false,
										   sourceModel: mapEditor.modelObjectives,
										   modelIconRole: "icon"
									   })

			d.accepted.connect(function(data) {
				mapEditor.run("objectiveAdd", {chapter: selectedChapterId, module: d.item.sourceModel.get(data).module })
			})
			d.open()
		}
	}


	Action {
		id: actionObjectiveRemove
		text: qsTr("Célpont törlése")
		icon.source: CosStyle.iconDelete
		enabled: !mapEditor.isBusy && (objectiveList.currentIndex !== -1 || modelObjectiveList.selectedCount)
		onTriggered: {
			if (modelObjectiveList.selectedCount)
				mapEditor.run("objectiveRemove", {"list": modelObjectiveList.getSelectedData("uuid")})
			else {
				var o = objectiveList.model.get(objectiveList.currentIndex)
				mapEditor.run("objectiveRemove", {"uuid": o.uuid})
			}
		}
	}


	Action {
		id: actionImport
		text: qsTr("Import")
		enabled: !mapEditor.isBusy && selectedChapterId != -1
		onTriggered: {
			importDialog.chapterId = selectedChapterId
			importDialog.open()
		}
	}

	onPopulated: objectiveList.forceActiveFocus()
}



