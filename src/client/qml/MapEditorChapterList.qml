import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	maximumWidth: 600
	layoutFillWidth: false

	title: qsTr("Célpontok")
	icon: CosStyle.iconUsers

	contextMenuFunc: function (m) {
		m.addAction(actionChapterNew)
		m.addAction(actionObjectiveNew)
	}


	SortFilterProxyModel {
		id: userProxyModel

		sourceModel: mapEditor.modelChapterList

		filters: [
			AllOf {
				RegExpFilter {
					enabled: toolbar.searchBar.text.length
					roleName: "name"
					pattern: toolbar.searchBar.text
					caseSensitivity: Qt.CaseInsensitive
					syntax: RegExpFilter.FixedString
				}
				ValueFilter {
					id: chaptersFilter
					enabled: false
					roleName: "type"
					value: 0
				}
				AllOf {
					id: objectivesFilter
					enabled: false
					ValueFilter {
						roleName: "type"
						value: 1
					}
					ValueFilter {
						id: objectiveIdFilter
						roleName: "id"
						value: -1
					}
				}
			}

		]
		sorters: [
			StringSorter { roleName: "name"; priority: 2 },
			RoleSorter { roleName: "id"; priority: 1 },
			StringSorter { roleName: "type"; priority: 0 }
		]
		proxyRoles: [
			ExpressionRole {
				name: "moduleName"
				expression: model.type === 0 ? model.name : mapEditor.objectiveDataToStringList(model.module, model.data)[0]
			},
			ExpressionRole {
				name: "subtitle"
				expression: model.type === 0 ? "" : mapEditor.objectiveDataToStringList(model.module, model.data)[1]
			},
			SwitchRole {
				name: "textColor"
				filters: ValueFilter {
					roleName: "type"
					value: 0
					SwitchRole.value: CosStyle.colorPrimaryLight
				}
				defaultValue: CosStyle.colorPrimaryLighter
			},
			SwitchRole {
				name: "fontWeight"
				filters: ValueFilter {
					roleName: "type"
					value: 0
					SwitchRole.value: Font.Medium
				}
				defaultValue: Font.Normal
			}
		]

	}



	QVariantMapProxyView {
		id: list
		anchors.fill: parent

		visible: mapEditor.modelChapterList.count

		model: userProxyModel
		modelTitleRole: "moduleName"
		modelSubtitleRole: "subtitle"
		modelDepthRole: "type"
		modelTitleColorRole: "textColor"
		modelTitleWeightRole: "fontWeight"

		depthWidth: CosStyle.baseHeight*0.5



		autoSelectorChange: false
		autoUnselectorChange: true

		leftComponent: QFontImage {
			width: visible ? list.delegateHeight*0.8 : 0
			height: width
			size: Math.min(height*0.8, 32)

			icon: model && model.module ? mapEditor.objectiveInfo(model.module).icon : ""

			visible: model && model.type === 1

			color: CosStyle.colorPrimary
		}


		rightComponent: Row {
			visible: model && model.type === 1
			spacing: 0


			QToolButton {
				anchors.verticalCenter: parent.verticalCenter
				ToolTip.text: qsTr("Új üres célpont")

				icon.source: CosStyle.iconAdd
				enabled: !mapEditor.isBusy
				onClicked: {
					mapEditor.run("objectiveAdd", {chapter: model.id, module: model.module})
				}
			}

			QToolButton {
				anchors.verticalCenter: parent.verticalCenter
				ToolTip.text: qsTr("Célpont kettőzése")

				icon.source: CosStyle.iconEdit
				enabled: !mapEditor.isBusy
				onClicked: {
					mapEditor.run("objectiveAdd", {chapter: model.id, module: model.module, data: model.data })
				}
			}
		}


		onClicked: {
			var o = list.model.get(index)
			if (o.type === 0) {
				mapEditor.objectiveSelected("")
			} else {
				mapEditor.objectiveSelected(o.uuid)
				mapEditor.run("objectiveLoad", {uuid: o.uuid})
			}
		}

		onRightClicked: contextMenu.popup()

		onLongPressed: {
			if (selectorSet) {
				contextMenu.popup()
				return
			}

			selectorSet = true

			var o = list.model.get(index)

			if (o.type === 0) {
				chaptersFilter.enabled = true
			} else if (o.type === 1) {
				objectiveIdFilter.value = o.id
				objectivesFilter.enabled = true
			}

			mapEditor.modelChapterList.select(list.model.mapToSource(index))
		}

		onSelectorSetChanged: {
			if (!selectorSet) {
				objectivesFilter.enabled = false
				chaptersFilter.enabled = false
			}
		}


		QMenu {
			id: contextMenu

			MenuItem { action: actionChapterNew }
			MenuItem { action: actionObjectiveNew }
			MenuItem { action: actionRename }
			MenuItem { action: actionRemove }
		}


		onKeyInsertPressed: actionChapterNew.trigger()
		onKeyF4Pressed: actionRename.trigger()
		onKeyDeletePressed: actionRemove.trigger()
		onKeyF2Pressed: actionObjectiveNew.trigger()
	}


	QPagePanelSearch {
		id: toolbar

		listView: list

		enabled: mapEditor.modelChapterList.count
		labelCountText: mapEditor.modelChapterList.selectedCount
		onSelectAll: JS.selectAllProxyModelToggle(userProxyModel)
	}


	QToolButtonBig {
		anchors.centerIn: parent
		visible: !mapEditor.modelChapterList.count
		action: actionChapterNew
	}




	Action {
		id: actionChapterNew
		text: qsTr("Új szakasz")
		icon.source: CosStyle.iconAdd
		enabled: !mapEditor.isBusy
		onTriggered: {
			var d = JS.dialogCreateQml("TextField")
			d.item.title = qsTr("Új szakasz neve")

			d.accepted.connect(function(data) {
				mapEditor.run("chapterAdd", {"name": data})
			})
			d.open()
		}
	}

	Action {
		id: actionRename
		text: qsTr("Átnevezés")
		icon.source: CosStyle.iconAdd
		enabled: !mapEditor.isBusy && list.currentIndex !== -1 && list.model.get(list.currentIndex).type === 0
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var d = JS.dialogCreateQml("TextField", {
										   title: qsTr("Szakasz neve"),
										   value: o.name
									   })

			d.accepted.connect(function(data) {
				mapEditor.run("chapterModify", {
								  "id": o.id,
								  "data": {"name": data}
							  })
			})
			d.open()
		}
	}






	Action {
		id: actionObjectiveNew
		text: qsTr("Új célpont")
		icon.source: CosStyle.iconAdd
		enabled: !mapEditor.isBusy && list.currentIndex !== -1
		onTriggered: {
			var model = mapEditor.newModel(["details", "name"])

			var o = list.model.get(list.currentIndex)

			var d = JS.dialogCreateQml("List", {
										   roles: ["name", "module"],
										   icon: CosStyle.iconLockAdd,
										   title: qsTr("Modul kiválasztása"),
										   selectorSet: false,
										   sourceModel: model
									   })

			model.setVariantList(cosClient.mapToList(cosClient.objectiveModuleMap(), "module"), "module")

			d.accepted.connect(function(data) {
				mapEditor.run("objectiveAdd", {chapter: o.id, module: d.item.sourceModel.get(data).module })
			})
			d.open()
		}
	}

	Action {
		id: actionRemove
		text: qsTr("Törlés")
		icon.source: CosStyle.iconDelete
		enabled: !mapEditor.isBusy && (list.currentIndex !== -1 || mapEditor.modelChapterList.selectedCount)
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var d = JS.dialogCreateQml("YesNo")
			d.item.title = o.name

			var more = mapEditor.modelChapterList.selectedCount

			if (more > 0) {
				d.item.text = o.type === 0 ? qsTr("Biztosan törlöd a kijelölt %1 szakaszt a célpontjaival együtt?").arg(more)
										   : qsTr("Biztosan törlöd a kijelölt %1 célpontot?").arg(more)
			} else {
				d.item.text = o.type === 0 ? qsTr("Biztosan törlöd a szakaszt a célpontjaival együtt?")
										   : qsTr("Biztosan törlöd a célpontot?")
			}


			d.accepted.connect(function(data) {
				if (o.type === 0) {
					if (more > 0)
						mapEditor.run("chapterRemove", {"list": mapEditor.modelChapterList.getSelectedData("id") })
					else
						mapEditor.run("chapterRemove", {"id": o.cid})
				} else {
					if (more > 0)
						mapEditor.run("objectiveRemove", {"list": mapEditor.modelChapterList.getSelectedData("uuid")})
					else
						mapEditor.run("objectiveRemove", {"uuid": o.uuid})
				}
			})
			d.open()
		}
	}


	Connections {
		target: mapEditor

		function onChapterListReloaded(list) {
			mapEditor.modelChapterList.unselectAll()
			mapEditor.modelChapterList.setVariantList(list, "uuid");
		}

		function onObjectiveAdded(rowid, uuid) {
			for (var i=0; i<list.model.count; i++) {
				if (list.model.get(i).uuid === uuid) {
					list.currentIndex = i
					break
				}
			}

			mapEditor.objectiveSelected(uuid)
			mapEditor.run("objectiveLoad", {uuid: uuid})
		}
	}


	Component.onCompleted: {
		mapEditor.run("chapterListReload")
	}

	onPanelActivated: {
		list.forceActiveFocus()
	}
}



