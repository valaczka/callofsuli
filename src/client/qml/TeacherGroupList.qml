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

	property alias list: list

	contextMenuFunc: function (m) {
		m.addAction(actionGroupNew)
		m.addSeparator()
		m.addAction(actionRename)
		m.addAction(actionRemove)
		m.addAction(actionViewMembers)
	}

	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: teacherGroups.modelGroupList
		sorters: [
			StringSorter { roleName: "name" }
		]
	}

	QVariantMapProxyView {
		id: list
		anchors.fill: parent

		visible: teacherGroups.modelGroupList.count

		refreshEnabled: true
		onRefreshRequest: teacherGroups.send("groupListGet")

		model: userProxyModel
		modelTitleRole: "name"
		modelSubtitleRole: "readableClassList"

		autoSelectorChange: true

		delegateHeight: CosStyle.twoLineHeight

		leftComponent: QFontImage {
			width: visible ? list.delegateHeight : 0
			height: width*0.8
			size: Math.min(height*0.8, 32)

			icon: "image://font/Academic/\uf118"

			visible: model

			color: CosStyle.colorPrimary
		}

		rightComponent: Row {
			visible: stackMode && pageStack
			spacing: 0

			QToolButton {
				anchors.verticalCenter: parent.verticalCenter
				ToolTip.text: qsTr("Tagok")

				icon.source: CosStyle.iconUsers
				onClicked: {
					list.currentIndex = modelIndex
					teacherGroups.selectedGroupId = model.id
					pageTeacherGroup.addStackPanel(pageTeacherGroup.panelMembers)
				}
			}

			QToolButton {
				anchors.verticalCenter: parent.verticalCenter
				ToolTip.text: qsTr("Pályák")

				icon.source: "image://font/AcademicI/\uf15f"
				onClicked: {
					list.currentIndex = modelIndex
					teacherGroups.selectedGroupId = model.id
					pageTeacherGroup.addStackPanel(pageTeacherGroup.panelMaps)
				}
			}
		}

		/*rightComponent: QFontImage {
			width: visible ? list.delegateHeight*0.8 : 0
			height: width
			size: Math.min(height*0.8, 32)

			icon: CosStyle.iconClock1

			visible: model && model.editLocked

			color: CosStyle.colorAccentLighter
		}*/


		onClicked: {
			var o = list.model.get(index)
			teacherGroups.selectedGroupId = o.id
			if (stackMode)
				actionViewMembers.trigger()
		}

		onRightClicked: {
			var o = list.model.get(index)
			teacherGroups.selectedGroupId = o.id
			contextMenu.popup()
		}

		onLongPressed: {
			if (selectorSet) {
				contextMenu.popup()
				return
			}

			/*selectorSet = true

			var o = list.model.get(index)

			if (o.type === 0) {
				chaptersFilter.enabled = true
			} else if (o.type === 1) {
				objectiveIdFilter.value = o.id
				objectivesFilter.enabled = true
			}*/

			//mapEditor.modelChapterList.select(serverList.model.mapToSource(serverList.currentIndex))
		}



		QMenu {
			id: contextMenu

			MenuItem { action: actionGroupNew }
			MenuSeparator { }
			MenuItem { action: actionRename }
			MenuItem { action: actionRemove }
			MenuItem { action: actionViewMembers }
		}


		onKeyInsertPressed: actionGroupNew.trigger()
		onKeyF2Pressed: actionRename.trigger()
		onKeyDeletePressed: actionRemove.trigger()
		//onKeyF4Pressed: actionObjectiveNew.trigger()*/
	}


	QToolButtonBig {
		anchors.centerIn: parent
		visible: !teacherGroups.modelGroupList.count
		action: actionGroupNew
		color: CosStyle.colorOK
	}



	Connections {
		target: teacherGroups

		function onGroupMemberListGet(jsonData, binaryData) {
			if (jsonData.id !== teacherGroups.selectedGroupId)
				return

			var d = JS.dialogCreateQml("UserList", {
										   icon: CosStyle.iconUsers,
										   title: qsTr("Résztvevők"),
										   selectorSet: false,
										   sourceModel: teacherGroups._dialogUserModel
									   })

			teacherGroups._dialogUserModel.unselectAll()
			teacherGroups._dialogUserModel.setVariantList(jsonData.list, "username")

			d.open()
		}
	}


	Action {
		id: actionGroupNew
		text: qsTr("Új csoport")
		icon.source: CosStyle.iconAdd
		enabled: !teacherGroups.isBusy
		onTriggered: {
			var d = JS.dialogCreateQml("TextField")
			d.item.title = qsTr("Új csoport neve")

			d.accepted.connect(function(data) {
				if (data.length)
					teacherGroups.send("groupCreate", {name: data})
			})
			d.open()
		}
	}


	Action {
		id: actionRename
		text: qsTr("Átnevezés")
		icon.source: CosStyle.iconRename
		enabled: !teacherGroups.isBusy && list.currentIndex !== -1
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var d = JS.dialogCreateQml("TextField", { title: qsTr("Csoport neve"), value: o.name })

			d.accepted.connect(function(data) {
				if (data.length)
					teacherGroups.send("groupUpdate", {id: o.id, name: data})
			})
			d.open()
		}
	}



	Action {
		id: actionRemove
		icon.source: CosStyle.iconDelete
		text: qsTr("Törlés")
		enabled: !teacherGroups.isBusy && (list.currentIndex !== -1 || teacherGroups.modelGroupList.selectedCount)
		onTriggered: {
			if (teacherGroups.modelGroupList.selectedCount) {
				var dd = JS.dialogCreateQml("YesNo", {
												title: qsTr("Csoportok törlése"),
												text: qsTr("Biztosan törlöd a kijelölt %1 csoportot?")
												.arg(teacherGroups.modelGroupList.selectedCount)
											})
				dd.accepted.connect(function () {
					teacherGroups.send("groupRemove", {"list": teacherGroups.modelGroupList.getSelectedData("id") })
				})
				dd.open()
			} else {
				var o = list.model.get(list.currentIndex)

				var d = JS.dialogCreateQml("YesNo", {
											   title: qsTr("Biztosan törlöd a csoportot?"),
											   text: o.name
										   })
				d.accepted.connect(function () {
					teacherGroups.send("groupRemove", {"id": o.id })
				})
				d.open()
			}
		}
	}


	Action {
		id: actionViewMembers
		text: qsTr("Résztvevők")
		icon.source: CosStyle.iconUsers
		enabled: teacherGroups.selectedGroupId != -1
		onTriggered: {
			teacherGroups.send("groupMemberListGet", {id: teacherGroups.selectedGroupId})
		}
	}

	/*Action {
		id: actionExport
		text: qsTr("Exportálás")
		icon.source: CosStyle.iconDrawer
		enabled: !teacherMaps.isBusy && list.currentIndex !== -1
		onTriggered: {
			var o = list.model.get(list.currentIndex)
			fileDialog.mapUuid = o.uuid
			fileDialog.open()
		}
	}*/

	onPopulated: list.forceActiveFocus()
}



