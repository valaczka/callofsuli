import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: pageTeacherGroup

	title: qsTr("Csoportok")

	mainToolBarComponent: QToolBusyIndicator { running: teacher.isBusy }

	signal groupSelected(var id, var name)

	Teacher {
		id: teacher
		client: cosClient

		onGroupCreated: listReload()
		onGroupUpdated: {
			if (groupData.error) {
				client.sendMessageError("ADATBÁZIS", "HIBA", groupData.error)
			}
			listReload()
		}
		onGroupRemoved: {
			if (groupData.error) {
				client.sendMessageError("ADATBÁZIS", "HIBA", groupData.error)
			}
			listReload()
		}
	}

	onGroupSelected: if (id !== -1)
						 swipeToPage(1)

	onPageActivated: listReload()

	panels: [
		{ url: "TeacherGroupList.qml", params: { teacher: teacher }, fillWidth: false},
		{ url: "TeacherGroupMenu.qml", params: { teacher: teacher }, fillWidth: true}
		//{ url: "AdminUserData.qml", params: { adminUsers: adminUsers }, fillWidth: true}
	]

	function windowClose() {
		return true
	}

	function pageStackBack() {
		return false
	}


	function listReload() {
		if (teacher)
			teacher.send({"class": "teacherGroups", "func": "getAllGroup"})
		groupSelected(-1, "")
	}

}
