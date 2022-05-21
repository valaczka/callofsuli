import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



QTabContainer {
	id: control

	title: qsTr("Résztvevők szerkesztése")
	icon: CosStyle.iconUsers

	backCallbackFunction: function () {
		if (modelUserList.selectedCount) {
			modelUserList.unselectAll()
			return true
		}
		if (modelClassList.selectedCount) {
			modelClassList.unselectAll()
			return true
		}

		return false
	}

	property ObjectListModel modelClassList: teacherGroups.newClassModel(control)
	property ObjectListModel modelUserList: teacherGroups.newUserModel(control)
	property ObjectListModel _modelDialogClassList: teacherGroups.newClassModel(control)
	property ObjectListModel _modelDialogUserList: teacherGroups.newUserModel(control)

	Connections {
		target: teacherGroups

		function onGroupUserGet(jsonData, bData) {
			modelUserList.unselectAll()
			modelUserList.updateJsonArray(jsonData.userList, "username")

			modelClassList.unselectAll()
			modelClassList.updateJsonArray(jsonData.classList, "classid")
		}


		function onGroupExcludedClassListGet(jsonData, binaryData) {
			if (jsonData.id !== teacherGroups.selectedGroupId)
				return

			if (!jsonData.list.length) {
				cosClient.sendMessageWarning(qsTr("Osztály hozzáadása"), qsTr("Nincs több hozzáadható osztály!"))
				return
			}

			var d = JS.dialogCreateQml("List", {
										   modelTitleRole: "name",
										   icon: CosStyle.iconGroup,
										   title: qsTr("Osztály hozzáadása"),
										   selectorSet: true,
										   model: _modelDialogClassList
									   })

			_modelDialogClassList.updateJsonArray(jsonData.list, "classid")

			d.accepted.connect(function(data) {
				var l = _modelDialogClassList.getSelectedFields("classid")
				if (l.length) {
					teacherGroups.send("groupClassAdd", {
										   id: teacherGroups.selectedGroupId,
										   list: l
									   })
				}
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
										   icon: CosStyle.iconUserAdd,
										   title: qsTr("Diák hozzáadása"),
										   selectorSet: true,
										   model: _modelDialogUserList
									   })

			_modelDialogUserList.updateJsonArray(jsonData.list, "username")

			d.accepted.connect(function(data) {
				var l = _modelDialogUserList.getSelectedFields("username")
				if (l.length) {
					teacherGroups.send("groupUserAdd", {
										   id: teacherGroups.selectedGroupId,
										   list: l
									   })
				}
			})

			d.open()
		}

		function onGroupUserAdd(jsonData, binaryData) { reloadGroup() }
		function onGroupUserRemove(jsonData, binaryData) { reloadGroup() }
		function onGroupClassAdd(jsonData, binaryData) { reloadGroup() }
		function onGroupClassRemove(jsonData, binaryData) { reloadGroup() }
	}


	QAccordion {
		id: accordion

		QTabHeader {
			tabContainer: control
			isPlaceholder: true
		}

		QCollapsible {
			id: collapsibleClass
			title: qsTr("Osztályok")
			backgroundColor: "transparent"

			QListBusyIndicator {
				anchors.horizontalCenter: parent.horizontalCenter
				visible: modelClassList.processing
			}


			QObjectListView {
				id: classList
				width: parent.width

				refreshEnabled: false
				delegateHeight: CosStyle.baseHeight

				visible: !modelClassList.processing

				leftComponent: QFontImage {
					icon: CosStyle.iconGroup
					width: classList.delegateHeight+10
					height: classList.delegateHeight*0.8
					size: height
				}

				model: SortFilterProxyModel {
					sourceModel: modelClassList

					sorters: [
						StringSorter { roleName: "name"; priority: 1 }
					]
				}

				modelTitleRole: "name"

				highlightCurrentItem: true

				autoSelectorChange: true

				footer: QToolButtonFooter {
					width: classList.width
					action: actionClassAdd
				}

				onRightClicked: contextMenuClass.popup()
				onLongPressed: contextMenuClass.popup()

				QMenu {
					id: contextMenuClass

					MenuItem { action: actionClassRemove }
				}

				onKeyInsertPressed: actionClassAdd.trigger()
				onKeyDeletePressed: actionClassRemove.trigger()
			}

		}

		QCollapsible {
			id: collapsibleUsers
			title: qsTr("Tanulók")
			backgroundColor: "transparent"

			QListBusyIndicator {
				anchors.horizontalCenter: parent.horizontalCenter
				visible: modelUserList.processing
			}

			QObjectListView {
				id: userList
				width: parent.width

				delegateHeight: CosStyle.twoLineHeight

				visible: !modelUserList.processing

				refreshEnabled: false

				section.property: "fullclassname"
				section.criteria: ViewSection.FullString
				section.delegate: Component {
					Rectangle {
						width: userList.width
						height: childrenRect.height
						color: CosStyle.colorPrimaryDark

						required property string section

						QLabel {
							text: parent.section
							font.pixelSize: CosStyle.pixelSize*0.8
							font.weight: Font.DemiBold
							font.capitalization: Font.AllUppercase
							color: "white"

							leftPadding: 5
							topPadding: 2
							bottomPadding: 2
							rightPadding: 5

							elide: Text.ElideRight
						}
					}
				}

				leftComponent: Image {
					source: model ? cosClient.rankImageSource(model.rankid, model.ranklevel, model.rankimage) : ""
					width: userList.delegateHeight+10
					height: userList.delegateHeight*0.8
					fillMode: Image.PreserveAspectFit
					asynchronous: true
				}

				model: SortFilterProxyModel {
					sourceModel: modelUserList

					sorters: [
						StringSorter { roleName: "fullclassname"; priority: 2 },
						StringSorter { roleName: "name"; priority: 1 }
					]

					proxyRoles: [
						ExpressionRole {
							name: "name"
							expression: firstname+" "+lastname
						},
						ExpressionRole {
							name: "fullclassname"
							expression: classid === -1 ? qsTr("[Osztály nélkül]") : classname
						},
						SwitchRole {
							name: "titlecolor"
							filters: ValueFilter {
								roleName: "active"
								value: false
								SwitchRole.value: CosStyle.colorPrimaryDark
							}
							defaultValue: CosStyle.colorPrimaryLighter
						}


					]
				}

				modelTitleRole: "name"
				modelSubtitleRole: "username"
				modelTitleColorRole: "titlecolor"
				modelSubtitleColorRole: "titlecolor"

				highlightCurrentItem: true

				autoSelectorChange: true

				footer: QToolButtonFooter {
					width: userList.width
					action: actionUserAdd
				}

				onRightClicked: contextMenuUser.popup()
				onLongPressed: contextMenuUser.popup()

				QMenu {
					id: contextMenuUser

					MenuItem { action: actionUserRemove }
				}

				onKeyInsertPressed: actionUserAdd.trigger()
				onKeyDeletePressed: actionUserRemove.trigger()
			}
		}
	}



	Action {
		id: actionUserAdd
		text: qsTr("Tanuló hozzáadása")
		icon.source: CosStyle.iconUserAdd
		onTriggered: teacherGroups.send("groupExcludedUserListGet", {id: teacherGroups.selectedGroupId})
	}

	Action {
		id: actionUserRemove
		text: qsTr("Tanuló eltávolítása")
		icon.source: CosStyle.iconRemove
		enabled: userList.currentIndex !== -1
		onTriggered: {
			var more = modelUserList.selectedCount

			if (more > 0) {
				var dd = JS.dialogCreateQml("YesNo", {
												title: qsTr("Tanulók eltávolítása"),
												text: qsTr("Biztosan eltávolítod a kijelölt %1 tanulót?").arg(more)
											})
				dd.accepted.connect(function () {
					teacherGroups.send("groupUserRemove", {
										   id: teacherGroups.selectedGroupId,
										   list: modelUserList.getSelectedFields("username")
									   })
					modelUserList.unselectAll()
				})
				dd.open()
			} else {
				var o = userList.model.get(userList.currentIndex)

				var d = JS.dialogCreateQml("YesNo", {
											   title: qsTr("Tanuló eltávolítása"),
											   text: qsTr("Biztosan eltávolítod a tanulót?\n%1").arg(o.name)
										   })
				d.accepted.connect(function () {
					teacherGroups.send("groupUserRemove", { id: teacherGroups.selectedGroupId, username: o.username })
					modelUserList.unselectAll()
				})
				d.open()
			}
		}
	}


	Action {
		id: actionClassAdd
		text: qsTr("Osztály hozzáadása")
		icon.source: CosStyle.iconAdd
		onTriggered: teacherGroups.send("groupExcludedClassListGet", {id: teacherGroups.selectedGroupId})
	}


	Action {
		id: actionClassRemove
		text: qsTr("Osztály eltávolítása")
		icon.source: CosStyle.iconRemove
		enabled: classList.currentIndex !== -1
		onTriggered: {
			var more = modelClassList.selectedCount

			if (more > 0) {
				var dd = JS.dialogCreateQml("YesNo", {
												title: qsTr("Osztályok eltávolítása"),
												text: qsTr("Biztosan eltávolítod a kijelölt %1 osztályt?").arg(more)
											})
				dd.accepted.connect(function () {
					teacherGroups.send("groupClassRemove", { id: teacherGroups.selectedGroupId, list: modelClassList.getSelectedFields("classid")})
					modelClassList.unselectAll()
				})
				dd.open()
			} else {
				var o = classList.model.get(classList.currentIndex)

				var d = JS.dialogCreateQml("YesNo", {
											   title: qsTr("Osztály eltávolítása"),
											   text: qsTr("Biztosan eltávolítod az osztályt?\n%1").arg(o.name)
										   })
				d.accepted.connect(function () {
					teacherGroups.send("groupClassRemove", { id: teacherGroups.selectedGroupId, classid: o.classid})
					modelClassList.unselectAll()
				})
				d.open()
			}
		}
	}


	function reloadGroup() {
		if (teacherGroups && teacherGroups.selectedGroupId != -1)
			teacherGroups.send("groupUserGet", {id: teacherGroups.selectedGroupId})
	}

	onPopulated: reloadGroup()


}

