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

	UserDetails {
		id: userData
	}


	mainMenuFunc: function (m) {
		m.addAction(actionMapAdd)
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

	}


	QSwipeComponent {
		id: swComponent
		anchors.fill: parent

		content: [
			TeacherGroupUserList {
				id: container1
				reparented: swComponent.swipeMode
				reparentedParent: placeholder1
			},
			TeacherGroupMapList {
				id: container2
				reparented: swComponent.swipeMode
				reparentedParent: placeholder2
			}
		]

		swipeContent: [
			Item { id: placeholder1 },
			Item { id: placeholder2 }
		]

		tabBarContent: [
			QSwipeButton { swipeContainer: container1 },
			QSwipeButton { swipeContainer: container2 }
		]

	}


	Action {
		id: actionMapAdd
		text: qsTr("Pálya hozzáadása")
		icon.source: CosStyle.iconAdd
		onTriggered: {
			if (teacherGroups.selectedGroupId > -1) {
				teacherGroups.send("groupExcludedMapListGet", {id: teacherGroups.selectedGroupId})
			}
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

