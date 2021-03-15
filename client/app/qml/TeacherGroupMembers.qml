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

	title: teacherGroups.selectedGroupId == -1 ? "" : qsTr("Hozzárendelt tagok")
	subtitle: ""
	icon: "image://font/AcademicI/\uf15f"

	property alias list: classList

	contextMenuFunc: function (m) {
		m.addAction(actionClassAdd)
		m.addAction(actionUserAdd)
		m.addSeparator()
		m.addAction(actionClassRemove)
		m.addAction(actionUserRemove)
	}


	Connections {
		target: teacherGroups

		function onGroupUserAdd(jsonData, binaryData) {
			teacherGroups.send("groupGet", {"id": teacherGroups.selectedGroupId})
		}

		function onGroupUserRemove(jsonData, binaryData) {
			teacherGroups.send("groupGet", {"id": teacherGroups.selectedGroupId})
		}


		function onGroupExcludedClassListGet(jsonData, binaryData) {
			if (jsonData.id !== teacherGroups.selectedGroupId)
				return

			if (!jsonData.list.length) {
				cosClient.sendMessageWarning(qsTr("Osztály hozzáadása"), qsTr("Nincs több hozzáadható osztály!"))
				return
			}

			var d = JS.dialogCreateQml("List", {
										   roles: ["name", "id"],
										   icon: CosStyle.iconLockAdd,
										   title: qsTr("Osztály hozzáadása"),
										   selectorSet: true,
										   sourceModel: teacherGroups._dialogClassModel
									   })

			teacherGroups._dialogClassModel.unselectAll()
			teacherGroups._dialogClassModel.setVariantList(jsonData.list, "id")

			d.accepted.connect(function(data) {
				teacherGroups.send("groupClassAdd", {"id": teacherGroups.selectedGroupId,
									   "classList": teacherGroups._dialogClassModel.getSelectedData("id") })
			})

			d.open()
		}



		function onGroupExcludedUserListGet(jsonData, binaryData) {
			if (jsonData.id !== teacherGroups.selectedGroupId)
				return

			if (!jsonData.list.length) {
				cosClient.sendMessageWarning(qsTr("Tanuló hozzáadása"), qsTr("Nincs több hozzáadható tanuló!"))
				return
			}

			var d = JS.dialogCreateQml("UserList", {
										   icon: CosStyle.iconLockAdd,
										   title: qsTr("Diák hozzáadása"),
										   selectorSet: true,
										   sourceModel: teacherGroups._dialogUserModel
									   })

			teacherGroups._dialogUserModel.unselectAll()
			teacherGroups._dialogUserModel.setVariantList(jsonData.list, "username")

			d.accepted.connect(function(data) {
				teacherGroups.send("groupUserAdd", {"id": teacherGroups.selectedGroupId,
									   "userList": teacherGroups._dialogUserModel.getSelectedData("username") })
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



	QAccordion {
		id: accordion
		anchors.fill: parent

		opacity: teacherGroups.selectedGroupId == -1 ? 0 : 1
		visible: opacity != 0

		Behavior on opacity { NumberAnimation { duration: 125 } }


		QCollapsible {
			title: qsTr("Osztályok")

			QToolButtonBig {
				anchors.horizontalCenter: parent.horizontalCenter
				action: actionClassAdd
				visible: !teacherGroups.modelClassList.count
				color: CosStyle.colorOK
			}

			QVariantMapProxyView {
				id: classList

				visible: teacherGroups.modelClassList.count

				model: SortFilterProxyModel {
					id: classProxyModel

					sourceModel: teacherGroups.modelClassList

					sorters: [
						StringSorter { roleName: "name" }
					]
				}

				autoSelectorChange: true

				delegateHeight: CosStyle.halfLineHeight

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

					MenuItem { action: actionClassAdd; text: qsTr("Hozzáadás") }
					MenuItem { action: actionClassRemove; text: qsTr("Eltávolítás") }
				}


				onKeyInsertPressed: actionClassAdd.trigger()
				//onKeyF2Pressed: actionRename.trigger()
				onKeyDeletePressed: actionClassRemove.trigger()
				//onKeyF4Pressed: actionObjectiveNew.trigger()*/
			}
		}


		QCollapsible {
			title: qsTr("Tanulók")

			QToolButtonBig {
				anchors.horizontalCenter: parent.horizontalCenter
				action: actionUserAdd
				visible: !teacherGroups.modelUserList.count
				color: CosStyle.colorOK
			}

			QVariantMapProxyView {
				id: userList

				visible: teacherGroups.modelUserList.count

				model: SortFilterProxyModel {
					id: userProxyModel

					sourceModel: teacherGroups.modelUserList

					sorters: [
						RoleSorter { roleName: "classid"; priority: 1 },
						StringSorter { roleName: "fullname" }
					]

					proxyRoles: [
						JoinRole {
							name: "fullname"
							roleNames: [ "firstname", "lastname" ]
						}
					]
				}

				autoSelectorChange: true

				delegateHeight: CosStyle.twoLineHeight

				modelTitleRole: "fullname"
				modelSubtitleRole: "classname"

				width: parent.width


				onRightClicked: contextMenuUser.popup()

				onLongPressed: {
					if (selectorSet) {
						contextMenuUser.popup()
						return
					}
				}


				QMenu {
					id: contextMenuUser

					MenuItem { action: actionUserAdd; text: qsTr("Hozzáadás") }
					MenuItem { action: actionUserRemove; text: qsTr("Eltávolítás") }
				}


				onKeyInsertPressed: actionUserAdd.trigger()
				//onKeyF2Pressed: actionRename.trigger()
				onKeyDeletePressed: actionUserRemove.trigger()
			}
		}
	}



	Action {
		id: actionClassAdd
		text: qsTr("Osztály")
		icon.source: CosStyle.iconAdd
		enabled: !teacherGroups.isBusy && teacherGroups.selectedGroupId != -1
		onTriggered: {
			teacherGroups.send("groupExcludedClassListGet", {id: teacherGroups.selectedGroupId})
		}
	}


	Action {
		id: actionUserAdd
		text: qsTr("Tanuló")
		icon.source: CosStyle.iconAdd
		enabled: !teacherGroups.isBusy && teacherGroups.selectedGroupId != -1
		onTriggered: {
			teacherGroups.send("groupExcludedUserListGet", {id: teacherGroups.selectedGroupId})
		}
	}







	Action {
		id: actionClassRemove
		icon.source: CosStyle.iconRemove
		text: qsTr("Osztály")
		enabled: !teacherGroups.isBusy && (classList.currentIndex !== -1 || teacherGroups.modelClassList.selectedCount)
		onTriggered: {
			if (teacherGroups.modelClassList.selectedCount) {
				var dd = JS.dialogCreateQml("YesNo", {
												title: qsTr("Osztályok eltávolítása"),
												text: qsTr("Biztosan eltávolítod a kijelölt %1 osztályt?")
												.arg(teacherGroups.modelClassList.selectedCount)
											})
				dd.accepted.connect(function () {
					teacherGroups.send("groupClassRemove", {"id": teacherGroups.selectedGroupId,
										   "classList": teacherGroups.modelClassList.getSelectedData("classid") })
				})
				dd.open()
			} else {
				var o = classList.model.get(classList.currentIndex)

				var d = JS.dialogCreateQml("YesNo", {
											   title: qsTr("Biztosan eltávolítod az osztályt?"),
											   text: o.name
										   })
				d.accepted.connect(function () {
					teacherGroups.send("groupClassRemove", {"id": teacherGroups.selectedGroupId, "classid": o.classid })
				})
				d.open()
			}
		}
	}


	Action {
		id: actionUserRemove
		icon.source: CosStyle.iconRemove
		text: qsTr("Tanuló")
		enabled: !teacherGroups.isBusy && (userList.currentIndex !== -1 || teacherGroups.modelUserList.selectedCount)
		onTriggered: {
			if (teacherGroups.modelUserList.selectedCount) {
				var dd = JS.dialogCreateQml("YesNo", {
												title: qsTr("Tanulók eltávolítása"),
												text: qsTr("Biztosan eltávolítod a kijelölt %1 tanulót?")
												.arg(teacherGroups.modelUserList.selectedCount)
											})
				dd.accepted.connect(function () {
					teacherGroups.send("groupUserRemove", {"id": teacherGroups.selectedGroupId,
										   "userList": teacherGroups.modelUserList.getSelectedData("username") })
				})
				dd.open()
			} else {
				var o = userList.model.get(userList.currentIndex)

				var d = JS.dialogCreateQml("YesNo", {
											   title: qsTr("Biztosan eltávolítod a tanulót?"),
											   text: o.fullname
										   })
				d.accepted.connect(function () {
					teacherGroups.send("groupUserRemove", {"id": teacherGroups.selectedGroupId, "username": o.username })
				})
				d.open()
			}
		}
	}

	onPopulated: classList.forceActiveFocus()
}



