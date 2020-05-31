import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import QtQuick.Dialogs 1.3
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: pageGroupEdit

	property int groupId: -1

	mainToolBarComponent: QToolBusyIndicator { running: teacher.isBusy }

	Teacher {
		id: teacher
		client: cosClient

		onGroupUpdated: {
			if (groupData.error) {
				client.sendMessageError("ADATBÁZIS", "HIBA", groupData.error)
			}
			getGroupData()
		}

		onGroupReceived: {
			pageGroupEdit.title = groupData.name
		}
	}


	onPageActivated: getGroupData()

	onGroupIdChanged:  panels = [
						   { url: "TeacherGroupClassList.qml", params: { teacher: teacher, groupId: groupId }, fillWidth: true},
						   { url: "TeacherGroupUserList.qml", params: { teacher: teacher, groupId: groupId }, fillWidth: true},
						   { url: "TeacherGroupMapList.qml", params: { teacher: teacher, groupId: groupId }, fillWidth: true}
					   ]


	function getGroupData() {
		teacher.send({"class": "teacherGroups", "func": "getGroup", "id": groupId})
	}

	function windowClose() {
		return true
	}

	function pageStackBack() {
		return false
	}
}
