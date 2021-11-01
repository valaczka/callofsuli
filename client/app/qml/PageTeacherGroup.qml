import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QBasePage {
	id: page

	defaultTitle: ""
	defaultSubTitle: ""

	property alias groupId: teacherGroups.selectedGroupId

	mainToolBarComponent: UserButton {
		userDetails: userData
		userNameVisible: page.width>800
	}

	mainMenuFunc: function (m) {
		m.addAction(actionGroupDelete)
	}

	UserDetails {
		id: userData
	}


	activity: TeacherGroups {
		id: teacherGroups

		property VariantMapModel _dialogMapModel: newModel(["uuid", "name"])


		onMapDownloadRequest: {
			var d = JS.dialogCreateQml("YesNo", {
										   title: qsTr("Letöltés"),
										   text: qsTr("A szerver %1 adatot akar küldeni. Elindítod a letöltést?").arg(formattedDataSize)
									   })
			d.accepted.connect(function() {
				var dd = JS.dialogCreateQml("Progress", { title: qsTr("Letöltés"), downloader: teacherGroups.downloader })
				dd.closePolicy = Popup.NoAutoClose
				dd.open()
			})

			d.open()
		}


		onGroupExcludedMapListGet: {
			if (!jsonData || jsonData.id !== selectedGroupId)
				return

			if (!jsonData.list || jsonData.list.length === 0) {
				cosClient.sendMessageWarning(qsTr("Pálya hozzáadása"), qsTr("Nincs több hozzáadható pálya!"))
				return
			}

			_dialogMapModel.unselectAll()
			_dialogMapModel.replaceList(jsonData.list)

			var d = JS.dialogCreateQml("List", {
										   icon: CosStyle.iconLockAdd,
										   title: qsTr("Pálya hozzáadása"),
										   roles: ["name", "uuid"],
										   modelTitleRole: "name",
										   selectorSet: true,
										   sourceModel: _dialogMapModel
									   })


			d.accepted.connect(function(dlgdata) {
				if (dlgdata !== true)
					return

				var l = _dialogMapModel.getSelectedData("uuid")
				if (l.length === 0)
					return

				send("groupMapAdd", {id: selectedGroupId, list: l})
			})
			d.open()
		}


		onGroupRemove: {
			if (jsonData.error === undefined) {
				var i = mainStack.get(page.StackView.index-1)

				if (i)
					mainStack.pop(i)
			}
		}


		onGroupModify: {
			if (jsonData.error === undefined && jsonData.id === selectedGroupId)
				send("groupGet", {id: selectedGroupId})
		}
	}


	QSwipeComponent {
		id: swComponent
		anchors.fill: parent

		basePage: page

		content: [
			TeacherGroupUserList {
				id: container1
				reparented: swComponent.swipeMode
				reparentedParent: placeholder1
				menuComponent: QToolButton {
					action: actionUserEdit
					display: AbstractButton.IconOnly
				}
				buttonEditAction: actionUserEdit
				groupName: "%1 (%2)".arg(defaultTitle).arg(defaultSubTitle)
			},
			TeacherGroupMapList {
				id: container2
				reparented: swComponent.swipeMode
				reparentedParent: placeholder2
				groupName: "%1 (%2)".arg(defaultTitle).arg(defaultSubTitle)
			}
		]

		swipeContent: [
			Item {
				id: placeholder1
				property var contextMenuFunc: function (m) {
					m.addAction(actionUserEdit)
				}
			},
			Item {
				id: placeholder2

				property var contextMenuFunc: function (m) {
					m.addAction(actionMapAdd)
				}
			}
		]

		tabBarContent: [
			QSwipeButton { swipeContainer: container1 },
			QSwipeButton { swipeContainer: container2 }
		]

	}


	Action {
		id: actionGroupDelete
		text: qsTr("Csoport törlése")
		icon.source: CosStyle.iconDelete
		enabled: teacherGroups.selectedGroupId > -1
		onTriggered: {
			var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan törlöd a csoportot?\n%1").arg(defaultTitle)})
			d.accepted.connect(function() {
				teacherGroups.send("groupRemove", {id: teacherGroups.selectedGroupId})
			})
			d.open()
		}
	}

	Action {
		id: actionGrouRename
		text: qsTr("Átnevezés")
		icon.source: CosStyle.iconRename
		enabled: teacherGroups.selectedGroupId > -1
		onTriggered: {
			var d = JS.dialogCreateQml("TextField", {
										   title: qsTr("Csoport átnevezése"),
										   text: qsTr("Csoport neve:"),
										   value: defaultTitle
									   })

			d.accepted.connect(function(data) {
				if (data.length)
					teacherGroups.send("groupModify", {id: teacherGroups.selectedGroupId, name: data})
			})
			d.open()
		}
	}

	Action {
		id: actionMapAdd
		text: qsTr("Pálya hozzáadása")
		icon.source: CosStyle.iconAdd
		enabled: teacherGroups.selectedGroupId > -1
		onTriggered: {
			teacherGroups.send("groupExcludedMapListGet", {id: teacherGroups.selectedGroupId})
		}
	}




	Action {
		id: actionUserEdit
		text: qsTr("Résztvevők szerkesztése")
		icon.source: CosStyle.iconEdit
		enabled: teacherGroups.selectedGroupId > -1
		onTriggered: {
			JS.createPage("TeacherGroupUsers", {
							  teacherGroups: teacherGroups,
							  groupId: groupId
						  })
		}
	}


	onPageActivated: {
		if (teacherGroups.selectedGroupId > -1) {
			teacherGroups.send("groupGet", {id: teacherGroups.selectedGroupId})
		} else {
			teacherGroups.modelMapList.clear()
			teacherGroups.modelUserList.clear()
		}

		container1.list.forceActiveFocus()
	}


	function windowClose() {
		return false
	}


	function pageStackBack() {
		if ((!swComponent.swipeMode || swComponent.swipeCurrentIndex == 1) && teacherGroups.modelMapList.selectedCount > 0) {
			teacherGroups.modelMapList.unselectAll()
			return true
		} else if (swComponent.layoutBack())
			return true

		return false
	}

}

