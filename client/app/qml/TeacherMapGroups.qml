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

	title: teacherMaps.selectedMapId.length ? qsTr("Hozzárendelt csoportok") : ""
	subtitle: ""
	icon: CosStyle.iconGroupsSmall

	property alias list: groupList

	contextMenuFunc: function (m) {
		m.addAction(actionGroupAdd)
		m.addAction(actionGroupRemove)
	}


	Connections {
		target: teacherMaps

		function onMapGroupAdd(jsonData, binaryData) {
			teacherMaps.send("mapGet", {"uuid": teacherMaps.selectedMapId})
		}

		function onMapGroupRemove(jsonData, binaryData) {
			teacherMaps.send("mapGet", {"uuid": teacherMaps.selectedMapId})
		}

		function onMapExcludedGroupListGet(jsonData, binaryData) {
			if (jsonData.uuid !== teacherMaps.selectedMapId)
				return

			if (!jsonData.list.length) {
				cosClient.sendMessageWarning(qsTr("Csoport hozzáadása"), qsTr("Nincs több hozzáadható csoport!"))
				return
			}

			var d = JS.dialogCreateQml("List", {
										   roles: ["name", "id", "readableClassList"],
										   icon: CosStyle.iconLockAdd,
										   title: qsTr("Csoport hozzáadása"),
										   modelSubTitleRole: "readableClassList",
										   delegateHeight: CosStyle.twoLineHeight,
										   selectorSet: true,
										   sourceModel: teacherMaps._dialogGroupModel
									   })

			teacherMaps._dialogGroupModel.unselectAll()
			teacherMaps._dialogGroupModel.setVariantList(jsonData.list, "id")

			d.accepted.connect(function(data) {
				teacherMaps.send("mapGroupAdd", {"uuid": teacherMaps.selectedMapId,
									   "groupList": teacherMaps._dialogGroupModel.getSelectedData("id") })
			})

			d.open()
		}

		function onMapGet(jsonData) {
			panel.subtitle = jsonData.name
		}

		function onSelectedMapIdChanged(mapid) {
			if (!mapid.length)
				panel.subtitle = ""
		}

	}




	QToolButtonBig {
		anchors.centerIn: parent
		action: actionGroupAdd
		visible: teacherMaps.selectedMapId.length && !teacherMaps.modelGroupList.count
		color: CosStyle.colorOK
	}

	QVariantMapProxyView {
		id: groupList

		visible: teacherMaps.selectedMapId.length && teacherMaps.modelGroupList.count

		model: SortFilterProxyModel {
			sourceModel: teacherMaps.modelGroupList

			sorters: [
				StringSorter { roleName: "name" }
			]
		}

		autoSelectorChange: true

		modelTitleRole: "name"
		modelSubtitleRole: "readableClassList"

		width: parent.width
		delegateHeight: CosStyle.twoLineHeight

		onRightClicked: contextMenu.popup()

		onLongPressed: {
			if (selectorSet) {
				contextMenu.popup()
				return
			}
		}

		QMenu {
			id: contextMenu

			MenuItem { action: actionGroupAdd }
			MenuItem { action: actionGroupRemove }
		}


		onKeyInsertPressed: actionGroupAdd.trigger()
		onKeyDeletePressed: actionGroupRemove.trigger()
	}

	Action {
		id: actionGroupAdd
		text: qsTr("Hozzáadás")
		icon.source: CosStyle.iconAdd
		enabled: !teacherMaps.isBusy && teacherMaps.selectedMapId.length
		onTriggered: {
			teacherMaps.send("mapExcludedGroupListGet", {uuid: teacherMaps.selectedMapId})
		}
	}


	Action {
		id: actionGroupRemove
		icon.source: CosStyle.iconRemove
		text: qsTr("Eltávolítás")
		enabled: !teacherMaps.isBusy && (groupList.currentIndex !== -1 || teacherMaps.modelGroupList.selectedCount)
		onTriggered: {
			if (teacherMaps.modelGroupList.selectedCount) {
				var dd = JS.dialogCreateQml("YesNo", {
												title: qsTr("Csoportok eltávolítása"),
												text: qsTr("Biztosan eltávolítod a kijelölt %1 csoportot?")
												.arg(teacherMaps.modelGroupList.selectedCount)
											})
				dd.accepted.connect(function () {
					teacherMaps.send("mapGroupRemove", {"uuid": teacherMaps.selectedMapId,
										   "groupList": teacherMaps.modelGroupList.getSelectedData("groupid") })
				})
				dd.open()
			} else {
				var o = groupList.model.get(groupList.currentIndex)

				var d = JS.dialogCreateQml("YesNo", {
											   title: qsTr("Biztosan eltávolítod a csoportot?"),
											   text: o.name+(o.readableClassList.length ? " ("+o.readableClassList+")" : "")
										   })
				d.accepted.connect(function () {
					teacherMaps.send("mapGroupRemove", {"uuid": teacherMaps.selectedMapId, "groupid": o.groupid })
				})
				d.open()
			}
		}
	}

	onPopulated: groupList.forceActiveFocus()
}



