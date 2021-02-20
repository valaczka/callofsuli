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

	title: qsTr("Csoportok")
	icon: "image://font/AcademicI/\uf15f"

	property alias list: groupList

	contextMenuFunc: function (m) {
		m.addAction(actionGroupAdd)
	}


	Connections {
		target: teacherMaps

		function onMapGroupAdd(jsonData, binaryData) {
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

	}




	QToolButtonBig {
		anchors.centerIn: parent
		action: actionGroupAdd
		visible: teacherMaps.selectedMapId.length && !teacherMaps.modelGroupList.count
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
		}


		onKeyInsertPressed: actionGroupAdd.trigger()
		//onKeyF2Pressed: actionRename.trigger()
		/*onKeyDeletePressed: actionRemove.trigger()
		onKeyF4Pressed: actionObjectiveNew.trigger()*/
	}

	Action {
		id: actionGroupAdd
		text: qsTr("Csoport")
		icon.source: CosStyle.iconAdd
		enabled: !teacherMaps.isBusy && teacherMaps.selectedMapId.length
		onTriggered: {
			teacherMaps.send("mapExcludedGroupListGet", {uuid: teacherMaps.selectedMapId})
		}
	}

}



