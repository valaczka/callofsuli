import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import SortFilterProxyModel 0.2
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property AdminUsers adminUsers: null

	title: qsTr("Felhasználók")

	UserListWidget {
		id: userListWidget

		anchors.fill: parent

		selectorLoader.sourceComponent: Flow {
			id: flow
			property int buttonDisplay: AbstractButton.IconOnly

			QToolButton { action: actionActiveSet; display: flow.buttonDisplay }
			QToolButton { action: actionActiveUnset; display: flow.buttonDisplay  }
			QToolButton { action: actionTeacherSet; display: flow.buttonDisplay  }
			QToolButton { action: actionTeacherUnset; display: flow.buttonDisplay }
			QToolButton { action: actionAdminSet; display: flow.buttonDisplay  }
			QToolButton { action: actionAdminUnset; display: flow.buttonDisplay  }


			QMenuButton {
				icon.source: CosStyle.iconOK
				ToolTip.text: qsTr("Osztályba sorol")

				MenuItem {
					text: qsTr("Osztály nélkül")
					onTriggered: runBatchFunction("classid", null)
				}

				MenuSeparator { }

				Repeater {
					model: userListWidget._classList
					MenuItem {
						text: modelData.name
						onTriggered: runBatchFunction("classid", modelData.id)
					}
				}
			}

			QToolButton { action: actionRemove; display: flow.buttonDisplay  }
		}


		delegate.onClicked: pageAdminUsers.userSelected(model.get(index).username)

		delegate.onRightClicked: {
			contextMenu.disableOwn = (model.get(index).username === cosClient.userName)
			contextMenu.popup()
		}

		QMenu {
			id: contextMenu

			property bool disableOwn: false

			MenuItem { action: actionActiveSet }
			MenuItem { action: actionActiveUnset; enabled: !contextMenu.disableOwn }
			MenuItem { action: actionTeacherSet }
			MenuItem { action: actionTeacherUnset }
			MenuItem { action: actionAdminSet }
			MenuItem { action: actionAdminUnset; enabled: !contextMenu.disableOwn }

			QMenu {
				title: qsTr("Osztályba sorol")

				MenuItem {
					text: qsTr("Osztály nélkül")
					onTriggered: runBatchFunction("classid", null)
				}

				MenuSeparator { }

				Repeater {
					model: userListWidget._classList
					MenuItem {
						text: modelData.name
						onTriggered: runBatchFunction("classid", modelData.id)
					}
				}
			}

			MenuItem { action: actionRemove; enabled: !contextMenu.disableOwn }
		}
	}


	Connections {
		target: pageAdminUsers
		onClassSelected: if (userListWidget.tagClasses.checked ){
							 var ii = []
							 ii.push(id)
							 userListWidget._selectedClasses = ii
							 var n = []
							 for (var i=0; i<userListWidget._classList.length; ++i) {
								 if (userListWidget._classList[i].id === id) {
									 userListWidget._selectedClassesRegExp = id
									 n.push({"name": userListWidget._classList[i].name})
									 userListWidget.tagClasses.tags = n
									 return
								 }
							 }
						 }
	}

	Connections {
		target: adminUsers

		onUserCreated: reloadUserList()
		onUserUpdated: reloadUserList()

		onUserListLoaded: getList(list)
		onClassListLoaded: {
			userListWidget._classList = list
			reloadUserList()
		}

		onUserBatchUpdated: {
			if (data.error)
				cosClient.sendMessageWarning(qsTr("Felhasználók módosítása"), qsTr("Szerver hiba"), data.error)
			else {
				userListWidget.delegate.selectAll(false)
				reloadUserList()
			}
		}

		onUserBatchRemoved: {
			if (data.error)
				cosClient.sendMessageWarning(qsTr("Felhasználók törlése"), qsTr("Szerver hiba"), data.error)
			else {
				userListWidget.delegate.selectAll(false)
				reloadUserList()
			}
		}
	}




	Action {
		id: actionActiveSet
		icon.source: CosStyle.iconChecked
		text: qsTr("Aktivál")
		onTriggered: runBatchFunction("active", true)
	}

	Action {
		id: actionActiveUnset
		icon.source: CosStyle.iconUnchecked
		text: qsTr("Inaktivál")
		onTriggered: runBatchFunction("active", false)
	}

	Action {
		id: actionTeacherSet
		icon.source: CosStyle.iconChecked
		text: qsTr("Tanár jogot ad")
		onTriggered: runBatchFunction("isTeacher", true)
	}

	Action {
		id: actionTeacherUnset
		icon.source: CosStyle.iconUnchecked
		text: qsTr("Tanár jogot töröl")
		onTriggered: runBatchFunction("isTeacher", false)
	}

	Action {
		id: actionAdminSet
		icon.source: CosStyle.iconChecked
		text: qsTr("Admin jogot ad")
		onTriggered: runBatchFunction("isAdmin", true)
	}

	Action {
		id: actionAdminUnset
		icon.source: CosStyle.iconUnchecked
		text: qsTr("Admin jogot töröl")
		onTriggered: runBatchFunction("isAdmin", false)
	}

	Action {
		id: actionRemove
		icon.source: CosStyle.iconRemove
		text: qsTr("Töröl")
		onTriggered: runBatchFunction("remove")
	}



	function populated() {
	}


	function reloadUserList() {
		adminUsers.send({"class": "user", "func": "getAllUser"})
	}


	function getList(_list) {
		JS.setModel(userListWidget.model, _list)
	}



	function runBatchFunction(p, value) {
		var l = []
		if (userListWidget.delegate.selectorSet)
			l = JS.getSelectedIndices(userListWidget.model, "username")
		else if (userListWidget.delegate.currentIndex != -1)
			l.push(userListWidget.delegate.model.get(userListWidget.delegate.currentIndex).username)

		if (l.length === 0)
			return

		if (p==="active" || p==="isAdmin" || p==="remove") {
			var i = l.indexOf(cosClient.userName)
			if (i !== -1)
				l.splice(i, 1)
		}

		var d = {}
		d["class"] = "user"
		if (p === "remove") {
			d["func"] = "userBatchRemove"
			d["list"] = l
		} else {
			d["func"] = "userBatchUpdate"
			d[p] = value
			d["users"] = l
		}

		var dd = JS.dialogCreateQml("YesNo")
		dd.item.title = p === "remove" ? qsTr("Felhasználók törlése") : qsTr("Felhasználók módosítása")

		var txt = ""
		if (p === "active" && value === true)
			txt = qsTr("aktiválod")
		else if (p === "active" && value === false)
			txt = qsTr("inaktiválod")
		else if (p === "isTeacher" && value === true)
			txt = qsTr("tanárrá teszed")
		else if (p === "isTeacher" && value === false)
			txt = qsTr("nem tanárrá teszed")
		else if (p === "isAdmin" && value === true)
			txt = qsTr("adminná teszed")
		else if (p === "isAdmin" && value === false)
			txt = qsTr("nem adminná teszed")
		else if (p === "remove")
			txt = qsTr("törlöd")

		dd.item.text = qsTr("Biztosan %1 a kiválasztott %2 felhasználót?").arg(txt).arg(l.length)
		dd.accepted.connect(function () {
			adminUsers.send(d)
		})
		dd.open()

	}

}

