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


		/*function onGroupExcludedUserListGet(jsonData, binaryData) {
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
		}*/
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
					//numbered: true

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

					highlightCurrentItem: false

					autoSelectorChange: true

					onRefreshRequest: reloadGroup()

					footer: QToolButtonFooter {
						width: classList.width
						action: actionClassAdd
					}
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
					//numbered: true

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

					highlightCurrentItem: false

					autoSelectorChange: true

					onRefreshRequest: reloadGroup()

					footer: QToolButtonFooter {
						width: userList.width
						action: actionUserAdd
					}
				}
			}
		]

		swipeContent: [
			Item { id: placeholder1 },
			Item { id: placeholder2
				/*property var contextMenuFunc: function (m) {
					m.addAction(actionA)
				}*/
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
	}


	Action {
		id: actionClassAdd
		text: qsTr("Osztály hozzáadása")
		icon.source: CosStyle.iconAdd
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

