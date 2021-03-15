import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	layoutFillWidth: true

	title: teacherGroups.selectedGroupId == -1 ? "" : qsTr("Hozzárendelt pályák")
	subtitle: ""
	icon: "image://font/AcademicI/\uf15f"

	property alias list: mapList

	contextMenuFunc: function (m) {
		m.addAction(actionMapAdd)
		m.addAction(actionMapRemove)
	}


	Connections {
		target: teacherGroups

		function onGroupMapAdd(jsonData, binaryData) {
			teacherGroups.send("groupGet", {"id": teacherGroups.selectedGroupId})
		}

		function onGroupMapRemove(jsonData, binaryData) {
			teacherGroups.send("groupGet", {"id": teacherGroups.selectedGroupId})
		}

		function onGroupExcludedMapListGet(jsonData, binaryData) {
			if (jsonData.id !== teacherGroups.selectedGroupId)
				return

			if (!jsonData.list.length) {
				cosClient.sendMessageWarning(qsTr("Pálya hozzáadása"), qsTr("Nincs több hozzáadható pálya!"))
				return
			}

			var d = JS.dialogCreateQml("List", {
										   roles: ["name", "uuid"],
										   icon: CosStyle.iconLockAdd,
										   title: qsTr("Pálya hozzáadása"),
										   selectorSet: true,
										   sourceModel: teacherGroups._dialogMapModel
									   })

			teacherGroups._dialogMapModel.unselectAll()
			teacherGroups._dialogMapModel.setVariantList(jsonData.list, "uuid")

			d.accepted.connect(function(data) {
				teacherGroups.send("groupMapAdd", {"id": teacherGroups.selectedGroupId,
									   "mapList": teacherGroups._dialogMapModel.getSelectedData("uuid") })
			})

			d.open()
		}


		function onGroupGet(jsonData) {
			panel.subtitle = jsonData.name
		}

		function onSelectedGroupIdChanged(groupid) {
			if (groupid === -1)
				panel.subtitle = ""
		}

	}




	QToolButtonBig {
		anchors.centerIn: parent
		action: actionMapAdd
		visible: teacherGroups.selectedGroupId != -1 && !teacherGroups.modelMapList.count
		color: CosStyle.colorOK
	}

	QVariantMapProxyView {
		id: mapList

		visible: teacherGroups.selectedGroupId != -1 && teacherGroups.modelMapList.count

		model: SortFilterProxyModel {
			id: classProxyModel

			sourceModel: teacherGroups.modelMapList

			sorters: [
				StringSorter { roleName: "name" }
			]
		}

		autoSelectorChange: true

		modelTitleRole: "name"

		width: parent.width

		onRightClicked: contextMenu.popup()

		onLongPressed: {
			if (selectorSet) {
				contextMenu.popup()
				return
			}
		}

		QMenu {
			id: contextMenu

			MenuItem { action: actionMapAdd }
			MenuItem { action: actionMapRemove }
		}


		onKeyInsertPressed: actionMapAdd.trigger()
		//onKeyF2Pressed: actionRename.trigger()
		/*onKeyDeletePressed: actionRemove.trigger()
		onKeyF4Pressed: actionObjectiveNew.trigger()*/
	}

	Action {
		id: actionMapAdd
		text: qsTr("Hozzáadás")
		icon.source: CosStyle.iconAdd
		enabled: !teacherGroups.isBusy && teacherGroups.selectedGroupId != -1
		onTriggered: {
			teacherGroups.send("groupExcludedMapListGet", {id: teacherGroups.selectedGroupId})
		}
	}


	Action {
		id: actionMapRemove
		icon.source: CosStyle.iconRemove
		text: qsTr("Eltávolítás")
		enabled: !teacherGroups.isBusy && (mapList.currentIndex !== -1 || teacherGroups.modelMapList.selectedCount)
		onTriggered: {
			if (teacherGroups.modelMapList.selectedCount) {
				var dd = JS.dialogCreateQml("YesNo", {
												title: qsTr("Pálya eltávolítása"),
												text: qsTr("Biztosan eltávolítod a kijelölt %1 pályát?")
												.arg(teacherGroups.modelMapList.selectedCount)
											})
				dd.accepted.connect(function () {
					teacherGroups.send("groupMapRemove", {"id": teacherGroups.selectedGroupId,
										   "mapList": teacherGroups.modelMapList.getSelectedData("uuid") })
				})
				dd.open()
			} else {
				var o = mapList.model.get(mapList.currentIndex)

				var d = JS.dialogCreateQml("YesNo", {
											   title: qsTr("Biztosan eltávolítod a pályát?"),
											   text: o.name
										   })
				d.accepted.connect(function () {
					teacherGroups.send("groupMapRemove", {"id": teacherGroups.selectedGroupId, "uuid": o.uuid })
				})
				d.open()
			}
		}
	}

	onPopulated: mapList.forceActiveFocus()
}



