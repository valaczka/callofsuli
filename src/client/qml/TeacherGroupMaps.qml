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

	title: qsTr("Pályák")
	icon: "image://font/AcademicI/\uf15f"

	property alias list: mapList

	contextMenuFunc: function (m) {
		m.addAction(actionMapAdd)
	}


	Connections {
		target: teacherGroups

		function onGroupMapAdd(jsonData, binaryData) {
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

	}




	QToolButtonBig {
		anchors.centerIn: parent
		action: actionMapAdd
		visible: teacherGroups.selectedGroupId != -1 && !teacherGroups.modelMapList.count
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
		}


		onKeyInsertPressed: actionMapAdd.trigger()
		//onKeyF2Pressed: actionRename.trigger()
		/*onKeyDeletePressed: actionRemove.trigger()
		onKeyF4Pressed: actionObjectiveNew.trigger()*/
	}

	Action {
		id: actionMapAdd
		text: qsTr("Pálya")
		icon.source: CosStyle.iconAdd
		enabled: !teacherGroups.isBusy && teacherGroups.selectedGroupId != -1
		onTriggered: {
			teacherGroups.send("groupExcludedMapListGet", {id: teacherGroups.selectedGroupId})
		}
	}

}



