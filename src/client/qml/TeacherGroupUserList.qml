import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Teacher teacher: null
	property int groupId: -1

	title: qsTr("Felhasználók")
	icon: CosStyle.iconUsers

	pageContextMenu: QMenu {
		MenuItem { action: actionAdd }
		MenuItem { action: actionRemove }
	}

	UserListWidget {
		id: userList
		anchors.fill: parent

		delegate.refreshEnabled: true
		delegate.onRefreshRequest: pageGroupEdit.getGroupData()

		delegate.onKeyDeletePressed: actionRemove.trigger()
		delegate.onKeyInsertPressed: actionAdd.trigger()


		Flow {
			id: flow

			visible: userList.delegate.selectorSet

			QToolButton { action: actionRemove; display: AbstractButton.TextBesideIcon  }
		}

	}



	Action {
		id: actionAdd
		text: qsTr("Hozzáadás")
		icon.source: CosStyle.iconAdd
		onTriggered: teacher.send({"class": "teacherGroups", "func": "getExcludedUsers", "id": groupId})
	}

	Action {
		id: actionRemove
		text: qsTr("Eltávolítás")
		icon.source: CosStyle.iconRemove
		enabled: userList.delegate.selectorSet || userList.delegate.currentIndex != -1
		onTriggered: {
			var s = []

			if (userList.delegate.selectorSet)
				s = JS.getSelectedIndices(userList.model, "username")
			else if (userList.delegate.currentIndex != -1){
				var id = userList.delegate.model.get(userList.delegate.currentIndex).username
				s.push(id)
			}

			var l = []

			for (var i=0; i<userList.model.count; ++i) {
				var o = userList.model.get(i)
				if (!s.includes(o.username))
					l.push(o.username)
			}

			var d = JS.dialogCreateQml("YesNo")
			d.item.title = qsTr("Felhasználók eltávolítása")
			d.item.text = qsTr("Biztosan eltávolítod a kiválasztott %1 felhasználót?").arg(s.length)
			d.accepted.connect(function () {
				teacher.send({"class": "teacherGroups", "func": "updateGroup", "id": groupId, "users": l})
				userList.delegate.selectAll(false)
			})
			d.open()
		}
	}

	Connections {
		target: teacher

		onGroupReceived: {
			JS.setModel(userList.model, groupData.users)
			userList.autoCreateClassList()
		}

		onGroupExcludedUsersReceived: loadDialogUsers(groupData.list)

	}

	onPanelActivated: userList.delegate.forceActiveFocus()


	function loadDialogUsers(list) {
		var d = JS.dialogCreateQml("UserList")
		d.item.title = qsTr("Felhasználók")

		JS.setModel(d.item.model, list)
		//d.item.userListWidget.autoCreateClassList()

		d.accepted.connect(function() {
			var users = JS.getSelectedIndices(d.item.model, "username")
			teacher.send({"class": "teacherGroups", "func": "addUsers", "id": groupId, "list": users})
		})
		d.open()
	}
}
