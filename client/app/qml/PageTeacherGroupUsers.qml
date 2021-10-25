import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



QBasePage {
	id: control

	defaultTitle: qsTr("Résztvevők szerkesztése")
	defaultSubTitle: ""

	property TeacherGroups teacherGroups: null

	mainToolBarComponent: UserButton {
		userDetails: userData
		userNameVisible: page.width>800
	}

	UserDetails {
		id: userData
	}

	//mainToolBarComponent: QToolButton { action: actionSave }

	/*toolBarMenu: QMenu {
		MenuItem {
			text: qsTr("Pályák")
			icon.source: CosStyle.iconBooks
		}
		MenuItem {
			text: qsTr("Fejezetek")
			icon.source: CosStyle.iconAdd
		}
		MenuItem {
			text: qsTr("Storages")
			icon.source: CosStyle.iconBooks
		}
	}*/


	property VariantMapModel modelClassList: cosClient.newModel([
																	"classid",
																	"name"
																])

	property VariantMapModel modelUserList: cosClient.newModel([
																   "username",
																   "firstname",
																   "lastname",
																   "classname",
																   "rankimage",
																   "rankid",
																   "ranklevel",
																   "active"
															   ])

	property VariantMapModel _modelDialogClassList: cosClient.newModel([
																		   "id",
																		   "name"
																	   ])

	property VariantMapModel _modelDialogUserList: cosClient.newModel([
																		  "username",
																		  "firstname",
																		  "lastname",
																		  "classname",
																		  "classid",
																		  "active"
																	  ])

	Connections {
		target: teacherGroups

		function onGroupUserGet(jsonData, bData) {
			groupId = jsonData.id
			defaultSubTitle = jsonData.name

			modelUserList.unselectAll()
			modelUserList.setVariantList(jsonData.userList, "username")

			modelClassList.unselectAll()
			modelClassList.setVariantList(jsonData.classList, "classid")
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
										   sourceModel: _modelDialogClassList
									   })

			_modelDialogClassList.unselectAll()
			_modelDialogClassList.setVariantList(jsonData.list, "id")

			d.accepted.connect(function(data) {
				teacherGroups.send("groupClassAdd", {
									   id: teacherGroups.selectedGroupId,
									   list: _modelDialogClassList.getSelectedData("id")
								   })
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
										   sourceModel: _modelDialogUserList
									   })

			_modelDialogUserList.unselectAll()
			_modelDialogUserList.setVariantList(jsonData.list, "username")

			d.accepted.connect(function(data) {
				teacherGroups.send("groupUserAdd", {
									   id: teacherGroups.selectedGroupId,
									   list: _modelDialogUserList.getSelectedData("username")
								   })
			})

			d.open()
		}

		function onGroupUserAdd(jsonData, binaryData) { reloadGroup() }
		function onGroupUserRemove(jsonData, binaryData) { reloadGroup() }
		function onGroupClassAdd(jsonData, binaryData) { reloadGroup() }
		function onGroupClassRemove(jsonData, binaryData) { reloadGroup() }
	}


	QSwipeComponent {
		id: swComponent
		anchors.fill: parent

		basePage: control

		//headerContent: QLabel {	}

		content: [
			QSwipeContainer {
				id: container1
				reparented: swComponent.swipeMode
				reparentedParent: placeholder1
				title: qsTr("Osztályok")
				icon: CosStyle.iconGroupsSmall

				QVariantMapProxyView {
					id: classList
					anchors.fill: parent

					refreshEnabled: true
					delegateHeight: CosStyle.baseHeight

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

					onRefreshRequest: reloadGroup()

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


				/*menuComponent: QToolButton {
					id: menuButton
					action: actionA
					display: AbstractButton.IconOnly
				}*/
			},

			QSwipeContainer {
				id: container2
				reparented: swComponent.swipeMode
				reparentedParent: placeholder2

				title: qsTr("Tanulók")
				icon: CosStyle.iconUsers

				QVariantMapProxyView {
					id: userList
					anchors.fill: parent

					refreshEnabled: true
					delegateHeight: CosStyle.twoLineHeight

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
								expression: model.firstname+" "+model.lastname
							},
							ExpressionRole {
								name: "fullclassname"
								expression: model.classname === "" ? qsTr("[Osztály nélkül]") : model.classname
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

					onRefreshRequest: reloadGroup()

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
		]

		swipeContent: [
			Item {
				id: placeholder1
				property var contextMenuFunc: function (m) {
					m.addAction(actionClassAdd)
					m.addAction(actionClassRemove)
				}
			},
			Item {
				id: placeholder2
				property var contextMenuFunc: function (m) {
					m.addAction(actionUserAdd)
					m.addAction(actionUserRemove)
				}
			}
		]

		tabBarContent: [
			QSwipeButton { swipeContainer: container1 },
			QSwipeButton { swipeContainer: container2 }
		]

	}

	onPageActivated: reloadGroup()


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
					teacherGroups.send("groupUserRemove", { id: teacherGroups.selectedGroupId, list: modelUserList.getSelectedData("username")})
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
					teacherGroups.send("groupClassRemove", { id: teacherGroups.selectedGroupId, list: modelClassList.getSelectedData("classid")})
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

	function windowClose() {
		return false
	}

	function pageStackBack() {
		if (modelUserList.selectedCount) {
			modelUserList.unselectAll()
			return true
		}

		if (swComponent.layoutBack())
			return true

		return false
	}
}

